/*
 * LED指示模块头文件
 */
#ifndef LED_H
#define LED_H

/* 开机初始化：IO37、IO38都拉高 */
void led_init(void);

/* STA模式：IO37拉高，IO38拉低 */
void led_set_sta(void);

/* AP模式：IO37拉低，IO38拉高 */
void led_set_ap(void);

#endif // LED_H
