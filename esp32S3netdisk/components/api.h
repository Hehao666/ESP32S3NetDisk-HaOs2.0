#ifndef ALLFILE_H
#define ALLFILE_H

#include "esp_http_server.h"

extern const httpd_uri_t allFile_routes[];
extern const size_t allFile_route_count;

esp_err_t http_req_handler(httpd_req_t *req);
esp_err_t http_get_storage_handler(httpd_req_t *req);
esp_err_t http_get_FirstWebis_handler(httpd_req_t *req);
esp_err_t http_look_txt_handler(httpd_req_t *req);
esp_err_t http_edit_txt_handler(httpd_req_t *req);
esp_err_t http_get_file_list_handler(httpd_req_t *req);
esp_err_t http_look_this_handler(httpd_req_t *req);
esp_err_t http_back_one_handler(httpd_req_t *req);
esp_err_t http_back_Root_handler(httpd_req_t *req);
esp_err_t http_get_RootFile_handler(httpd_req_t *req);
esp_err_t http_get_file_list_handler(httpd_req_t *req);
esp_err_t http_delete_File_handler(httpd_req_t *req);
esp_err_t http_upload_File_handler(httpd_req_t *req);
esp_err_t http_get_wifi_handler(httpd_req_t *req);
esp_err_t http_serve_file_range(httpd_req_t *req, const char *filepath);
const char *get_mime_type(const char *filepath);

#endif