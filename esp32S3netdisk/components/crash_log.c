#include "crash_log.h"
#include "esp_attr.h"
#include "esp_debug_helpers.h"
#include "esp_system.h"
#include "sd_card.h"
#include "soc/rtc.h"
#include "esp_private/panic_internal.h"
#include "xtensa_context.h"
#include <stdio.h>
#include <string.h>

#ifndef RTC_NOINIT_ATTR
#define RTC_NOINIT_ATTR __attribute__((section(".rtc.noinit")))
#endif

#define CRASH_LOG_MAGIC 0xC45A4C47U
#define BT_MAX 24

typedef struct {
  uint32_t magic;
  uint32_t pc;
  uint32_t bt_count;
  uint32_t bt[BT_MAX];
} crash_info_t;

/* RTC NOINIT: 上电清零，panic/wdt 等复位后保留 */
RTC_NOINIT_ATTR static crash_info_t s_crash_info;

static const char *reset_reason_str(esp_reset_reason_t r) {
  switch (r) {
  case ESP_RST_POWERON:
    return "上电复位";
  case ESP_RST_EXT:
    return "外部复位";
  case ESP_RST_SW:
    return "软件复位";
  case ESP_RST_PANIC:
    return "异常/崩溃(Panic)";
  case ESP_RST_INT_WDT:
    return "中断看门狗";
  case ESP_RST_TASK_WDT:
    return "任务看门狗";
  case ESP_RST_WDT:
    return "其他看门狗";
  case ESP_RST_DEEPSLEEP:
    return "深度睡眠";
  case ESP_RST_BROWNOUT:
    return "掉电复位";
  case ESP_RST_SDIO:
    return "SDIO复位";
  default:
    return "未知";
  }
}

/* 用 ld 链接时 --wrap=esp_panic_handler 包装 ESP-IDF 的 panic handler。
 *
 * 背景：esp_register_shutdown_handler 注册的 handler 只在 esp_restart() 软重启时被调用，
 * panic 时走的是 panic_restart() → esp_restart_noos()，根本不调 shutdown handler，
 * 所以 RTC_NOINIT_ATTR 永远不会被写入 → 重启后看到"未捕获调用栈"。
 *
 * 正确方案：用 ld --wrap=esp_panic_handler 在 panic 时被调用，写 backtrace 到 RTC 后
 * 再调用真实 panic handler 走原始流程。
 *
 * panic_handler.c (IRAM) 在调本函数前已 panic_enable_cache()，cache 已恢复，
 * 可安全执行 flash 代码和写 RTC。 */
extern void __real_esp_panic_handler(panic_info_t *info);

IRAM_ATTR void __wrap_esp_panic_handler(panic_info_t *info) {
  if (s_crash_info.magic != CRASH_LOG_MAGIC) {
    s_crash_info.magic = CRASH_LOG_MAGIC;

    /* 从 panic_info_t->frame 提取 backtrace 起点（跟 panic_arch.c 的
     * panic_print_backtrace 同样方式），不要用 esp_backtrace_get_start()，
     * 那个返回的是当前 __wrap_esp_panic_handler 栈帧而非崩溃点 */
    XtExcFrame *xt_frame = (XtExcFrame *)info->frame;
    esp_backtrace_frame_t frame = {
        .pc = xt_frame->pc,
        .sp = xt_frame->a1,
        .next_pc = xt_frame->a0,
        .exc_frame = xt_frame,
    };
    s_crash_info.pc = frame.pc;
    s_crash_info.bt[0] = frame.pc;
    uint32_t i = 1;
    while (i < BT_MAX && esp_backtrace_get_next_frame(&frame)) {
      s_crash_info.bt[i] = frame.pc;
      i++;
    }
    s_crash_info.bt_count = i;
  }
  __real_esp_panic_handler(info);
}

void crash_log_init(void) {
  /* panic 捕获由 ld --wrap=esp_panic_handler 注入，不需要再注册 shutdown handler
   * （shutdown handler 只在 esp_restart() 软重启时调用，panic 时根本不调用） */
}

void crash_log_check_save(void) {
  esp_reset_reason_t cur_reason = esp_reset_reason();
  bool has_bt = (s_crash_info.magic == CRASH_LOG_MAGIC);

  bool is_abnormal = (cur_reason == ESP_RST_PANIC ||
                      cur_reason == ESP_RST_INT_WDT ||
                      cur_reason == ESP_RST_TASK_WDT ||
                      cur_reason == ESP_RST_WDT ||
                      cur_reason == ESP_RST_BROWNOUT);

  if (!is_abnormal) {
    /* 正常重启（上电/软件复位/深度睡眠等），清除标记 */
    s_crash_info.magic = 0;
    return;
  }

  /* 异常重启 → 追加到 SD 卡崩溃日志 */
  const char *mount = sd_get_mount_point();
  char path[128];
  snprintf(path, sizeof(path), "%s/崩溃日志.txt", mount);

  FILE *f = fopen(path, "a");
  if (!f) {
    s_crash_info.magic = 0;
    return;
  }

  fprintf(f, "===== 崩溃记录 =====\n");
  fprintf(f, "重启原因: %s\n", reset_reason_str(cur_reason));

  if (has_bt) {
    fprintf(f, "PC: 0x%08X\n", (unsigned int)s_crash_info.pc);
    fprintf(f, "Backtrace:\n");
    for (uint32_t i = 0; i < s_crash_info.bt_count; i++) {
      fprintf(f, "  #%-2u 0x%08X\n", (unsigned int)i,
              (unsigned int)s_crash_info.bt[i]);
    }
  } else {
    /* WDT/Brownout 等硬件复位未经过 shutdown handler，无 backtrace */
    fprintf(f, "(硬件复位，未捕获调用栈)\n");
  }
  fprintf(f, "\n");

  fclose(f);

  /* 清除标记，避免下次重复记录 */
  s_crash_info.magic = 0;
}
