/* HTTP服务器模块实现文件（支持分片上传） */
#include "http_server.h"
#include "api.h"
#include "esp_http_server.h"
#include "sd_card.h"
#include "webdav.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* 静态变量跟踪服务器运行状态 */
static bool s_http_server_running = false;

/* HTTP服务器配置参数 */
#define HTML_FOLDER "系统"
#define DEFAULT_HTML_FILE "index.html"
#define HTTP_SERVER_PORT 80

static httpd_handle_t http_server = NULL;

// 独立上传任务，完全不干扰下载/音视频/网页
void upload_task(void *pv) {
  upload_ctx_t *ctx = pv;
  httpd_req_t *req = ctx->req;
  int remaining = ctx->content_len;

  char *buf = heap_caps_malloc(4096, MALLOC_CAP_DMA);
  if (!buf) {
    httpd_resp_send_err(req, 500, "No Memory");
    goto exit;
  }

  // 删除旧文件
  remove(ctx->filepath);

  // 创建文件
  if (!sd_begin_batch_write(ctx->filepath)) {
    httpd_resp_send_err(req, 500, "Create Failed");
    free(buf);
    goto exit;
  }

  // 接收数据
  int recv;
  while (remaining > 0) {
    int to_read = (remaining < 4096) ? remaining : 4096;
    recv = httpd_req_recv(req, buf, to_read);

    if (recv <= 0) {
      if (recv == HTTPD_SOCK_ERR_TIMEOUT)
        continue;
      break;
    }

    sd_batch_write(buf, recv);
    remaining -= recv;
  }

  sd_end_batch();
  sd_close_current_file();

  if (remaining != 0) {
    remove(ctx->filepath);
    httpd_resp_send_err(req, 500, "Error");
  } else {
    sd_update_storage_info();
    httpd_resp_sendstr(req, "Upload Success");
  }

  free(buf);

exit:
  free(ctx);
  httpd_req_async_handler_complete(req);
  vTaskDelete(NULL);
}
// 唯一通用异步发送任务，全场景复用
// 只负责：下载 / 音视频 / 网页（恢复纯净）
void async_send_task(void *pv) {
  async_send_ctx_t *ctx = pv;
  httpd_req_t *req = ctx->req;

  char *buf = heap_caps_malloc(4096, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
  if (!buf) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No mem");
    fclose(ctx->fd); // 立刻关文件
    free(ctx);
    httpd_req_async_handler_complete(req);
    vTaskDelete(NULL);
    return;
  }

  // 原有发送逻辑不变
  fseek(ctx->fd, ctx->start, SEEK_SET);
  bool send_end = true;
  while (ctx->remaining > 0) {
    size_t to_read = (ctx->remaining > 4096) ? 4096 : ctx->remaining;
    size_t read = 0;
    for (int retry = 0; retry < 3 && read == 0; retry++) {
      if (retry > 0) vTaskDelay(pdMS_TO_TICKS(10));
      read = fread(buf, 1, to_read, ctx->fd);
    }
    if (read <= 0) {
      send_end = false;
      break;
    }
    if (httpd_resp_send_chunk(req, buf, read) != ESP_OK) {
      send_end = false;
      break;
    }
    ctx->remaining -= read;
  }

  if (send_end)
    httpd_resp_send_chunk(req, NULL, 0);
  free(buf);
  fclose(ctx->fd);
  free(ctx);
  httpd_req_async_handler_complete(req);
  vTaskDelete(NULL);
}

/**
 * 通用文件分段发送API（异步非阻塞版）
 */
esp_err_t http_send_file_in_chunks(httpd_req_t *req, const char *filepath,
                                   size_t chunk_size) {
  if (!req || !filepath || chunk_size == 0)
    return ESP_ERR_INVALID_ARG;

  long file_size = sd_looksize_file(filepath);
  if (file_size == -1) {
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
    return ESP_FAIL;
  }

  FILE *file = fopen(filepath, "rb");
  // SD卡找不到，尝试内部存储
  if (!file && strncmp(filepath, "/sdcard", 7) == 0) {
    char storage_path[256];
    snprintf(storage_path, sizeof(storage_path), "/storage%s", filepath + 7);
    file = fopen(storage_path, "rb");
  }
  if (!file) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "Failed to open file");
    return ESP_FAIL;
  }

  // ==============================
  // 🔥 异步启动（和你下载/音乐完全统一）
  // ==============================
  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);

  async_send_ctx_t *ctx = calloc(1, sizeof(async_send_ctx_t));
  if (!ctx) {
    fclose(file);
    httpd_req_async_handler_complete(async_req);
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "NO_MEM");
    return ESP_FAIL;
  }
  ctx->req = async_req;
  ctx->fd = file; // 直接传已打开的文件
  ctx->start = 0;
  ctx->remaining = file_size;

  if (xTaskCreatePinnedToCore(async_send_task, "async_send", 8192, ctx, 5,
                              NULL, 1) != pdPASS) {
    free(ctx);
    fclose(file);
    httpd_req_async_handler_complete(async_req);
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "task fail");
    return ESP_FAIL;
  }

  return ESP_OK;
}

esp_err_t http_serve_file(httpd_req_t *req, const char *filepath) {
  // 打开文件（保留你原有逻辑：SD + 内部FAT）
  FILE *fd = fopen(filepath, "rb");
  if (!fd && strncmp(filepath, "/sdcard", 7) == 0) {
    char storage_path[256];
    snprintf(storage_path, sizeof(storage_path), "/storage%s", filepath + 7);
    fd = fopen(storage_path, "rb");
  }
  // 文件打开重试
  if (!fd) {
    vTaskDelay(pdMS_TO_TICKS(10));
    fd = fopen(filepath, "rb");
    if (!fd && strncmp(filepath, "/sdcard", 7) == 0) {
      char storage_path[256];
      snprintf(storage_path, sizeof(storage_path), "/storage%s", filepath + 7);
      fd = fopen(storage_path, "rb");
    }
  }
  if (!fd) {
    return http_send_404(req);
  }

  // 获取文件大小
  fseek(fd, 0, SEEK_END);
  size_t file_size = ftell(fd);
  fseek(fd, 0, SEEK_SET);

  // 保留你原有MIME、缓存、响应头
  httpd_resp_set_type(req, get_mime_type(filepath));

  const char *ext = strrchr(filepath, '.');
  if (ext && (strcmp(ext, ".webp") == 0 || strcmp(ext, ".css") == 0 ||
              strcmp(ext, ".js") == 0 || strcmp(ext, ".woff2") == 0 ||
              strcmp(ext, ".woff") == 0)) {
    httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=43200");
  }
  httpd_resp_set_hdr(req, "Connection", "keep-alive");
  httpd_resp_set_hdr(req, "Accept-Ranges", "bytes");

  // HEAD 请求直接返回
  if (req->method == HTTP_HEAD) {
    char len_str[16];
    snprintf(len_str, sizeof(len_str), "%zu", file_size);
    httpd_resp_set_hdr(req, "Content-Length", len_str);
    httpd_resp_send(req, NULL, 0);
    fclose(fd);
    return ESP_OK;
  }

  // ≤64KB 文件同步发送，避免 xTaskCreate 开销
  if (file_size > 0 && file_size <= 65536) {
    char *buf = malloc(file_size);
    if (buf) {
      size_t read = 0;
      for (int retry = 0; retry < 3 && read < file_size; retry++) {
        if (retry > 0) {
          fseek(fd, 0, SEEK_SET);
          vTaskDelay(pdMS_TO_TICKS(10));
        }
        read = fread(buf, 1, file_size, fd);
      }
      fclose(fd);
      if (read == file_size) {
        httpd_resp_send(req, buf, read);
      } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Read failed");
      }
      free(buf);
      return ESP_OK;
    }
  }

  // ======================
  // 🔥 唯一异步入口（和你音乐、下载完全共用）
  // ======================
  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);

  async_send_ctx_t *ctx = calloc(1, sizeof(async_send_ctx_t));
  if (!ctx) {
    fclose(fd);
    httpd_req_async_handler_complete(async_req);
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "NO_MEM");
    return ESP_FAIL;
  }
  ctx->req = async_req;
  ctx->fd = fd;
  ctx->start = 0;
  ctx->remaining = file_size;

  if (xTaskCreatePinnedToCore(async_send_task, "async_send", 8192, ctx, 5,
                              NULL, 1) != pdPASS) {
    free(ctx);
    fclose(fd);
    httpd_req_async_handler_complete(async_req);
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "task fail");
    return ESP_FAIL;
  }

  return ESP_OK;
}

/* 发送404响应 */
esp_err_t http_send_404(httpd_req_t *req) {
  httpd_resp_set_status(req, "404 Not Found");
  httpd_resp_set_type(req, "text/plain");
  httpd_resp_send(req, "无", strlen("无"));
  return ESP_OK;
}

/* 启动HTTP服务器 */
void http_start_server(void) {
  if (s_http_server_running) {
    return;
  }

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = HTTP_SERVER_PORT;
  config.uri_match_fn = httpd_uri_match_wildcard;
  config.max_uri_handlers = allFile_route_count + 10;

  // ======= 连接管理与复用配置 =======
  // 最大同时连接数（ESP32 内核限制为 7）
  config.max_open_sockets = 7;
  // LRU 清理：连接满时主动断开最旧的空闲连接
  config.lru_purge_enable = true;

  // ======= Keep-Alive 配置 =======
  config.keep_alive_enable = true; // 启用 HTTP keep-alive
  config.keep_alive_idle = 5;      // 空闲超时：5 秒无请求则关闭连接
  config.keep_alive_interval = 3;  // TCP keep-alive 探测间隔
  config.keep_alive_count = 2;     // 最多探测次数
  // ======================================

  if (httpd_start(&http_server, &config) == ESP_OK) {
    for (size_t i = 0; i < allFile_route_count; i++) {
      httpd_register_uri_handler(http_server, &allFile_routes[i]);
    }
    webdav_register_routes(http_server);
    s_http_server_running = true;
  } else {
    s_http_server_running = false;
  }
}

// ===============================================================================
// 异步任务：目录列表（只使用你现有风格，不新增任何结构体！）
// ===============================================================================
void listFile_task(void *pv) {
  // 用法 = 和你 async_send_task / upload_task 完全一样！
  // 你自己的风格：直接传一个指针，里面包含 req 和 path
  typedef struct {
    httpd_req_t *req;
    char path[256];
    char mount[64];
  } listFile_ctx_t;

  listFile_ctx_t *ctx = (listFile_ctx_t *)pv;
  httpd_req_t *req = ctx->req;
  const char *path = ctx->path;
  const char *mount = ctx->mount;

  const size_t LIMIT = 8192;
  char *buf = (char *)malloc(LIMIT);
  if (!buf) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "Memory allocation failed");
    goto clean;
  }

  char fullDirPath[319];
  snprintf(fullDirPath, sizeof(fullDirPath), "%s%s", mount, path);

  DIR *dir = opendir(fullDirPath);
  if (!dir) {
    free(buf);
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Directory Error");
    goto clean;
  }

  httpd_resp_set_hdr(req, "Connection", "keep-alive");
  httpd_resp_set_type(req, "application/json");

  int cur_len = snprintf(buf, LIMIT, "{\"files\": [");
  bool isFirst = true;
  struct dirent *entry;
  char tmpPath[575];
  size_t fullDirLen = strlen(fullDirPath);

  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;
    if (strcmp(entry->d_name, "System Volume Information") == 0)
      continue;

    if (fullDirLen + strlen(entry->d_name) + 2 > sizeof(tmpPath))
      continue;
    snprintf(tmpPath, sizeof(tmpPath), "%s/%s", fullDirPath, entry->d_name);

    struct stat st;
    if (stat(tmpPath, &st) == 0) {
      char logicalPath[512];
      if (path[strlen(path) - 1] == '/')
        snprintf(logicalPath, sizeof(logicalPath), "%s%s", path, entry->d_name);
      else
        snprintf(logicalPath, sizeof(logicalPath), "%s/%s", path,
                 entry->d_name);

      char item[600];
      int item_len;

      if (S_ISDIR(st.st_mode)) {
        item_len = snprintf(item, sizeof(item),
                            "%s{\"filepath\":\"%s\",\"filesize\":\"-1\"}",
                            isFirst ? "" : ",", logicalPath);
      } else {
        unsigned long long realSize = (unsigned long long)st.st_size;
        item_len = snprintf(item, sizeof(item),
                            "%s{\"filepath\":\"%s\",\"filesize\":\"%llu\"}",
                            isFirst ? "" : ",", logicalPath, realSize);
      }

      if (item_len >= sizeof(item) || (cur_len + item_len + 5 >= LIMIT))
        break;

      memcpy(buf + cur_len, item, item_len);
      cur_len += item_len;
      isFirst = false;
    }
  }

  snprintf(buf + cur_len, LIMIT - cur_len, "]}");
  httpd_resp_send(req, buf, strlen(buf));
  free(buf);
  closedir(dir);

clean:
  httpd_req_async_handler_complete(req);
  free(ctx);
  vTaskDelete(NULL);
}

// ===============================================================================
// 对外接口 listFile (完全使用你现有风格，不新增任何东西！)
// ===============================================================================
void listFile(httpd_req_t *req, const char *path) {
  // 解析 prefix（你原来的逻辑）
  char prefix[64] = {0};
  char qbuf[128];
  if (httpd_req_get_url_query_str(req, qbuf, sizeof(qbuf)) == ESP_OK) {
    httpd_query_key_value(qbuf, "prefix", prefix, sizeof(prefix));
  }
  const char *mount = (prefix[0] != '\0') ? prefix : sd_get_mount_point();

  // 上下文 = 和你 upload / async_send 完全一样
  typedef struct {
    httpd_req_t *req;
    char path[256];
    char mount[65];
  } listFile_ctx_t;

  listFile_ctx_t *ctx = (listFile_ctx_t *)malloc(sizeof(listFile_ctx_t));
  strncpy(ctx->path, path, sizeof(ctx->path) - 1);
  strncpy(ctx->mount, mount, sizeof(ctx->mount) - 1);

  // 异步接管
  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  // 启动任务（栈 8K）
  xTaskCreate(listFile_task, "listFile", 8192, ctx, 5, NULL);
}

// 异步任务：游戏列表扫描（HTML模式）
void scanGame_task(void *pv) {
  httpd_req_t *req = (httpd_req_t *)pv;

  const size_t JSON_LIMIT = 4096;
  char *buf = (char *)malloc(JSON_LIMIT);
  if (!buf) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "malloc failed");
    httpd_req_async_handler_complete(req);
    vTaskDelete(NULL);
    return;
  }
  int cur_len = 0;

  httpd_resp_set_hdr(req, "Connection", "keep-alive");
  httpd_resp_set_type(req, "application/json");

  buf[cur_len++] = '[';

  const char *mount = sd_get_mount_point();
  char full_scan_dir[128];
  int dlen = 0;
  for (const char *p = mount; *p && dlen < 127; p++)
    full_scan_dir[dlen++] = *p;
  const char *scan_path = "/游戏";
  for (const char *p = scan_path; *p && dlen < 127; p++)
    full_scan_dir[dlen++] = *p;
  full_scan_dir[dlen] = 0;

  DIR *dir = opendir(full_scan_dir);
  if (dir != NULL) {
    struct dirent *entry;
    bool first_game = true;
    struct stat st;

    while ((entry = readdir(dir)) != NULL) {
      if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        continue;
      if (!strcmp(entry->d_name, "老游戏"))
        continue;

      char game_dir[128];
      int g_len = 0;
      for (const char *p = full_scan_dir; *p && g_len < 127; p++)
        game_dir[g_len++] = *p;
      if (g_len < 127)
        game_dir[g_len++] = '/';
      for (const char *p = entry->d_name; *p && g_len < 127; p++)
        game_dir[g_len++] = *p;
      game_dir[g_len] = 0;

      if (stat(game_dir, &st) != 0 || !S_ISDIR(st.st_mode))
        continue;

      typedef struct {
        const char *html;
        const char *txt;
        const char *url;
      } check_t;
      check_t list[] = {
          {"/index.html", "/HaOsread.txt", "/index.html"},
          {"/dist/index.html", "/dist/HaOsread.txt", "/dist/index.html"},
          {"/public/index.html", "/public/HaOsread.txt", "/public/index.html"}};

      char test_html[128];
      char found_txt[128] = {0};
      char found_url[128] = {0};
      int ok = 0;

      for (int i = 0; i < 3; i++) {
        int t_len = 0;
        for (const char *p = game_dir; *p && t_len < 127; p++)
          test_html[t_len++] = *p;
        for (const char *p = list[i].html; *p && t_len < 127; p++)
          test_html[t_len++] = *p;
        test_html[t_len] = 0;

        if (stat(test_html, &st) == 0) {
          t_len = 0;
          for (const char *p = game_dir; *p && t_len < 127; p++)
            found_txt[t_len++] = *p;
          for (const char *p = list[i].txt; *p && t_len < 127; p++)
            found_txt[t_len++] = *p;
          found_txt[t_len] = 0;

          t_len = 0;
          for (const char *p = scan_path; *p && t_len < 127; p++)
            found_url[t_len++] = *p;
          if (t_len < 127)
            found_url[t_len++] = '/';
          for (const char *p = entry->d_name; *p && t_len < 127; p++)
            found_url[t_len++] = *p;
          for (const char *p = list[i].url; *p && t_len < 127; p++)
            found_url[t_len++] = *p;
          found_url[t_len] = 0;

          ok = 1;
          break;
        }
      }

      if (!ok)
        continue;

      char name[257] = {0};
      char desc[64] = {0};
      FILE *f = fopen(found_txt, "r");
      if (f) {
        fgets(name, 64, f);
        fgets(desc, 64, f);
        fclose(f);
        name[strcspn(name, "\r\n")] = 0;
        desc[strcspn(desc, "\r\n")] = 0;
      }
      if (strlen(name) == 0) {
        strncpy(name, entry->d_name, sizeof(name) - 1);
        desc[0] = 0;
      }

      if (!first_game && cur_len < JSON_LIMIT - 1)
        buf[cur_len++] = ',';
      first_game = false;

      if (cur_len >= JSON_LIMIT - 1)
        continue;
      buf[cur_len++] = '{';

      const char n_head[] = "\"name\":\"";
      for (int i = 0; n_head[i] && cur_len < JSON_LIMIT - 1; i++)
        buf[cur_len++] = n_head[i];
      for (const char *p = name; *p && cur_len < JSON_LIMIT - 1; p++)
        buf[cur_len++] = *p;
      if (cur_len < JSON_LIMIT - 1)
        buf[cur_len++] = '"';

      if (cur_len < JSON_LIMIT - 1)
        buf[cur_len++] = ',';

      const char d_head[] = "\"desc\":\"";
      for (int i = 0; d_head[i] && cur_len < JSON_LIMIT - 1; i++)
        buf[cur_len++] = d_head[i];
      for (const char *p = desc; *p && cur_len < JSON_LIMIT - 1; p++)
        buf[cur_len++] = *p;
      if (cur_len < JSON_LIMIT - 1)
        buf[cur_len++] = '"';

      if (cur_len < JSON_LIMIT - 1)
        buf[cur_len++] = ',';

      const char u_head[] = "\"url\":\"";
      for (int i = 0; u_head[i] && cur_len < JSON_LIMIT - 1; i++)
        buf[cur_len++] = u_head[i];
      for (const char *p = found_url; *p && cur_len < JSON_LIMIT - 1; p++)
        buf[cur_len++] = *p;
      if (cur_len < JSON_LIMIT - 1)
        buf[cur_len++] = '"';

      if (cur_len < JSON_LIMIT - 1)
        buf[cur_len++] = '}';
    }
    closedir(dir);
  }

  if (cur_len < JSON_LIMIT - 1)
    buf[cur_len++] = ']';
  buf[cur_len] = 0;

  httpd_resp_send(req, buf, cur_len);
  free(buf);

  httpd_req_async_handler_complete(req);
  vTaskDelete(NULL);
}

// 对外接口：只负责开启异步
void scanGameAndListFile(httpd_req_t *req, const char *scan_path) {
  // 异步接管（和你 listFile / download 完全一样）
  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);

  // 扔到任务里执行
  xTaskCreate(scanGame_task, "scanGame", 8192, async_req, 5, NULL);
}

/* 停止HTTP服务器 */
void http_stop_server(void) {
  if (http_server) {
    httpd_stop(http_server);
    http_server = NULL;
    s_http_server_running = false;
  }
}

/* 检查HTTP服务器是否正在运行 */
bool http_server_is_running(void) { return s_http_server_running; }