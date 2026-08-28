/*
 * WiFi模块实现文件
 */
#include "wifi_module.h"
#include "LED.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "mdns.h"
#include "sdkconfig.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static EventGroupHandle_t s_wifi_event_group = NULL;
static int s_current_elapsed = 0;
static int s_wait_time = 5;
static wifi_connected_cb_t s_connected_callback = NULL;

static bool s_mdns_active = false;
/*
 * true  = 首次 STA 连接流程中，用于 OLED 倒计时和超时判断
 * false = 非首次连接流程，或者已经拿到 IP
 */
static volatile bool s_sta_connecting = false;

/*
 * true  = STA 模式下允许断线自动重连
 * false = 手动切 AP 或停止 STA，不允许自动重连
 */
static volatile bool s_sta_should_reconnect = false;

/*
 * WiFi 栈初始化状态
 */
static bool s_netif_inited = false;
static bool s_event_loop_inited = false;
static bool s_wifi_driver_inited = false;
static bool s_sta_netif_created = false;
static bool s_ap_netif_created = false;
static bool s_timer_started = false;
static bool s_event_handler_registered = false;

extern int wifiConnectTime;

extern char hotspotName[31];
extern char hotspotPassword[63];
extern char hotspotChannel[5];
extern int autoWifinum;
extern char wifiSsid[10][64];
extern char wifiPassword[10][64];

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

void mdns_start_service(void) {
  if (s_mdns_active)
    return;
  esp_err_t ret = mdns_init();
  if (ret != ESP_OK) {
    printf("mdns init fail: %s\n", esp_err_to_name(ret));
    return;
  }
  mdns_hostname_set(hotspotName);
  mdns_instance_name_set(hotspotName);
  mdns_service_add(hotspotName, "_http", "_tcp", 80, NULL, 0);
  s_mdns_active = true;
}

void mdns_stop_service(void) {
  if (!s_mdns_active)
    return;
  mdns_free();
  s_mdns_active = false;
}

static void wifi_common_init(void) {
  if (!s_wifi_event_group) {
    s_wifi_event_group = xEventGroupCreate();
  }

  if (!s_netif_inited) {
    esp_netif_init();
    s_netif_inited = true;
  }

  if (!s_event_loop_inited) {
    esp_err_t ret = esp_event_loop_create_default();
    if (ret == ESP_OK || ret == ESP_ERR_INVALID_STATE) {
      s_event_loop_inited = true;
    }
  }

  if (!s_wifi_driver_inited) {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    s_wifi_driver_inited = true;
  }
}

/* WiFi事件处理函数 */
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    /*
     * 第一次 STA 启动后连接。
     * 这里保留，防止首次 esp_wifi_start() 后没有真正连接。
     */
    if (s_sta_should_reconnect) {
      esp_wifi_connect();
    }
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
    /* 【必须添加】物理连上WiFi后，开启IPv6本地链路地址协商，否则底层不分配v6 */
    esp_netif_t *sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (sta_netif) {
      esp_netif_create_ip6_linklocal(sta_netif);
    }
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (s_wifi_event_group) {
      xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }

    /*
     * STA 模式下允许自动重连。
     * 切 AP 或超时时会把 s_sta_should_reconnect 设为 false。
     */
    if (s_sta_should_reconnect) {
      esp_wifi_connect();
    }
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {

    /*
     * 拿到 IP 后，只结束首次连接计时。
     * 不关闭 s_sta_should_reconnect，否则后续断线就不会重连。
     */
    s_sta_connecting = false;
    s_sta_should_reconnect = true;
    s_current_elapsed = 0;

    if (s_wifi_event_group) {
      xEventGroupClearBits(s_wifi_event_group, WIFI_FAIL_BIT);
      xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }


    if (s_connected_callback) {
      s_connected_callback();
    }
  }
}

// 状态查询函数
bool wifi_is_connected(void) {
  if (!s_wifi_event_group)
    return false;

  EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);
  return (bits & WIFI_CONNECTED_BIT) != 0;
}

// 计时任务，每秒更新显示
static void wifi_timer_task(void *pvParameters) {
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));

    if (!s_wifi_event_group)
      continue;

    /*
     * 只在首次 STA 连接时显示倒计时。
     * 已经拿到 IP 后，s_sta_connecting=false，不再覆盖 OLED。
     */
    if (!s_sta_connecting)
      continue;

    EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);

    if (!(bits & WIFI_CONNECTED_BIT) && !(bits & WIFI_FAIL_BIT)) {
      s_current_elapsed++;


      if (s_current_elapsed >= s_wait_time) {
        s_sta_connecting = false;
        s_sta_should_reconnect = false;

        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);

        esp_wifi_disconnect();
      }
    }
  }
}

void wifi_init(const char *ssid, const char *password,
               wifi_connected_cb_t connected_cb) {
  if (!ssid || !password)
    return;

  s_connected_callback = connected_cb;

  s_wait_time = (wifiConnectTime > 0) ? wifiConnectTime : 5;

  wifi_common_init();

  if (!s_sta_netif_created) {
    esp_netif_create_default_wifi_sta();
    s_sta_netif_created = true;
  }

  if (!s_event_handler_registered) {
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                        &event_handler, NULL, NULL);

    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                        &event_handler, NULL, NULL);

    /* 【必须注册】添加对 IPv6 地址获取事件的监听，否则拿到 v6 时不进
     * event_handler */
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_GOT_IP6,
                                        &event_handler, NULL, NULL);

    s_event_handler_registered = true;
  }

  if (!s_timer_started) {
    xTaskCreate(wifi_timer_task, "wifi_timer", 2048, NULL, 5, NULL);
    s_timer_started = true;
  }

  wifi_config_t wifi_config = {0};

  strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);

  strncpy((char *)wifi_config.sta.password, password,
          sizeof(wifi_config.sta.password) - 1);

  xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

  s_current_elapsed = 0;
  s_sta_connecting = true;
  s_sta_should_reconnect = true;



  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  led_set_sta();

  /*
   * 这里必须保留 esp_wifi_connect()。
   * 因为你的 app_main 会循环调用 wifi_init() 尝试多个 WiFi。
   * 后续尝试时 WiFi 可能已经 start，不一定再次触发 STA_START。
   */
  esp_err_t ret = esp_wifi_start();
  (void)ret;

  esp_wifi_set_ps(WIFI_PS_NONE); // 关闭省电

  ret = esp_wifi_connect();
  (void)ret;

  EventBits_t bits = xEventGroupWaitBits(
      s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE,
      pdMS_TO_TICKS((s_wait_time + 1) * 1000));

  if (bits & WIFI_CONNECTED_BIT) {
    s_sta_connecting = false;
    s_sta_should_reconnect = true;
    return;
  }

  s_sta_connecting = false;

  if ((bits & WIFI_FAIL_BIT) == 0) {
    xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);

    s_sta_should_reconnect = false;
    esp_wifi_disconnect();
  }
}

// 一次性获取：IP地址 + WiFi名称
void wifi_get_ip_and_ssid(char *ip_buf, int ip_len, char *ssid_buf,
                          int ssid_len) {
  if (!ip_buf || !ssid_buf || ip_len <= 0 || ssid_len <= 0)
    return;

  ip_buf[0] = '\0';
  ssid_buf[0] = '\0';

  wifi_mode_t mode;
  esp_wifi_get_mode(&mode);

  esp_netif_ip_info_t ip_info = {0};

  if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA) {
    esp_netif_t *sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (sta_netif) {
      esp_netif_get_ip_info(sta_netif, &ip_info);
    }
  } else {
    esp_netif_t *ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (ap_netif) {
      esp_netif_get_ip_info(ap_netif, &ip_info);
    }
  }

  esp_ip4addr_ntoa(&ip_info.ip, ip_buf, ip_len);

  if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA) {
    wifi_config_t wifi_config = {0};

    if (esp_wifi_get_config(WIFI_IF_STA, &wifi_config) == ESP_OK) {
      strncpy(ssid_buf, (char *)wifi_config.sta.ssid, ssid_len - 1);
      ssid_buf[ssid_len - 1] = '\0';
    }
  } else {
    wifi_config_t ap_config = {0};

    if (esp_wifi_get_config(WIFI_IF_AP, &ap_config) == ESP_OK) {
      strncpy(ssid_buf, (char *)ap_config.ap.ssid, ssid_len - 1);
      ssid_buf[ssid_len - 1] = '\0';
    }
  }
}

void wifi_init_ap(void) {
  wifi_common_init();

  /*
   * 进入 AP 模式前，关闭 STA 自动重连。
   */
  s_sta_connecting = false;
  s_sta_should_reconnect = false;
  s_current_elapsed = 0;

  if (s_wifi_event_group) {
    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
  }

  esp_wifi_disconnect();

  if (!s_ap_netif_created) {
    esp_netif_create_default_wifi_ap();
    s_ap_netif_created = true;
  }

  int channel = atoi(hotspotChannel);
  if (channel < 1 || channel > 13) {
    channel = 1;
  }

  wifi_config_t ap_config = {0};

  strncpy((char *)ap_config.ap.ssid, hotspotName,
          sizeof(ap_config.ap.ssid) - 1);

  strncpy((char *)ap_config.ap.password, hotspotPassword,
          sizeof(ap_config.ap.password) - 1);

  ap_config.ap.ssid_len = strlen(hotspotName);
  ap_config.ap.channel = channel;
  ap_config.ap.max_connection = 4;
  ap_config.ap.beacon_interval = 100;

  if (strlen(hotspotPassword) == 0) {
    ap_config.ap.authmode = WIFI_AUTH_OPEN;
  } else {
    ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
  }

  esp_wifi_set_mode(WIFI_MODE_AP);
  esp_wifi_set_config(WIFI_IF_AP, &ap_config);
  led_set_ap();

  esp_err_t ret = esp_wifi_start();
  (void)ret;

}

void wifi_switch_sta_ap(void) {
  wifi_common_init();

  if (!s_timer_started) {
    xTaskCreate(wifi_timer_task, "wifi_timer", 2048, NULL, 5, NULL);
    s_timer_started = true;
  }

  s_wait_time = (wifiConnectTime > 0) ? wifiConnectTime : 5;

  wifi_mode_t current_mode;
  esp_wifi_get_mode(&current_mode);

  /*
   * 当前是 AP：尝试切 STA 连接保存的 WiFi。
   */
  if (current_mode == WIFI_MODE_AP) {
    bool connected = false;

    if (!s_sta_netif_created) {
      esp_netif_create_default_wifi_sta();
      s_sta_netif_created = true;
    }

    if (!s_event_handler_registered) {
      esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                          &event_handler, NULL, NULL);

      esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                          &event_handler, NULL, NULL);

      /* 【必须注册】切换STA逻辑中，也补充对 IPv6 地址获取事件的监听 */
      esp_event_handler_instance_register(IP_EVENT, IP_EVENT_GOT_IP6,
                                          &event_handler, NULL, NULL);

      s_event_handler_registered = true;
    }

    for (int i = 0; i < autoWifinum && i < 10; i++) {
      if (strlen(wifiSsid[i]) == 0) {
        continue;
      }


      wifi_config_t wifi_config = {0};

      strlcpy((char *)wifi_config.sta.ssid, wifiSsid[i],
              sizeof(wifi_config.sta.ssid));

      strlcpy((char *)wifi_config.sta.password, wifiPassword[i],
              sizeof(wifi_config.sta.password));

      xEventGroupClearBits(s_wifi_event_group,
                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

      s_current_elapsed = 0;
      s_sta_connecting = true;
      s_sta_should_reconnect = true;

      esp_wifi_set_mode(WIFI_MODE_STA);
      esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
      led_set_sta();

      /*
       * 这里也保留 esp_wifi_connect()。
       */
      esp_err_t ret = esp_wifi_start();
      (void)ret;

      ret = esp_wifi_connect();
      (void)ret;


      EventBits_t bits = xEventGroupWaitBits(
          s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE,
          pdFALSE, pdMS_TO_TICKS((s_wait_time + 1) * 1000));

      if (bits & WIFI_CONNECTED_BIT) {
        s_sta_connecting = false;
        s_sta_should_reconnect = true;
        connected = true;
        break;
      }

      s_sta_connecting = false;
      s_sta_should_reconnect = false;

      xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
      esp_wifi_disconnect();
    }

    if (!connected) {
      wifi_init_ap();
    }
  }
  /*
   * 当前是 STA 或其他模式：直接切 AP。
   */
  else {
    wifi_init_ap();
  }
}
void wifi_get_ipv6_address(char *buf, int len) {
  if (!buf || len <= 0)
    return;
  buf[0] = '\0';

  // 自动适配 STA / AP
  esp_netif_t *netif = NULL;
  wifi_mode_t mode;
  esp_wifi_get_mode(&mode);

  if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA) {
    netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  } else {
    netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
  }

  if (!netif)
    return;

  // 你原来的写法，完全不变
  esp_ip6_addr_t if_ip6[10];
  int count = esp_netif_get_all_ip6(netif, if_ip6);

  // 把所有 IPv6 拼起来（有多少拼多少）
  for (int i = 0; i < count; i++) {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), IPV6STR, IPV62STR(if_ip6[i]));

    strncat(buf, tmp, len - strlen(buf) - 1);

    // 最后一个不加逗号
    if (i < count - 1) {
      strncat(buf, ", ", len - strlen(buf) - 1);
    }
  }
}
