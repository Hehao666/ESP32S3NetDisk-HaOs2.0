/*
 * WiFi模块头文件
 */
#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

/*
 * WiFi连接成功回调函数类型
 */
typedef void (*wifi_connected_cb_t)(void);

/*
 * 初始化WiFi并连接到指定网络
 * 参数: connected_cb - 连接成功后的回调函数
 */
void wifi_init(const char *ssid, const char *password,
               wifi_connected_cb_t connected_cb);
bool wifi_is_connected(void);
void wifi_get_ip_and_ssid(char *ip_buf, int ip_len, char *ssid_buf,
                          int ssid_len);
void wifi_init_ap(void);
void wifi_switch_sta_ap(void);
void wifi_get_ipv6_address(char *buf, int len);
void mdns_start_service(void);

#endif // WIFI_MODULE_H
