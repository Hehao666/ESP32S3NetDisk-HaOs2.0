/*
 * LED指示模块：IO37=STA指示灯，IO38=AP指示灯
 */
#include "LED.h"
#include "driver/gpio.h"

#define LED_STA_PIN GPIO_NUM_37
#define LED_AP_PIN GPIO_NUM_38

void led_init(void) {
  gpio_config_t io = {
      .pin_bit_mask = (1ULL << LED_STA_PIN) | (1ULL << LED_AP_PIN),
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&io);

  /* 开机两个LED都拉高 */
  gpio_set_level(LED_STA_PIN, 1);
  gpio_set_level(LED_AP_PIN, 1);
}

void led_set_sta(void) {
  /* STA模式：37高，38低 */
  gpio_set_level(LED_STA_PIN, 1);
  gpio_set_level(LED_AP_PIN, 0);
}

void led_set_ap(void) {
  /* AP模式：37低，38高 */
  gpio_set_level(LED_STA_PIN, 0);
  gpio_set_level(LED_AP_PIN, 1);
}
