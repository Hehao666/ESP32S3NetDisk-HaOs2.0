#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_ota_ops.h"

// 引入各功能模块
#include "LED.h"
#include "crash_log.h"
#include "http_server.h"
#include "myfat.h"
#include "sd_card.h"
#include "wifi_module.h"

extern char wifiSsid[10][64];
extern char wifiPassword[10][64];
extern int wifiConnectTime;
extern int wifiNum;
extern int autoWifinum;
extern char hostName[64];

void app_main(void) {
  /* 开机把IO37、IO38都拉高 */
  led_init();

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }

  crash_log_init();

  esp_err_t sd_ret = sd_init();

  int fat_ret = fat_mount();

  if (sd_ret == ESP_OK || fat_ret == 0) {
    crash_log_check_save();
    readConfig("/配置/配置.txt");

    sd_load_ap_config("/配置/热点.txt");

    sd_read_wifi_list("/配置/无线网络.txt");

    bool connected = false;
    for (int i = 0; i < autoWifinum; i++) {
      wifi_init(wifiSsid[i], wifiPassword[i], NULL);
      if (wifi_is_connected()) {
        connected = true;
        break;
      }
    }
    if (!connected) {
      wifi_init_ap();
    }
    http_start_server();
    mdns_start_service();
    /* OTA后新固件启动，确认保留此版本，防止重启回滚 */
    esp_ota_mark_app_valid_cancel_rollback();
  }
}
