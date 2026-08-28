#ifndef SD_CARD_H
#define SD_CARD_H

#include "esp_err.h"
#include "sdmmc_cmd.h"
#include <stdint.h>

#define SD_MOUNT_POINT "/sdcard"
#define HOST_BUFFER_SIZE 128

// ====================== ESP32-S3-GEEK V1.1 SDMMC引脚定义 (按原理图)
// ======================
// #define SD_PIN_CLK GPIO_NUM_36
// #define SD_PIN_CMD GPIO_NUM_35
// #define SD_PIN_D0 GPIO_NUM_37
// #define SD_PIN_D1 GPIO_NUM_33
// #define SD_PIN_D2 GPIO_NUM_38
// #define SD_PIN_D3 GPIO_NUM_34
// #define SD_PIN_CD GPIO_NUM_NC
// #define SD_PIN_WP GPIO_NUM_NC
// =================================================================================

// ====================== ESP32-S3
#define SD_PIN_CLK GPIO_NUM_39
#define SD_PIN_CMD GPIO_NUM_40
#define SD_PIN_D0 GPIO_NUM_47
#define SD_PIN_D1 GPIO_NUM_21
#define SD_PIN_D2 GPIO_NUM_42
#define SD_PIN_D3 GPIO_NUM_41
#define SD_PIN_CD GPIO_NUM_NC
#define SD_PIN_WP GPIO_NUM_NC
// ========================================

extern int defaultfat;

// 配置参数结构体
typedef struct {
  int wifi_connect_time;       // 对应wifiConnectTime配置项
  int wifi_num;                // 对应wifiNum配置项
  char host[HOST_BUFFER_SIZE]; // 对应hostName配置项
  int first_web_is;            // 对应FirstWebis配置项
  uint64_t total_storage;      // 总存储容量（字节）
  uint64_t used_storage;       // 已用存储容量（字节）
} SdConfig;

/* SD卡初始化与基础功能 */
esp_err_t sd_init(void);                // 初始化SD卡并挂载文件系统
const char *sd_get_mount_point(void);   // 获取SD卡挂载点路径
sdmmc_card_t *sd_get_card_handle(void); // 获取SD卡设备句柄

int hex_digit_value(char c);
void url_decode(const char *url, char *decoded, size_t max_len);
void remove_query_parameters(char *url);
bool json_extract_string(const char *json, const char *key, char *value,
                         size_t max_len);
bool json_extract_int(const char *json, const char *key, int *value);

/* 文件操作函数 */
long sd_looksize_file(const char *path); // 查看文件大小
esp_err_t sd_write_file(const char *path,
                        const char *data); // 写入文件（覆盖模式）
esp_err_t sd_read_file(const char *path, char *buffer,
                       size_t buf_len);                       // 读取文件内容
esp_err_t sd_append_file(const char *path, const char *data); // 追加内容到文件
esp_err_t sd_rename(const char *old_path,
                    const char *new_path);  // 重命名文件/目录
esp_err_t sd_delete_file(const char *path); // 删除文件
esp_err_t sd_delete_dir(const char *path);  // 删除目录
esp_err_t sd_write_file_len(const char *path, const char *data, size_t len);
esp_err_t sd_append_file_len(const char *path, const char *data, size_t len);
esp_err_t sd_create_dir(const char *path); // 创建文件夹（支持多级目录）

/* 存储信息与配置管理 */
esp_err_t sd_update_storage_info(void); // 更新存储容量信息
SdConfig sd_get_config(void);           // 获取当前配置信息
void sd_read_wifi_list(const char *path);
void readConfig(const char *path);
void sd_load_ap_config(const char *path);
esp_err_t write_config_key_value(const char *path, const char *key,
                                 const char *value);
void sd_test_file_io(uint32_t *written_bytes, uint32_t *write_time,
                     uint32_t *read_bytes, uint32_t *read_time);
esp_err_t sd_delete_dir_recursive(const char *path);
// 在 sd_card.h 中添加以下声明

/**
 * @brief 关闭当前打开的文件句柄
 */
void sd_close_current_file(void);

/**
 * @brief 刷新当前文件缓冲区
 */
void sd_flush_current_file(void);

/**
 * @brief 开始批量写入（覆盖模式）
 * @param path 文件路径
 * @return 文件句柄，失败返回NULL
 */
FILE *sd_begin_batch_write(const char *path);

/**
 * @brief 开始批量追加
 * @param path 文件路径
 * @return 文件句柄，失败返回NULL
 */
FILE *sd_begin_batch_append(const char *path);

/**
 * @brief 批量写入数据
 * @param data 数据指针
 * @param len 数据长度
 * @return ESP_OK 成功，ESP_FAIL 失败
 */
esp_err_t sd_batch_write(const char *data, size_t len);

/**
 * @brief 批量写入字符串
 * @param str 字符串
 * @return ESP_OK 成功，ESP_FAIL 失败
 */
esp_err_t sd_batch_write_str(const char *str);

/**
 * @brief 结束批量操作（刷新缓冲区但不关闭文件）
 */
void sd_end_batch(void);

#endif // SD_CARD_H