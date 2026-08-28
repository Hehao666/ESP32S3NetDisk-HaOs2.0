/*
HTTP 服务器模块头文件
*/
#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "esp_err.h"
#include "esp_http_server.h"

#define POST_BUFFER_SIZE 512
#define MAX_PATH_LEN 256
#define MAX_CONTENT_LEN 512

// 👉 只写这一行，不写结构体内容
typedef struct
{
  httpd_req_t *req;
  FILE *fd;
  size_t start;
  size_t remaining;
} async_send_ctx_t;

typedef struct
{
  httpd_req_t *req;
  char filepath[512];
  int content_len;
} upload_ctx_t;
// 👉 只加这一个新结构体！通用压缩/解压！
typedef struct
{
  httpd_req_t *req;
  char path1[512]; // 源路径/zip路径
  char path2[512]; // 目标路径/输出目录
  int count;       // 文件数量（压缩用）
} zip_ctx_t;
typedef struct
{
  httpd_req_t *req;
  char path[512];  // 主路径
  char path2[512]; // 仅重命名用（第二个路径）
} file_op_ctx_t;
// 👉 函数前面加 static （必须和.c保持一致）
void async_send_task(void *pv);
void upload_task(void *pv);

// 新增WS广播函数，api.c可直接调用
void ws_broadcast(const char *msg);
/**
  @brief 启动 HTTP 服务器
*/
void http_start_server(void);

/**
  @brief 停止 HTTP 服务器
*/
void http_stop_server(void);

bool http_server_is_running(void);
void listFile(httpd_req_t *req, const char *path);
// 公共函数
esp_err_t http_serve_file(httpd_req_t *req, const char *filepath);
esp_err_t http_send_404(httpd_req_t *req);
esp_err_t http_send_file_in_chunks(httpd_req_t *req, const char *filepath, size_t chunk_size);
void scanGameAndListFile(httpd_req_t *req, const char *scan_path);
#endif // HTTP_SERVER_H