#include "api.h"
#include "esp_heap_caps.h"
#include "esp_http_server.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "ff.h"
#include "http_server.h"
#include "lua_port.h"
#include "miniz.h"
#include "myfat.h"
#include "sd_card.h"
#include "version.h"
#include "wifi_module.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int FirstWebis;
extern char wifiSsid[10][64];
extern char wifiPassword[10][64];
extern char hotspotName[64];
extern char hotspotPassword[64];
extern char hotspotChannel[5];
extern int wifiConnectTime;
extern int autoWifinum;
extern char hostName[64];

static esp_ota_handle_t ota_handle = 0;
static const esp_partition_t *ota_part;

#define OTA_BUF_SIZE 4096

bool json_extract_string(const char *json, const char *key, char *out,
                         size_t out_len);
bool json_extract_int(const char *json, const char *key, int *out);
void url_decode(const char *src, char *dst, size_t dst_len);
void remove_query_parameters(char *uri);
static esp_err_t scan_directory_simple(const char *dir_path,
                                       const char *vfs_base, char ***file_list,
                                       int *count, int *max);

/* 通用挂载点获取（根据请求中的 prefix 参数） */
/* 通用挂载点获取（根据请求中的 prefix / set 参数） */
static const char *get_mount_from_req(httpd_req_t *req) {
  static char mount[64];
  char buf[128];

  // 先清空
  mount[0] = 0;

  if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK) {
    // 1. 优先看 prefix（原有逻辑不动，兼容旧接口）
    char prefix[64] = {0};
    httpd_query_key_value(buf, "prefix", prefix, sizeof(prefix));
    if (prefix[0] != '\0') {
      strncpy(mount, prefix, sizeof(mount) - 1);
      mount[sizeof(mount) - 1] = '\0';
      return mount;
    }

    // ======================================================
    // 2. 你要的逻辑：有 set=1 参数 → 根据 defaultfat 决定路径
    // ======================================================
    char set[8] = {0};
    httpd_query_key_value(buf, "set", set, sizeof(set));
    if (strcmp(set, "1") == 0) {
      if (defaultfat == 1) {
        // 内部存储
        return "/storage";
      } else {
        // SD卡
        return sd_get_mount_point();
      }
    }
  }

  // 3. 都没有 → 默认SD卡（旧逻辑不变）
  return sd_get_mount_point();
}

/* HTTP请求处理函数 */
esp_err_t http_req_handler(httpd_req_t *req) {
  char filepath[270] = {0};
  char decoded_uri[256] = {0};

  url_decode(req->uri, decoded_uri, sizeof(decoded_uri));
  remove_query_parameters(decoded_uri);

  // 直接用你自己写的函数获取 mount（prefix 自动识别）
  const char *mount = get_mount_from_req(req);

  // ================================
  // 有 prefix = 内部FAT请求 → 不处理优先，直接返回
  // ================================
  char buf[128];
  if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK) {
    char prefix[64] = {0};
    httpd_query_key_value(buf, "prefix", prefix, sizeof(prefix));
    if (strlen(prefix) > 0) {
      const char *uri_path = decoded_uri;
      while (*uri_path == '/')
        uri_path++;

      if (mount[strlen(mount) - 1] == '/')
        snprintf(filepath, sizeof(filepath), "%s%s", mount, uri_path);
      else
        snprintf(filepath, sizeof(filepath), "%s/%s", mount, uri_path);


      const char *ext = strrchr(decoded_uri, '.');
      if (ext) {
        if (strcasecmp(ext, ".mp3") == 0 || strcasecmp(ext, ".wav") == 0 ||
            strcasecmp(ext, ".ogg") == 0 || strcasecmp(ext, ".flac") == 0 ||
            strcasecmp(ext, ".aac") == 0 || strcasecmp(ext, ".m4a") == 0 ||
            strcasecmp(ext, ".mp4") == 0 || strcasecmp(ext, ".avi") == 0 ||
            strcasecmp(ext, ".webm") == 0) {
          return http_serve_file_range(req, filepath);
        }
      }
      return http_serve_file(req, filepath) == ESP_OK ? ESP_OK
                                                      : http_send_404(req);
    }
  }

  // ================================
  // 无 prefix → SD 请求
  // 仅 系统/配置 走 defaultfat 优先
  // ================================
  bool is_sys_cfg =
      (strstr(decoded_uri, "/系统/") == decoded_uri ||
       strstr(decoded_uri, "/配置/") == decoded_uri ||
       strcmp(decoded_uri, "/系统") == 0 || strcmp(decoded_uri, "/配置") == 0);

  if (is_sys_cfg) {
    if (defaultfat == 0) {
      snprintf(filepath, sizeof(filepath), "%s%s", sd_get_mount_point(),
               decoded_uri);
      if (http_serve_file(req, filepath) == ESP_OK) {
        return ESP_OK;
      }
      snprintf(filepath, sizeof(filepath), "/storage%s", decoded_uri);
      if (http_serve_file(req, filepath) == ESP_OK) {
        return ESP_OK;
      }
    } else {
      snprintf(filepath, sizeof(filepath), "/storage%s", decoded_uri);
      if (http_serve_file(req, filepath) == ESP_OK) {
        return ESP_OK;
      }
      snprintf(filepath, sizeof(filepath), "%s%s", sd_get_mount_point(),
               decoded_uri);
      if (http_serve_file(req, filepath) == ESP_OK) {
        return ESP_OK;
      }
    }
    return http_send_404(req);
  }

  // ================================
  // 你原来的逻辑 ↓ 一字没动
  // ================================
  if (strcmp(decoded_uri, "/") == 0) {
    snprintf(filepath, sizeof(filepath), "%s/%s/%s", sd_get_mount_point(),
             "系统", "index.html");
  } else {
    const char *uri_path = decoded_uri;
    while (*uri_path == '/')
      uri_path++;

    size_t mount_len = strlen(mount);

    if (mount_len > 0 && mount[mount_len - 1] == '/') {
      strlcpy(filepath, mount, sizeof(filepath));
      strlcat(filepath, uri_path, sizeof(filepath));
    } else {
      strlcpy(filepath, mount, sizeof(filepath));
      strlcat(filepath, "/", sizeof(filepath));
      strlcat(filepath, uri_path, sizeof(filepath));
    }
  }


  const char *ext = strrchr(decoded_uri, '.');
  if (ext) {
    if (strcasecmp(ext, ".mp3") == 0 || strcasecmp(ext, ".wav") == 0 ||
        strcasecmp(ext, ".ogg") == 0 || strcasecmp(ext, ".flac") == 0 ||
        strcasecmp(ext, ".aac") == 0 || strcasecmp(ext, ".m4a") == 0 ||
        strcasecmp(ext, ".mp4") == 0 || strcasecmp(ext, ".avi") == 0 ||
        strcasecmp(ext, ".webm") == 0) {
      return http_serve_file_range(req, filepath);
    }
  }

  return http_serve_file(req, filepath) == ESP_OK ? ESP_OK : http_send_404(req);
}

// 正确：不用传 req 直接 free，用极简上下文
typedef struct {
  httpd_req_t *req;
} storage_ctx_t;

static void get_storage_task(void *pv) {
  // 1. 取出上下文
  storage_ctx_t *ctx = pv;
  httpd_req_t *req = ctx->req;

  // 2. 原有业务不变
  SdConfig cfg = sd_get_config();
  uint64_t fat_total = 0, fat_used = 0;
  fat_get_fs_info("/storage", &fat_total, &fat_used);

  char buffer[256];
  int len = snprintf(buffer, sizeof(buffer),
                     "{\"totalstorage\":%llu,\"storage\":%llu,"
                     "\"fat_total\":%llu,\"fat_used\":%llu}",
                     cfg.total_storage, cfg.used_storage, fat_total, fat_used);

  if (len > 0 && len < sizeof(buffer)) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, buffer, len);
  } else {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "JSON format error");
  }

  // 3. 释放上下文 + 结束异步
  httpd_req_async_handler_complete(req);
  free(ctx); // 只 free 自己 malloc 的
  vTaskDelete(NULL);
}

esp_err_t http_get_storage_handler(httpd_req_t *req) {
  // 分配上下文
  storage_ctx_t *ctx = malloc(sizeof(storage_ctx_t));
  if (!ctx) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  // 接管异步
  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(get_storage_task, "get_storage", 4096, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }
  return ESP_OK;
}

typedef struct {
  httpd_req_t *req;
} firstweb_ctx_t;

static void get_firstwebis_task(void *pv) {
  firstweb_ctx_t *ctx = pv;
  httpd_req_t *req = ctx->req;

  char buffer[50];
  int len =
      snprintf(buffer, sizeof(buffer), "{\"FirstWebis\": %d}", FirstWebis);

  if (len > 0 && len < sizeof(buffer)) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, buffer, len);
  } else {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "JSON format error");
  }

  httpd_req_async_handler_complete(req);
  free(ctx);
  vTaskDelete(NULL);
}

esp_err_t http_get_FirstWebis_handler(httpd_req_t *req) {
  firstweb_ctx_t *ctx = malloc(sizeof(firstweb_ctx_t));
  if (!ctx)
    return ESP_ERR_NO_MEM;

  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(get_firstwebis_task, "get_firstweb", 4096, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }
  return ESP_OK;
}

esp_err_t http_look_txt_handler(httpd_req_t *req) {
  char buf[256] = {0};
  char encoded_path[256] = {0};

  // 1. 读取 URL 参数
  if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) != ESP_OK) {
    return ESP_FAIL;
  }

  // 2. 获取 filename
  httpd_query_key_value(buf, "filename", encoded_path, sizeof(encoded_path));
  if (strlen(encoded_path) == 0) {
    return ESP_FAIL;
  }

  // 3. URL解码
  url_decode(encoded_path, encoded_path, sizeof(encoded_path));

  // ==============================
  // 🔥 只看 defaultfat 0/1 决定路径
  // ==============================
  char full_path[300] = {0};
  if (defaultfat == 0) {
    // 0 → SD卡
    snprintf(full_path, sizeof(full_path), "%s%s", sd_get_mount_point(),
             encoded_path);
  } else {
    // 1 → 内部存储
    snprintf(full_path, sizeof(full_path), "/storage%s", encoded_path);
  }


  httpd_resp_set_type(req, "text/plain");
  return http_send_file_in_chunks(req, full_path, 1024);
}

typedef struct {
  httpd_req_t *req;
} edit_txt_ctx_t;

static void edit_txt_task(void *pv) {
  edit_txt_ctx_t *ctx = (edit_txt_ctx_t *)pv;
  httpd_req_t *req = ctx->req;

  // 1. 读数据（循环读取，确保读完所有 content_len）
  if (req->content_len == 0) {
    httpd_resp_sendstr(req, "OK");
    goto clean;
  }
  char *buf = malloc(req->content_len + 1);
  if (!buf) {
    httpd_resp_send_err(req, 500, "No memory");
    goto clean;
  }

  int recv_total = 0;
  int recv = 0;
  while (recv_total < req->content_len) {
    recv = httpd_req_recv(req, buf + recv_total, req->content_len - recv_total);
    if (recv <= 0) {
      httpd_resp_send_err(req, 400, "Recv fail");
      free(buf);
      goto clean;
    }
    recv_total += recv;
  }
  buf[recv_total] = 0;

  // 2. 解析JSON参数：支持 currentChunk / totalChunks 分片上传
  char path[256] = {0};
  const int CHUNK_MAX = 16384;

  int is_clipboard = 0;
  int currentChunk = 0;
  int totalChunks = 0;

  json_extract_string(buf, "txtpath", path, sizeof(path));
  json_extract_int(buf, "clipboard", &is_clipboard);
  json_extract_int(buf, "currentChunk", &currentChunk);
  json_extract_int(buf, "totalChunks", &totalChunks);

  if (!strlen(path)) {
    httpd_resp_send_err(req, 400, "Path empty");
    free(buf);
    goto clean;
  }

  // ==============================
  // 路径逻辑
  // ==============================
  char full_path[300] = {0};

  if (is_clipboard) {
    if (defaultfat == 0) {
      snprintf(full_path, sizeof(full_path), "%s%s", sd_get_mount_point(),
               path);
    } else {
      snprintf(full_path, sizeof(full_path), "/storage%s", path);
    }
  } else {
    const char *mount = get_mount_from_req(req);
    if (mount[strlen(mount) - 1] == '/') {
      snprintf(full_path, sizeof(full_path), "%s%s", mount, path);
    } else {
      snprintf(full_path, sizeof(full_path), "%s/%s", mount, path);
    }
  }

  // 内容缓冲区用 malloc（3_1.7 没开 PSRAM）
  char *content = malloc(CHUNK_MAX);
  if (!content) {
    httpd_resp_send_err(req, 500, "content malloc fail");
    free(buf);
    goto clean;
  }
  memset(content, 0, CHUNK_MAX);
  json_extract_string(buf, "con", content, CHUNK_MAX);

  // 核心判断：第0分片覆盖清空文件，其余分片追加写入
  esp_err_t wr_ret;
  if (currentChunk == 0) {
    wr_ret = sd_write_file_len(full_path, content, strlen(content));
  } else {
    wr_ret = sd_append_file_len(full_path, content, strlen(content));
  }

  if (wr_ret == ESP_OK) {
    httpd_resp_sendstr(req, "OK");
  } else {
    httpd_resp_send_err(req, 500, "Write fail");
  }

  free(content);
  free(buf);

clean:
  httpd_req_async_handler_complete(req);
  free(ctx);
  vTaskDelete(NULL);
}

esp_err_t http_edit_txt_handler(httpd_req_t *req) {
  edit_txt_ctx_t *ctx = malloc(sizeof(edit_txt_ctx_t));
  if (!ctx) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  httpd_req_t *async_req;
  if (httpd_req_async_handler_begin(req, &async_req) != ESP_OK) {
    free(ctx);
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }
  ctx->req = async_req;

  if (xTaskCreate(edit_txt_task, "edit_txt", 8192, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_req_async_handler_complete(async_req);
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }
  return ESP_OK;
}

esp_err_t http_get_file_list_handler(httpd_req_t *req) {
  char post_data[256] = {0};
  ssize_t recv_len = httpd_req_recv(req, post_data, sizeof(post_data) - 1);
  if (recv_len <= 0) {
    const char *error = "[{\"error\":\"未收到有效参数\"}]";
    httpd_resp_send(req, error, HTTPD_RESP_USE_STRLEN);
    return ESP_FAIL;
  }
  post_data[recv_len] = '\0';

  char dirname[256] = "";
  bool extract_ok =
      json_extract_string(post_data, "path", dirname, sizeof(dirname) - 1);
  if (!extract_ok || strlen(dirname) == 0) {
    const char *error = "[{\"error\":\"缺少path参数\"}]";
    httpd_resp_send(req, error, HTTPD_RESP_USE_STRLEN);
    return ESP_FAIL;
  }

  const size_t BUFFER_SIZE = 512;
  char buffer[BUFFER_SIZE];
  size_t bufLen = 0;
  bool isFirst = true;

  httpd_resp_set_type(req, "application/json");

  const char *mount = get_mount_from_req(req);
  char fullPath[512];
  if (mount[strlen(mount) - 1] == '/')
    snprintf(fullPath, sizeof(fullPath), "%s%s", mount, dirname);
  else
    snprintf(fullPath, sizeof(fullPath), "%s/%s", mount, dirname);

  FF_DIR dir;
  FRESULT fr = f_opendir(&dir, fullPath);
  if (fr != FR_OK) {
    const char *error = "[{\"error\":\"目录打开失败\"}]";
    httpd_resp_send(req, error, HTTPD_RESP_USE_STRLEN);
    return ESP_FAIL;
  }

  httpd_resp_send_chunk(req, "[", 1);

  FILINFO fileInfo;
  while (f_readdir(&dir, &fileInfo) == FR_OK && fileInfo.fname[0] != '\0') {
    if ((fileInfo.fname[0] == '.' &&
         (fileInfo.fname[1] == '\0' ||
          (fileInfo.fname[1] == '.' && fileInfo.fname[2] == '\0'))) ||
        strcmp(fileInfo.fname, "System Volume Information") == 0) {
      continue;
    }

    char item[256];
    size_t itemLen;
    if (fileInfo.fattrib & AM_DIR) {
      itemLen =
          snprintf(item, sizeof(item), "{\"fold\":\"%s\"}", fileInfo.fname);
    } else {
      itemLen = snprintf(item, sizeof(item),
                         "{\"file\":\"%s\",\"filesize\":\"%lld\"}",
                         fileInfo.fname, (long long)fileInfo.fsize);
    }
    if (itemLen >= sizeof(item))
      continue;

    size_t needed = itemLen + (isFirst ? 0 : 1);
    if (bufLen + needed > BUFFER_SIZE) {
      httpd_resp_send_chunk(req, buffer, bufLen);
      bufLen = 0;
    }

    if (!isFirst)
      buffer[bufLen++] = ',';
    isFirst = false;
    memcpy(buffer + bufLen, item, itemLen);
    bufLen += itemLen;
  }

  if (bufLen > 0)
    httpd_resp_send_chunk(req, buffer, bufLen);

  httpd_resp_send_chunk(req, "]", 1);
  httpd_resp_send_chunk(req, NULL, 0);
  f_closedir(&dir);
  return ESP_OK;
}

esp_err_t http_get_RootFile_handler(httpd_req_t *req) {
  listFile(req, "/"); // 直接传根目录，不用全局变量
  return ESP_OK;
}

esp_err_t http_look_this_handler(httpd_req_t *req) {
  char path[256] = {0};

  // 获取参数
  httpd_req_get_url_query_str(req, path, sizeof(path));

  // 解析 lookthisPath
  if (httpd_query_key_value(path, "lookthisPath", path, sizeof(path)) ==
      ESP_OK) {
    // URL解码
    url_decode(path, path, sizeof(path));

    // 直接列出，不使用 foldPath！
    listFile(req, path);
    return ESP_OK;
  }

  // 失败返回400，和你原来完全一样
  httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Directory Error");
  return ESP_FAIL;
}

esp_err_t http_back_Root_handler(httpd_req_t *req) {
  listFile(req, "/");
  return ESP_OK;
}

esp_err_t http_back_one_handler(httpd_req_t *req) {
  char path[256] = {0};

  // 1. 从前端获取 currentPath
  httpd_req_get_url_query_str(req, path, sizeof(path));
  httpd_query_key_value(path, "currentPath", path, sizeof(path));
  url_decode(path, path, sizeof(path));

  // 2. 没传路径 → 默认根目录（和你原来逻辑一样）
  if (strlen(path) == 0) {
    listFile(req, "/");
    return ESP_OK;
  }

  // 3. 你原来的逻辑 → 直接搬到局部变量 path 上
  if (strcmp(path, "/") != 0 && strlen(path) > 0) {
    char *last_slash = strrchr(path, '/');
    if (last_slash != NULL) {
      if (last_slash == path)
        strcpy(path, "/");
      else
        *last_slash = '\0';
    }
    listFile(req, path);
  } else {
    listFile(req, "/");
  }

  return ESP_OK;
}

esp_err_t http_list_Dir_handler(httpd_req_t *req) {
  char path[256] = {0};

  // 只读取并解析 path
  httpd_req_get_url_query_str(req, path, sizeof(path));
  httpd_query_key_value(path, "path", path, sizeof(path));
  url_decode(path, path, sizeof(path));

  // 空 → 404
  if (!*path)
    return httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Not Found");

  // 正常列出
  listFile(req, path);
  return ESP_OK;
}

static void rename_task(void *pv) {
  file_op_ctx_t *ctx = pv;

  if (rename(ctx->path, ctx->path2) == 0)
    httpd_resp_sendstr(ctx->req, "OK");
  else
    httpd_resp_send_err(ctx->req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "Rename Fail");

  httpd_req_async_handler_complete(ctx->req);
  free(ctx);
  vTaskDelete(NULL);
}

esp_err_t http_rename_File_handler(httpd_req_t *req) {
  char buf[256];
  char old_name_raw[100] = {0}, new_name_raw[100] = {0};
  char old_name_dec[100] = {0}, new_name_dec[100] = {0};

  if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) != ESP_OK)
    return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Query Err");

  // 只改参数名：oldPath → 接收完整路径
  httpd_query_key_value(buf, "oldPath", old_name_raw, sizeof(old_name_raw));
  httpd_query_key_value(buf, "newName", new_name_raw, sizeof(new_name_raw));

  url_decode(old_name_raw, old_name_dec, sizeof(old_name_dec));
  url_decode(new_name_raw, new_name_dec, sizeof(new_name_dec));

  const char *mount = get_mount_from_req(req);

  file_op_ctx_t *ctx = malloc(sizeof(file_op_ctx_t));

  // 旧文件 = 直接用前端传的完整路径
  snprintf(ctx->path, sizeof(ctx->path), "%s%s", mount, old_name_dec);

  // 新文件 = 目录部分不变，只换文件名
  char *p = strrchr(old_name_dec, '/');
  if (p) {
    *p = 0; // 切掉文件名，保留目录
    snprintf(ctx->path2, sizeof(ctx->path2), "%s%s/%s", mount, old_name_dec,
             new_name_dec);
  } else {
    snprintf(ctx->path2, sizeof(ctx->path2), "%s/%s", mount, new_name_dec);
  }

  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(rename_task, "rename", 4096, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }
  return ESP_OK;
}

static void delete_task(void *pv) {
  file_op_ctx_t *ctx = pv;

  if (remove(ctx->path) == 0)
    httpd_resp_sendstr(ctx->req, "OK");
  else if (sd_delete_dir(ctx->path) == ESP_OK)
    httpd_resp_sendstr(ctx->req, "OK");
  else
    httpd_resp_send_err(ctx->req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "Delete Fail");

  httpd_req_async_handler_complete(ctx->req);
  free(ctx);
  vTaskDelete(NULL);
}

esp_err_t http_delete_File_handler(httpd_req_t *req) {
  char buf[256];
  char delete_path_raw[256] = {0};
  char delete_path_dec[256] = {0};

  if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) != ESP_OK)
    return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Query Err");

  httpd_query_key_value(buf, "deletePath", delete_path_raw,
                        sizeof(delete_path_raw));
  url_decode(delete_path_raw, delete_path_dec, sizeof(delete_path_dec));

  const char *mount = get_mount_from_req(req);
  file_op_ctx_t *ctx = malloc(sizeof(file_op_ctx_t));
  snprintf(ctx->path, sizeof(ctx->path), "%s%s", mount, delete_path_dec);

  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(delete_task, "delete", 4096, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }
  return ESP_OK;
}

static void mkdir_task(void *pv) {
  file_op_ctx_t *ctx = pv;

  if (sd_create_dir(ctx->path) == ESP_OK)
    httpd_resp_sendstr(ctx->req, "OK");
  else
    httpd_resp_send_err(ctx->req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "Create Fail");

  httpd_req_async_handler_complete(ctx->req);
  free(ctx);
  vTaskDelete(NULL);
}

esp_err_t http_upload_File_handler(httpd_req_t *req) {
  char buf[256];
  char foldname[128] = {0};
  char currentPath[256] = {0};

  // 读取参数
  if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) != ESP_OK)
    return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Param Err");

  // 直接解码取出
  httpd_query_key_value(buf, "foldname", foldname, sizeof(foldname));
  url_decode(foldname, foldname, sizeof(foldname));

  httpd_query_key_value(buf, "currentPath", currentPath, sizeof(currentPath));
  url_decode(currentPath, currentPath, sizeof(currentPath));

  const char *mount = get_mount_from_req(req);
  file_op_ctx_t *ctx = malloc(sizeof(file_op_ctx_t));
  strcpy(ctx->path, mount);

  // 拼接路径
  if (strcmp(currentPath, "/") != 0)
    strcat(ctx->path, currentPath);

  strcat(ctx->path, "/");
  strcat(ctx->path, foldname);

  // 异步任务
  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(mkdir_task, "mkdir", 4096, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }
  return ESP_OK;
}

esp_err_t http_modify_handler(httpd_req_t *req) {
  char buf[256];
  char filename_raw[128] = {0};
  char filename_dec[128] = {0};
  char full_path[310] = {0};

  if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) != ESP_OK)
    return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Query Err");

  if (httpd_query_key_value(buf, "filename", filename_raw,
                            sizeof(filename_raw)) == ESP_OK)
    url_decode(filename_raw, filename_dec, sizeof(filename_dec));

  if (strlen(filename_dec) == 0)
    return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Filename missing");

  const char *mount = get_mount_from_req(req);
  snprintf(full_path, sizeof(full_path), "%s%s", mount, filename_dec);


  httpd_resp_set_type(req, "text/plain; charset=utf-8");
  return http_send_file_in_chunks(req, full_path, 4096);
}

// 🔥 仅使用你现有框架 → 异步上传文件
esp_err_t http_upload_all_handler(httpd_req_t *req) {
  char filename_hdr[128] = {0};
  char dir[256] = {0};

  // 获取文件名
  if (httpd_req_get_hdr_value_str(req, "File-Name", filename_hdr,
                                  sizeof(filename_hdr)) != ESP_OK) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File-Name Missing");
    return ESP_FAIL;
  }

  // 获取当前目录 dir（一次解码完成）
  httpd_req_get_url_query_str(req, dir, sizeof(dir));
  httpd_query_key_value(dir, "dir", dir, sizeof(dir));
  url_decode(dir, dir, sizeof(dir));

  // 文件名解码
  char final_name[128];
  url_decode(filename_hdr, final_name, sizeof(final_name));
  url_decode(final_name, final_name, sizeof(final_name));

  // 分配上下文
  upload_ctx_t *ctx = heap_caps_malloc(sizeof(upload_ctx_t), MALLOC_CAP_DMA);
  if (!ctx) {
    httpd_resp_send_500(req);
    return ESP_ERR_NO_MEM;
  }

  // 拼接路径（极简）
  const char *mount = get_mount_from_req(req);
  snprintf(ctx->filepath, sizeof(ctx->filepath), "%s%s/%s", mount, dir,
           final_name);

  ctx->req = req;
  ctx->content_len = req->content_len;

  // 异步任务
  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreatePinnedToCore(upload_task, "upload", 8192, ctx, 5, NULL, 1) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }

  return ESP_OK;
}

/* WiFi 配置部分继续使用硬编码路径，不修改 */
typedef struct {
  httpd_req_t *req;
} wifi_list_ctx_t;

static void get_wifi_task(void *pv) {
  wifi_list_ctx_t *ctx = pv;
  httpd_req_t *req = ctx->req;

  readConfig("/配置/配置.txt");
  sd_read_wifi_list("/配置/无线网络.txt");
  const size_t buf_size = 1024;
  char *jsonBuffer = (char *)malloc(buf_size);
  if (!jsonBuffer) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No memory");
    goto done;
  }

  int cur = snprintf(
      jsonBuffer, buf_size,
      "{\"autoWifinum\":\"%d\",\"ssid\":\"%s\",\"password\":\"%s\",\"channel\":"
      "\"%s\",\"wifiConnectTime\":\"%d\",\"wifilist\":[",
      autoWifinum, hotspotName, hotspotPassword, hotspotChannel, wifiConnectTime);

  for (int i = 0; i < autoWifinum; i++) {
    if (cur >= buf_size - 64)
      break;
    int len =
        snprintf(jsonBuffer + cur, buf_size - cur,
                 "{\"wifiname\":\"%s\",\"wifipass\":\"%s\"}%s", wifiSsid[i],
                 wifiPassword[i], (i < autoWifinum - 1) ? "," : "");
    cur += len;
  }
  if (cur < buf_size - 3)
    strcat(jsonBuffer, "]}");

  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, jsonBuffer, HTTPD_RESP_USE_STRLEN);
  free(jsonBuffer);

done:
  httpd_req_async_handler_complete(req);
  free(ctx);
  vTaskDelete(NULL);
}

esp_err_t http_get_wifi_handler(httpd_req_t *req) {
  wifi_list_ctx_t *ctx = malloc(sizeof(wifi_list_ctx_t));
  if (!ctx)
    return ESP_ERR_NO_MEM;

  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(get_wifi_task, "get_wifi", 4096, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }
  return ESP_OK;
}

typedef struct {
  httpd_req_t *req;
  char buf[1024];
} config_ap_ctx_t;

static void config_ap_task(void *pv) {
  config_ap_ctx_t *ctx = pv;
  httpd_req_t *req = ctx->req;
  char *buf = ctx->buf;

  char param[128];
  if (httpd_query_key_value(buf, "hotspotName", param, sizeof(param)) == ESP_OK)
    url_decode(param, hotspotName, sizeof(hotspotName));
  if (httpd_query_key_value(buf, "hotspotPassword", param, sizeof(param)) ==
      ESP_OK)
    url_decode(param, hotspotPassword, sizeof(hotspotPassword));
  if (httpd_query_key_value(buf, "channel", param, sizeof(param)) == ESP_OK)
    url_decode(param, hotspotChannel, sizeof(hotspotChannel));

  char message[256];
  snprintf(message, sizeof(message), "%s\n%s\n%s\n", hotspotName,
           hotspotPassword, hotspotChannel);
  esp_err_t ret = sd_write_file("/配置/热点.txt", message);
  if (ret == ESP_OK)
    httpd_resp_sendstr(req, "OK");
  else
    httpd_resp_send_500(req);

  httpd_req_async_handler_complete(req);
  free(ctx);
  vTaskDelete(NULL);
}

esp_err_t http_config_ap_handler(httpd_req_t *req) {
  size_t buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len <= 1) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing query string");
    return ESP_FAIL;
  }

  config_ap_ctx_t *ctx = malloc(sizeof(config_ap_ctx_t));
  if (!ctx)
    return ESP_ERR_NO_MEM;

  if (httpd_req_get_url_query_str(req, ctx->buf, sizeof(ctx->buf)) != ESP_OK) {
    free(ctx);
    return ESP_FAIL;
  }

  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(config_ap_task, "config_ap", 4096, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }
  return ESP_OK;
}

typedef struct {
  httpd_req_t *req;
  char buf[128];
} config_file_ctx_t;

static void config_file_task(void *pv) {
  config_file_ctx_t *ctx = pv;
  httpd_req_t *req = ctx->req;
  char *buf = ctx->buf;

  char val[32];
  if (httpd_query_key_value(buf, "wifiConnectTime", val, sizeof(val)) ==
      ESP_OK) {
    wifiConnectTime = atoi(val);
    write_config_key_value("/配置/配置.txt", "wifiConnectTime", val);
    readConfig("/配置/配置.txt");
    httpd_resp_sendstr(req, "OK");
  } else {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Param error");
  }

  httpd_req_async_handler_complete(req);
  free(ctx);
  vTaskDelete(NULL);
}

esp_err_t http_config_file_handler(httpd_req_t *req) {
  config_file_ctx_t *ctx = malloc(sizeof(config_file_ctx_t));
  if (!ctx)
    return ESP_ERR_NO_MEM;

  httpd_req_get_url_query_str(req, ctx->buf, sizeof(ctx->buf));

  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(config_file_task, "config_file", 4096, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }
  return ESP_OK;
}

typedef struct {
  httpd_req_t *req;
} addwifi_ctx_t;

static void add_wifi_task(void *pv) {
  addwifi_ctx_t *ctx = pv;
  httpd_req_t *req = ctx->req;

  if (autoWifinum < 5) {
    autoWifinum++;
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", autoWifinum);
    write_config_key_value("/配置/配置.txt", "autoWifinum", buf);
    httpd_resp_sendstr(req, "OK");
  } else {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "WiFi数量不能大于5");
  }

  httpd_req_async_handler_complete(req);
  free(ctx);
  vTaskDelete(NULL);
}

esp_err_t http_add_wifi_handler(httpd_req_t *req) {
  addwifi_ctx_t *ctx = malloc(sizeof(addwifi_ctx_t));
  if (!ctx)
    return ESP_ERR_NO_MEM;

  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(add_wifi_task, "add_wifi", 4096, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }
  return ESP_OK;
}

typedef struct {
  httpd_req_t *req;
} dedwifi_ctx_t;

static void deduce_wifi_task(void *pv) {
  dedwifi_ctx_t *ctx = pv;
  httpd_req_t *req = ctx->req;

  if (autoWifinum > 1) {
    autoWifinum--;
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", autoWifinum);
    write_config_key_value("/配置/配置.txt", "autoWifinum", buf);
    httpd_resp_sendstr(req, "OK");
  } else {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "WiFi数量不能小于1");
  }

  httpd_req_async_handler_complete(req);
  free(ctx);
  vTaskDelete(NULL);
}

esp_err_t http_deduce_wifi_handler(httpd_req_t *req) {
  dedwifi_ctx_t *ctx = malloc(sizeof(dedwifi_ctx_t));
  if (!ctx)
    return ESP_ERR_NO_MEM;

  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(deduce_wifi_task, "ded_wifi", 4096, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }
  return ESP_OK;
}

typedef struct {
  httpd_req_t *req;
  char buf[1024];
} config_wifi_ctx_t;

static void config_wifi_task(void *pv) {
  config_wifi_ctx_t *ctx = pv;
  httpd_req_t *req = ctx->req;
  char *buf = ctx->buf;

  char *file_data = (char *)calloc(1, 2048);
  if (!file_data) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No memory");
    goto done;
  }

  char key_name[32], key_pass[32];
  char val_name[128], val_pass[128];
  char decoded_name[128], decoded_pass[128];
  int offset = 0, real_count = 0;

  for (int i = 0; i < autoWifinum && i < 5; i++) {
    snprintf(key_name, sizeof(key_name), "WifiName%d", i + 1);
    snprintf(key_pass, sizeof(key_pass), "WifiPassword%d", i + 1);
    decoded_name[0] = '\0';
    decoded_pass[0] = '\0';

    if (httpd_query_key_value(buf, key_name, val_name, sizeof(val_name)) ==
        ESP_OK)
      url_decode(val_name, decoded_name, sizeof(decoded_name));
    if (httpd_query_key_value(buf, key_pass, val_pass, sizeof(val_pass)) ==
        ESP_OK)
      url_decode(val_pass, decoded_pass, sizeof(decoded_pass));

    if (strlen(decoded_name) == 0)
      continue;

    int written = snprintf(file_data + offset, 2048 - offset, "%s\n%s\n",
                           decoded_name, decoded_pass);
    if (written < 0 || written >= 2048 - offset)
      break;
    offset += written;
    real_count++;
  }

  if (real_count == 0) {
    free(file_data);
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "No valid WiFi config");
    goto done;
  }

  esp_err_t ret = sd_write_file("/配置/无线网络.txt", file_data);
  if (ret != ESP_OK) {
    free(file_data);
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "Write wifi file failed");
    goto done;
  }
  sd_read_wifi_list("/配置/无线网络.txt");
  readConfig("/配置/配置.txt");
  free(file_data);
  httpd_resp_sendstr(req, "OK");

done:
  httpd_req_async_handler_complete(req);
  free(ctx);
  vTaskDelete(NULL);
}

esp_err_t http_config_wifi_handler(httpd_req_t *req) {
  config_wifi_ctx_t *ctx = malloc(sizeof(config_wifi_ctx_t));
  if (!ctx)
    return ESP_ERR_NO_MEM;

  httpd_req_get_url_query_str(req, ctx->buf, sizeof(ctx->buf));

  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(config_wifi_task, "config_wifi", 4096, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }
  return ESP_OK;
}

esp_err_t http_game_list_handler(httpd_req_t *req) {
  // 1. 定义缓冲区接收 URL 参数
  char param_buf[16] = {0};

  // 2. 解析 URL 参数 ?type=xxx
  if (httpd_req_get_url_query_str(req, param_buf, sizeof(param_buf)) ==
      ESP_OK) {
    char type_buf[8] = {0};

    // 3. 获取 type 参数值
    if (httpd_query_key_value(param_buf, "type", type_buf, sizeof(type_buf)) ==
        ESP_OK) {
      // 4. FLASH：返回 /视频 列表
      if (strcmp(type_buf, "flash") == 0) {
        listFile(req, "/游戏/老游戏");
        return ESP_OK;
      }
      // 5. HTML：返回 /游戏 + /游戏/老游戏 列表
      else if (strcmp(type_buf, "html") == 0) {
        scanGameAndListFile(req, "/游戏");
        return ESP_OK;
      }
    }
  }
  return ESP_OK;
}
esp_err_t http_get_video_handler(httpd_req_t *req) {
  listFile(req, "/视频");
  return ESP_OK;
}
esp_err_t http_list_audio_handler(httpd_req_t *req) {
  listFile(req, "/音乐");
  return ESP_OK;
}

typedef struct {
  httpd_req_t *req;
} version_ctx_t;

static void get_version_task(void *pv) {
  version_ctx_t *ctx = pv;
  httpd_req_t *req = ctx->req;

  char json[768];
  char ip_str[32] = {0}, ipv6_str[256] = {0}, ssid_str[64] = {0};

  // 你原来的调用方式，完全不变
  wifi_get_ip_and_ssid(ip_str, sizeof(ip_str), ssid_str, sizeof(ssid_str));
  wifi_get_ipv6_address(ipv6_str, sizeof(ipv6_str)); // ✅ 获取所有 IPv6

  // IPv6 放最后面！！！
  snprintf(json, sizeof(json),
           "{\"version\":\"%s\",\"text\":\"%s\",\"FirstWebis\":%d,\"IPAD\":\"%"
           "s\",\"WIFI\":\"%s\",\"IPV6\":\"%s\"}",
           VERSION, hostName, FirstWebis, ip_str, ssid_str, ipv6_str);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);

  httpd_req_async_handler_complete(req);
  free(ctx);
  vTaskDelete(NULL);
}

esp_err_t http_get_version_handler(httpd_req_t *req) {
  version_ctx_t *ctx = malloc(sizeof(version_ctx_t));
  if (!ctx)
    return ESP_ERR_NO_MEM;

  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(get_version_task, "get_ver", 4096, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }
  return ESP_OK;
}

typedef struct {
  httpd_req_t *req;
  char buf[256];
} selpath_ctx_t;

static void select_path_task(void *pv) {
  selpath_ctx_t *ctx = pv;
  httpd_req_t *req = ctx->req;
  char *buf = ctx->buf;

  char path_buf[256] = {0};

  // 解析 path
  if (httpd_query_key_value(buf, "path", path_buf, sizeof(path_buf)) ==
      ESP_OK) {
    url_decode(path_buf, path_buf, sizeof(path_buf));

    // ==============================
    // 🔥 你项目真正的文件列表：listFile
    // ==============================
    listFile(req, path_buf);
  } else {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Path error");
  }

  httpd_req_async_handler_complete(req);
  free(ctx);
  vTaskDelete(NULL);
}

esp_err_t http_select_path_handler(httpd_req_t *req) {
  selpath_ctx_t *ctx = malloc(sizeof(selpath_ctx_t));
  if (!ctx)
    return ESP_ERR_NO_MEM;

  httpd_req_get_url_query_str(req, ctx->buf, sizeof(ctx->buf));

  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(select_path_task, "sel_path", 4096, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }
  return ESP_OK;
}

typedef struct {
  httpd_req_t *req;
  char buf[256];
} firstweb_cfg_ctx_t;

static void first_web_task(void *pv) {
  firstweb_cfg_ctx_t *ctx = pv;
  httpd_req_t *req = ctx->req;
  char *buf = ctx->buf;

  char val[32];
  if (httpd_query_key_value(buf, "FirstWebis", val, sizeof(val)) == ESP_OK) {
    FirstWebis = atoi(val);
    write_config_key_value("/配置/配置.txt", "FirstWebis", val);
    readConfig("/配置/配置.txt");
    httpd_resp_sendstr(req, "OK");
  } else {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Error");
  }

  httpd_req_async_handler_complete(req);
  free(ctx);
  vTaskDelete(NULL);
}

esp_err_t http_first_web_handler(httpd_req_t *req) {
  firstweb_cfg_ctx_t *ctx = malloc(sizeof(firstweb_cfg_ctx_t));
  if (!ctx)
    return ESP_ERR_NO_MEM;

  httpd_req_get_url_query_str(req, ctx->buf, sizeof(ctx->buf));

  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(first_web_task, "first_web", 4096, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }
  return ESP_OK;
}

// OTA 异步上下文
typedef struct {
  httpd_req_t *req;
} ota_ctx_t;

static void ota_update_task(void *pv) {
  ota_ctx_t *ctx = pv;
  httpd_req_t *req = ctx->req;

  uint8_t *buf =
      heap_caps_malloc(OTA_BUF_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
  if (!buf) {
    buf = heap_caps_malloc(OTA_BUF_SIZE, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  }
  if (!buf) {
    httpd_resp_send_500(req);
    goto exit;
  }

  if (ota_handle == 0) {
    ota_part = esp_ota_get_next_update_partition(NULL);
    if (!ota_part) {
      free(buf);
      httpd_resp_send_err(req, 500, "NoOTA");
      goto exit;
    }
    if (esp_ota_begin(ota_part, OTA_SIZE_UNKNOWN, &ota_handle) != ESP_OK) {
      esp_ota_abort(ota_handle);
      ota_handle = 0;
      free(buf);
      httpd_resp_send_err(req, 500, "BeginFail");
      goto exit;
    }
  }

  ssize_t recv_len;
  while ((recv_len = httpd_req_recv(req, (char *)buf, OTA_BUF_SIZE)) > 0) {
    if (esp_ota_write(ota_handle, buf, recv_len) != ESP_OK) {
      esp_ota_abort(ota_handle);
      ota_handle = 0;
      free(buf);
      httpd_resp_send_err(req, 500, "WriteFail");
      goto exit;
    }
  }

  if (esp_ota_end(ota_handle) != ESP_OK) {
    ota_handle = 0;
    free(buf);
    httpd_resp_send_err(req, 500, "InvalidFW");
    goto exit;
  }
  if (esp_ota_set_boot_partition(ota_part) != ESP_OK) {
    ota_handle = 0;
    free(buf);
    httpd_resp_send_err(req, 500, "BootFail");
    goto exit;
  }

  free(buf);
  ota_handle = 0;
  httpd_resp_sendstr(req, "OK");
  vTaskDelay(pdMS_TO_TICKS(500));
  esp_restart();

exit:
  httpd_req_async_handler_complete(req);
  free(ctx);
  vTaskDelete(NULL);
}

// OTA 异步入口
esp_err_t http_ota_update_handler(httpd_req_t *req) {
  ota_ctx_t *ctx = malloc(sizeof(ota_ctx_t));
  if (!ctx) {
    httpd_resp_send_500(req);
    return ESP_ERR_NO_MEM;
  }

  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(ota_update_task, "ota_update", 8192, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }

  return ESP_OK;
}

typedef struct {
  httpd_req_t *req;
  char buf[64];
} nametext_ctx_t;

static void name_text_task(void *pv) {
  nametext_ctx_t *ctx = pv;
  httpd_req_t *req = ctx->req;
  char *buf = ctx->buf;

  char decoded_val[63] = {0};
  if (httpd_query_key_value(buf, "nameText", decoded_val,
                            sizeof(decoded_val)) == ESP_OK) {
    strncpy(hostName, decoded_val, sizeof(hostName) - 1);
    write_config_key_value("/配置/配置.txt", "hostName", decoded_val);
    readConfig("/配置/配置.txt");
    httpd_resp_sendstr(req, "OK");
  } else {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Field nameText missing");
  }

  httpd_req_async_handler_complete(req);
  free(ctx);
  vTaskDelete(NULL);
}

esp_err_t http_name_Text_handler(httpd_req_t *req) {
  nametext_ctx_t *ctx = malloc(sizeof(nametext_ctx_t));
  if (!ctx)
    return ESP_ERR_NO_MEM;

  httpd_req_get_url_query_str(req, ctx->buf, sizeof(ctx->buf));

  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(name_text_task, "name_text", 4096, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }
  return ESP_OK;
}

// 异步测试IO任务
typedef struct {
  httpd_req_t *req;
} test_io_ctx_t;

static void test_file_io_task(void *pv) {
  test_io_ctx_t *ctx = pv;
  httpd_req_t *req = ctx->req;

  uint32_t w_bytes = 0, w_time = 0, r_bytes = 0, r_time = 0;
  sd_test_file_io(&w_bytes, &w_time, &r_bytes, &r_time);

  char json[384];
  snprintf(json, sizeof(json),
           "{\"writtenBytes\":%lu,\"writeTime\":%lu,\"readBytes\":%lu,"
           "\"readTime\":%lu}",
           (unsigned long)w_bytes, (unsigned long)w_time,
           (unsigned long)r_bytes, (unsigned long)r_time);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);

  // 异步结束
  httpd_req_async_handler_complete(req);
  free(ctx);
  vTaskDelete(NULL);
}

// 异步入口（和你项目统一风格）
esp_err_t http_testFileIO_handler(httpd_req_t *req) {
  test_io_ctx_t *ctx = malloc(sizeof(test_io_ctx_t));
  if (!ctx) {
    httpd_resp_send_500(req);
    return ESP_ERR_NO_MEM;
  }

  // 开启HTTP异步
  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(test_file_io_task, "test_io", 4096, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }

  return ESP_OK;
}

void wifi_switch_task(void *pv) {
  wifi_switch_sta_ap();
  vTaskDelete(NULL);
}

esp_err_t http_wifi_connect_handler(httpd_req_t *req) {
  if (xTaskCreate(wifi_switch_task, "wifi_switch", 4096, NULL, 5, NULL) != pdPASS) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }
  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, "{\"status\":\"ok\",\"msg\":\"WiFi 切换中...\"}",
                  HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

const char *get_mime_type(const char *filepath) {
  const char *ext = strrchr(filepath, '.');
  if (!ext)
    return "application/octet-stream";
  if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0)
    return "text/html";
  if (strcmp(ext, ".css") == 0)
    return "text/css";
  if (strcmp(ext, ".js") == 0)
    return "application/javascript";
  if (strcmp(ext, ".json") == 0)
    return "application/json";
  if (strcmp(ext, ".png") == 0)
    return "image/png";
  if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0)
    return "image/jpeg";
  if (strcmp(ext, ".gif") == 0)
    return "image/gif";
  if (strcmp(ext, ".svg") == 0)
    return "image/svg+xml";
  if (strcmp(ext, ".ico") == 0)
    return "image/x-icon";
  if (strcmp(ext, ".webp") == 0)
    return "image/webp";
  if (strcmp(ext, ".woff") == 0)
    return "font/woff";
  if (strcmp(ext, ".woff2") == 0)
    return "font/woff2";
  if (strcmp(ext, ".mp3") == 0)
    return "audio/mpeg";
  if (strcmp(ext, ".wav") == 0)
    return "audio/wav";
  if (strcmp(ext, ".ogg") == 0)
    return "audio/ogg";
  if (strcmp(ext, ".flac") == 0)
    return "audio/flac";
  if (strcmp(ext, ".aac") == 0)
    return "audio/aac";
  if (strcmp(ext, ".m4a") == 0)
    return "audio/mp4";
  if (strcmp(ext, ".mp4") == 0)
    return "video/mp4";
  if (strcmp(ext, ".webm") == 0)
    return "video/webm";
  if (strcmp(ext, ".avi") == 0)
    return "video/x-msvideo";
  return "application/octet-stream";
}

esp_err_t http_serve_file_range(httpd_req_t *req, const char *filepath) {
  FILE *fd = fopen(filepath, "rb");
  if (!fd) {
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
    return ESP_FAIL;
  }

  fseek(fd, 0, SEEK_END);
  size_t file_size = ftell(fd);
  fseek(fd, 0, SEEK_SET);

  char range_str[128] = {0};
  size_t range_len = httpd_req_get_hdr_value_len(req, "Range");
  if (range_len > 0 && range_len < sizeof(range_str)) {
    httpd_req_get_hdr_value_str(req, "Range", range_str, sizeof(range_str));
  }

  httpd_resp_set_type(req, get_mime_type(filepath));
  httpd_resp_set_hdr(req, "Accept-Ranges", "bytes");

  if (req->method == HTTP_HEAD) {
    fclose(fd);
    char len[32];
    snprintf(len, sizeof(len), "%zu", file_size);
    httpd_resp_set_hdr(req, "Content-Length", len);
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
  }

  size_t start = 0, end = file_size - 1;
  bool has_range = false;

  if (strstr(range_str, "bytes=") != NULL) {
    char *p = range_str + 6;
    char *dash = strchr(p, '-');
    if (dash) {
      *dash = 0;
      if (strlen(p) > 0)
        start = atoll(p);
      if (strlen(dash + 1) > 0)
        end = atoll(dash + 1);
    }
    has_range = true;
  }

  if (end >= file_size)
    end = file_size - 1;

  if (start > end || start >= file_size) {
    char cr[64];
    snprintf(cr, sizeof(cr), "bytes */%zu", file_size);
    httpd_resp_set_status(req, "416 Range Not Satisfiable");
    httpd_resp_set_hdr(req, "Content-Range", cr);
    httpd_resp_send(req, NULL, 0);
    fclose(fd);
    return ESP_OK;
  }

  size_t content_len = end - start + 1;
  if (has_range) {
    httpd_resp_set_status(req, "206 Partial Content");
    char cr[64];
    snprintf(cr, sizeof(cr), "bytes %zu-%zu/%zu", start, end, file_size);
    httpd_resp_set_hdr(req, "Content-Range", cr);
  } else {
    httpd_resp_set_status(req, "200 OK");
  }

  // ============ IDF6.0 官方异步 ============
  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);

  async_send_ctx_t *ctx = calloc(1, sizeof(async_send_ctx_t));
  ctx->req = async_req;
  ctx->fd = fd;
  ctx->start = start;
  ctx->remaining = content_len;

  // 绑核1：与 http_serve_file 路径一致，避开核0的WiFi驱动任务
  if (xTaskCreatePinnedToCore(async_send_task, "async_send", 8192, ctx, 5,
                              NULL, 1) != pdPASS) {
    fclose(fd);
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }

  return ESP_OK;
}

esp_err_t http_download_handler(httpd_req_t *req) {
  char buf[256] = {0};
  char fullPath[310] = {0};

  // 取前端传过来的完整路径（已包含目录 + 文件名）
  httpd_req_get_url_query_str(req, buf, sizeof(buf));
  httpd_query_key_value(buf, "path", fullPath, sizeof(fullPath));
  url_decode(fullPath, fullPath, sizeof(fullPath));

  // 拼接挂载点 + 完整路径（不再用 foldPath）
  char file_path[310] = {0};
  const char *mount = get_mount_from_req(req);
  strcpy(file_path, mount);
  strcat(file_path, fullPath);

  // 提取文件名
  const char *filename = strrchr(fullPath, '/');
  if (filename)
    filename++;
  else
    filename = fullPath;


  FILE *fd = fopen(file_path, "rb");
  if (!fd) {
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
    return ESP_FAIL;
  }

  fseek(fd, 0, SEEK_END);
  size_t file_size = ftell(fd);
  fseek(fd, 0, SEEK_SET);

  char disp[334];
  snprintf(disp, sizeof(disp), "attachment; filename=\"%s\"", filename);
  httpd_resp_set_hdr(req, "Content-Disposition", disp);
  httpd_resp_set_type(req, "application/octet-stream");

  // 异步发送
  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);

  async_send_ctx_t *ctx = calloc(1, sizeof(async_send_ctx_t));
  ctx->req = async_req;
  ctx->fd = fd;
  ctx->remaining = file_size;

  // 绑核1：与 http_serve_file 路径一致，避开核0的WiFi驱动任务
  if (xTaskCreatePinnedToCore(async_send_task, "async_send", 8192, ctx, 5,
                              NULL, 1) != pdPASS) {
    fclose(fd);
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }

  return ESP_OK;
}

// 异步 LUA 任务
typedef struct {
  httpd_req_t *req;
  char buf[4096];
} lua_run_ctx_t;

static void run_lua_task(void *pv) {
  lua_run_ctx_t *ctx = pv;
  httpd_req_t *req = ctx->req;
  char *buf = ctx->buf;

  bool is_long = false;
  if (strncmp(buf, "long ", 5) == 0) {
    is_long = true;
    memmove(buf, buf + 5, strlen(buf + 5) + 1);
  }

  if (strncmp(buf, "run ", 4) == 0) {
    char *cmd = buf + 4;
    while (*cmd == ' ')
      cmd++;

    char *path = cmd;
    char *arg = strchr(path, ' ');
    if (arg) {
      *arg = '\0';
      arg++;
      while (*arg == ' ')
        arg++;
    }

    char *end = path + strlen(path) - 1;
    while (end > path && (*end == ' ' || *end == '\n' || *end == '\r')) {
      *end-- = 0;
    }

    if (*path == '/') {
      char full_path[350];
      snprintf(full_path, sizeof(full_path), "%s%s", sd_get_mount_point(),
               path);

      FILE *f = fopen(full_path, "r");
      if (!f) {
        httpd_resp_send(req, "读文件失败", 10);
        goto done;
      }

      fseek(f, 0, SEEK_END);
      int size = ftell(f);
      fseek(f, 0, SEEK_SET);

      char *code = malloc(size + 1);
      if (!code) {
        fclose(f);
        httpd_resp_send(req, "内存不足", 12);
        goto done;
      }
      fread(code, 1, size, f);
      code[size] = 0;
      fclose(f);

      if (is_long) {
        lua_run_global(code);
      } else {
        lua_run_with_arg(code, arg);
      }
      free(code);
    } else {
      httpd_resp_send(req, "路径必须以 / 开头", 14);
    }
  } else {
    if (is_long) {
      lua_run_global(buf);
    } else {
      lua_run_with_arg(buf, NULL);
    }
  }

  const char *result = lua_output_get();
  httpd_resp_send(req, result, strlen(result));

done:
  httpd_req_async_handler_complete(req);
  free(ctx);
  vTaskDelete(NULL);
}

// 异步入口（和你项目所有异步接口统一风格）
esp_err_t http_run_lua_handler(httpd_req_t *req) {
  lua_run_ctx_t *ctx = malloc(sizeof(lua_run_ctx_t));
  if (!ctx) {
    httpd_resp_send(req, "内存不足", 8);
    return ESP_ERR_NO_MEM;
  }

  // 接收数据
  int ret = httpd_req_recv(req, ctx->buf, 4095);
  ctx->buf[ret > 0 ? ret : 0] = 0;

  // 开启 HTTP 异步
  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  // 启动任务
  if (xTaskCreate(run_lua_task, "run_lua", 8192, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }

  return ESP_OK;
}

// ==========================
// 创建文件（自动补 .txt）
// ==========================
static void create_file_task(void *pv) {
  file_op_ctx_t *ctx = pv;

  FILE *f = fopen(ctx->path, "w");
  if (f) {
    fclose(f);
    httpd_resp_sendstr(ctx->req, "OK");
  } else {
    httpd_resp_send_err(ctx->req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "创建文件失败");
  }

  httpd_req_async_handler_complete(ctx->req);
  free(ctx);
  vTaskDelete(NULL);
}

esp_err_t http_create_file_handler(httpd_req_t *req) {
  char buf[256];
  char filename[128] = {0};
  char currentPath[256] = {0};

  // 读取参数
  if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) != ESP_OK)
    return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Param Err");

  // 取文件名 + 解码
  httpd_query_key_value(buf, "filename", filename, sizeof(filename));
  url_decode(filename, filename, sizeof(filename));

  if (strlen(filename) == 0)
    return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "文件名不能为空");

  // 自动补 .txt
  char *dot = strrchr(filename, '.');
  if (dot == NULL || strlen(dot) < 2)
    strcat(filename, ".txt");

  // 取前端传的当前目录
  httpd_query_key_value(buf, "currentPath", currentPath, sizeof(currentPath));
  url_decode(currentPath, currentPath, sizeof(currentPath));

  // 拼接路径
  const char *mount = get_mount_from_req(req);
  file_op_ctx_t *ctx = malloc(sizeof(file_op_ctx_t));
  strcpy(ctx->path, mount);

  if (strcmp(currentPath, "/") != 0)
    strcat(ctx->path, currentPath);

  strcat(ctx->path, "/");
  strcat(ctx->path, filename);

  // 异步任务
  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(create_file_task, "mkfile", 4096, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }
  return ESP_OK;
}

// ==========================
// ZIP 预览（返回 文件名+大小）
// ==========================
esp_err_t http_zip_list_handler(httpd_req_t *req) {
  size_t qlen = httpd_req_get_url_query_len(req) + 1;

  char *buf = heap_caps_malloc(qlen, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!buf) {
    httpd_resp_send_500(req);
    return ESP_ERR_NO_MEM;
  }

  if (httpd_req_get_url_query_str(req, buf, qlen) != ESP_OK) {
    free(buf);
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  char *zip_path = heap_caps_malloc(512, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  char *full_path =
      heap_caps_malloc(512, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!zip_path || !full_path) {
    free(zip_path);
    free(full_path);
    free(buf);
    httpd_resp_send_500(req);
    return ESP_ERR_NO_MEM;
  }

  httpd_query_key_value(buf, "path", zip_path, 512);
  url_decode(zip_path, zip_path, 512);

  const char *mount = get_mount_from_req(req);
  snprintf(full_path, 512, "%s%s", mount, zip_path);

  // 打开ZIP（你已实现）
  mz_zip_archive zip;
  if (!mz_zip_reader_init_file(&zip, full_path)) {
    free(full_path);
    free(zip_path);
    free(buf);
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  int file_cnt = mz_zip_reader_get_num_files(&zip);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_send_chunk(req, "[", 1);

  for (int i = 0; i < file_cnt; i++) {
    // 取文件名（你已实现）
    const char *name = mz_zip_reader_get_filename(&zip, i);
    if (!name)
      continue;

    // 取大小（直接读你已存好的字段）
    size_t size = zip.file_list[i].uncomp_size;

    // 输出 JSON
    char *item = heap_caps_malloc(300, MALLOC_CAP_INTERNAL);
    if (!item)
      continue;
    if (i == 0) {
      snprintf(item, 300, "{\"name\":\"%s\",\"size\":%u}", name, size);
    } else {
      snprintf(item, 300, ",{\"name\":\"%s\",\"size\":%u}", name, size);
    }

    httpd_resp_send_chunk(req, item, strlen(item));
    free(item);
  }

  httpd_resp_send_chunk(req, "]", 1);
  httpd_resp_send_chunk(req, NULL, 0);

  // 关闭释放（你已实现）
  mz_zip_reader_end(&zip);
  free(full_path);
  free(zip_path);
  free(buf);

  return ESP_OK;
}
// 强制删除：文件 / 文件夹 / 非空文件夹 全部删掉
static void delete_all_task(void *pv) {
  file_op_ctx_t *ctx = pv;

  if (sd_delete_dir_recursive(ctx->path) == ESP_OK)
    httpd_resp_sendstr(ctx->req, "OK");
  else
    httpd_resp_send_err(ctx->req, HTTPD_500_INTERNAL_SERVER_ERROR, "FAIL");

  httpd_req_async_handler_complete(ctx->req);
  free(ctx);
  vTaskDelete(NULL);
}

esp_err_t http_delete_all_handler(httpd_req_t *req) {
  char buf[256];
  char del_path_raw[256];

  httpd_req_get_url_query_str(req, buf, sizeof(buf));
  httpd_query_key_value(buf, "deletePath", del_path_raw, sizeof(del_path_raw));
  url_decode(del_path_raw, del_path_raw, sizeof(del_path_raw));

  const char *mount = get_mount_from_req(req);
  file_op_ctx_t *ctx = malloc(sizeof(file_op_ctx_t));
  snprintf(ctx->path, sizeof(ctx->path), "%s%s", mount, del_path_raw);

  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;

  if (xTaskCreate(delete_all_task, "del_all", 8192, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }
  return ESP_OK;
}
static void zip_extract_task(void *pv) {
  zip_ctx_t *ctx = pv;
  sd_zip_extract_all(ctx->path1, ctx->path2);

  httpd_resp_sendstr(ctx->req, "OK");
  httpd_req_async_handler_complete(ctx->req);
  free(ctx); // 纯堆释放
  vTaskDelete(NULL);
}

esp_err_t http_zip_extract_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/plain; charset=utf-8");

  size_t qlen = httpd_req_get_url_query_len(req) + 1;
  char *buf = malloc(qlen);
  if (!buf) {
    httpd_resp_sendstr(req, "NO_MEM");
    return ESP_ERR_NO_MEM;
  }

  if (httpd_req_get_url_query_str(req, buf, qlen) != ESP_OK) {
    free(buf);
    httpd_resp_sendstr(req, "QUERY_ERR");
    return ESP_FAIL;
  }

  char zip_path[256] = {0};
  char out_dir[256] = {0};
  httpd_query_key_value(buf, "zipPath", zip_path, 255);
  httpd_query_key_value(buf, "outDir", out_dir, 255);

  // 解码
  url_decode(zip_path, zip_path, 255);
  url_decode(out_dir, out_dir, 255);

  const char *mount = get_mount_from_req(req);

  zip_ctx_t *ctx = calloc(1, sizeof(zip_ctx_t));

  // 拼接 ZIP 完整路径
  snprintf(ctx->path1, sizeof(ctx->path1), "%s%s", mount, zip_path);

  // 拼接输出目录（修复双斜杠）
  if (mount[strlen(mount) - 1] == '/') {
    snprintf(ctx->path2, sizeof(ctx->path2), "%s%s", mount, out_dir);
  } else {
    snprintf(ctx->path2, sizeof(ctx->path2), "%s/%s", mount, out_dir);
  }

  // 创建目录
  sd_create_dir(ctx->path2);
  free(buf);

  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;
  if (xTaskCreate(zip_extract_task, "zip_ext", 8192, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }

  return ESP_OK;
}
static void zip_compress_task(void *pv) {
  zip_ctx_t *ctx = pv;
  char *buf = ctx->path2;

  char **files = NULL;
  int file_cnt = 0;
  int max_cnt = 0;
  int idx = 0;

  while (1) {
    char key[20];
    snprintf(key, sizeof(key), "filePath%d", idx);

    char src_path[512] = {0};
    if (httpd_query_key_value(buf, key, src_path, sizeof(src_path) - 1) !=
        ESP_OK) {
      break;
    }

    const char *path_trim = src_path;
    while (*path_trim == '/') {
      path_trim++;
    }

    char mount[256] = "/sdcard";
    httpd_query_key_value(buf, "mount", mount, sizeof(mount) - 1);

    char full_path[769];
    snprintf(full_path, sizeof(full_path), "%s/%s", mount, path_trim);

    const char *fat_path = full_path;
    if (strstr(full_path, "/sdcard/") == full_path) {
      fat_path += 7;
    }

    FILINFO fno;
    if (f_stat(fat_path, &fno) != FR_OK) {
      idx++;
      continue;
    }

    if (file_cnt >= max_cnt) {
      int new_max = (max_cnt == 0) ? 32 : (max_cnt * 2);
      char **new_list = realloc(files, new_max * sizeof(char *));
      if (!new_list) {
        idx++;
        continue;
      }
      files = new_list;
      max_cnt = new_max;
    }

    if (fno.fattrib & AM_DIR) {
      char scan_path[512];
      if (strstr(full_path, "/sdcard/") == full_path) {
        strcpy(scan_path, full_path + 7);
      } else {
        strcpy(scan_path, full_path);
      }
      scan_directory_simple(scan_path, full_path, &files, &file_cnt, &max_cnt);
    } else {
      files[file_cnt] = strdup(full_path);
      if (files[file_cnt]) {
        file_cnt++;
      }
    }
    idx++;
  }

  char *p = ctx->path2;
  int remain = sizeof(ctx->path2);
  int copied = 0;
  for (int i = 0; i < file_cnt; i++) {
    int len = strlen(files[i]) + 1;
    if (len > remain) {
      // 缓冲区不够，释放剩余的文件路径避免泄漏
      for (int j = i; j < file_cnt; j++) {
        free(files[j]);
      }
      break;
    }
    strcpy(p, files[i]);
    p += len;
    remain -= len;
    copied++;
    free(files[i]);
  }
  if (files)
    free(files);

  ctx->count = copied;

  if (ctx->count > 0) {
    char **zfiles = malloc(ctx->count * sizeof(char *));
    char *pp = ctx->path2;
    for (int i = 0; i < ctx->count; i++) {
      zfiles[i] = pp;
      pp += strlen(pp) + 1;
    }
    sd_zip_compress_files(ctx->path1, (const char **)zfiles, ctx->count);
    free(zfiles);
  }

  httpd_resp_sendstr(ctx->req, "OK");
  httpd_req_async_handler_complete(ctx->req);
  free(ctx);
  vTaskDelete(NULL);
}
// 🔥 迭代遍历（非递归）无限子文件夹 + 无 truncation 警告
static esp_err_t scan_directory_simple(const char *dir_path,
                                       const char *vfs_base, char ***file_list,
                                       int *count, int *max) {
  if (!dir_path || !file_list || !count || !max)
    return ESP_ERR_INVALID_ARG;

  typedef struct dir_node {
    char fat_path[512];
    char vfs_path[512];
    struct dir_node *next;
  } dir_node_t;

  dir_node_t *head = malloc(sizeof(dir_node_t));
  if (!head)
    return ESP_ERR_NO_MEM;

  strlcpy(head->fat_path, dir_path, sizeof(head->fat_path));
  strlcpy(head->vfs_path, vfs_base, sizeof(head->vfs_path));
  head->next = NULL;
  dir_node_t *tail = head;

  while (head != NULL) {
    dir_node_t *current = head;
    FF_DIR dir;
    FRESULT res = f_opendir(&dir, current->fat_path);
    if (res != FR_OK) {
      head = head->next;
      free(current);
      continue;
    }

    FILINFO fno;
    while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != '\0') {
      if (!strcmp(fno.fname, ".") || !strcmp(fno.fname, ".."))
        continue;

      char fat_sub[512] = {0};
      char vfs_sub[512] = {0};

      // ✅ 安全拼接，无警告
      strlcpy(fat_sub, current->fat_path, sizeof(fat_sub));
      strlcat(fat_sub, "/", sizeof(fat_sub));
      strlcat(fat_sub, fno.fname, sizeof(fat_sub));

      strlcpy(vfs_sub, current->vfs_path, sizeof(vfs_sub));
      strlcat(vfs_sub, "/", sizeof(vfs_sub));
      strlcat(vfs_sub, fno.fname, sizeof(vfs_sub));

      // 目录入队
      if (fno.fattrib & AM_DIR) {
        dir_node_t *new_node = malloc(sizeof(dir_node_t));
        if (!new_node)
          continue;
        strlcpy(new_node->fat_path, fat_sub, sizeof(new_node->fat_path));
        strlcpy(new_node->vfs_path, vfs_sub, sizeof(new_node->vfs_path));
        new_node->next = NULL;
        tail->next = new_node;
        tail = new_node;
        continue;
      }

      // 文件加入列表
      if (*count >= *max) {
        int new_max = (*max == 0) ? 64 : (*max * 2);
        char **new_list = realloc(*file_list, new_max * sizeof(char *));
        if (!new_list)
          continue;
        *file_list = new_list;
        *max = new_max;
      }
      (*file_list)[*count] = strdup(vfs_sub);
      if ((*file_list)[*count])
        (*count)++;
    }

    f_closedir(&dir);
    head = head->next;
    free(current);
  }

  return ESP_OK;
}
esp_err_t http_zip_compress_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/plain; charset=utf-8");
  size_t content_len = req->content_len;

  if (content_len == 0 || content_len > 4096) {
    httpd_resp_sendstr(req, "ERR");
    return ESP_FAIL;
  }

  char *buf = malloc(content_len + 1);
  if (!buf) {
    httpd_resp_sendstr(req, "NO_MEM");
    return ESP_ERR_INVALID_ARG;
  }
  httpd_req_recv(req, buf, content_len);
  buf[content_len] = 0;

  zip_ctx_t *ctx = calloc(1, sizeof(zip_ctx_t));
  if (!ctx) {
    free(buf);
    httpd_resp_sendstr(req, "NO_MEM");
    return ESP_FAIL;
  }

  char zip_name[255] = {0};
  char currentPath[255] = {0};
  httpd_query_key_value(buf, "zipName", zip_name, sizeof(zip_name) - 1);
  httpd_query_key_value(buf, "currentPath", currentPath,
                        sizeof(currentPath) - 1);
  const char *mount = get_mount_from_req(req);

  if (currentPath[0] == '/') {
    memmove(currentPath, currentPath + 1, strlen(currentPath));
  }

  snprintf(ctx->path1, sizeof(ctx->path1), "%s/%s/%s", mount, currentPath,
           zip_name);
  strncpy(ctx->path2, buf, sizeof(ctx->path2) - 1);

  free(buf);

  httpd_req_t *async_req;
  httpd_req_async_handler_begin(req, &async_req);
  ctx->req = async_req;
  if (xTaskCreate(zip_compress_task, "zip_comp", 20480, ctx, 5, NULL) != pdPASS) {
    free(ctx);
    httpd_resp_send_500(async_req);
    httpd_req_async_handler_complete(async_req);
    return ESP_FAIL;
  }

  return ESP_OK;
}

const httpd_uri_t allFile_routes[] = {
    {.uri = "/get_storage",
     .method = HTTP_GET,
     .handler = http_get_storage_handler},
    {.uri = "/get_FirstWebis",
     .method = HTTP_GET,
     .handler = http_get_FirstWebis_handler},
    {.uri = "/editTxt", .method = HTTP_POST, .handler = http_edit_txt_handler},
    {.uri = "/looktxt", .method = HTTP_GET, .handler = http_look_txt_handler},
    {.uri = "/get_File", .method = HTTP_GET, .handler = http_list_Dir_handler},
    {.uri = "/get_RootFile",
     .method = HTTP_GET,
     .handler = http_get_RootFile_handler},
    {.uri = "/lookthis", .method = HTTP_GET, .handler = http_look_this_handler},
    {.uri = "/backRoot", .method = HTTP_GET, .handler = http_back_Root_handler},
    {.uri = "/backone", .method = HTTP_GET, .handler = http_back_one_handler},
    {.uri = "/renameFile",
     .method = HTTP_GET,
     .handler = http_rename_File_handler},
    {.uri = "/deleteUploadFile",
     .method = HTTP_GET,
     .handler = http_delete_File_handler},
    {.uri = "/uploadaddFold",
     .method = HTTP_GET,
     .handler = http_upload_File_handler},
    {.uri = "/modify", .method = HTTP_GET, .handler = http_modify_handler},
    {.uri = "/uploadAll",
     .method = HTTP_POST,
     .handler = http_upload_all_handler},
    {.uri = "/get_wifi", .method = HTTP_GET, .handler = http_get_wifi_handler},
    {.uri = "/configAP", .method = HTTP_GET, .handler = http_config_ap_handler},
    {.uri = "/addWifi", .method = HTTP_GET, .handler = http_add_wifi_handler},
    {.uri = "/deduceWifi",
     .method = HTTP_GET,
     .handler = http_deduce_wifi_handler},
    {.uri = "/configFile",
     .method = HTTP_GET,
     .handler = http_config_file_handler},
    {.uri = "/configWifi",
     .method = HTTP_GET,
     .handler = http_config_wifi_handler},
    {.uri = "/gamelist", .method = HTTP_GET, .handler = http_game_list_handler},
    {.uri = "/get_video",
     .method = HTTP_GET,
     .handler = http_get_video_handler},
    {.uri = "/get_version",
     .method = HTTP_GET,
     .handler = http_get_version_handler},
    {.uri = "/select_path",
     .method = HTTP_GET,
     .handler = http_select_path_handler},
    {.uri = "/FirstWeb", .method = HTTP_GET, .handler = http_first_web_handler},
    {.uri = "/listaudio",
     .method = HTTP_GET,
     .handler = http_list_audio_handler},
    {.uri = "/update", .method = HTTP_POST, .handler = http_ota_update_handler},
    {.uri = "/nameText", .method = HTTP_GET, .handler = http_name_Text_handler},
    {.uri = "/testFileIO",
     .method = HTTP_GET,
     .handler = http_testFileIO_handler},
    {.uri = "/wificonnect",
     .method = HTTP_GET,
     .handler = http_wifi_connect_handler},
    {.uri = "/download", .method = HTTP_GET, .handler = http_download_handler},
    {.uri = "/run_lua", .method = HTTP_POST, .handler = http_run_lua_handler},
    {.uri = "/createFile",
     .method = HTTP_GET,
     .handler = http_create_file_handler},
    {.uri = "/zip_list", .method = HTTP_GET, .handler = http_zip_list_handler},
    {.uri = "/extract",
     .method = HTTP_GET,
     .handler = http_zip_extract_handler},
    {.uri = "/compress",
     .method = HTTP_POST,
     .handler = http_zip_compress_handler},
    {.uri = "/deleteFileAll",
     .method = HTTP_GET,
     .handler = http_delete_all_handler},
    {.uri = "/*", .method = HTTP_GET, .handler = http_req_handler},
    {.uri = "/*", .method = HTTP_HEAD, .handler = http_req_handler},
};

const size_t allFile_route_count = sizeof(allFile_routes) / sizeof(httpd_uri_t);