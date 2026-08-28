#include "sd_card.h"
#include "driver/sdmmc_host.h"
#include "esp_heap_caps.h"
#include "esp_vfs_fat.h"
#include "ff.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "http_server.h"
#include "sdkconfig.h"
#include "sdmmc_cmd.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

// ============================================
// SD IO 优化参数
// ============================================
#define SD_FILE_IO_BUFFER_SIZE (64 * 1024)
#define SD_TEST_IO_BUFFER_SIZE (64 * 1024)

// ============================================
// 静态全局变量
// ============================================
static sdmmc_card_t *s_sd_card = NULL;
static SdConfig s_config = {0};

// ============================================
// 文件句柄缓存相关变量
// ============================================
static FILE *s_current_file = NULL;
static char s_current_path[256] = {0};
static char s_current_mode[8] = {0};
static void *s_current_file_buf = NULL;

// ============================================
// 全局配置变量
// ============================================
char wifiSsid[10][64];
char wifiPassword[10][64];
int wifiConnectTime = 10;
int wifiNum = 0;
int autoWifinum = 0;
int FirstWebis = 0;
char hostName[64] = {0};
char hotspotName[64] = {0};
char hotspotPassword[64] = {0};
char hotspotChannel[5] = {0};
int defaultfat = 0;

// ============================================
// 内部工具函数
// ============================================
static void *sd_alloc_file_buffer(size_t size) {
  // 优先分配支持 DMA 的内部内存
  void *buf = heap_caps_malloc(size, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
  if (!buf) {
    // 降级：至少保证内部内存
    buf = heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  }
  if (!buf) {
    // 最后手段
    buf = malloc(size);
  }
  return buf;
}

static void sd_attach_file_buffer(FILE *f, void **buf_ptr, size_t size) {
  if (!f || !buf_ptr || size == 0)
    return;

  *buf_ptr = sd_alloc_file_buffer(size);
  if (*buf_ptr) {
    setvbuf(f, (char *)(*buf_ptr), _IOFBF, size);
  }
}

static void sd_free_current_file_buffer(void) {
  if (s_current_file_buf) {
    free(s_current_file_buf);
    s_current_file_buf = NULL;
  }
}

static const char *sd_fatfs_path(const char *path) {
  if (!path)
    return NULL;

  if (strncmp(path, "/sdcard", 7) == 0) {
    return path + 7;
  }

  return path;
}

// ============================================
// 文件句柄缓存管理函数
// ============================================
void sd_close_current_file(void) {
  if (s_current_file != NULL) {
    fflush(s_current_file);
    fclose(s_current_file);
    s_current_file = NULL;
    s_current_path[0] = '\0';
    s_current_mode[0] = '\0';
  }
  sd_free_current_file_buffer();
}

void sd_flush_current_file(void) {
  if (s_current_file != NULL) {
    fflush(s_current_file);
  }
}

// ============================================
// 批量写入接口
// ============================================
FILE *sd_begin_batch_write(const char *path) {
  if (!path)
    return NULL;

  sd_close_current_file();

  s_current_file = fopen(path, "wb");
  if (s_current_file != NULL) {
    strncpy(s_current_path, path, sizeof(s_current_path) - 1);
    s_current_path[sizeof(s_current_path) - 1] = '\0';

    strncpy(s_current_mode, "wb", sizeof(s_current_mode) - 1);
    s_current_mode[sizeof(s_current_mode) - 1] = '\0';

    sd_attach_file_buffer(s_current_file, &s_current_file_buf,
                          SD_FILE_IO_BUFFER_SIZE);
  }
  return s_current_file;
}

FILE *sd_begin_batch_append(const char *path) {
  if (!path)
    return NULL;

  sd_close_current_file();

  s_current_file = fopen(path, "ab");
  if (s_current_file != NULL) {
    strncpy(s_current_path, path, sizeof(s_current_path) - 1);
    s_current_path[sizeof(s_current_path) - 1] = '\0';

    strncpy(s_current_mode, "ab", sizeof(s_current_mode) - 1);
    s_current_mode[sizeof(s_current_mode) - 1] = '\0';

    sd_attach_file_buffer(s_current_file, &s_current_file_buf,
                          SD_FILE_IO_BUFFER_SIZE);
  }
  return s_current_file;
}

esp_err_t sd_batch_write(const char *data, size_t len) {
  if (!s_current_file || !data || len == 0)
    return ESP_FAIL;

  size_t written = fwrite(data, 1, len, s_current_file);
  return (written == len) ? ESP_OK : ESP_FAIL;
}

esp_err_t sd_batch_write_str(const char *str) {
  if (!s_current_file || !str)
    return ESP_FAIL;

  size_t len = strlen(str);
  size_t written = fwrite(str, 1, len, s_current_file);
  return (written == len) ? ESP_OK : ESP_FAIL;
}

void sd_end_batch(void) {
  if (s_current_file != NULL) {
    fflush(s_current_file);
  }
}

// ============================================
// 配置文件写入
// ============================================
esp_err_t write_config_key_value(const char *path, const char *key,
                                 const char *value) {
  if (!path || !key || !value)
    return ESP_FAIL;

  char full_path[256];
  if (defaultfat == 0) {
    snprintf(full_path, sizeof(full_path), "%s%s", sd_get_mount_point(), path);
  } else {
    snprintf(full_path, sizeof(full_path), "/storage%s", path);
  }

  FILE *f = fopen(full_path, "rb");
  char *fileContent = NULL;
  size_t fileSize = 0;
  char *newContent = NULL;

  if (f) {
    void *io_buf = NULL;
    sd_attach_file_buffer(f, &io_buf, SD_FILE_IO_BUFFER_SIZE);

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    if (size < 0) {
      fclose(f);
      if (io_buf)
        free(io_buf);
      goto write_end;
    }

    fileSize = (size_t)size;
    fseek(f, 0, SEEK_SET);

    fileContent = (char *)malloc(fileSize + 1);
    if (fileContent) {
      size_t read_bytes = fread(fileContent, 1, fileSize, f);
      fileContent[read_bytes] = '\0';
    }
    fclose(f);
    if (io_buf)
      free(io_buf);
  }

  size_t allocSize = (fileContent ? strlen(fileContent) : 0) + strlen(key) +
                     strlen(value) + 512;
  newContent = (char *)calloc(1, allocSize);
  if (!newContent) {
    if (fileContent)
      free(fileContent);
    goto write_end;
  }

  bool keyFound = false;
  char search_pattern[128];
  snprintf(search_pattern, sizeof(search_pattern), "%s=", key);

  if (fileContent) {
    char *line_ptr = fileContent;
    while (line_ptr && *line_ptr != '\0') {
      char *next_line = strchr(line_ptr, '\n');
      if (next_line)
        *next_line = '\0';

      size_t line_len = strlen(line_ptr);
      while (line_len > 0 &&
             (line_ptr[line_len - 1] == '\r' || line_ptr[line_len - 1] == '\n'))
        line_ptr[--line_len] = '\0';

      if (strncmp(line_ptr, search_pattern, strlen(search_pattern)) == 0) {
        strncat(newContent, key, allocSize - strlen(newContent) - 1);
        strncat(newContent, "=", allocSize - strlen(newContent) - 1);
        strncat(newContent, value, allocSize - strlen(newContent) - 1);
        strncat(newContent, "\n", allocSize - strlen(newContent) - 1);
        keyFound = true;
      } else {
        if (strlen(line_ptr) > 0) {
          strncat(newContent, line_ptr, allocSize - strlen(newContent) - 1);
          strncat(newContent, "\n", allocSize - strlen(newContent) - 1);
        }
      }
      line_ptr = next_line ? next_line + 1 : NULL;
    }
    free(fileContent);
  }

  if (!keyFound) {
    strncat(newContent, key, allocSize - strlen(newContent) - 1);
    strncat(newContent, "=", allocSize - strlen(newContent) - 1);
    strncat(newContent, value, allocSize - strlen(newContent) - 1);
    strncat(newContent, "\n", allocSize - strlen(newContent) - 1);
  }

write_end:
  esp_err_t ret = ESP_FAIL;
  if (newContent) {
    ret = sd_write_file_len(full_path, newContent, strlen(newContent));
    free(newContent);
  }
  return ret;
}

// ============================================
// AP 配置读取
// ============================================
void sd_load_ap_config(const char *path) {
  if (!path)
    return;

  char full_path[256];
  FILE *f = NULL;

  if (defaultfat == 0) {
    snprintf(full_path, sizeof(full_path), "%s%s", sd_get_mount_point(), path);
    f = fopen(full_path, "rb");
    printf("[sd_read_wifi_list] try sdcard(defaultfat=0): %s -> %s\n", full_path, f ? "OK" : "FAIL");
    if (!f) {
      snprintf(full_path, sizeof(full_path), "/storage%s", path);
      f = fopen(full_path, "rb");
      printf("[sd_read_wifi_list] try internal: %s -> %s\n", full_path, f ? "OK" : "FAIL");
    }
  } else {
    snprintf(full_path, sizeof(full_path), "/storage%s", path);
    f = fopen(full_path, "rb");
    printf("[sd_read_wifi_list] try internal(defaultfat=1): %s -> %s\n", full_path, f ? "OK" : "FAIL");
    if (!f) {
      snprintf(full_path, sizeof(full_path), "%s%s", sd_get_mount_point(),
               path);
      f = fopen(full_path, "rb");
      printf("[sd_read_wifi_list] try sdcard: %s -> %s\n", full_path, f ? "OK" : "FAIL");
    }
  }

  if (!f) {
    printf("[sd_read_wifi_list] all failed\n");
    return;
  }

  void *io_buf = NULL;
  sd_attach_file_buffer(f, &io_buf, SD_FILE_IO_BUFFER_SIZE);

  memset(hotspotName, 0, sizeof(hotspotName));
  memset(hotspotPassword, 0, sizeof(hotspotPassword));
  memset(hotspotChannel, 0, sizeof(hotspotChannel));

  char line[128];
  int line_idx = 0;
  while (fgets(line, sizeof(line), f) != NULL && line_idx < 3) {
    size_t len = strlen(line);
    while (len > 0 && isspace((unsigned char)line[len - 1]))
      line[--len] = '\0';

    if (line_idx == 0)
      strncpy(hotspotName, line, sizeof(hotspotName) - 1);
    else if (line_idx == 1)
      strncpy(hotspotPassword, line, sizeof(hotspotPassword) - 1);
    else if (line_idx == 2)
      strncpy(hotspotChannel, line, sizeof(hotspotChannel) - 1);
    line_idx++;
  }

  fclose(f);
  if (io_buf)
    free(io_buf);
}

// ============================================
// WiFi 列表读取
// ============================================
void sd_read_wifi_list(const char *path) {
  if (!path)
    return;

  char full_path[256];
  FILE *f = NULL;

  if (defaultfat == 0) {
    snprintf(full_path, sizeof(full_path), "%s%s", sd_get_mount_point(), path);
    f = fopen(full_path, "rb");
    if (!f) {
      snprintf(full_path, sizeof(full_path), "/storage%s", path);
      f = fopen(full_path, "rb");
    }
  } else {
    snprintf(full_path, sizeof(full_path), "/storage%s", path);
    f = fopen(full_path, "rb");
    if (!f) {
      snprintf(full_path, sizeof(full_path), "%s%s", sd_get_mount_point(),
               path);
      f = fopen(full_path, "rb");
    }
  }

  if (!f) {
    memset(wifiSsid, 0, sizeof(wifiSsid));
    memset(wifiPassword, 0, sizeof(wifiPassword));
    autoWifinum = 0;
    wifiNum = 0;
    s_config.wifi_num = 0;
    return;
  }

  void *io_buf = NULL;
  sd_attach_file_buffer(f, &io_buf, SD_FILE_IO_BUFFER_SIZE);

  char ssid[64], password[64];
  int index = 0;
  memset(wifiSsid, 0, sizeof(wifiSsid));
  memset(wifiPassword, 0, sizeof(wifiPassword));

  while (index < 10) {
    if (!fgets(ssid, sizeof(ssid), f))
      break;
    printf("[wifi_read] raw ssid line[%d]: '%s' (len=%d)\n", index, ssid, (int)strlen(ssid));
    ssid[strcspn(ssid, "\r\n")] = 0;
    printf("[wifi_read] processed ssid[%d]: '%s' (len=%d)\n", index, ssid, (int)strlen(ssid));
    if (strlen(ssid) == 0 || ssid[0] == '#') {
      printf("[wifi_read] SKIP (empty or comment)\n");
      continue;
    }

    if (fgets(password, sizeof(password), f)) {
      password[strcspn(password, "\r\n")] = 0;
      printf("[wifi_read] password[%d]: '%s'\n", index, password);
    } else {
      password[0] = 0;
      printf("[wifi_read] password[%d]: EOF\n", index);
    }

    strncpy(wifiSsid[index], ssid, sizeof(wifiSsid[index]) - 1);
    wifiSsid[index][sizeof(wifiSsid[index]) - 1] = '\0';
    strncpy(wifiPassword[index], password, sizeof(wifiPassword[index]) - 1);
    wifiPassword[index][sizeof(wifiPassword[index]) - 1] = '\0';
    printf("[wifi_read] STORED[%d]: ssid='%s' pass='%s'\n", index, wifiSsid[index], wifiPassword[index]);
    index++;
  }
  printf("[wifi_read] total entries read: %d\n", index);

  fclose(f);
  if (io_buf)
    free(io_buf);
  wifiNum = index;
  s_config.wifi_num = index;
}

// ============================================
// 主配置读取
// ============================================
void readConfig(const char *path) {
  if (!path)
    return;

  char full_path[256];
  FILE *file = NULL;

  snprintf(full_path, sizeof(full_path), "/storage%s", path);
  file = fopen(full_path, "rb");
  printf("[readConfig] try internal: %s -> %s\n", full_path, file ? "OK" : "FAIL");

  if (file) {
    defaultfat = 1;
  } else {
    defaultfat = 0;
    snprintf(full_path, sizeof(full_path), "%s%s", sd_get_mount_point(), path);
    file = fopen(full_path, "rb");
    printf("[readConfig] try sdcard: %s -> %s\n", full_path, file ? "OK" : "FAIL");
  }

  if (!file) {
    printf("[readConfig] both failed\n");
    return;
  }
  printf("[readConfig] using defaultfat=%d\n", defaultfat);

  // 下面原有读取代码完全不动
  void *io_buf = NULL;
  sd_attach_file_buffer(file, &io_buf, SD_FILE_IO_BUFFER_SIZE);
  char line[256];
  while (fgets(line, sizeof(line), file)) {
    size_t len = strlen(line);
    while (len > 0) {
      char c = line[len - 1];
      if (c == '\n' || c == '\r' || c == ' ')
        len--;
      else
        break;
    }
    line[len] = '\0';

    if (len == 0 || line[0] == '#')
      continue;

    char *eq_pos = strchr(line, '=');
    if (!eq_pos)
      continue;

    *eq_pos = '\0';
    const char *key = line;
    const char *val = eq_pos + 1;

    if (strcmp(key, "wifiConnectTime") == 0) {
      wifiConnectTime = atoi(val);
    } else if (strcmp(key, "autoWifinum") == 0) {
      autoWifinum = atoi(val);
    } else if (strcmp(key, "hostName") == 0) {
      strncpy(hostName, val, sizeof(hostName) - 1);
      hostName[sizeof(hostName) - 1] = '\0';
    } else if (strcmp(key, "FirstWebis") == 0) {
      FirstWebis = atoi(val);
    }
  }

  fclose(file);
  if (io_buf)
    free(io_buf);
}

// ============================================
// URL / JSON 工具函数
// ============================================
int hex_digit_value(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'A' && c <= 'F')
    return 10 + c - 'A';
  if (c >= 'a' && c <= 'f')
    return 10 + c - 'a';
  return -1;
}

void url_decode(const char *url, char *decoded, size_t max_len) {
  if (!url || !decoded || max_len == 0)
    return;

  size_t i = 0, j = 0;
  while (url[i] && j < max_len - 1) {
    if (url[i] == '+') {
      decoded[j++] = ' ';
      i++;
    } else if (url[i] == '%' && isxdigit((unsigned char)url[i + 1]) &&
               isxdigit((unsigned char)url[i + 2])) {
      char hex[3] = {url[i + 1], url[i + 2], '\0'};
      decoded[j++] = (char)strtol(hex, NULL, 16);
      i += 3;
    } else {
      decoded[j++] = url[i++];
    }
  }
  decoded[j] = '\0';
}

void remove_query_parameters(char *url) {
  if (!url)
    return;
  char *query_start = strchr(url, '?');
  if (query_start)
    *query_start = '\0';
}

bool json_extract_string(const char *json, const char *key, char *value,
                         size_t max_len) {
  if (!json || !key || !value || max_len <= 1)
    return false;
  memset(value, 0, max_len);

  char key_pattern[300];
  snprintf(key_pattern, sizeof(key_pattern), "\"%s\":\"", key);
  const char *key_pos = strstr(json, key_pattern);
  if (!key_pos)
    return false;

  const char *p = key_pos + strlen(key_pattern);
  const char *end_p = NULL;
  bool escape = false;

  // 第一步：先找到真正结束的双引号，不中途停止
  while (*p != '\0') {
    if (escape) {
      escape = false;
      p++;
      continue;
    }
    if (*p == '\\') {
      escape = true;
      p++;
      continue;
    }
    if (*p == '"') {
      end_p = p;
      break;
    }
    p++;
  }
  if (!end_p)
    return false;

  // 第二步：从头拷贝到结束位置，严格控制输出长度
  p = key_pos + strlen(key_pattern);
  size_t out = 0;
  escape = false;
  while (p < end_p && out < max_len - 1) {
    if (escape) {
      escape = false;
      switch (*p) {
      case 'n':
        value[out++] = '\n';
        break;
      case 'r':
        value[out++] = '\r';
        break;
      case '\\':
        value[out++] = '\\';
        break;
      case '"':
        value[out++] = '"';
        break;
      default:
        value[out++] = '\\';
        if (out < max_len - 1)
          value[out++] = *p;
        break;
      }
      p++;
      continue;
    }
    if (*p == '\\') {
      escape = true;
      p++;
      continue;
    }
    value[out++] = *p;
    p++;
  }
  value[out] = '\0';
  return true;
}

bool json_extract_int(const char *json, const char *key, int *value) {
  if (!json || !key || !value)
    return false;

  char key_pattern[300];
  snprintf(key_pattern, sizeof(key_pattern), "\"%s\":", key);
  const char *key_pos = strstr(json, key_pattern);
  if (!key_pos)
    return false;

  const char *value_start = key_pos + strlen(key_pattern);
  const char *value_end = value_start;
  while (*value_end != ',' && *value_end != '}' && *value_end != '\0')
    value_end++;

  char num_str[32] = {0};
  size_t len = value_end - value_start;
  if (len >= sizeof(num_str))
    len = sizeof(num_str) - 1;
  strncpy(num_str, value_start, len);
  num_str[len] = '\0';
  *value = atoi(num_str);
  return true;
}

esp_err_t sd_init(void) {
  if (s_sd_card) {
    return ESP_OK;
  }

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 10,
      .allocation_unit_size = 64 * 1024};

  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  host.slot = SDMMC_HOST_SLOT_1;
  // ESP32-P4 SDMMC 40MHz高速时钟
  host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

  sdmmc_slot_config_t slot_config = {.clk = SD_PIN_CLK,
                                     .cmd = SD_PIN_CMD,
                                     .d0 = SD_PIN_D0,
                                     .d1 = SD_PIN_D1,
                                     .d2 = SD_PIN_D2,
                                     .d3 = SD_PIN_D3,
                                     .cd = GPIO_NUM_NC,
                                     .wp = GPIO_NUM_NC,
                                     .width = 4,
                                     // 开启内部上拉，解决高速信号衰减降速问题
                                     .flags = SDMMC_SLOT_FLAG_INTERNAL_PULLUP};

  esp_err_t ret = esp_vfs_fat_sdmmc_mount(SD_MOUNT_POINT, &host, &slot_config,
                                          &mount_config, &s_sd_card);

  if (ret != ESP_OK) {
    s_sd_card = NULL;
    return ret;
  }

  sd_update_storage_info();

  // 诊断：列出SD卡根目录
  printf("[sd_init] listing /sdcard root:\n");
  DIR *d = opendir("/sdcard");
  if (d) {
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
      printf("  %s [hex:", ent->d_name);
      for (int i = 0; ent->d_name[i] && i < 32; i++) {
        printf(" %02X", (unsigned char)ent->d_name[i]);
      }
      printf("]\n");
    }
    closedir(d);
  } else {
    printf("[sd_init] opendir /sdcard FAIL\n");
  }
  // 诊断：列出 /storage 根目录
  printf("[sd_init] listing /storage root:\n");
  d = opendir("/storage");
  if (d) {
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
      printf("  %s\n", ent->d_name);
    }
    closedir(d);
  } else {
    printf("[sd_init] opendir /storage FAIL\n");
  }
  return ESP_OK;
}

const char *sd_get_mount_point(void) { return SD_MOUNT_POINT; }

sdmmc_card_t *sd_get_card_handle(void) { return s_sd_card; }

// 路径解析：SD 卡没有就自动切到内部存储
static void sd_resolve_path(const char *path, char *resolved, size_t size) {
  struct stat st;
  if (stat(path, &st) == 0) {
    strncpy(resolved, path, size - 1);
    resolved[size - 1] = '\0';
    return;
  }
  if (strncmp(path, "/sdcard", 7) == 0) {
    snprintf(resolved, size, "/storage%s", path + 7);
    return;
  }
  strncpy(resolved, path, size - 1);
  resolved[size - 1] = '\0';
}

// ============================================
// 文件基础操作
// ============================================
long sd_looksize_file(const char *path) {
  char resolved[256];
  sd_resolve_path(path, resolved, sizeof(resolved));
  struct stat st;
  if (stat(resolved, &st) != 0)
    return -1;
  return (long)st.st_size;
}

esp_err_t sd_read_file(const char *path, char *buffer, size_t buf_len) {
  if (!path || !buffer || buf_len <= 0)
    return ESP_FAIL;

  char resolved[256];
  sd_resolve_path(path, resolved, sizeof(resolved));
  FILE *f = fopen(resolved, "rb");
  if (!f) {
    buffer[0] = '\0';
    return ESP_FAIL;
  }

  void *io_buf = NULL;
  sd_attach_file_buffer(f, &io_buf, SD_FILE_IO_BUFFER_SIZE);
  size_t read_len = fread(buffer, 1, buf_len - 1, f);
  buffer[read_len] = '\0';
  fclose(f);
  if (io_buf)
    free(io_buf);
  return ESP_OK;
}

esp_err_t sd_write_file(const char *path, const char *data) {
  if (!path || !data)
    return ESP_FAIL;

  char full_path[256];
  if (defaultfat == 0)
    snprintf(full_path, sizeof(full_path), "%s%s", sd_get_mount_point(), path);
  else
    snprintf(full_path, sizeof(full_path), "/storage%s", path);

  return sd_write_file_len(full_path, data, strlen(data));
}

esp_err_t sd_append_file(const char *path, const char *data) {
  if (!path || !data)
    return ESP_FAIL;
  return sd_append_file_len(path, data, strlen(data));
}

esp_err_t sd_write_file_len(const char *path, const char *data, size_t len) {
  if (!path || !data)
    return ESP_FAIL;
  FILE *f = fopen(path, "wb");
  if (!f)
    return ESP_FAIL;

  void *io_buf = NULL;
  sd_attach_file_buffer(f, &io_buf, SD_FILE_IO_BUFFER_SIZE);
  if (len > 16384) {
    if (fseek(f, len - 1, SEEK_SET) == 0) {
      fputc('\0', f);
      fseek(f, 0, SEEK_SET);
    }
  }
  size_t written = fwrite(data, 1, len, f);
  fflush(f);
  fclose(f);
  if (io_buf)
    free(io_buf);
  return (written == len) ? ESP_OK : ESP_FAIL;
}

esp_err_t sd_append_file_len(const char *path, const char *data, size_t len) {
  if (!path || !data)
    return ESP_FAIL;
  FILE *f = fopen(path, "ab");
  if (!f)
    return ESP_FAIL;

  void *io_buf = NULL;
  sd_attach_file_buffer(f, &io_buf, SD_FILE_IO_BUFFER_SIZE);
  size_t written = fwrite(data, 1, len, f);
  fflush(f);
  fclose(f);
  if (io_buf)
    free(io_buf);
  return (written == len) ? ESP_OK : ESP_FAIL;
}

esp_err_t sd_rename(const char *old_path, const char *new_path) {
  if (!old_path || !new_path)
    return ESP_FAIL;
  sd_close_current_file();
  if (rename(old_path, new_path) != 0)
    return ESP_FAIL;
  return ESP_OK;
}

esp_err_t sd_delete_file(const char *path) {
  if (!path)
    return ESP_FAIL;
  sd_close_current_file();
  if (remove(path) != 0)
    return ESP_FAIL;
  return ESP_OK;
}

esp_err_t sd_delete_dir(const char *path) {
  if (!path)
    return ESP_FAIL;
  sd_close_current_file();
  const char *p = sd_fatfs_path(path);
  if (!p)
    return ESP_FAIL;
  if (f_rmdir(p) != FR_OK)
    return ESP_FAIL;
  return ESP_OK;
}

esp_err_t sd_create_dir(const char *path) {
  if (!path)
    return ESP_FAIL;
  int ret = mkdir(path, 0777);
  if (ret == 0)
    return ESP_OK;
  struct stat st;
  if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
    return ESP_OK;
  return ESP_FAIL;
}

// ============================================
// SD卡容量信息
// ============================================
esp_err_t sd_update_storage_info(void) {
  FATFS *fs;
  DWORD free_clusters;
  FRESULT fr = f_getfree(SD_MOUNT_POINT, &free_clusters, &fs);
  if (fr != FR_OK)
    return ESP_FAIL;

  uint64_t sec_size = 512;
  uint64_t total_clusters = fs->n_fatent - 2;
  s_config.total_storage = total_clusters * fs->csize * sec_size;
  s_config.used_storage =
      (total_clusters - free_clusters) * fs->csize * sec_size;
  return ESP_OK;
}

SdConfig sd_get_config(void) { return s_config; }

esp_err_t sd_delete_dir_recursive(const char *path) {
  if (!path)
    return ESP_FAIL;
  struct stat st;
  if (stat(path, &st) != 0)
    return ESP_OK;
  if (!S_ISDIR(st.st_mode)) {
    remove(path);
    return ESP_OK;
  }

  const char *fat_dir = sd_fatfs_path(path);
  FF_DIR *dir = malloc(sizeof(FF_DIR));
  FILINFO *fno = malloc(sizeof(FILINFO));
  char *sub_path = malloc(1024);
  if (!dir || !fno || !sub_path) {
    free(sub_path);
    free(fno);
    free(dir);
    return ESP_FAIL;
  }

  FRESULT fr = f_opendir(dir, fat_dir);
  if (fr != FR_OK) {
    free(sub_path);
    free(fno);
    free(dir);
    return ESP_FAIL;
  }

  while (f_readdir(dir, fno) == FR_OK && fno->fname[0] != 0) {
    if (strcmp(fno->fname, ".") == 0 || strcmp(fno->fname, "..") == 0)
      continue;
    strncpy(sub_path, path, 1023);
    strncat(sub_path, "/", 1023 - strlen(sub_path) - 1);
    strncat(sub_path, fno->fname, 1023 - strlen(sub_path) - 1);
    sub_path[1023] = 0;
    sd_delete_dir_recursive(sub_path);
  }

  free(sub_path);
  free(fno);
  f_closedir(dir);
  free(dir);
  f_rmdir(fat_dir);
  return ESP_OK;
}

// ============================================
// SD卡测速 完全原样未修改
// ============================================
void sd_test_file_io(uint32_t *written_bytes, uint32_t *write_time,
                     uint32_t *read_bytes, uint32_t *read_time) {
  if (!written_bytes || !write_time || !read_bytes || !read_time)
    return;
  *written_bytes = 0;
  *write_time = 0;
  *read_bytes = 0;
  *read_time = 0;

  const char *test_path = NULL;
  struct stat st;
  if (stat("/sdcard", &st) == 0 && S_ISDIR(st.st_mode))
    test_path = "/sdcard/test_speed.tmp";
  else if (stat("/storage", &st) == 0 && S_ISDIR(st.st_mode))
    test_path = "/storage/test_speed.tmp";
  else
    return;

  int write_loop = (test_path[1] == 's' && test_path[2] == 'd') ? 64 : 1;
  unsigned char *io_buf_test = sd_alloc_file_buffer(SD_FILE_IO_BUFFER_SIZE);
  if (!io_buf_test)
    return;
  memset(io_buf_test, 0xAA, SD_FILE_IO_BUFFER_SIZE);
  remove(test_path);

  // 写入测速
  TickType_t write_start = xTaskGetTickCount();
  FILE *fw = fopen(test_path, "wb");
  if (!fw) {
    free(io_buf_test);
    return;
  }
  void *fw_io_buf = NULL;
  sd_attach_file_buffer(fw, &fw_io_buf, SD_FILE_IO_BUFFER_SIZE);
  long total_size = (long)SD_FILE_IO_BUFFER_SIZE * write_loop;
  if (fseek(fw, total_size - 1, SEEK_SET) == 0) {
    fputc('\0', fw);
    fseek(fw, 0, SEEK_SET);
  }
  for (int i = 0; i < write_loop; i++) {
    size_t w = fwrite(io_buf_test, 1, SD_FILE_IO_BUFFER_SIZE, fw);
    if (w != SD_FILE_IO_BUFFER_SIZE)
      break;
    *written_bytes += w;
  }
  fflush(fw);
  fclose(fw);
  if (fw_io_buf)
    free(fw_io_buf);
  *write_time = (xTaskGetTickCount() - write_start) * portTICK_PERIOD_MS;
  if (*written_bytes == 0 || *write_time == 0) {
    remove(test_path);
    free(io_buf_test);
    return;
  }

  // 读取测速
  TickType_t read_start = xTaskGetTickCount();
  FILE *fr = fopen(test_path, "rb");
  if (!fr) {
    remove(test_path);
    free(io_buf_test);
    return;
  }
  void *fr_io_buf = NULL;
  sd_attach_file_buffer(fr, &fr_io_buf, SD_FILE_IO_BUFFER_SIZE);
  while (1) {
    size_t r = fread(io_buf_test, 1, SD_FILE_IO_BUFFER_SIZE, fr);
    if (r > 0)
      *read_bytes += r;
    if (r < SD_FILE_IO_BUFFER_SIZE)
      break;
  }
  fclose(fr);
  if (fr_io_buf)
    free(fr_io_buf);
  *read_time = (xTaskGetTickCount() - read_start) * portTICK_PERIOD_MS;

  remove(test_path);
  free(io_buf_test);
}