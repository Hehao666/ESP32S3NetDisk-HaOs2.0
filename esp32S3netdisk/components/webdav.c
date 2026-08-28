/*
 * WebDAV 模块
 * 让 PC 可以直接挂载 SD 卡为网络驱动器
 */
#include "webdav.h"
#include "esp_http_server.h"
#include "sd_card.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

/* ── URL 解码（复制自 sd_card.c，避免链接依赖） ── */
static void wd_url_decode(const char *src, char *dst, size_t max_len) {
  size_t i = 0, j = 0;
  char a, b;
  while (src[i] && j < max_len - 1) {
    if (src[i] == '%' && src[i + 1] && src[i + 2]) {
      a = src[i + 1];
      b = src[i + 2];
      if (((a >= '0' && a <= '9') || (a >= 'A' && a <= 'F') ||
           (a >= 'a' && a <= 'f')) &&
          ((b >= '0' && b <= '9') || (b >= 'A' && b <= 'F') ||
           (b >= 'a' && b <= 'f'))) {
        int high = (a <= '9') ? (a - '0') : ((a <= 'F') ? (a - 'A' + 10)
                                                        : (a - 'a' + 10));
        int low = (b <= '9') ? (b - '0')
                             : ((b <= 'F') ? (b - 'A' + 10) : (b - 'a' + 10));
        dst[j++] = (char)(high * 16 + low);
        i += 3;
        continue;
      }
    }
    dst[j++] = src[i++];
  }
  dst[j] = '\0';
}

/* ── URI 转 SD 卡路径 ── */
static void uri_to_path(const char *uri, char *out, size_t size) {
  const char *mount = sd_get_mount_point();
  snprintf(out, size, "%s%s", mount, uri);
}

/* ── 时间转 HTTP 日期格式 ── */
static void format_http_time(time_t t, char *buf, size_t size) {
  struct tm *tm_info = gmtime(&t);
  strftime(buf, size, "%a, %d %b %Y %H:%M:%S GMT", tm_info);
}

/* ── 追加 XML 响应条目 ── */
static void xml_append_entry(char **buf, size_t *cap, size_t *len,
                             const char *href, const char *displayname,
                             int64_t size, time_t mtime, bool is_dir,
                             int64_t quota_avail, int64_t quota_used) {
  // 计算所需空间
  char time_str[64];
  format_http_time(mtime, time_str, sizeof(time_str));

  char entry[1280];
  int n;
  if (quota_avail >= 0 && quota_used >= 0) {
    n = snprintf(
        entry, sizeof(entry),
        "<D:response>"
        "<D:href>%s</D:href>"
        "<D:propstat>"
        "<D:prop>"
        "<D:displayname>%s</D:displayname>"
        "<D:getcontentlength>%lld</D:getcontentlength>"
        "<D:getlastmodified>%s</D:getlastmodified>"
        "<D:resourcetype>%s</D:resourcetype>"
        "<D:quota-available-bytes>%lld</D:quota-available-bytes>"
        "<D:quota-used-bytes>%lld</D:quota-used-bytes>"
        "</D:prop>"
        "<D:status>HTTP/1.1 200 OK</D:status>"
        "</D:propstat>"
        "</D:response>",
        href, displayname, (long long)size, time_str,
        is_dir ? "<D:collection/>" : "",
        (long long)quota_avail, (long long)quota_used);
  } else {
    n = snprintf(
        entry, sizeof(entry),
        "<D:response>"
        "<D:href>%s</D:href>"
        "<D:propstat>"
        "<D:prop>"
        "<D:displayname>%s</D:displayname>"
        "<D:getcontentlength>%lld</D:getcontentlength>"
        "<D:getlastmodified>%s</D:getlastmodified>"
        "<D:resourcetype>%s</D:resourcetype>"
        "</D:prop>"
        "<D:status>HTTP/1.1 200 OK</D:status>"
        "</D:propstat>"
        "</D:response>",
        href, displayname, (long long)size, time_str,
        is_dir ? "<D:collection/>" : "");
  }

  // 扩容
  while (*len + n + 1 > *cap) {
    *cap *= 2;
    char *new_buf = realloc(*buf, *cap);
    if (!new_buf) return;
    *buf = new_buf;
  }
  memcpy(*buf + *len, entry, n);
  *len += n;
}

/* ── PROPFIND ── */
static esp_err_t webdav_propfind(httpd_req_t *req) {
  char uri_decoded[512];
  wd_url_decode(req->uri, uri_decoded, sizeof(uri_decoded));

  // 去掉末尾 query string
  char *q = strchr(uri_decoded, '?');
  if (q)
    *q = '\0';

  // 读取 Depth header 中的值
  char depth[16] = "1";
  httpd_req_get_hdr_value_str(req, "Depth", depth, sizeof(depth));

  char fs_path[512];
  uri_to_path(uri_decoded, fs_path, sizeof(fs_path));

  struct stat st;
  if (stat(fs_path, &st) != 0) {
    httpd_resp_set_status(req, "404 Not Found");
    httpd_resp_send(req, NULL, 0);
    return ESP_FAIL;
  }

  // 分配 XML buffer (最小 4KB)
  size_t cap = 4096;
  char *xml = malloc(cap);
  if (!xml) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  size_t len = 0;
  const char *header =
      "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
      "<D:multistatus xmlns:D=\"DAV:\">\r\n";
  strcpy(xml, header);
  len = strlen(header);

  if (S_ISDIR(st.st_mode)) {
    // 目录本身 — 根目录带配额信息
    SdConfig cfg = sd_get_config();
    int64_t quota_avail = -1, quota_used = -1;
    if (strcmp(uri_decoded, "/") == 0) {
      quota_avail = (int64_t)(cfg.total_storage - cfg.used_storage);
      quota_used  = (int64_t)cfg.used_storage;
    }
    xml_append_entry(&xml, &cap, &len, uri_decoded,
                     strrchr(uri_decoded, '/') ? strrchr(uri_decoded, '/') + 1
                                               : uri_decoded,
                     0, st.st_mtime, true, quota_avail, quota_used);

    if (strcmp(depth, "0") != 0) {
      // 列出子项
      DIR *dir = opendir(fs_path);
      if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
          if (strcmp(entry->d_name, ".") == 0 ||
              strcmp(entry->d_name, "..") == 0)
            continue;

          char child_uri[768];
          char child_path[768];
          if (strcmp(uri_decoded, "/") == 0) {
            snprintf(child_uri, sizeof(child_uri), "/%s", entry->d_name);
          } else {
            snprintf(child_uri, sizeof(child_uri), "%s/%s", uri_decoded,
                     entry->d_name);
          }
          snprintf(child_path, sizeof(child_path), "%s/%s", fs_path,
                   entry->d_name);

          struct stat child_st;
          if (stat(child_path, &child_st) == 0) {
            xml_append_entry(&xml, &cap, &len, child_uri, entry->d_name,
                             S_ISDIR(child_st.st_mode) ? 0 : child_st.st_size,
                             child_st.st_mtime, S_ISDIR(child_st.st_mode),
                             -1, -1);
          }
        }
        closedir(dir);
      }
    }
  } else {
    // 单个文件
    xml_append_entry(&xml, &cap, &len, uri_decoded,
                     strrchr(uri_decoded, '/') ? strrchr(uri_decoded, '/') + 1
                                               : uri_decoded,
                     st.st_size, st.st_mtime, false, -1, -1);
  }

  // 结尾
  const char *footer = "</D:multistatus>\r\n";
  size_t footer_len = strlen(footer);
  while (len + footer_len + 1 > cap) {
    cap *= 2;
    char *new_xml = realloc(xml, cap);
    if (!new_xml) { free(xml); httpd_resp_send_500(req); return ESP_FAIL; }
    xml = new_xml;
  }
  memcpy(xml + len, footer, footer_len);
  len += footer_len;

  httpd_resp_set_status(req, "207 Multi-Status");
  httpd_resp_set_type(req, "application/xml; charset=utf-8");
  httpd_resp_set_hdr(req, "DAV", "1,2");
  httpd_resp_send(req, xml, len);
  free(xml);
  return ESP_OK;
}

/* ── MKCOL ── */
static esp_err_t webdav_mkcol(httpd_req_t *req) {
  char uri_decoded[512];
  wd_url_decode(req->uri, uri_decoded, sizeof(uri_decoded));
  char *q = strchr(uri_decoded, '?');
  if (q)
    *q = '\0';

  char fs_path[512];
  uri_to_path(uri_decoded, fs_path, sizeof(fs_path));

  int ret = mkdir(fs_path, 0777);
  if (ret == 0) {
    httpd_resp_set_status(req, "201 Created");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
  }

  // 可能已存在
  struct stat st;
  if (stat(fs_path, &st) == 0 && S_ISDIR(st.st_mode)) {
    httpd_resp_set_status(req, "405 Method Not Allowed");
    httpd_resp_send(req, NULL, 0);
    return ESP_FAIL;
  }

  httpd_resp_send_404(req);
  return ESP_FAIL;
}

/* ── DELETE ── */
static esp_err_t webdav_delete(httpd_req_t *req) {
  char uri_decoded[512];
  wd_url_decode(req->uri, uri_decoded, sizeof(uri_decoded));
  char *q = strchr(uri_decoded, '?');
  if (q)
    *q = '\0';

  char fs_path[512];
  uri_to_path(uri_decoded, fs_path, sizeof(fs_path));

  struct stat st;
  if (stat(fs_path, &st) != 0) {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  if (S_ISDIR(st.st_mode)) {
    int ret = rmdir(fs_path);
    if (ret == 0) {
      httpd_resp_set_status(req, "204 No Content");
      httpd_resp_send(req, NULL, 0);
      return ESP_OK;
    }
  } else {
    int ret = remove(fs_path);
    if (ret == 0) {
      httpd_resp_set_status(req, "204 No Content");
      httpd_resp_send(req, NULL, 0);
      return ESP_OK;
    }
  }

  httpd_resp_send_500(req);
  return ESP_FAIL;
}

/* ── MOVE ── */
static esp_err_t webdav_move(httpd_req_t *req) {
  char uri_decoded[512];
  wd_url_decode(req->uri, uri_decoded, sizeof(uri_decoded));
  char *q = strchr(uri_decoded, '?');
  if (q)
    *q = '\0';

  // 读取 Destination 头
  char dest_hdr[512] = {0};
  httpd_req_get_hdr_value_str(req, "Destination", dest_hdr, sizeof(dest_hdr));

  // Destination 是完整的 URL，提取 path 部分
  char *dest_path = strstr(dest_hdr, "//");
  if (dest_path) {
    dest_path = strchr(dest_path + 2, '/');
  }
  if (!dest_path)
    dest_path = dest_hdr;

  char dest_decoded[512];
  wd_url_decode(dest_path, dest_decoded, sizeof(dest_decoded));
  q = strchr(dest_decoded, '?');
  if (q)
    *q = '\0';

  char src_fs[512], dst_fs[512];
  uri_to_path(uri_decoded, src_fs, sizeof(src_fs));
  uri_to_path(dest_decoded, dst_fs, sizeof(dst_fs));

  if (rename(src_fs, dst_fs) == 0) {
    httpd_resp_set_status(req, "201 Created");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
  }

  httpd_resp_send_500(req);
  return ESP_FAIL;
}

/* ── COPY ── */
static esp_err_t webdav_copy(httpd_req_t *req) {
  char uri_decoded[512];
  wd_url_decode(req->uri, uri_decoded, sizeof(uri_decoded));
  char *q = strchr(uri_decoded, '?');
  if (q)
    *q = '\0';

  char dest_hdr[512] = {0};
  httpd_req_get_hdr_value_str(req, "Destination", dest_hdr, sizeof(dest_hdr));
  char *dest_path = strstr(dest_hdr, "//");
  if (dest_path) {
    dest_path = strchr(dest_path + 2, '/');
  }
  if (!dest_path)
    dest_path = dest_hdr;

  char dest_decoded[512];
  wd_url_decode(dest_path, dest_decoded, sizeof(dest_decoded));
  q = strchr(dest_decoded, '?');
  if (q)
    *q = '\0';

  char src_fs[512], dst_fs[512];
  uri_to_path(uri_decoded, src_fs, sizeof(src_fs));
  uri_to_path(dest_decoded, dst_fs, sizeof(dst_fs));

  // 简单文件拷贝
  struct stat st;
  if (stat(src_fs, &st) != 0 || S_ISDIR(st.st_mode)) {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  FILE *src = fopen(src_fs, "rb");
  if (!src) {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  FILE *dst = fopen(dst_fs, "wb");
  if (!dst) {
    fclose(src);
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  char buf[8192];
  size_t n;
  while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
    fwrite(buf, 1, n, dst);
  }
  fclose(src);
  fclose(dst);

  httpd_resp_set_status(req, "201 Created");
  httpd_resp_send(req, NULL, 0);
  return ESP_OK;
}

/* ── PUT ── */
static esp_err_t webdav_put(httpd_req_t *req) {
  char uri_decoded[512];
  wd_url_decode(req->uri, uri_decoded, sizeof(uri_decoded));
  char *q = strchr(uri_decoded, '?');
  if (q)
    *q = '\0';

  char fs_path[512];
  uri_to_path(uri_decoded, fs_path, sizeof(fs_path));

  // 确保父目录存在
  char *last_slash = strrchr(fs_path, '/');
  if (last_slash && last_slash != fs_path) {
    *last_slash = '\0';
    mkdir(fs_path, 0777);
    *last_slash = '/';
  }

  // 获取 Content-Length
  char len_str[32] = "0";
  httpd_req_get_hdr_value_str(req, "Content-Length", len_str, sizeof(len_str));
  int total = atoi(len_str);

  FILE *f = fopen(fs_path, "wb");
  if (!f) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  int remaining = total;
  while (remaining > 0) {
    char buf[4096];
    int recv = httpd_req_recv(req, buf, (remaining < sizeof(buf)) ? remaining
                                                                   : sizeof(buf));
    if (recv <= 0)
      break;
    fwrite(buf, 1, recv, f);
    remaining -= recv;
  }
  fclose(f);

  if (remaining == 0) {
    httpd_resp_set_status(req, "201 Created");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
  }

  httpd_resp_set_status(req, "204 No Content");
  httpd_resp_send(req, NULL, 0);
  return ESP_OK;
}

/* ── OPTIONS ── */
static esp_err_t webdav_options(httpd_req_t *req) {
  httpd_resp_set_hdr(req, "Allow",
                     "OPTIONS,GET,HEAD,PUT,DELETE,MKCOL,COPY,MOVE,PROPFIND");
  httpd_resp_set_hdr(req, "DAV", "1,2");
  httpd_resp_set_hdr(req, "MS-Author-Via", "DAV");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Methods",
                     "GET,PUT,DELETE,MKCOL,COPY,MOVE,PROPFIND,OPTIONS");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Headers",
                     "Content-Type,Depth,Destination,Overwrite");
  httpd_resp_send(req, NULL, 0);
  return ESP_OK;
}

/* ── 注册所有 WebDAV 路由 ── */
void webdav_register_routes(httpd_handle_t server) {
  const httpd_uri_t routes[] = {
      {.uri = "/*", .method = HTTP_PROPFIND, .handler = webdav_propfind},
      {.uri = "/*", .method = HTTP_MKCOL, .handler = webdav_mkcol},
      {.uri = "/*", .method = HTTP_DELETE, .handler = webdav_delete},
      {.uri = "/*", .method = HTTP_MOVE, .handler = webdav_move},
      {.uri = "/*", .method = HTTP_COPY, .handler = webdav_copy},
      {.uri = "/*", .method = HTTP_PUT, .handler = webdav_put},
      {.uri = "/*", .method = HTTP_OPTIONS, .handler = webdav_options},
  };

  for (size_t i = 0; i < sizeof(routes) / sizeof(httpd_uri_t); i++) {
    httpd_register_uri_handler(server, &routes[i]);
  }
}
