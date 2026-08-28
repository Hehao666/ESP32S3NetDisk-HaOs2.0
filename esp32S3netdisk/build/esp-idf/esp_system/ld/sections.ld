/* Automatically generated file; DO NOT EDIT */
/* Espressif IoT Development Framework Linker Script */
/* Generated from: D:\ESP32_project\ESP32S3\esp32S3netdisk_3_1.7\esp32S3netdisk\build\esp-idf\esp_system\ld\sections.ld.in */

/*

 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD

 *

 * SPDX-License-Identifier: Apache-2.0

 */
/*

 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD

 *

 * SPDX-License-Identifier: Apache-2.0

 */
/*

 * Automatically generated file. DO NOT EDIT.

 * Espressif IoT Development Framework (ESP-IDF) 6.0.0 Configuration Header

 */
       
/* List of deprecated options */
/* CPU instruction prefetch padding size for flash mmap scenario */
/* Copy from esp_secure_boot.h */
/*

 * PMP region granularity size

 * Software may determine the PMP granularity by writing zero to pmp0cfg, then writing all ones

 * to pmpaddr0, then reading back pmpaddr0. If G is the index of the least-significant bit set,

 * the PMP granularity is 2^G+2 bytes.

 */
/* CPU instruction prefetch padding size for memory protection scenario */
/* Memory alignment size for PMS */
    /* rtc timer data (s_rtc_timer_retain_mem, see esp_clk.c files). For rtc_timer_data_in_rtc_mem section. */
/* Default entry point */
ENTRY(call_start_cpu0);
SECTIONS
{
  /**

   * RTC fast memory holds RTC wake stub code,

   * including from any source file named rtc_wake_stub*.c

   */
  .rtc.text :
  {
    
 . = ALIGN(4); 
 _rtc_fast_start = ABSOLUTE(.);
    
 . = ALIGN(4); 
 _rtc_text_start = ABSOLUTE(.);
    HIDDEN(_rtc_code_start = .);
    *(.rtc.entry.literal .rtc.entry.text)
    *(.rtc.literal .rtc.text .rtc.text.*)
    *rtc_wake_stub*.*(.literal .text .literal.* .text.*)
    *(.rtc_text_end_test)
    HIDDEN(_rtc_code_end = .);
    /* Padding for possible CPU prefetch + 4B alignment for PMS split lines. */
    . = ((_rtc_code_end - _rtc_code_start) == 0) ?
        ALIGN(0) : 16 + ALIGN(4);
    _rtc_text_end = ABSOLUTE(.);
  } > rtc_iram_seg
  /**

   * This section located in RTC FAST Memory area.

   * It holds data marked with RTC_FAST_ATTR attribute.

   * See the file "esp_attr.h" for more information.

   */
  .rtc.force_fast :
  {
    
 . = ALIGN(4); 
 _rtc_force_fast_start = ABSOLUTE(.);
    *(.rtc.force_fast .rtc.force_fast.*)
    
 . = ALIGN(4); 
 _rtc_force_fast_end = ABSOLUTE(.);
  } > rtc_data_seg
  /**

   * RTC data section holds RTC wake stub

   * data/rodata, including from any source file

   * named rtc_wake_stub*.c and the data marked with

   * RTC_DATA_ATTR, RTC_RODATA_ATTR attributes.

   * The memory location of the data is dependent on

   * CONFIG_ESP32S3_RTCDATA_IN_FAST_MEM option.

   */
  .rtc.data :
  {
    _rtc_data_start = ABSOLUTE(.);
    *(.rtc.data .rtc.data.*)
    *(.rtc.rodata .rtc.rodata.*)
    *rtc_wake_stub*.*(.data .rodata .data.* .rodata.*)
    _rtc_data_end = ABSOLUTE(.);
  } > rtc_data_location
  /* RTC bss, from any source file named rtc_wake_stub*.c */
  .rtc.bss (NOLOAD) :
  {
    _rtc_bss_start = ABSOLUTE(.);
    *rtc_wake_stub*.*(.bss .bss.*)
    *rtc_wake_stub*.*(COMMON)
    *(.rtc.bss)
    _rtc_bss_end = ABSOLUTE(.);
  } > rtc_data_location
  /**

   * This section holds data that should not be initialized at power up

   * and will be retained during deep sleep.

   * User data marked with RTC_NOINIT_ATTR will be placed

   * into this section. See the file "esp_attr.h" for more information.

   * The memory location of the data is dependent on

   * CONFIG_ESP32S3_RTCDATA_IN_FAST_MEM option.

   */
  .rtc_noinit (NOLOAD):
  {
    
 . = ALIGN(4); 
 _rtc_noinit_start = ABSOLUTE(.);
    *(.rtc_noinit .rtc_noinit.*)
    
 . = ALIGN(4); 
 _rtc_noinit_end = ABSOLUTE(.);
  } > rtc_data_location
  /**

   * This section located in RTC SLOW Memory area.

   * It holds data marked with RTC_SLOW_ATTR attribute.

   * See the file "esp_attr.h" for more information.

   */
  .rtc.force_slow :
  {
    
 . = ALIGN(4); 
 _rtc_force_slow_start = ABSOLUTE(.);
    *(.rtc.force_slow .rtc.force_slow.*)
    
 . = ALIGN(4); 
 _rtc_force_slow_end = ABSOLUTE(.);
  } > rtc_slow_seg
  /**

   * This section holds RTC data that should have fixed addresses.

   * The data are not initialized at power-up and are retained during deep

   * sleep.

   */
  .rtc_reserved (NOLOAD):
  {
    
 . = ALIGN(4); 
 _rtc_reserved_start = ABSOLUTE(.);
    /**

     * New data can only be added here to ensure existing data are not moved.

     * Because data have adhered to the end of the segment and code is relied

     * on it.

     * >> put new data here <<

     */
    *(.rtc_timer_data_in_rtc_mem .rtc_timer_data_in_rtc_mem.*)
    KEEP(*(.bootloader_data_rtc_mem .bootloader_data_rtc_mem.*))
    _rtc_reserved_end = ABSOLUTE(.);
  } > rtc_reserved_seg
  _rtc_reserved_length = _rtc_reserved_end - _rtc_reserved_start;
  ASSERT((_rtc_reserved_length <= LENGTH(rtc_reserved_seg)),
          "RTC reserved segment data does not fit.")
  /* Get size of rtc slow data based on rtc_data_location alias */
  _rtc_slow_length = (ORIGIN(rtc_slow_seg) == ORIGIN(rtc_data_location))
                        ? (_rtc_force_slow_end - _rtc_data_start)
                        : (_rtc_force_slow_end - _rtc_force_slow_start);
  _rtc_fast_length = (ORIGIN(rtc_slow_seg) == ORIGIN(rtc_data_location))
                        ? (_rtc_force_fast_end - _rtc_fast_start)
                        : (_rtc_noinit_end - _rtc_fast_start);
  ASSERT((_rtc_slow_length <= LENGTH(rtc_slow_seg)),
          "RTC_SLOW segment data does not fit.")
  ASSERT((_rtc_fast_length <= LENGTH(rtc_data_seg)),
          "RTC_FAST segment data does not fit.")
  /* Send .iram0 code to iram */
  .iram0.vectors :
  {
    _iram_start = ABSOLUTE(.);
    /* Vectors go to IRAM */
    _vector_table = ABSOLUTE(.);
    . = 0x0;
    KEEP(*(.WindowVectors.text));
    . = 0x180;
    KEEP(*(.Level2InterruptVector.text));
    . = 0x1c0;
    KEEP(*(.Level3InterruptVector.text));
    . = 0x200;
    KEEP(*(.Level4InterruptVector.text));
    . = 0x240;
    KEEP(*(.Level5InterruptVector.text));
    . = 0x280;
    KEEP(*(.DebugExceptionVector.text));
    . = 0x2c0;
    KEEP(*(.NMIExceptionVector.text));
    . = 0x300;
    KEEP(*(.KernelExceptionVector.text));
    . = 0x340;
    KEEP(*(.UserExceptionVector.text));
    . = 0x3C0;
    KEEP(*(.DoubleExceptionVector.text));
    . = 0x400;
    KEEP(*(._invalid_pc_placeholder.text));
    *(.*Vector.literal)
  } > iram0_0_seg
  .iram0.text :
  {
    /* Code marked as running out of IRAM */
    _iram_text_start = ABSOLUTE(.);
    *(.iram1 .iram1.*)
    *libclang_rt.builtins.a:_divsf3.*(.literal .literal.* .text .text.*)
    *libesp_driver_dma.a:gdma.*(.literal.gdma_append .text.gdma_append)
    *libesp_driver_dma.a:gdma.*(.literal.gdma_default_rx_isr .text.gdma_default_rx_isr)
    *libesp_driver_dma.a:gdma.*(.literal.gdma_default_tx_isr .text.gdma_default_tx_isr)
    *libesp_driver_dma.a:gdma.*(.literal.gdma_reset .text.gdma_reset)
    *libesp_driver_dma.a:gdma.*(.literal.gdma_start .text.gdma_start)
    *libesp_driver_dma.a:gdma.*(.literal.gdma_stop .text.gdma_stop)
    *libesp_event.a:default_event_loop.*(.literal.esp_event_isr_post .text.esp_event_isr_post)
    *libesp_event.a:esp_event.*(.literal.esp_event_isr_post_to .text.esp_event_isr_post_to)
    *libesp_hal_dma.a:gdma_hal_ahb_v1.*(.literal.gdma_ahb_hal_append .text.gdma_ahb_hal_append)
    *libesp_hal_dma.a:gdma_hal_ahb_v1.*(.literal.gdma_ahb_hal_clear_intr .text.gdma_ahb_hal_clear_intr)
    *libesp_hal_dma.a:gdma_hal_ahb_v1.*(.literal.gdma_ahb_hal_get_eof_desc_addr .text.gdma_ahb_hal_get_eof_desc_addr)
    *libesp_hal_dma.a:gdma_hal_ahb_v1.*(.literal.gdma_ahb_hal_read_intr_status .text.gdma_ahb_hal_read_intr_status)
    *libesp_hal_dma.a:gdma_hal_ahb_v1.*(.literal.gdma_ahb_hal_reset .text.gdma_ahb_hal_reset)
    *libesp_hal_dma.a:gdma_hal_ahb_v1.*(.literal.gdma_ahb_hal_start_with_desc .text.gdma_ahb_hal_start_with_desc)
    *libesp_hal_dma.a:gdma_hal_ahb_v1.*(.literal.gdma_ahb_hal_stop .text.gdma_ahb_hal_stop)
    *libesp_hal_dma.a:gdma_hal_top.*(.literal.gdma_hal_append .text.gdma_hal_append)
    *libesp_hal_dma.a:gdma_hal_top.*(.literal.gdma_hal_clear_intr .text.gdma_hal_clear_intr)
    *libesp_hal_dma.a:gdma_hal_top.*(.literal.gdma_hal_get_eof_desc_addr .text.gdma_hal_get_eof_desc_addr)
    *libesp_hal_dma.a:gdma_hal_top.*(.literal.gdma_hal_read_intr_status .text.gdma_hal_read_intr_status)
    *libesp_hal_dma.a:gdma_hal_top.*(.literal.gdma_hal_reset .text.gdma_hal_reset)
    *libesp_hal_dma.a:gdma_hal_top.*(.literal.gdma_hal_start_with_desc .text.gdma_hal_start_with_desc)
    *libesp_hal_dma.a:gdma_hal_top.*(.literal.gdma_hal_stop .text.gdma_hal_stop)
    *libesp_hal_gpio.a:gpio_hal.*(.literal.gpio_hal_isolate_in_sleep .text.gpio_hal_isolate_in_sleep)
    *libesp_hal_gpspi.a:spi_hal_iram.*(.literal .literal.* .text .text.*)
    *libesp_hal_gpspi.a:spi_slave_hal_iram.*(.literal .literal.* .text .text.*)
    *libesp_hal_mspi.a:spi_flash_encrypt_hal_iram.*(.literal .literal.* .text .text.*)
    *libesp_hal_mspi.a:spi_flash_hal_gpspi.*(.literal .literal.* .text .text.*)
    *libesp_hal_mspi.a:spi_flash_hal_iram.*(.literal .literal.* .text .text.*)
    *libesp_hal_wdt.a:rom_patch.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:clk_utils.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:cpu.*(.literal.esp_cpu_compare_and_set .text.esp_cpu_compare_and_set)
    *libesp_hw_support.a:cpu.*(.literal.esp_cpu_reset .text.esp_cpu_reset)
    *libesp_hw_support.a:cpu.*(.literal.esp_cpu_stall .text.esp_cpu_stall)
    *libesp_hw_support.a:cpu.*(.literal.esp_cpu_unstall .text.esp_cpu_unstall)
    *libesp_hw_support.a:cpu.*(.literal.esp_cpu_wait_for_intr .text.esp_cpu_wait_for_intr)
    *libesp_hw_support.a:esp_clk_tree.*(.literal.esp_clk_tree_enable_power .text.esp_clk_tree_enable_power)
    *libesp_hw_support.a:esp_clk_tree.*(.literal.esp_clk_tree_enable_src .text.esp_clk_tree_enable_src)
    *libesp_hw_support.a:esp_clk_tree.*(.literal.esp_clk_tree_is_power_on .text.esp_clk_tree_is_power_on)
    *libesp_hw_support.a:esp_memory_utils.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:mspi_timing_by_mspi_delay.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:mspi_timing_config.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:mspi_timing_tuning.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:periph_ctrl.*(.literal.periph_module_reset .text.periph_module_reset)
    *libesp_hw_support.a:periph_ctrl.*(.literal.periph_rcc_acquire_enter .text.periph_rcc_acquire_enter)
    *libesp_hw_support.a:periph_ctrl.*(.literal.periph_rcc_acquire_exit .text.periph_rcc_acquire_exit)
    *libesp_hw_support.a:periph_ctrl.*(.literal.periph_rcc_enter .text.periph_rcc_enter)
    *libesp_hw_support.a:periph_ctrl.*(.literal.periph_rcc_exit .text.periph_rcc_exit)
    *libesp_hw_support.a:periph_ctrl.*(.literal.periph_rcc_release_enter .text.periph_rcc_release_enter)
    *libesp_hw_support.a:periph_ctrl.*(.literal.periph_rcc_release_exit .text.periph_rcc_release_exit)
    *libesp_hw_support.a:periph_ctrl.*(.literal.wifi_module_disable .text.wifi_module_disable)
    *libesp_hw_support.a:periph_ctrl.*(.literal.wifi_module_enable .text.wifi_module_enable)
    *libesp_hw_support.a:regi2c_ctrl.*(.literal.regi2c_ctrl_read_reg .text.regi2c_ctrl_read_reg)
    *libesp_hw_support.a:regi2c_ctrl.*(.literal.regi2c_ctrl_read_reg_mask .text.regi2c_ctrl_read_reg_mask)
    *libesp_hw_support.a:regi2c_ctrl.*(.literal.regi2c_ctrl_write_reg .text.regi2c_ctrl_write_reg)
    *libesp_hw_support.a:regi2c_ctrl.*(.literal.regi2c_ctrl_write_reg_mask .text.regi2c_ctrl_write_reg_mask)
    *libesp_hw_support.a:regi2c_ctrl.*(.literal.regi2c_enter_critical .text.regi2c_enter_critical)
    *libesp_hw_support.a:regi2c_ctrl.*(.literal.regi2c_exit_critical .text.regi2c_exit_critical)
    *libesp_hw_support.a:rtc_clk.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:rtc_init.*(.literal.rtc_vddsdio_get_config .text.rtc_vddsdio_get_config)
    *libesp_hw_support.a:rtc_init.*(.literal.rtc_vddsdio_set_config .text.rtc_vddsdio_set_config)
    *libesp_hw_support.a:rtc_sleep.*(.literal.rtc_sleep_pu .text.rtc_sleep_pu)
    *libesp_hw_support.a:rtc_sleep.*(.literal.rtc_sleep_start .text.rtc_sleep_start)
    *libesp_hw_support.a:rtc_time.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:systimer.*(.literal .literal.* .text .text.*)
    *libesp_libc.a:abort.*(.literal .literal.* .text .text.*)
    *libesp_libc.a:assert.*(.literal .literal.* .text .text.*)
    *libesp_libc.a:heap.*(.literal .literal.* .text .text.*)
    *libesp_libc.a:stdatomic.*(.literal .literal.* .text .text.*)
    *libesp_mm.a:esp_cache_msync.*(.literal .literal.* .text .text.*)
    *libesp_mm.a:esp_cache_utils.*(.literal .literal.* .text .text.*)
    *libesp_netif.a:esp_netif_lwip.*(.literal.esp_netif_free_rx_buffer .text.esp_netif_free_rx_buffer)
    *libesp_netif.a:esp_netif_lwip.*(.literal.esp_netif_receive .text.esp_netif_receive)
    *libesp_netif.a:esp_netif_lwip.*(.literal.esp_netif_transmit_wrap .text.esp_netif_transmit_wrap)
    *libesp_netif.a:esp_pbuf_ref.*(.literal.esp_pbuf_allocate .text.esp_pbuf_allocate)
    *libesp_netif.a:esp_pbuf_ref.*(.literal.esp_pbuf_free .text.esp_pbuf_free)
    *libesp_netif.a:ethernetif.*(.literal.ethernet_low_level_output .text.ethernet_low_level_output)
    *libesp_netif.a:ethernetif.*(.literal.ethernetif_input .text.ethernetif_input)
    *libesp_netif.a:wlanif.*(.literal.low_level_output .text.low_level_output)
    *libesp_netif.a:wlanif.*(.literal.wlanif_input .text.wlanif_input)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvAcquireItemNoSplit .text.prvAcquireItemNoSplit)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvCheckItemAvail .text.prvCheckItemAvail)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvCheckItemFitsByteBuffer .text.prvCheckItemFitsByteBuffer)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvCheckItemFitsDefault .text.prvCheckItemFitsDefault)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvCopyItemAllowSplit .text.prvCopyItemAllowSplit)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvCopyItemByteBuf .text.prvCopyItemByteBuf)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvCopyItemNoSplit .text.prvCopyItemNoSplit)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvGetItemByteBuf .text.prvGetItemByteBuf)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvGetItemDefault .text.prvGetItemDefault)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvReceiveGenericFromISR .text.prvReceiveGenericFromISR)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvReturnItemByteBuf .text.prvReturnItemByteBuf)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvReturnItemDefault .text.prvReturnItemDefault)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvSendItemDoneNoSplit .text.prvSendItemDoneNoSplit)
    *libesp_ringbuf.a:ringbuf.*(.literal.vRingbufferReturnItemFromISR .text.vRingbufferReturnItemFromISR)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferReceiveFromISR .text.xRingbufferReceiveFromISR)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferReceiveSplitFromISR .text.xRingbufferReceiveSplitFromISR)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferReceiveUpToFromISR .text.xRingbufferReceiveUpToFromISR)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferSendFromISR .text.xRingbufferSendFromISR)
    *libesp_rom.a:esp_rom_cache_esp32s2_esp32s3.*(.literal .literal.* .text .text.*)
    *libesp_rom.a:esp_rom_cache_writeback_esp32s3.*(.literal .literal.* .text .text.*)
    *libesp_rom.a:esp_rom_print.*(.literal .literal.* .text .text.*)
    *libesp_rom.a:esp_rom_spiflash.*(.literal .literal.* .text .text.*)
    *libesp_rom.a:esp_rom_sys.*(.literal .literal.* .text .text.*)
    *libesp_rom.a:esp_rom_systimer.*(.literal .literal.* .text .text.*)
    *libesp_system.a:esp_err.*(.literal .literal.* .text .text.*)
    *libesp_system.a:esp_system_chip.*(.literal.esp_system_abort .text.esp_system_abort)
    *libesp_system.a:freertos_hooks.*(.literal.esp_vApplicationTickHook .text.esp_vApplicationTickHook)
    *libesp_system.a:image_process.*(.literal .literal.* .text .text.*)
    *libesp_system.a:panic.*(.literal.panic_abort .text.panic_abort)
    *libesp_system.a:reset_reason.*(.literal.esp_reset_reason_set_hint .text.esp_reset_reason_set_hint)
    *libesp_system.a:system_internal.*(.literal.esp_restart_noos .text.esp_restart_noos)
    *libesp_system.a:system_internal.*(.literal.esp_system_reset_modules_on_exit .text.esp_system_reset_modules_on_exit)
    *libesp_system.a:system_time.*(.literal.esp_system_get_time .text.esp_system_get_time)
    *libesp_system.a:system_time.*(.literal.esp_system_get_time_resolution .text.esp_system_get_time_resolution)
    *libesp_system.a:ubsan.*(.literal .literal.* .text .text.*)
    *libesp_wifi.a:esp_adapter.*(.literal.coex_pti_get_wrapper .text.coex_pti_get_wrapper)
    *libesp_wifi.a:wifi_netif.*(.literal.wifi_sta_receive .text.wifi_sta_receive)
    *libesp_wifi.a:wifi_netif.*(.literal.wifi_transmit_wrap .text.wifi_transmit_wrap)
    *libfreertos.a:event_groups.*(.literal.xEventGroupGetBitsFromISR .text.xEventGroupGetBitsFromISR)
    *libfreertos.a:list.*(.literal.uxListRemove .text.uxListRemove)
    *libfreertos.a:list.*(.literal.vListInsertEnd .text.vListInsertEnd)
    *libfreertos.a:port.*(.literal.vPortAssertIfInISR .text.vPortAssertIfInISR)
    *libfreertos.a:port.*(.literal.vPortExitCritical .text.vPortExitCritical)
    *libfreertos.a:port.*(.literal.vPortSetStackWatchpoint .text.vPortSetStackWatchpoint)
    *libfreertos.a:port.*(.literal.vPortYieldOtherCore .text.vPortYieldOtherCore)
    *libfreertos.a:port.*(.literal.xPortEnterCriticalTimeout .text.xPortEnterCriticalTimeout)
    *libfreertos.a:port.*(.literal.xPortInIsrContext .text.xPortInIsrContext)
    *libfreertos.a:port.*(.literal.xPortInterruptedFromISRContext .text.xPortInterruptedFromISRContext)
    *libfreertos.a:port.*(.literal.xPortStartScheduler .text.xPortStartScheduler)
    *libfreertos.a:port_systick.*(.literal .literal.* .text .text.*)
    *libfreertos.a:portasm.*(.literal .literal.* .text .text.*)
    *libfreertos.a:queue.*(.literal.prvCopyDataFromQueue .text.prvCopyDataFromQueue)
    *libfreertos.a:queue.*(.literal.prvCopyDataToQueue .text.prvCopyDataToQueue)
    *libfreertos.a:queue.*(.literal.prvNotifyQueueSetContainer .text.prvNotifyQueueSetContainer)
    *libfreertos.a:queue.*(.literal.uxQueueMessagesWaitingFromISR .text.uxQueueMessagesWaitingFromISR)
    *libfreertos.a:queue.*(.literal.xQueueGenericSendFromISR .text.xQueueGenericSendFromISR)
    *libfreertos.a:queue.*(.literal.xQueueGetMutexHolderFromISR .text.xQueueGetMutexHolderFromISR)
    *libfreertos.a:queue.*(.literal.xQueueGiveFromISR .text.xQueueGiveFromISR)
    *libfreertos.a:queue.*(.literal.xQueueIsQueueEmptyFromISR .text.xQueueIsQueueEmptyFromISR)
    *libfreertos.a:queue.*(.literal.xQueueIsQueueFullFromISR .text.xQueueIsQueueFullFromISR)
    *libfreertos.a:queue.*(.literal.xQueuePeekFromISR .text.xQueuePeekFromISR)
    *libfreertos.a:queue.*(.literal.xQueueReceiveFromISR .text.xQueueReceiveFromISR)
    *libfreertos.a:queue.*(.literal.xQueueSelectFromSetFromISR .text.xQueueSelectFromSetFromISR)
    *libfreertos.a:stream_buffer.*(.literal.prvBytesInBuffer .text.prvBytesInBuffer)
    *libfreertos.a:stream_buffer.*(.literal.prvReadMessageFromBuffer .text.prvReadMessageFromBuffer)
    *libfreertos.a:stream_buffer.*(.literal.prvWriteMessageToBuffer .text.prvWriteMessageToBuffer)
    *libfreertos.a:stream_buffer.*(.literal.xStreamBufferReceiveCompletedFromISR .text.xStreamBufferReceiveCompletedFromISR)
    *libfreertos.a:stream_buffer.*(.literal.xStreamBufferReceiveFromISR .text.xStreamBufferReceiveFromISR)
    *libfreertos.a:stream_buffer.*(.literal.xStreamBufferSendCompletedFromISR .text.xStreamBufferSendCompletedFromISR)
    *libfreertos.a:stream_buffer.*(.literal.xStreamBufferSendFromISR .text.xStreamBufferSendFromISR)
    *libfreertos.a:tasks.*(.literal.prvCheckTaskCanBeScheduledSMP .text.prvCheckTaskCanBeScheduledSMP)
    *libfreertos.a:tasks.*(.literal.prvIsYieldRequiredSMP .text.prvIsYieldRequiredSMP)
    *libfreertos.a:tasks.*(.literal.prvResetNextTaskUnblockTime .text.prvResetNextTaskUnblockTime)
    *libfreertos.a:tasks.*(.literal.prvSelectHighestPriorityTaskSMP .text.prvSelectHighestPriorityTaskSMP)
    *libfreertos.a:tasks.*(.literal.prvTaskIsTaskSuspended .text.prvTaskIsTaskSuspended)
    *libfreertos.a:tasks.*(.literal.uxTaskPriorityGetFromISR .text.uxTaskPriorityGetFromISR)
    *libfreertos.a:tasks.*(.literal.vTaskGenericNotifyGiveFromISR .text.vTaskGenericNotifyGiveFromISR)
    *libfreertos.a:tasks.*(.literal.vTaskGetSnapshot .text.vTaskGetSnapshot)
    *libfreertos.a:tasks.*(.literal.vTaskSwitchContext .text.vTaskSwitchContext)
    *libfreertos.a:tasks.*(.literal.xTaskGenericNotifyFromISR .text.xTaskGenericNotifyFromISR)
    *libfreertos.a:tasks.*(.literal.xTaskGetSchedulerState .text.xTaskGetSchedulerState)
    *libfreertos.a:tasks.*(.literal.xTaskGetTickCount .text.xTaskGetTickCount)
    *libfreertos.a:tasks.*(.literal.xTaskGetTickCountFromISR .text.xTaskGetTickCountFromISR)
    *libfreertos.a:tasks.*(.literal.xTaskIncrementTick .text.xTaskIncrementTick)
    *libfreertos.a:tasks.*(.literal.xTaskIncrementTickOtherCores .text.xTaskIncrementTickOtherCores)
    *libfreertos.a:tasks.*(.literal.xTaskRemoveFromEventList .text.xTaskRemoveFromEventList)
    *libfreertos.a:tasks.*(.literal.xTaskResumeFromISR .text.xTaskResumeFromISR)
    *libfreertos.a:timers.*(.literal.xTimerGenericCommand .text.xTimerGenericCommand)
    *libfreertos.a:timers.*(.literal.xTimerPendFunctionCallFromISR .text.xTimerPendFunctionCallFromISR)
    *libgcc.a:_divsf3.*(.literal .literal.* .text .text.*)
    *libgcc.a:lib2funcs.*(.literal .literal.* .text .text.*)
    *libgcov.a:(.literal .literal.* .text .text.*)
    *libhal.a:cache_hal.*(.literal .literal.* .text .text.*)
    *libhal.a:mmu_hal.*(.literal .literal.* .text .text.*)
    *libhal.a:systimer_hal.*(.literal .literal.* .text .text.*)
    *libheap.a:multi_heap.*(.literal.assert_valid_block .text.assert_valid_block)
    *libheap.a:multi_heap.*(.literal.multi_heap_aligned_alloc_impl .text.multi_heap_aligned_alloc_impl)
    *libheap.a:multi_heap.*(.literal.multi_heap_aligned_alloc_impl_offs .text.multi_heap_aligned_alloc_impl_offs)
    *libheap.a:multi_heap.*(.literal.multi_heap_aligned_alloc_offs .text.multi_heap_aligned_alloc_offs)
    *libheap.a:multi_heap.*(.literal.multi_heap_free_impl .text.multi_heap_free_impl)
    *libheap.a:multi_heap.*(.literal.multi_heap_get_allocated_size_impl .text.multi_heap_get_allocated_size_impl)
    *libheap.a:multi_heap.*(.literal.multi_heap_get_block_address_impl .text.multi_heap_get_block_address_impl)
    *libheap.a:multi_heap.*(.literal.multi_heap_get_first_block .text.multi_heap_get_first_block)
    *libheap.a:multi_heap.*(.literal.multi_heap_get_full_block_size .text.multi_heap_get_full_block_size)
    *libheap.a:multi_heap.*(.literal.multi_heap_get_next_block .text.multi_heap_get_next_block)
    *libheap.a:multi_heap.*(.literal.multi_heap_internal_lock .text.multi_heap_internal_lock)
    *libheap.a:multi_heap.*(.literal.multi_heap_internal_unlock .text.multi_heap_internal_unlock)
    *libheap.a:multi_heap.*(.literal.multi_heap_is_free .text.multi_heap_is_free)
    *libheap.a:multi_heap.*(.literal.multi_heap_malloc_impl .text.multi_heap_malloc_impl)
    *libheap.a:multi_heap.*(.literal.multi_heap_realloc_impl .text.multi_heap_realloc_impl)
    *libheap.a:multi_heap.*(.literal.multi_heap_set_lock .text.multi_heap_set_lock)
    *libheap.a:tlsf.*(.literal.tlsf_alloc_overhead .text.tlsf_alloc_overhead)
    *libheap.a:tlsf.*(.literal.tlsf_block_size .text.tlsf_block_size)
    *libheap.a:tlsf.*(.literal.tlsf_free .text.tlsf_free)
    *libheap.a:tlsf.*(.literal.tlsf_get_pool .text.tlsf_get_pool)
    *libheap.a:tlsf.*(.literal.tlsf_malloc .text.tlsf_malloc)
    *libheap.a:tlsf.*(.literal.tlsf_memalign .text.tlsf_memalign)
    *libheap.a:tlsf.*(.literal.tlsf_memalign_offs .text.tlsf_memalign_offs)
    *libheap.a:tlsf.*(.literal.tlsf_realloc .text.tlsf_realloc)
    *libheap.a:tlsf.*(.literal.tlsf_size .text.tlsf_size)
    *liblog.a:log.*(.literal .literal.* .text .text.*)
    *liblog.a:log_format_text.*(.literal .literal.* .text .text.*)
    *liblog.a:log_lock.*(.literal .literal.* .text .text.*)
    *liblog.a:log_print.*(.literal .literal.* .text .text.*)
    *liblog.a:log_timestamp.*(.literal.esp_log_early_timestamp .text.esp_log_early_timestamp)
    *liblog.a:log_timestamp.*(.literal.esp_log_timestamp .text.esp_log_timestamp)
    *liblog.a:log_timestamp_common.*(.literal .literal.* .text .text.*)
    *liblog.a:log_write.*(.literal.esp_log_write .text.esp_log_write)
    *liblog.a:log_write.*(.literal.esp_log_writev .text.esp_log_writev)
    *liblog.a:tag_log_level.*(.literal.esp_log_level_get_timeout .text.esp_log_level_get_timeout)
    *liblog.a:util.*(.literal .literal.* .text .text.*)
    *liblwip.a:api_lib.*(.literal.netconn_apimsg .text.netconn_apimsg)
    *liblwip.a:api_lib.*(.literal.netconn_recv .text.netconn_recv)
    *liblwip.a:api_lib.*(.literal.netconn_recv_data .text.netconn_recv_data)
    *liblwip.a:api_lib.*(.literal.netconn_recv_data_tcp .text.netconn_recv_data_tcp)
    *liblwip.a:api_lib.*(.literal.netconn_recv_tcp_pbuf_flags .text.netconn_recv_tcp_pbuf_flags)
    *liblwip.a:api_lib.*(.literal.netconn_recv_udp_raw_netbuf_flags .text.netconn_recv_udp_raw_netbuf_flags)
    *liblwip.a:api_lib.*(.literal.netconn_send .text.netconn_send)
    *liblwip.a:api_lib.*(.literal.netconn_sendto .text.netconn_sendto)
    *liblwip.a:api_lib.*(.literal.netconn_tcp_recvd .text.netconn_tcp_recvd)
    *liblwip.a:api_lib.*(.literal.netconn_tcp_recvd_msg .text.netconn_tcp_recvd_msg)
    *liblwip.a:api_lib.*(.literal.netconn_write_partly .text.netconn_write_partly)
    *liblwip.a:api_lib.*(.literal.netconn_write_vectors_partly .text.netconn_write_vectors_partly)
    *liblwip.a:api_msg.*(.literal.lwip_netconn_do_send .text.lwip_netconn_do_send)
    *liblwip.a:api_msg.*(.literal.lwip_netconn_do_write .text.lwip_netconn_do_write)
    *liblwip.a:api_msg.*(.literal.recv_udp .text.recv_udp)
    *liblwip.a:etharp.*(.literal.etharp_output .text.etharp_output)
    *liblwip.a:etharp.*(.literal.etharp_output_to_arp_index .text.etharp_output_to_arp_index)
    *liblwip.a:ethernet.*(.literal.ethernet_input .text.ethernet_input)
    *liblwip.a:ethernet.*(.literal.ethernet_output .text.ethernet_output)
    *liblwip.a:inet_chksum.*(.literal.inet_chksum .text.inet_chksum)
    *liblwip.a:inet_chksum.*(.literal.inet_chksum_pbuf .text.inet_chksum_pbuf)
    *liblwip.a:inet_chksum.*(.literal.inet_chksum_pseudo .text.inet_chksum_pseudo)
    *liblwip.a:inet_chksum.*(.literal.inet_cksum_pseudo_base .text.inet_cksum_pseudo_base)
    *liblwip.a:inet_chksum.*(.literal.ip_chksum_pseudo .text.ip_chksum_pseudo)
    *liblwip.a:inet_chksum.*(.literal.lwip_standard_chksum .text.lwip_standard_chksum)
    *liblwip.a:ip4.*(.literal.ip4_input .text.ip4_input)
    *liblwip.a:ip4.*(.literal.ip4_output .text.ip4_output)
    *liblwip.a:ip4.*(.literal.ip4_output_if .text.ip4_output_if)
    *liblwip.a:ip4.*(.literal.ip4_output_if_opt .text.ip4_output_if_opt)
    *liblwip.a:ip4.*(.literal.ip4_output_if_opt_src .text.ip4_output_if_opt_src)
    *liblwip.a:ip4.*(.literal.ip4_output_if_src .text.ip4_output_if_src)
    *liblwip.a:ip4.*(.literal.ip4_route .text.ip4_route)
    *liblwip.a:ip4.*(.literal.ip4_route_src .text.ip4_route_src)
    *liblwip.a:ip4_addr.*(.literal.ip4_addr_isbroadcast_u32 .text.ip4_addr_isbroadcast_u32)
    *liblwip.a:lwip_default_hooks.*(.literal.ip4_route_src_hook .text.ip4_route_src_hook)
    *liblwip.a:mem.*(.literal.mem_malloc .text.mem_malloc)
    *liblwip.a:memp.*(.literal.do_memp_malloc_pool .text.do_memp_malloc_pool)
    *liblwip.a:memp.*(.literal.memp_malloc .text.memp_malloc)
    *liblwip.a:netbuf.*(.literal.netbuf_alloc .text.netbuf_alloc)
    *liblwip.a:netbuf.*(.literal.netbuf_free .text.netbuf_free)
    *liblwip.a:pbuf.*(.literal.pbuf_add_header .text.pbuf_add_header)
    *liblwip.a:pbuf.*(.literal.pbuf_add_header_impl .text.pbuf_add_header_impl)
    *liblwip.a:pbuf.*(.literal.pbuf_alloc .text.pbuf_alloc)
    *liblwip.a:pbuf.*(.literal.pbuf_alloced_custom .text.pbuf_alloced_custom)
    *liblwip.a:pbuf.*(.literal.pbuf_clone .text.pbuf_clone)
    *liblwip.a:pbuf.*(.literal.pbuf_copy .text.pbuf_copy)
    *liblwip.a:pbuf.*(.literal.pbuf_copy_partial_pbuf .text.pbuf_copy_partial_pbuf)
    *liblwip.a:pbuf.*(.literal.pbuf_free .text.pbuf_free)
    *liblwip.a:pbuf.*(.literal.pbuf_header .text.pbuf_header)
    *liblwip.a:pbuf.*(.literal.pbuf_header_impl .text.pbuf_header_impl)
    *liblwip.a:pbuf.*(.literal.pbuf_init_alloced_pbuf .text.pbuf_init_alloced_pbuf)
    *liblwip.a:pbuf.*(.literal.pbuf_remove_header .text.pbuf_remove_header)
    *liblwip.a:raw.*(.literal.raw_input .text.raw_input)
    *liblwip.a:raw.*(.literal.raw_input_local_match .text.raw_input_local_match)
    *liblwip.a:sockets.*(.literal.done_socket .text.done_socket)
    *liblwip.a:sockets.*(.literal.event_callback .text.event_callback)
    *liblwip.a:sockets.*(.literal.get_socket .text.get_socket)
    *liblwip.a:sockets.*(.literal.lwip_recv_tcp .text.lwip_recv_tcp)
    *liblwip.a:sockets.*(.literal.lwip_recv_tcp_from .text.lwip_recv_tcp_from)
    *liblwip.a:sockets.*(.literal.lwip_recvfrom .text.lwip_recvfrom)
    *liblwip.a:sockets.*(.literal.lwip_recvfrom_udp_raw .text.lwip_recvfrom_udp_raw)
    *liblwip.a:sockets.*(.literal.lwip_select .text.lwip_select)
    *liblwip.a:sockets.*(.literal.lwip_send .text.lwip_send)
    *liblwip.a:sockets.*(.literal.lwip_sendto .text.lwip_sendto)
    *liblwip.a:sockets.*(.literal.select_check_waiters .text.select_check_waiters)
    *liblwip.a:sockets.*(.literal.sock_inc_used .text.sock_inc_used)
    *liblwip.a:sockets.*(.literal.tryget_socket .text.tryget_socket)
    *liblwip.a:sockets.*(.literal.tryget_socket_unconn .text.tryget_socket_unconn)
    *liblwip.a:sockets.*(.literal.tryget_socket_unconn_locked .text.tryget_socket_unconn_locked)
    *liblwip.a:sockets.*(.literal.tryget_socket_unconn_nouse .text.tryget_socket_unconn_nouse)
    *liblwip.a:sys_arch.*(.literal.sys_arch_mbox_fetch .text.sys_arch_mbox_fetch)
    *liblwip.a:sys_arch.*(.literal.sys_arch_protect .text.sys_arch_protect)
    *liblwip.a:sys_arch.*(.literal.sys_arch_sem_wait .text.sys_arch_sem_wait)
    *liblwip.a:sys_arch.*(.literal.sys_arch_unprotect .text.sys_arch_unprotect)
    *liblwip.a:sys_arch.*(.literal.sys_mbox_post .text.sys_mbox_post)
    *liblwip.a:sys_arch.*(.literal.sys_mbox_trypost .text.sys_mbox_trypost)
    *liblwip.a:sys_arch.*(.literal.sys_mutex_lock .text.sys_mutex_lock)
    *liblwip.a:sys_arch.*(.literal.sys_mutex_unlock .text.sys_mutex_unlock)
    *liblwip.a:sys_arch.*(.literal.sys_sem_signal .text.sys_sem_signal)
    *liblwip.a:sys_arch.*(.literal.sys_thread_tcpip .text.sys_thread_tcpip)
    *liblwip.a:tcp.*(.literal.tcp_fasttmr .text.tcp_fasttmr)
    *liblwip.a:tcp.*(.literal.tcp_process_refused_data .text.tcp_process_refused_data)
    *liblwip.a:tcp.*(.literal.tcp_recv_null .text.tcp_recv_null)
    *liblwip.a:tcp.*(.literal.tcp_recved .text.tcp_recved)
    *liblwip.a:tcp.*(.literal.tcp_seg_copy .text.tcp_seg_copy)
    *liblwip.a:tcp.*(.literal.tcp_seg_free .text.tcp_seg_free)
    *liblwip.a:tcp.*(.literal.tcp_segs_free .text.tcp_segs_free)
    *liblwip.a:tcp.*(.literal.tcp_slowtmr .text.tcp_slowtmr)
    *liblwip.a:tcp.*(.literal.tcp_tmr .text.tcp_tmr)
    *liblwip.a:tcp.*(.literal.tcp_update_rcv_ann_wnd .text.tcp_update_rcv_ann_wnd)
    *liblwip.a:tcp_in.*(.literal.tcp_free_acked_segments .text.tcp_free_acked_segments)
    *liblwip.a:tcp_in.*(.literal.tcp_input .text.tcp_input)
    *liblwip.a:tcp_in.*(.literal.tcp_input_delayed_close .text.tcp_input_delayed_close)
    *liblwip.a:tcp_in.*(.literal.tcp_oos_insert_segment .text.tcp_oos_insert_segment)
    *liblwip.a:tcp_in.*(.literal.tcp_parseopt .text.tcp_parseopt)
    *liblwip.a:tcp_in.*(.literal.tcp_process .text.tcp_process)
    *liblwip.a:tcp_in.*(.literal.tcp_receive .text.tcp_receive)
    *liblwip.a:tcp_out.*(.literal.tcp_create_segment .text.tcp_create_segment)
    *liblwip.a:tcp_out.*(.literal.tcp_output_alloc_header .text.tcp_output_alloc_header)
    *liblwip.a:tcp_out.*(.literal.tcp_output_alloc_header_common .text.tcp_output_alloc_header_common)
    *liblwip.a:tcp_out.*(.literal.tcp_output_control_segment_netif .text.tcp_output_control_segment_netif)
    *liblwip.a:tcp_out.*(.literal.tcp_output_fill_options .text.tcp_output_fill_options)
    *liblwip.a:tcp_out.*(.literal.tcp_output_segment .text.tcp_output_segment)
    *liblwip.a:tcp_out.*(.literal.tcp_pbuf_prealloc .text.tcp_pbuf_prealloc)
    *liblwip.a:tcp_out.*(.literal.tcp_rexmit .text.tcp_rexmit)
    *liblwip.a:tcp_out.*(.literal.tcp_rexmit_fast .text.tcp_rexmit_fast)
    *liblwip.a:tcp_out.*(.literal.tcp_rexmit_rto_prepare .text.tcp_rexmit_rto_prepare)
    *liblwip.a:tcp_out.*(.literal.tcp_route .text.tcp_route)
    *liblwip.a:tcp_out.*(.literal.tcp_rst .text.tcp_rst)
    *liblwip.a:tcp_out.*(.literal.tcp_send_empty_ack .text.tcp_send_empty_ack)
    *liblwip.a:tcp_out.*(.literal.tcp_split_unsent_seg .text.tcp_split_unsent_seg)
    *liblwip.a:tcp_out.*(.literal.tcp_write .text.tcp_write)
    *liblwip.a:tcp_out.*(.literal.tcp_write_checks .text.tcp_write_checks)
    *liblwip.a:tcpip.*(.literal.tcpip_callback .text.tcpip_callback)
    *liblwip.a:tcpip.*(.literal.tcpip_inpkt .text.tcpip_inpkt)
    *liblwip.a:tcpip.*(.literal.tcpip_input .text.tcpip_input)
    *liblwip.a:tcpip.*(.literal.tcpip_send_msg_wait_sem .text.tcpip_send_msg_wait_sem)
    *liblwip.a:tcpip.*(.literal.tcpip_thread .text.tcpip_thread)
    *liblwip.a:tcpip.*(.literal.tcpip_thread_handle_msg .text.tcpip_thread_handle_msg)
    *liblwip.a:tcpip.*(.literal.tcpip_try_callback .text.tcpip_try_callback)
    *liblwip.a:udp.*(.literal.udp_input .text.udp_input)
    *liblwip.a:udp.*(.literal.udp_input_local_match .text.udp_input_local_match)
    *liblwip.a:udp.*(.literal.udp_send .text.udp_send)
    *liblwip.a:udp.*(.literal.udp_sendto .text.udp_sendto)
    *liblwip.a:udp.*(.literal.udp_sendto_if .text.udp_sendto_if)
    *liblwip.a:udp.*(.literal.udp_sendto_if_src .text.udp_sendto_if_src)
    *libnet80211.a:(.wifi0iram .wifi0iram.*)
    *libnet80211.a:(.wifiextrairam .wifiextrairam.*)
    *libnet80211.a:(.wifirxiram .wifirxiram.*)
    *libnet80211.a:(.wifislprxiram .wifislprxiram.*)
    *libpp.a:(.wifi0iram .wifi0iram.*)
    *libpp.a:(.wifiextrairam .wifiextrairam.*)
    *libpp.a:(.wifiorslpiram .wifiorslpiram.*)
    *libpp.a:(.wifirxiram .wifirxiram.*)
    *libpp.a:(.wifislprxiram .wifislprxiram.*)
    *librtc.a:(.literal .literal.* .text .text.*)
    *libsoc.a:lldesc.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:esp_flash_api.*(.literal.check_chip_pointer_default .text.check_chip_pointer_default)
    *libspi_flash.a:esp_flash_api.*(.literal.detect_spi_flash_chip .text.detect_spi_flash_chip)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_chip_driver_initialized .text.esp_flash_chip_driver_initialized)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_erase_chip .text.esp_flash_erase_chip)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_erase_region .text.esp_flash_erase_region)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_get_chip_write_protect .text.esp_flash_get_chip_write_protect)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_get_io_mode .text.esp_flash_get_io_mode)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_get_physical_size .text.esp_flash_get_physical_size)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_get_protected_region .text.esp_flash_get_protected_region)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_get_size .text.esp_flash_get_size)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_init .text.esp_flash_init)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_init_main .text.esp_flash_init_main)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_is_quad_mode .text.esp_flash_is_quad_mode)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_read .text.esp_flash_read)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_read_encrypted .text.esp_flash_read_encrypted)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_set_chip_write_protect .text.esp_flash_set_chip_write_protect)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_set_io_mode .text.esp_flash_set_io_mode)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_set_protected_region .text.esp_flash_set_protected_region)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_write .text.esp_flash_write)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_write_encrypted .text.esp_flash_write_encrypted)
    *libspi_flash.a:esp_flash_api.*(.literal.flash_end_flush_cache .text.flash_end_flush_cache)
    *libspi_flash.a:esp_flash_api.*(.literal.read_unique_id .text.read_unique_id)
    *libspi_flash.a:esp_flash_api.*(.literal.spiflash_end_default .text.spiflash_end_default)
    *libspi_flash.a:esp_flash_api.*(.literal.spiflash_start_core .text.spiflash_start_core)
    *libspi_flash.a:esp_flash_api.*(.literal.spiflash_start_default .text.spiflash_start_default)
    *libspi_flash.a:flash_brownout_hook.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:memspi_host_driver.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_boya.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_gd.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_generic.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_issi.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_mxic.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_mxic_opi.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_th.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_winbond.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_hpm_enable.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_oct_flash_init.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.delay_us .text.delay_us)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.get_buffer_malloc .text.get_buffer_malloc)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.main_flash_op_status .text.main_flash_op_status)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.main_flash_region_protected .text.main_flash_region_protected)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.release_buffer_malloc .text.release_buffer_malloc)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.spi23_end .text.spi23_end)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.spi23_start .text.spi23_start)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.spi_flash_os_check_yield .text.spi_flash_os_check_yield)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.spi_flash_os_yield .text.spi_flash_os_yield)
    *libspi_flash.a:spi_flash_os_func_noos.*(.literal.delay_us .text.delay_us)
    *libspi_flash.a:spi_flash_os_func_noos.*(.literal.esp_flash_app_disable_os_functions .text.esp_flash_app_disable_os_functions)
    *libspi_flash.a:spi_flash_os_func_noos.*(.literal.get_temp_buffer_not_supported .text.get_temp_buffer_not_supported)
    *libspi_flash.a:spi_flash_wrap.*(.literal .literal.* .text .text.*)
    *libxt_hal.a:(.literal .literal.* .text .text.*)
    *libxtensa.a:(EXCLUDE_FILE(*libxtensa.a:xt_trax.* *libxtensa.a:xtensa_intr.*) .literal EXCLUDE_FILE(*libxtensa.a:xt_trax.* *libxtensa.a:xtensa_intr.*) .literal.* EXCLUDE_FILE(*libxtensa.a:xt_trax.* *libxtensa.a:xtensa_intr.*) .text EXCLUDE_FILE(*libxtensa.a:xt_trax.* *libxtensa.a:xtensa_intr.*) .text.*)
  } > iram0_0_seg
  /**

   * This section is required to skip .iram0.text area because iram0_0_seg and

   * dram0_0_seg reflect the same address space on different buses.

   */
  .dram0.dummy (NOLOAD):
  {
    /* MAX() uses unsigned long arithmetic. Add offset to prevent underflow when _iram_end < _diram_i_start */
    . = ORIGIN(dram0_0_seg) + MAX(_iram_end - _diram_i_start + (_diram_i_start - ORIGIN(iram0_0_seg)), (_diram_i_start - ORIGIN(iram0_0_seg))) - (_diram_i_start - ORIGIN(iram0_0_seg));
  } > dram0_0_seg
  .dram0.data :
  {
    _data_start = ABSOLUTE(.);
    *(.gnu.linkonce.d.*)
    *(.data1)
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
    *(.gnu.linkonce.s2.*)
    *(.jcr)
    *(.data .data.*)
    *(.dram1 .dram1.*)
    *libclang_rt.builtins.a:_divsf3.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hal_gpspi.a:spi_hal_iram.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hal_gpspi.a:spi_slave_hal_iram.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hal_mspi.a:spi_flash_encrypt_hal_iram.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hal_mspi.a:spi_flash_hal_gpspi.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hal_mspi.a:spi_flash_hal_iram.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hal_wdt.a:rom_patch.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hw_support.a:clk_utils.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hw_support.a:cpu.*(.rodata.esp_cpu_stall .rodata.esp_cpu_stall.str1.4 .sdata2.esp_cpu_stall .srodata.esp_cpu_stall)
    *libesp_hw_support.a:esp_memory_utils.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hw_support.a:mspi_timing_by_mspi_delay.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hw_support.a:mspi_timing_config.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hw_support.a:mspi_timing_tuning.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hw_support.a:rtc_clk.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hw_support.a:systimer.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_libc.a:abort.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_libc.a:assert.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_libc.a:heap.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_libc.a:stdatomic.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_mm.a:esp_cache_msync.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_mm.a:esp_cache_utils.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_rom.a:esp_rom_cache_esp32s2_esp32s3.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_rom.a:esp_rom_cache_writeback_esp32s3.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_rom.a:esp_rom_print.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_rom.a:esp_rom_spiflash.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_rom.a:esp_rom_sys.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_rom.a:esp_rom_systimer.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_system.a:esp_err.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_system.a:image_process.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_system.a:reset_reason.*(.rodata.esp_reset_reason_set_hint .rodata.esp_reset_reason_set_hint.str1.4 .sdata2.esp_reset_reason_set_hint .srodata.esp_reset_reason_set_hint)
    *libesp_system.a:ubsan.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libgcc.a:_divsf3.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libgcov.a:(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:cache_hal.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:mmu_hal.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:systimer_hal.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libheap.a:multi_heap.*(.rodata.assert_valid_block .rodata.assert_valid_block.str1.4 .sdata2.assert_valid_block .srodata.assert_valid_block)
    *libheap.a:multi_heap.*(.rodata.multi_heap_get_first_block .rodata.multi_heap_get_first_block.str1.4 .sdata2.multi_heap_get_first_block .srodata.multi_heap_get_first_block)
    *libheap.a:multi_heap.*(.rodata.multi_heap_get_next_block .rodata.multi_heap_get_next_block.str1.4 .sdata2.multi_heap_get_next_block .srodata.multi_heap_get_next_block)
    *libheap.a:tlsf.*(.rodata.tlsf_free .rodata.tlsf_free.str1.4 .sdata2.tlsf_free .srodata.tlsf_free)
    *libheap.a:tlsf.*(.rodata.tlsf_malloc .rodata.tlsf_malloc.str1.4 .sdata2.tlsf_malloc .srodata.tlsf_malloc)
    *libheap.a:tlsf.*(.rodata.tlsf_memalign_offs .rodata.tlsf_memalign_offs.str1.4 .sdata2.tlsf_memalign_offs .srodata.tlsf_memalign_offs)
    *libheap.a:tlsf.*(.rodata.tlsf_realloc .rodata.tlsf_realloc.str1.4 .sdata2.tlsf_realloc .srodata.tlsf_realloc)
    *liblog.a:log.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *liblog.a:log_format_text.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *liblog.a:log_lock.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *liblog.a:log_print.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *liblog.a:log_timestamp_common.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *liblog.a:util.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libphy.a:(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libsoc.a:lldesc.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:flash_brownout_hook.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:memspi_host_driver.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_boya.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_gd.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_generic.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_issi.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_mxic.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_mxic_opi.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_th.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_winbond.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_hpm_enable.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_oct_flash_init.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_os_func_app.*(.rodata.spi_flash_os_check_yield .rodata.spi_flash_os_check_yield.str1.4 .sdata2.spi_flash_os_check_yield .srodata.spi_flash_os_check_yield)
    *libspi_flash.a:spi_flash_wrap.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    _data_end = ABSOLUTE(.);
  } > dram0_0_seg
  /**

   * This section holds data that should not be initialized at power up.

   * The section located in Internal SRAM memory region. The macro _NOINIT

   * can be used as attribute to place data into this section.

   * See the "esp_attr.h" file for more information.

   */
  .noinit (NOLOAD):
  {
    
 . = ALIGN(4); 
 _noinit_start = ABSOLUTE(.);
    *(.noinit .noinit.*)
    
 . = ALIGN(4); 
 _noinit_end = ABSOLUTE(.);
  } > dram0_0_seg
  /* Shared RAM */
  .dram0.bss (NOLOAD) :
  {
    
 . = ALIGN(8); 
 _bss_start = ABSOLUTE(.);
    /**

     * ldgen places all bss-related data to mapping[dram0_bss]

     * (See components/esp_system/app.lf).

     */
    *(.bss .bss.*)
    *(.dynbss .dynsbss .gnu.linkonce.b .gnu.linkonce.b.* .gnu.linkonce.sb .gnu.linkonce.sb.* .gnu.linkonce.sb2 .gnu.linkonce.sb2.* .sbss .sbss.* .sbss2 .sbss2.* .scommon .share.mem)
    *(.ext_ram.bss .ext_ram.bss.*)
    *(COMMON)
    
 . = ALIGN(16); 
 _bss_end = ABSOLUTE(.);
  } > dram0_0_seg
  ASSERT(((_bss_end - ORIGIN(dram0_0_seg)) <= LENGTH(dram0_0_seg)),
         "DRAM segment data does not fit.")
  .flash.text :
  {
    _stext = .;
    /**

     * Mark the start of flash.text.

     * This can be used by the MMU driver to maintain the virtual address.

     */
    _instruction_reserved_start = ABSOLUTE(.);
    _text_start = ABSOLUTE(.);
    *(EXCLUDE_FILE(*libgcov.a *librtc.a *libxt_hal.a *libxtensa.a *libclang_rt.builtins.a:_divsf3.* *libesp_driver_dma.a:gdma.* *libesp_event.a:default_event_loop.* *libesp_event.a:esp_event.* *libesp_hal_dma.a:gdma_hal_ahb_v1.* *libesp_hal_dma.a:gdma_hal_top.* *libesp_hal_gpio.a:gpio_hal.* *libesp_hal_gpspi.a:spi_hal_iram.* *libesp_hal_gpspi.a:spi_slave_hal_iram.* *libesp_hal_mspi.a:spi_flash_encrypt_hal_iram.* *libesp_hal_mspi.a:spi_flash_hal_gpspi.* *libesp_hal_mspi.a:spi_flash_hal_iram.* *libesp_hal_wdt.a:rom_patch.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_clk_tree.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_by_mspi_delay.* *libesp_hw_support.a:mspi_timing_config.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:periph_ctrl.* *libesp_hw_support.a:regi2c_ctrl.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:rtc_init.* *libesp_hw_support.a:rtc_sleep.* *libesp_hw_support.a:rtc_time.* *libesp_hw_support.a:systimer.* *libesp_libc.a:abort.* *libesp_libc.a:assert.* *libesp_libc.a:heap.* *libesp_libc.a:stdatomic.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_netif.a:esp_netif_lwip.* *libesp_netif.a:esp_pbuf_ref.* *libesp_netif.a:ethernetif.* *libesp_netif.a:wlanif.* *libesp_ringbuf.a:ringbuf.* *libesp_rom.a:esp_rom_cache_esp32s2_esp32s3.* *libesp_rom.a:esp_rom_cache_writeback_esp32s3.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_rom.a:esp_rom_systimer.* *libesp_system.a:esp_err.* *libesp_system.a:esp_system_chip.* *libesp_system.a:freertos_hooks.* *libesp_system.a:image_process.* *libesp_system.a:panic.* *libesp_system.a:reset_reason.* *libesp_system.a:system_internal.* *libesp_system.a:system_time.* *libesp_system.a:ubsan.* *libesp_wifi.a:esp_adapter.* *libesp_wifi.a:wifi_netif.* *libfreertos.a:event_groups.* *libfreertos.a:list.* *libfreertos.a:port.* *libfreertos.a:port_systick.* *libfreertos.a:portasm.* *libfreertos.a:queue.* *libfreertos.a:stream_buffer.* *libfreertos.a:tasks.* *libfreertos.a:timers.* *libgcc.a:_divsf3.* *libgcc.a:lib2funcs.* *libhal.a:cache_hal.* *libhal.a:mmu_hal.* *libhal.a:systimer_hal.* *libheap.a:multi_heap.* *libheap.a:tlsf.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp.* *liblog.a:log_timestamp_common.* *liblog.a:log_write.* *liblog.a:tag_log_level.* *liblog.a:util.* *liblwip.a:api_lib.* *liblwip.a:api_msg.* *liblwip.a:etharp.* *liblwip.a:ethernet.* *liblwip.a:inet_chksum.* *liblwip.a:ip4.* *liblwip.a:ip4_addr.* *liblwip.a:lwip_default_hooks.* *liblwip.a:mem.* *liblwip.a:memp.* *liblwip.a:netbuf.* *liblwip.a:pbuf.* *liblwip.a:raw.* *liblwip.a:sockets.* *liblwip.a:sys_arch.* *liblwip.a:tcp.* *liblwip.a:tcp_in.* *liblwip.a:tcp_out.* *liblwip.a:tcpip.* *liblwip.a:udp.* *libsoc.a:lldesc.* *libspi_flash.a:esp_flash_api.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_mxic_opi.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_hpm_enable.* *libspi_flash.a:spi_flash_oct_flash_init.* *libspi_flash.a:spi_flash_os_func_app.* *libspi_flash.a:spi_flash_os_func_noos.* *libspi_flash.a:spi_flash_wrap.*) .literal EXCLUDE_FILE(*libgcov.a *librtc.a *libxt_hal.a *libxtensa.a *libclang_rt.builtins.a:_divsf3.* *libesp_driver_dma.a:gdma.* *libesp_event.a:default_event_loop.* *libesp_event.a:esp_event.* *libesp_hal_dma.a:gdma_hal_ahb_v1.* *libesp_hal_dma.a:gdma_hal_top.* *libesp_hal_gpio.a:gpio_hal.* *libesp_hal_gpspi.a:spi_hal_iram.* *libesp_hal_gpspi.a:spi_slave_hal_iram.* *libesp_hal_mspi.a:spi_flash_encrypt_hal_iram.* *libesp_hal_mspi.a:spi_flash_hal_gpspi.* *libesp_hal_mspi.a:spi_flash_hal_iram.* *libesp_hal_wdt.a:rom_patch.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_clk_tree.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_by_mspi_delay.* *libesp_hw_support.a:mspi_timing_config.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:periph_ctrl.* *libesp_hw_support.a:regi2c_ctrl.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:rtc_init.* *libesp_hw_support.a:rtc_sleep.* *libesp_hw_support.a:rtc_time.* *libesp_hw_support.a:systimer.* *libesp_libc.a:abort.* *libesp_libc.a:assert.* *libesp_libc.a:heap.* *libesp_libc.a:stdatomic.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_netif.a:esp_netif_lwip.* *libesp_netif.a:esp_pbuf_ref.* *libesp_netif.a:ethernetif.* *libesp_netif.a:wlanif.* *libesp_ringbuf.a:ringbuf.* *libesp_rom.a:esp_rom_cache_esp32s2_esp32s3.* *libesp_rom.a:esp_rom_cache_writeback_esp32s3.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_rom.a:esp_rom_systimer.* *libesp_system.a:esp_err.* *libesp_system.a:esp_system_chip.* *libesp_system.a:freertos_hooks.* *libesp_system.a:image_process.* *libesp_system.a:panic.* *libesp_system.a:reset_reason.* *libesp_system.a:system_internal.* *libesp_system.a:system_time.* *libesp_system.a:ubsan.* *libesp_wifi.a:esp_adapter.* *libesp_wifi.a:wifi_netif.* *libfreertos.a:event_groups.* *libfreertos.a:list.* *libfreertos.a:port.* *libfreertos.a:port_systick.* *libfreertos.a:portasm.* *libfreertos.a:queue.* *libfreertos.a:stream_buffer.* *libfreertos.a:tasks.* *libfreertos.a:timers.* *libgcc.a:_divsf3.* *libgcc.a:lib2funcs.* *libhal.a:cache_hal.* *libhal.a:mmu_hal.* *libhal.a:systimer_hal.* *libheap.a:multi_heap.* *libheap.a:tlsf.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp.* *liblog.a:log_timestamp_common.* *liblog.a:log_write.* *liblog.a:tag_log_level.* *liblog.a:util.* *liblwip.a:api_lib.* *liblwip.a:api_msg.* *liblwip.a:etharp.* *liblwip.a:ethernet.* *liblwip.a:inet_chksum.* *liblwip.a:ip4.* *liblwip.a:ip4_addr.* *liblwip.a:lwip_default_hooks.* *liblwip.a:mem.* *liblwip.a:memp.* *liblwip.a:netbuf.* *liblwip.a:pbuf.* *liblwip.a:raw.* *liblwip.a:sockets.* *liblwip.a:sys_arch.* *liblwip.a:tcp.* *liblwip.a:tcp_in.* *liblwip.a:tcp_out.* *liblwip.a:tcpip.* *liblwip.a:udp.* *libsoc.a:lldesc.* *libspi_flash.a:esp_flash_api.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_mxic_opi.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_hpm_enable.* *libspi_flash.a:spi_flash_oct_flash_init.* *libspi_flash.a:spi_flash_os_func_app.* *libspi_flash.a:spi_flash_os_func_noos.* *libspi_flash.a:spi_flash_wrap.*) .literal.* EXCLUDE_FILE(*libgcov.a *librtc.a *libxt_hal.a *libxtensa.a *libclang_rt.builtins.a:_divsf3.* *libesp_driver_dma.a:gdma.* *libesp_event.a:default_event_loop.* *libesp_event.a:esp_event.* *libesp_hal_dma.a:gdma_hal_ahb_v1.* *libesp_hal_dma.a:gdma_hal_top.* *libesp_hal_gpio.a:gpio_hal.* *libesp_hal_gpspi.a:spi_hal_iram.* *libesp_hal_gpspi.a:spi_slave_hal_iram.* *libesp_hal_mspi.a:spi_flash_encrypt_hal_iram.* *libesp_hal_mspi.a:spi_flash_hal_gpspi.* *libesp_hal_mspi.a:spi_flash_hal_iram.* *libesp_hal_wdt.a:rom_patch.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_clk_tree.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_by_mspi_delay.* *libesp_hw_support.a:mspi_timing_config.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:periph_ctrl.* *libesp_hw_support.a:regi2c_ctrl.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:rtc_init.* *libesp_hw_support.a:rtc_sleep.* *libesp_hw_support.a:rtc_time.* *libesp_hw_support.a:systimer.* *libesp_libc.a:abort.* *libesp_libc.a:assert.* *libesp_libc.a:heap.* *libesp_libc.a:stdatomic.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_netif.a:esp_netif_lwip.* *libesp_netif.a:esp_pbuf_ref.* *libesp_netif.a:ethernetif.* *libesp_netif.a:wlanif.* *libesp_ringbuf.a:ringbuf.* *libesp_rom.a:esp_rom_cache_esp32s2_esp32s3.* *libesp_rom.a:esp_rom_cache_writeback_esp32s3.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_rom.a:esp_rom_systimer.* *libesp_system.a:esp_err.* *libesp_system.a:esp_system_chip.* *libesp_system.a:freertos_hooks.* *libesp_system.a:image_process.* *libesp_system.a:panic.* *libesp_system.a:reset_reason.* *libesp_system.a:system_internal.* *libesp_system.a:system_time.* *libesp_system.a:ubsan.* *libesp_wifi.a:esp_adapter.* *libesp_wifi.a:wifi_netif.* *libfreertos.a:event_groups.* *libfreertos.a:list.* *libfreertos.a:port.* *libfreertos.a:port_systick.* *libfreertos.a:portasm.* *libfreertos.a:queue.* *libfreertos.a:stream_buffer.* *libfreertos.a:tasks.* *libfreertos.a:timers.* *libgcc.a:_divsf3.* *libgcc.a:lib2funcs.* *libhal.a:cache_hal.* *libhal.a:mmu_hal.* *libhal.a:systimer_hal.* *libheap.a:multi_heap.* *libheap.a:tlsf.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp.* *liblog.a:log_timestamp_common.* *liblog.a:log_write.* *liblog.a:tag_log_level.* *liblog.a:util.* *liblwip.a:api_lib.* *liblwip.a:api_msg.* *liblwip.a:etharp.* *liblwip.a:ethernet.* *liblwip.a:inet_chksum.* *liblwip.a:ip4.* *liblwip.a:ip4_addr.* *liblwip.a:lwip_default_hooks.* *liblwip.a:mem.* *liblwip.a:memp.* *liblwip.a:netbuf.* *liblwip.a:pbuf.* *liblwip.a:raw.* *liblwip.a:sockets.* *liblwip.a:sys_arch.* *liblwip.a:tcp.* *liblwip.a:tcp_in.* *liblwip.a:tcp_out.* *liblwip.a:tcpip.* *liblwip.a:udp.* *libsoc.a:lldesc.* *libspi_flash.a:esp_flash_api.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_mxic_opi.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_hpm_enable.* *libspi_flash.a:spi_flash_oct_flash_init.* *libspi_flash.a:spi_flash_os_func_app.* *libspi_flash.a:spi_flash_os_func_noos.* *libspi_flash.a:spi_flash_wrap.*) .text EXCLUDE_FILE(*libgcov.a *librtc.a *libxt_hal.a *libxtensa.a *libclang_rt.builtins.a:_divsf3.* *libesp_driver_dma.a:gdma.* *libesp_event.a:default_event_loop.* *libesp_event.a:esp_event.* *libesp_hal_dma.a:gdma_hal_ahb_v1.* *libesp_hal_dma.a:gdma_hal_top.* *libesp_hal_gpio.a:gpio_hal.* *libesp_hal_gpspi.a:spi_hal_iram.* *libesp_hal_gpspi.a:spi_slave_hal_iram.* *libesp_hal_mspi.a:spi_flash_encrypt_hal_iram.* *libesp_hal_mspi.a:spi_flash_hal_gpspi.* *libesp_hal_mspi.a:spi_flash_hal_iram.* *libesp_hal_wdt.a:rom_patch.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_clk_tree.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_by_mspi_delay.* *libesp_hw_support.a:mspi_timing_config.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:periph_ctrl.* *libesp_hw_support.a:regi2c_ctrl.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:rtc_init.* *libesp_hw_support.a:rtc_sleep.* *libesp_hw_support.a:rtc_time.* *libesp_hw_support.a:systimer.* *libesp_libc.a:abort.* *libesp_libc.a:assert.* *libesp_libc.a:heap.* *libesp_libc.a:stdatomic.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_netif.a:esp_netif_lwip.* *libesp_netif.a:esp_pbuf_ref.* *libesp_netif.a:ethernetif.* *libesp_netif.a:wlanif.* *libesp_ringbuf.a:ringbuf.* *libesp_rom.a:esp_rom_cache_esp32s2_esp32s3.* *libesp_rom.a:esp_rom_cache_writeback_esp32s3.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_rom.a:esp_rom_systimer.* *libesp_system.a:esp_err.* *libesp_system.a:esp_system_chip.* *libesp_system.a:freertos_hooks.* *libesp_system.a:image_process.* *libesp_system.a:panic.* *libesp_system.a:reset_reason.* *libesp_system.a:system_internal.* *libesp_system.a:system_time.* *libesp_system.a:ubsan.* *libesp_wifi.a:esp_adapter.* *libesp_wifi.a:wifi_netif.* *libfreertos.a:event_groups.* *libfreertos.a:list.* *libfreertos.a:port.* *libfreertos.a:port_systick.* *libfreertos.a:portasm.* *libfreertos.a:queue.* *libfreertos.a:stream_buffer.* *libfreertos.a:tasks.* *libfreertos.a:timers.* *libgcc.a:_divsf3.* *libgcc.a:lib2funcs.* *libhal.a:cache_hal.* *libhal.a:mmu_hal.* *libhal.a:systimer_hal.* *libheap.a:multi_heap.* *libheap.a:tlsf.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp.* *liblog.a:log_timestamp_common.* *liblog.a:log_write.* *liblog.a:tag_log_level.* *liblog.a:util.* *liblwip.a:api_lib.* *liblwip.a:api_msg.* *liblwip.a:etharp.* *liblwip.a:ethernet.* *liblwip.a:inet_chksum.* *liblwip.a:ip4.* *liblwip.a:ip4_addr.* *liblwip.a:lwip_default_hooks.* *liblwip.a:mem.* *liblwip.a:memp.* *liblwip.a:netbuf.* *liblwip.a:pbuf.* *liblwip.a:raw.* *liblwip.a:sockets.* *liblwip.a:sys_arch.* *liblwip.a:tcp.* *liblwip.a:tcp_in.* *liblwip.a:tcp_out.* *liblwip.a:tcpip.* *liblwip.a:udp.* *libsoc.a:lldesc.* *libspi_flash.a:esp_flash_api.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_mxic_opi.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_hpm_enable.* *libspi_flash.a:spi_flash_oct_flash_init.* *libspi_flash.a:spi_flash_os_func_app.* *libspi_flash.a:spi_flash_os_func_noos.* *libspi_flash.a:spi_flash_wrap.*) .text.*)
    *(EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifi0iram EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifi0iram.*)
    *(EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifiextrairam EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifiextrairam.*)
    *(EXCLUDE_FILE(*libpp.a) .wifiorslpiram EXCLUDE_FILE(*libpp.a) .wifiorslpiram.*)
    *(EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifirxiram EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifirxiram.*)
    *(.wifislpiram .wifislpiram.*)
    *(EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifislprxiram EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifislprxiram.*)
    *libesp_driver_dma.a:gdma.*(.literal.do_allocate_gdma_channel .literal.gdma_apply_strategy .literal.gdma_config_transfer .literal.gdma_connect .literal.gdma_del_channel .literal.gdma_del_rx_channel .literal.gdma_del_tx_channel .literal.gdma_disconnect .literal.gdma_get_free_m2m_trig_id_mask .literal.gdma_install_rx_interrupt .literal.gdma_install_tx_interrupt .literal.gdma_new_ahb_channel .literal.gdma_register_rx_event_callbacks .literal.gdma_register_tx_event_callbacks .literal.gdma_set_priority .literal.gdma_try_alloc_group_handle .literal.gdma_try_alloc_pair_handle .literal.gdma_try_free_group_handle .literal.gdma_try_free_pair_handle .text .text.do_allocate_gdma_channel .text.gdma_apply_strategy .text.gdma_config_transfer .text.gdma_connect .text.gdma_del_channel .text.gdma_del_rx_channel .text.gdma_del_tx_channel .text.gdma_disconnect .text.gdma_get_alignment_constraints .text.gdma_get_free_m2m_trig_id_mask .text.gdma_get_group_channel_id .text.gdma_install_rx_interrupt .text.gdma_install_tx_interrupt .text.gdma_new_ahb_channel .text.gdma_register_rx_event_callbacks .text.gdma_register_tx_event_callbacks .text.gdma_set_priority .text.gdma_try_alloc_group_handle .text.gdma_try_alloc_pair_handle .text.gdma_try_free_group_handle .text.gdma_try_free_pair_handle)
    *libesp_event.a:default_event_loop.*(.literal.esp_event_handler_instance_register .literal.esp_event_handler_instance_unregister .literal.esp_event_handler_register .literal.esp_event_handler_unregister .literal.esp_event_loop_create_default .literal.esp_event_loop_delete_default .literal.esp_event_post .text .text.esp_event_handler_instance_register .text.esp_event_handler_instance_unregister .text.esp_event_handler_register .text.esp_event_handler_unregister .text.esp_event_loop_create_default .text.esp_event_loop_delete_default .text.esp_event_post)
    *libesp_event.a:esp_event.*(.literal.base_node_add_handler .literal.base_node_remove_all_handler .literal.base_node_remove_handler .literal.esp_event_handler_instance_register_with .literal.esp_event_handler_instance_unregister_with .literal.esp_event_handler_register_with .literal.esp_event_handler_register_with_internal .literal.esp_event_handler_unregister_with .literal.esp_event_handler_unregister_with_internal .literal.esp_event_loop_create .literal.esp_event_loop_delete .literal.esp_event_loop_run .literal.esp_event_loop_run_task .literal.esp_event_post_to .literal.find_and_unregister_handler .literal.handler_instances_add .literal.handler_instances_remove .literal.handler_instances_remove_all .literal.loop_node_add_handler .literal.loop_node_remove_all_handler .literal.loop_node_remove_handler .literal.loop_remove_handler .text .text.base_node_add_handler .text.base_node_remove_all_handler .text.base_node_remove_handler .text.esp_event_dump .text.esp_event_handler_instance_register_with .text.esp_event_handler_instance_unregister_with .text.esp_event_handler_register_with .text.esp_event_handler_register_with_internal .text.esp_event_handler_unregister_with .text.esp_event_handler_unregister_with_internal .text.esp_event_loop_create .text.esp_event_loop_delete .text.esp_event_loop_run .text.esp_event_loop_run_task .text.esp_event_post_to .text.find_and_unregister_handler .text.handler_execute .text.handler_instances_add .text.handler_instances_remove .text.handler_instances_remove_all .text.loop_node_add_handler .text.loop_node_remove_all_handler .text.loop_node_remove_handler .text.loop_remove_handler)
    *libesp_hal_dma.a:gdma_hal_ahb_v1.*(.literal.gdma_ahb_hal_init .literal.gdma_ahb_hal_set_burst_size .literal.gdma_ahb_hal_set_strategy .literal.gdma_ll_rx_set_burst_size .literal.gdma_ll_tx_set_burst_size .text .text.gdma_ahb_hal_connect_mem .text.gdma_ahb_hal_connect_peri .text.gdma_ahb_hal_disconnect_all .text.gdma_ahb_hal_enable_burst .text.gdma_ahb_hal_enable_intr .text.gdma_ahb_hal_get_intr_status_reg .text.gdma_ahb_hal_init .text.gdma_ahb_hal_set_burst_size .text.gdma_ahb_hal_set_priority .text.gdma_ahb_hal_set_strategy .text.gdma_ll_rx_set_burst_size .text.gdma_ll_tx_set_burst_size)
    *libesp_hal_dma.a:gdma_hal_top.*(.text .text.gdma_hal_connect_mem .text.gdma_hal_connect_peri .text.gdma_hal_deinit .text.gdma_hal_disconnect_all .text.gdma_hal_enable_burst .text.gdma_hal_enable_intr .text.gdma_hal_get_intr_status_reg .text.gdma_hal_set_burst_size .text.gdma_hal_set_priority .text.gdma_hal_set_strategy)
    *libesp_hal_gpio.a:gpio_hal.*(.literal.gpio_hal_intr_disable .literal.gpio_hal_intr_enable_on_core .literal.gpio_hal_iomux_in .literal.gpio_hal_iomux_out .literal.gpio_hal_matrix_in .literal.gpio_hal_matrix_out .text .text.gpio_hal_intr_disable .text.gpio_hal_intr_enable_on_core .text.gpio_hal_iomux_in .text.gpio_hal_iomux_out .text.gpio_hal_matrix_in .text.gpio_hal_matrix_out)
    *libesp_hw_support.a:cpu.*(.literal.esp_cpu_set_watchpoint .text .text.esp_cpu_clear_breakpoint .text.esp_cpu_clear_watchpoint .text.esp_cpu_set_breakpoint .text.esp_cpu_set_watchpoint)
    *libesp_hw_support.a:esp_clk_tree.*(.literal.esp_clk_tree_src_get_freq_hz .text .text.esp_clk_tree_initialize .text.esp_clk_tree_src_get_freq_hz)
    *libesp_hw_support.a:periph_ctrl.*(.literal.periph_ll_disable_clk_set_rst .literal.periph_ll_enable_clk_clear_rst .literal.periph_ll_get_clk_en_mask .literal.periph_ll_get_rst_en_mask .literal.periph_ll_reset .literal.periph_module_disable .literal.periph_module_enable .text .text.periph_ll_disable_clk_set_rst .text.periph_ll_enable_clk_clear_rst .text.periph_ll_get_clk_en_mask .text.periph_ll_get_rst_en_mask .text.periph_ll_reset .text.periph_module_disable .text.periph_module_enable)
    *libesp_hw_support.a:regi2c_ctrl.*(.literal.regi2c_saradc_disable .literal.regi2c_saradc_enable .text .text.regi2c_saradc_disable .text.regi2c_saradc_enable)
    *libesp_hw_support.a:rtc_init.*(.literal.calibrate_ocode .literal.get_dig1v3_dbias_by_efuse .literal.get_dig_dbias_by_efuse .literal.get_rtc_dbias_by_efuse .literal.rtc_init .literal.rtc_set_stored_dbias .literal.set_ocode_by_efuse .text .text.calibrate_ocode .text.get_dig1v3_dbias_by_efuse .text.get_dig_dbias_by_efuse .text.get_rtc_dbias_by_efuse .text.rtc_init .text.rtc_set_stored_dbias .text.set_ocode_by_efuse)
    *libesp_hw_support.a:rtc_sleep.*(.literal.rtc_sleep_get_default_config .literal.rtc_sleep_init .literal.rtc_sleep_low_init .text .text.rtc_sleep_get_default_config .text.rtc_sleep_init .text.rtc_sleep_low_init)
    *libesp_netif.a:esp_netif_lwip.*(.literal.dns_clear_servers .literal.esp_ip4addr_aton .literal.esp_ip4addr_ntoa .literal.esp_netif_add_ip6_address .literal.esp_netif_add_ip6_address_api .literal.esp_netif_add_ip6_address_priv .literal.esp_netif_api_cb .literal.esp_netif_attach .literal.esp_netif_config_sanity_check .literal.esp_netif_create_ip6_linklocal .literal.esp_netif_create_ip6_linklocal_api .literal.esp_netif_deinit .literal.esp_netif_destroy .literal.esp_netif_destroy_api .literal.esp_netif_dhcpc_option .literal.esp_netif_dhcpc_option_api .literal.esp_netif_dhcpc_start .literal.esp_netif_dhcpc_start_api .literal.esp_netif_dhcpc_stop .literal.esp_netif_dhcpc_stop_api .literal.esp_netif_dhcps_cb .literal.esp_netif_dhcps_get_clients_by_mac .literal.esp_netif_dhcps_get_clients_by_mac_api .literal.esp_netif_dhcps_option .literal.esp_netif_dhcps_option_api .literal.esp_netif_dhcps_start .literal.esp_netif_dhcps_start_api .literal.esp_netif_dhcps_stop .literal.esp_netif_dhcps_stop_api .literal.esp_netif_down .literal.esp_netif_down_api .literal.esp_netif_find_if .literal.esp_netif_find_if_api .literal.esp_netif_get_all_ip6 .literal.esp_netif_get_all_preferred_ip6 .literal.esp_netif_get_default_netif .literal.esp_netif_get_dns_info .literal.esp_netif_get_dns_info_api .literal.esp_netif_get_handle_from_ifkey .literal.esp_netif_get_hostname .literal.esp_netif_get_ip6_global .literal.esp_netif_get_ip6_linklocal .literal.esp_netif_get_ip_info .literal.esp_netif_get_mac .literal.esp_netif_get_mtu .literal.esp_netif_get_mtu_api .literal.esp_netif_get_netif_impl_name .literal.esp_netif_get_netif_impl_name_api .literal.esp_netif_get_old_ip_info .literal.esp_netif_init .literal.esp_netif_init_configuration .literal.esp_netif_internal_dhcpc_cb .literal.esp_netif_internal_nd6_cb .literal.esp_netif_ip6_get_addr_type .literal.esp_netif_ip_lost_timer .literal.esp_netif_is_active .literal.esp_netif_join_ip6_multicast_group .literal.esp_netif_join_ip6_multicast_group_api .literal.esp_netif_leave_ip6_multicast_group .literal.esp_netif_leave_ip6_multicast_group_api .literal.esp_netif_lwip_add .literal.esp_netif_lwip_ipc_call .literal.esp_netif_lwip_ipc_call_fn .literal.esp_netif_lwip_ipc_call_get_netif .literal.esp_netif_lwip_ipc_call_msg .literal.esp_netif_lwip_remove .literal.esp_netif_netstack_buf_free .literal.esp_netif_netstack_buf_ref .literal.esp_netif_new .literal.esp_netif_new_api .literal.esp_netif_ppp_set_auth .literal.esp_netif_ppp_set_auth_api .literal.esp_netif_remove_ip6_address .literal.esp_netif_remove_ip6_address_api .literal.esp_netif_set_default_netif .literal.esp_netif_set_default_netif_internal .literal.esp_netif_set_dns_info .literal.esp_netif_set_dns_info_api .literal.esp_netif_set_driver_config .literal.esp_netif_set_hostname .literal.esp_netif_set_hostname_api .literal.esp_netif_set_ip_info .literal.esp_netif_set_ip_info_api .literal.esp_netif_set_ip_old_info_api .literal.esp_netif_set_mac .literal.esp_netif_set_mac_api .literal.esp_netif_set_mtu .literal.esp_netif_set_old_ip_info .literal.esp_netif_start .literal.esp_netif_start_api .literal.esp_netif_start_ip_lost_timer .literal.esp_netif_stop .literal.esp_netif_stop_api .literal.esp_netif_str_to_ip4 .literal.esp_netif_str_to_ip6 .literal.esp_netif_tcpip_exec .literal.esp_netif_transmit .literal.esp_netif_tx_rx_event_api .literal.esp_netif_tx_rx_event_disable .literal.esp_netif_tx_rx_event_enable .literal.esp_netif_up .literal.esp_netif_up_api .literal.esp_netif_update_default_netif .literal.esp_netif_update_default_netif_lwip .literal.get_handle_from_ifkey_api .literal.netif_callback_fn .literal.netif_igmp_mac_filter_cb .literal.netif_mld_mac_filter_cb .literal.netif_send_garp .literal.netif_send_mldv6 .literal.netif_set_garp_flag .literal.netif_set_mldv6_flag .literal.netif_unset_garp_flag .literal.netif_unset_mldv6_flag .literal.tcpip_init_done .text .text.dns_clear_servers .text.esp_ip4addr_aton .text.esp_ip4addr_ntoa .text.esp_netif_add_ip6_address .text.esp_netif_add_ip6_address_api .text.esp_netif_add_ip6_address_priv .text.esp_netif_api_cb .text.esp_netif_attach .text.esp_netif_config_sanity_check .text.esp_netif_create_ip6_linklocal .text.esp_netif_create_ip6_linklocal_api .text.esp_netif_deinit .text.esp_netif_destroy .text.esp_netif_destroy_api .text.esp_netif_dhcpc_get_status .text.esp_netif_dhcpc_option .text.esp_netif_dhcpc_option_api .text.esp_netif_dhcpc_start .text.esp_netif_dhcpc_start_api .text.esp_netif_dhcpc_stop .text.esp_netif_dhcpc_stop_api .text.esp_netif_dhcps_cb .text.esp_netif_dhcps_get_clients_by_mac .text.esp_netif_dhcps_get_clients_by_mac_api .text.esp_netif_dhcps_get_status .text.esp_netif_dhcps_option .text.esp_netif_dhcps_option_api .text.esp_netif_dhcps_start .text.esp_netif_dhcps_start_api .text.esp_netif_dhcps_stop .text.esp_netif_dhcps_stop_api .text.esp_netif_down .text.esp_netif_down_api .text.esp_netif_find_if .text.esp_netif_find_if_api .text.esp_netif_get_all_ip6 .text.esp_netif_get_all_preferred_ip6 .text.esp_netif_get_default_netif .text.esp_netif_get_desc .text.esp_netif_get_dns_info .text.esp_netif_get_dns_info_api .text.esp_netif_get_event_id .text.esp_netif_get_flags .text.esp_netif_get_handle_from_ifkey .text.esp_netif_get_handle_from_netif_impl .text.esp_netif_get_hostname .text.esp_netif_get_ifkey .text.esp_netif_get_io_driver .text.esp_netif_get_ip6_global .text.esp_netif_get_ip6_linklocal .text.esp_netif_get_ip_info .text.esp_netif_get_mac .text.esp_netif_get_mtu .text.esp_netif_get_mtu_api .text.esp_netif_get_netif_impl .text.esp_netif_get_netif_impl_index .text.esp_netif_get_netif_impl_name .text.esp_netif_get_netif_impl_name_api .text.esp_netif_get_old_ip_info .text.esp_netif_get_route_prio .text.esp_netif_init .text.esp_netif_init_configuration .text.esp_netif_internal_dhcpc_cb .text.esp_netif_internal_nd6_cb .text.esp_netif_ip6_get_addr_type .text.esp_netif_ip_lost_timer .text.esp_netif_is_active .text.esp_netif_is_netif_up .text.esp_netif_is_valid_static_ip .text.esp_netif_join_ip6_multicast_group .text.esp_netif_join_ip6_multicast_group_api .text.esp_netif_leave_ip6_multicast_group .text.esp_netif_leave_ip6_multicast_group_api .text.esp_netif_lwip_add .text.esp_netif_lwip_ipc_call .text.esp_netif_lwip_ipc_call_fn .text.esp_netif_lwip_ipc_call_get_netif .text.esp_netif_lwip_ipc_call_msg .text.esp_netif_lwip_remove .text.esp_netif_napt_disable .text.esp_netif_napt_enable .text.esp_netif_netstack_buf_free .text.esp_netif_netstack_buf_ref .text.esp_netif_new .text.esp_netif_new_api .text.esp_netif_ppp_set_auth .text.esp_netif_ppp_set_auth_api .text.esp_netif_remove_ip6_address .text.esp_netif_remove_ip6_address_api .text.esp_netif_reset_ip_info .text.esp_netif_set_default_netif .text.esp_netif_set_default_netif_internal .text.esp_netif_set_dns_info .text.esp_netif_set_dns_info_api .text.esp_netif_set_driver_config .text.esp_netif_set_hostname .text.esp_netif_set_hostname_api .text.esp_netif_set_ip4_addr .text.esp_netif_set_ip_info .text.esp_netif_set_ip_info_api .text.esp_netif_set_ip_old_info_api .text.esp_netif_set_link_speed .text.esp_netif_set_mac .text.esp_netif_set_mac_api .text.esp_netif_set_mtu .text.esp_netif_set_mtu_api .text.esp_netif_set_old_ip_info .text.esp_netif_set_route_prio .text.esp_netif_start .text.esp_netif_start_api .text.esp_netif_start_ip_lost_timer .text.esp_netif_stop .text.esp_netif_stop_api .text.esp_netif_str_to_ip4 .text.esp_netif_str_to_ip6 .text.esp_netif_tcpip_exec .text.esp_netif_transmit .text.esp_netif_tx_rx_event_api .text.esp_netif_tx_rx_event_disable .text.esp_netif_tx_rx_event_enable .text.esp_netif_up .text.esp_netif_up_api .text.esp_netif_update_default_netif .text.esp_netif_update_default_netif_lwip .text.get_handle_from_ifkey_api .text.netif_callback_fn .text.netif_igmp_mac_filter_cb .text.netif_mld_mac_filter_cb .text.netif_send_garp .text.netif_send_mldv6 .text.netif_set_garp_flag .text.netif_set_mldv6_flag .text.netif_unset_garp_flag .text.netif_unset_mldv6_flag .text.tcpip_exec_api .text.tcpip_init_done)
    *libesp_netif.a:esp_pbuf_ref.*(.text)
    *libesp_netif.a:ethernetif.*(.literal.ethernetif_init .text .text.ethernet_low_level_init .text.ethernetif_init)
    *libesp_netif.a:wlanif.*(.literal.wlanif_init .literal.wlanif_init_ap .literal.wlanif_init_nan .literal.wlanif_init_sta .text .text.low_level_init .text.wlanif_init .text.wlanif_init_ap .text.wlanif_init_nan .text.wlanif_init_sta)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvGetFreeSize .literal.prvInitializeNewRingbuffer .literal.prvReceiveGeneric .literal.prvSendAcquireGeneric .literal.vRingbufferDelete .literal.vRingbufferDeleteWithCaps .literal.vRingbufferGetInfo .literal.vRingbufferReset .literal.vRingbufferReturnItem .literal.xRingbufferAddToQueueSetRead .literal.xRingbufferCreate .literal.xRingbufferCreateNoSplit .literal.xRingbufferCreateStatic .literal.xRingbufferCreateWithCaps .literal.xRingbufferGetCurFreeSize .literal.xRingbufferGetMaxItemSize .literal.xRingbufferGetStaticBuffer .literal.xRingbufferPrintInfo .literal.xRingbufferReceive .literal.xRingbufferReceiveSplit .literal.xRingbufferReceiveUpTo .literal.xRingbufferRemoveFromQueueSetRead .literal.xRingbufferSend .literal.xRingbufferSendAcquire .literal.xRingbufferSendComplete .text .text.prvGetCurMaxSizeAllowSplit .text.prvGetCurMaxSizeByteBuf .text.prvGetCurMaxSizeNoSplit .text.prvGetFreeSize .text.prvInitializeNewRingbuffer .text.prvReceiveGeneric .text.prvSendAcquireGeneric .text.vRingbufferDelete .text.vRingbufferDeleteWithCaps .text.vRingbufferGetInfo .text.vRingbufferReset .text.vRingbufferReturnItem .text.xRingbufferAddToQueueSetRead .text.xRingbufferCreate .text.xRingbufferCreateNoSplit .text.xRingbufferCreateStatic .text.xRingbufferCreateWithCaps .text.xRingbufferGetCurFreeSize .text.xRingbufferGetMaxItemSize .text.xRingbufferGetStaticBuffer .text.xRingbufferPrintInfo .text.xRingbufferReceive .text.xRingbufferReceiveSplit .text.xRingbufferReceiveUpTo .text.xRingbufferRemoveFromQueueSetRead .text.xRingbufferSend .text.xRingbufferSendAcquire .text.xRingbufferSendComplete)
    *libesp_system.a:esp_system_chip.*(.literal.esp_get_free_heap_size .literal.esp_get_free_internal_heap_size .literal.esp_get_idf_version .literal.esp_get_minimum_free_heap_size .text .text.esp_get_free_heap_size .text.esp_get_free_internal_heap_size .text.esp_get_idf_version .text.esp_get_minimum_free_heap_size)
    *libesp_system.a:freertos_hooks.*(.literal.esp_deregister_freertos_idle_hook .literal.esp_deregister_freertos_idle_hook_for_cpu .literal.esp_deregister_freertos_tick_hook .literal.esp_deregister_freertos_tick_hook_for_cpu .literal.esp_register_freertos_idle_hook .literal.esp_register_freertos_idle_hook_for_cpu .literal.esp_register_freertos_tick_hook .literal.esp_register_freertos_tick_hook_for_cpu .literal.esp_vApplicationIdleHook .text .text.esp_deregister_freertos_idle_hook .text.esp_deregister_freertos_idle_hook_for_cpu .text.esp_deregister_freertos_tick_hook .text.esp_deregister_freertos_tick_hook_for_cpu .text.esp_register_freertos_idle_hook .text.esp_register_freertos_idle_hook_for_cpu .text.esp_register_freertos_tick_hook .text.esp_register_freertos_tick_hook_for_cpu .text.esp_vApplicationIdleHook)
    *libesp_system.a:panic.*(.literal.disable_all_wdts .literal.esp_panic_handler .literal.esp_panic_handler_disable_timg_wdts .literal.esp_panic_handler_enable_rtc_wdt .literal.esp_panic_handler_feed_wdts .literal.esp_panic_handler_increment_entry_count .literal.panic_print_char .literal.panic_print_char_uart .literal.panic_print_dec .literal.panic_print_hex .literal.panic_print_str .literal.print_abort_details .text .text.disable_all_wdts .text.esp_panic_handler .text.esp_panic_handler_disable_timg_wdts .text.esp_panic_handler_enable_rtc_wdt .text.esp_panic_handler_feed_wdts .text.esp_panic_handler_increment_entry_count .text.esp_reset_reason_get_hint .text.esp_reset_reason_set_hint .text.panic_print_char .text.panic_print_char_uart .text.panic_print_dec .text.panic_print_hex .text.panic_print_str .text.print_abort_details)
    *libesp_system.a:reset_reason.*(.literal.esp_reset_reason .literal.esp_reset_reason_clear_hint .literal.esp_reset_reason_get_hint .literal.esp_reset_reason_init .text .text.esp_reset_reason .text.esp_reset_reason_clear_hint .text.esp_reset_reason_get_hint .text.esp_reset_reason_init .text.get_reset_reason)
    *libesp_system.a:system_internal.*(.text)
    *libesp_system.a:system_time.*(.text)
    *libesp_wifi.a:esp_adapter.*(.literal.esp_cpu_intr_disable .literal.esp_cpu_intr_enable .literal.esp_event_post_wrapper .literal.esp_phy_disable_wrapper .literal.esp_phy_enable_wrapper .literal.event_group_wait_bits_wrapper .literal.get_time_wrapper .literal.mutex_create_wrapper .literal.mutex_delete_wrapper .literal.queue_create_wrapper .literal.queue_delete_wrapper .literal.queue_recv_wrapper .literal.queue_send_to_back_wrapper .literal.queue_send_to_front_wrapper .literal.queue_send_wrapper .literal.recursive_mutex_create_wrapper .literal.set_intr_wrapper .literal.set_isr_wrapper .literal.task_create_pinned_to_core_wrapper .literal.task_create_wrapper .literal.wifi_clock_disable_wrapper .literal.wifi_clock_enable_wrapper .literal.wifi_create_queue .literal.wifi_create_queue_wrapper .literal.wifi_delete_queue .literal.wifi_delete_queue_wrapper .literal.wifi_reset_mac_wrapper .literal.wifi_thread_semphr_free .literal.wifi_thread_semphr_get_wrapper .text .text.clear_intr_wrapper .text.coex_deinit_wrapper .text.coex_disable_wrapper .text.coex_enable_wrapper .text.coex_init_wrapper .text.coex_register_start_cb_wrapper .text.coex_schm_curr_period_get_wrapper .text.coex_schm_curr_phase_get_wrapper .text.coex_schm_flexible_period_get_wrapper .text.coex_schm_flexible_period_set_wrapper .text.coex_schm_get_phase_by_idx_wrapper .text.coex_schm_interval_get_wrapper .text.coex_schm_process_restart_wrapper .text.coex_schm_register_cb_wrapper .text.coex_schm_status_bit_clear_wrapper .text.coex_schm_status_bit_set_wrapper .text.coex_wifi_channel_set_wrapper .text.coex_wifi_request_wrapper .text.esp_cpu_intr_disable .text.esp_cpu_intr_enable .text.esp_event_post_wrapper .text.esp_phy_disable_wrapper .text.esp_phy_enable_wrapper .text.event_group_wait_bits_wrapper .text.get_time_wrapper .text.mutex_create_wrapper .text.mutex_delete_wrapper .text.queue_create_wrapper .text.queue_delete_wrapper .text.queue_recv_wrapper .text.queue_send_to_back_wrapper .text.queue_send_to_front_wrapper .text.queue_send_wrapper .text.recursive_mutex_create_wrapper .text.set_intr_wrapper .text.set_isr_wrapper .text.task_create_pinned_to_core_wrapper .text.task_create_wrapper .text.task_get_max_priority_wrapper .text.wifi_clock_disable_wrapper .text.wifi_clock_enable_wrapper .text.wifi_create_queue .text.wifi_create_queue_wrapper .text.wifi_delete_queue .text.wifi_delete_queue_wrapper .text.wifi_reset_mac_wrapper .text.wifi_thread_semphr_free .text.wifi_thread_semphr_get_wrapper)
    *libesp_wifi.a:wifi_netif.*(.literal.esp_wifi_create_if_driver .literal.esp_wifi_destroy_if_driver .literal.esp_wifi_get_if_mac .literal.esp_wifi_register_if_rxcb .literal.wifi_ap_receive .literal.wifi_driver_start .literal.wifi_free .literal.wifi_transmit .text .text.esp_wifi_create_if_driver .text.esp_wifi_destroy_if_driver .text.esp_wifi_get_if_mac .text.esp_wifi_is_if_ready_when_started .text.esp_wifi_register_if_rxcb .text.wifi_ap_receive .text.wifi_driver_start .text.wifi_free .text.wifi_transmit)
    *libfreertos.a:event_groups.*(.literal.vEventGroupClearBitsCallback .literal.vEventGroupDelete .literal.vEventGroupSetBitsCallback .literal.xEventGroupClearBits .literal.xEventGroupCreate .literal.xEventGroupCreateStatic .literal.xEventGroupGetStaticBuffer .literal.xEventGroupSetBits .literal.xEventGroupSync .literal.xEventGroupWaitBits .text .text.prvTestWaitCondition .text.vEventGroupClearBitsCallback .text.vEventGroupDelete .text.vEventGroupSetBitsCallback .text.xEventGroupClearBits .text.xEventGroupCreate .text.xEventGroupCreateStatic .text.xEventGroupGetStaticBuffer .text.xEventGroupSetBits .text.xEventGroupSync .text.xEventGroupWaitBits)
    *libfreertos.a:list.*(.text .text.vListInitialise .text.vListInitialiseItem .text.vListInsert)
    *libfreertos.a:port.*(.literal.pxPortInitialiseStack .literal.vApplicationStackOverflowHook .literal.vPortCleanUpCoprocArea .literal.vPortEndScheduler .literal.vPortExitCriticalCompliance .literal.vPortTCBPreDeleteHook .literal.vPortTLSPointersDelCb .literal.vPortTaskWrapper .literal.xPortEnterCriticalTimeoutCompliance .text .text.pxPortInitialiseStack .text.vApplicationStackOverflowHook .text.vPortCleanUpCoprocArea .text.vPortEndScheduler .text.vPortExitCriticalCompliance .text.vPortTCBPreDeleteHook .text.vPortTLSPointersDelCb .text.vPortTaskWrapper .text.xPortEnterCriticalTimeoutCompliance .text.xPortGetTickRateHz)
    *libfreertos.a:queue.*(.literal.prvInitialiseMutex .literal.prvInitialiseNewQueue .literal.uxQueueMessagesWaiting .literal.uxQueueSpacesAvailable .literal.vQueueDelete .literal.vQueueWaitForMessageRestricted .literal.xQueueAddToSet .literal.xQueueCreateCountingSemaphore .literal.xQueueCreateCountingSemaphoreStatic .literal.xQueueCreateMutex .literal.xQueueCreateMutexStatic .literal.xQueueCreateSet .literal.xQueueGenericCreate .literal.xQueueGenericCreateStatic .literal.xQueueGenericGetStaticBuffers .literal.xQueueGenericReset .literal.xQueueGenericSend .literal.xQueueGetMutexHolder .literal.xQueueGiveMutexRecursive .literal.xQueuePeek .literal.xQueueReceive .literal.xQueueRemoveFromSet .literal.xQueueSelectFromSet .literal.xQueueSemaphoreTake .literal.xQueueTakeMutexRecursive .text .text.prvGetDisinheritPriorityAfterTimeout .text.prvInitialiseMutex .text.prvInitialiseNewQueue .text.uxQueueMessagesWaiting .text.uxQueueSpacesAvailable .text.vQueueDelete .text.vQueueWaitForMessageRestricted .text.xQueueAddToSet .text.xQueueCreateCountingSemaphore .text.xQueueCreateCountingSemaphoreStatic .text.xQueueCreateMutex .text.xQueueCreateMutexStatic .text.xQueueCreateSet .text.xQueueGenericCreate .text.xQueueGenericCreateStatic .text.xQueueGenericGetStaticBuffers .text.xQueueGenericReset .text.xQueueGenericSend .text.xQueueGetMutexHolder .text.xQueueGiveMutexRecursive .text.xQueuePeek .text.xQueueReceive .text.xQueueRemoveFromSet .text.xQueueSelectFromSet .text.xQueueSemaphoreTake .text.xQueueTakeMutexRecursive)
    *libfreertos.a:stream_buffer.*(.literal.prvInitialiseNewStreamBuffer .literal.prvReadBytesFromBuffer .literal.prvWriteBytesToBuffer .literal.vStreamBufferDelete .literal.xStreamBufferBytesAvailable .literal.xStreamBufferGenericCreate .literal.xStreamBufferGenericCreateStatic .literal.xStreamBufferGetStaticBuffers .literal.xStreamBufferIsEmpty .literal.xStreamBufferIsFull .literal.xStreamBufferNextMessageLengthBytes .literal.xStreamBufferReceive .literal.xStreamBufferReset .literal.xStreamBufferSend .literal.xStreamBufferSetTriggerLevel .literal.xStreamBufferSpacesAvailable .text .text.prvInitialiseNewStreamBuffer .text.prvReadBytesFromBuffer .text.prvWriteBytesToBuffer .text.vStreamBufferDelete .text.xStreamBufferBytesAvailable .text.xStreamBufferGenericCreate .text.xStreamBufferGenericCreateStatic .text.xStreamBufferGetStaticBuffers .text.xStreamBufferIsEmpty .text.xStreamBufferIsFull .text.xStreamBufferNextMessageLengthBytes .text.xStreamBufferReceive .text.xStreamBufferReset .text.xStreamBufferSend .text.xStreamBufferSetTriggerLevel .text.xStreamBufferSpacesAvailable)
    *libfreertos.a:tasks.*(.literal.eTaskGetState .literal.pcTaskGetName .literal.prvAddCurrentTaskToDelayedList .literal.prvAddNewTaskToReadyList .literal.prvCheckTasksWaitingTermination .literal.prvCreateIdleTasks .literal.prvDeleteTCB .literal.prvIdleTask .literal.prvInitialiseNewTask .literal.prvInitialiseTaskLists .literal.prvReleaseKernelLock .literal.prvTakeKernelLock .literal.prvTaskPriorityRaise .literal.prvTaskPriorityRestore .literal.pvTaskGetCurrentTCBForCore .literal.pvTaskGetThreadLocalStoragePointer .literal.pvTaskIncrementMutexHeldCount .literal.pxGetTaskListByIndex .literal.pxTaskGetStackStart .literal.ulTaskGenericNotifyTake .literal.ulTaskGenericNotifyValueClear .literal.uxTaskGetNumberOfTasks .literal.uxTaskGetSnapshotAll .literal.uxTaskGetStackHighWaterMark .literal.uxTaskGetStackHighWaterMark2 .literal.uxTaskPriorityGet .literal.uxTaskResetEventItemValue .literal.vTaskDelay .literal.vTaskDelete .literal.vTaskEndScheduler .literal.vTaskInternalSetTimeOutState .literal.vTaskMissedYield .literal.vTaskPlaceOnEventList .literal.vTaskPlaceOnEventListRestricted .literal.vTaskPlaceOnUnorderedEventList .literal.vTaskPriorityDisinheritAfterTimeout .literal.vTaskPrioritySet .literal.vTaskRemoveFromUnorderedEventList .literal.vTaskResume .literal.vTaskSetThreadLocalStoragePointer .literal.vTaskSetThreadLocalStoragePointerAndDelCallback .literal.vTaskSetTimeOutState .literal.vTaskStartScheduler .literal.vTaskSuspend .literal.vTaskSuspendAll .literal.xTaskAbortDelay .literal.xTaskCatchUpTicks .literal.xTaskCheckForTimeOut .literal.xTaskCreatePinnedToCore .literal.xTaskCreateStaticPinnedToCore .literal.xTaskDelayUntil .literal.xTaskGenericNotify .literal.xTaskGenericNotifyStateClear .literal.xTaskGenericNotifyWait .literal.xTaskGetCoreID .literal.xTaskGetCurrentTaskHandle .literal.xTaskGetCurrentTaskHandleForCore .literal.xTaskGetHandle .literal.xTaskGetIdleTaskHandle .literal.xTaskGetIdleTaskHandleForCore .literal.xTaskGetNext .literal.xTaskGetStackStart .literal.xTaskGetStaticBuffers .literal.xTaskPriorityDisinherit .literal.xTaskPriorityInherit .literal.xTaskResumeAll .text .text.eTaskGetState .text.pcTaskGetName .text.prvAddCurrentTaskToDelayedList .text.prvAddNewTaskToReadyList .text.prvCheckTasksWaitingTermination .text.prvCreateIdleTasks .text.prvDeleteTCB .text.prvIdleTask .text.prvInitialiseNewTask .text.prvInitialiseTaskLists .text.prvReleaseKernelLock .text.prvSearchForNameWithinSingleList .text.prvTakeKernelLock .text.prvTaskCheckFreeStackSpace .text.prvTaskPriorityRaise .text.prvTaskPriorityRestore .text.pvTaskGetCurrentTCBForCore .text.pvTaskGetThreadLocalStoragePointer .text.pvTaskIncrementMutexHeldCount .text.pxGetTaskListByIndex .text.pxTaskGetStackStart .text.ulTaskGenericNotifyTake .text.ulTaskGenericNotifyValueClear .text.uxTaskGetNumberOfTasks .text.uxTaskGetSnapshotAll .text.uxTaskGetStackHighWaterMark .text.uxTaskGetStackHighWaterMark2 .text.uxTaskPriorityGet .text.uxTaskResetEventItemValue .text.vTaskDelay .text.vTaskDelete .text.vTaskEndScheduler .text.vTaskInternalSetTimeOutState .text.vTaskMissedYield .text.vTaskPlaceOnEventList .text.vTaskPlaceOnEventListRestricted .text.vTaskPlaceOnUnorderedEventList .text.vTaskPriorityDisinheritAfterTimeout .text.vTaskPrioritySet .text.vTaskRemoveFromUnorderedEventList .text.vTaskResume .text.vTaskSetThreadLocalStoragePointer .text.vTaskSetThreadLocalStoragePointerAndDelCallback .text.vTaskSetTimeOutState .text.vTaskStartScheduler .text.vTaskSuspend .text.vTaskSuspendAll .text.xTaskAbortDelay .text.xTaskCatchUpTicks .text.xTaskCheckForTimeOut .text.xTaskCreatePinnedToCore .text.xTaskCreateStaticPinnedToCore .text.xTaskDelayUntil .text.xTaskGenericNotify .text.xTaskGenericNotifyStateClear .text.xTaskGenericNotifyWait .text.xTaskGetCoreID .text.xTaskGetCurrentTaskHandle .text.xTaskGetCurrentTaskHandleForCore .text.xTaskGetHandle .text.xTaskGetIdleTaskHandle .text.xTaskGetIdleTaskHandleForCore .text.xTaskGetNext .text.xTaskGetStackStart .text.xTaskGetStaticBuffers .text.xTaskPriorityDisinherit .text.xTaskPriorityInherit .text.xTaskResumeAll .text.xTimerCreateTimerTask)
    *libfreertos.a:timers.*(.literal.pcTimerGetName .literal.prvCheckForValidListAndQueue .literal.prvGetNextExpireTime .literal.prvInitialiseNewTimer .literal.prvInsertTimerInActiveList .literal.prvProcessExpiredTimer .literal.prvProcessReceivedCommands .literal.prvProcessTimerOrBlockTask .literal.prvReloadTimer .literal.prvSampleTimeNow .literal.prvSwitchTimerLists .literal.prvTimerTask .literal.pvTimerGetTimerID .literal.uxTimerGetReloadMode .literal.vTimerSetReloadMode .literal.vTimerSetTimerID .literal.xTimerCreate .literal.xTimerCreateStatic .literal.xTimerCreateTimerTask .literal.xTimerGetExpiryTime .literal.xTimerGetPeriod .literal.xTimerGetReloadMode .literal.xTimerGetStaticBuffer .literal.xTimerGetTimerDaemonTaskHandle .literal.xTimerIsTimerActive .literal.xTimerPendFunctionCall .text .text.pcTimerGetName .text.prvCheckForValidListAndQueue .text.prvGetNextExpireTime .text.prvInitialiseNewTimer .text.prvInsertTimerInActiveList .text.prvProcessExpiredTimer .text.prvProcessReceivedCommands .text.prvProcessTimerOrBlockTask .text.prvReloadTimer .text.prvSampleTimeNow .text.prvSwitchTimerLists .text.prvTimerTask .text.pvTimerGetTimerID .text.uxTimerGetReloadMode .text.vTimerSetReloadMode .text.vTimerSetTimerID .text.xTimerCreate .text.xTimerCreateStatic .text.xTimerCreateTimerTask .text.xTimerGetExpiryTime .text.xTimerGetPeriod .text.xTimerGetReloadMode .text.xTimerGetStaticBuffer .text.xTimerGetTimerDaemonTaskHandle .text.xTimerIsTimerActive .text.xTimerPendFunctionCall)
    *libheap.a:multi_heap.*(.literal.multi_heap_check .literal.multi_heap_dump .literal.multi_heap_dump_tlsf .literal.multi_heap_find_containing_block_impl .literal.multi_heap_get_info_impl .literal.multi_heap_register_impl .literal.multi_heap_reset_minimum_free_bytes .literal.multi_heap_restore_minimum_free_bytes .literal.multi_heap_walk .text .text.multi_heap_check .text.multi_heap_dump .text.multi_heap_dump_tlsf .text.multi_heap_find_containing_block_impl .text.multi_heap_free_size_impl .text.multi_heap_get_info_impl .text.multi_heap_get_info_tlsf .text.multi_heap_minimum_free_size_impl .text.multi_heap_register_impl .text.multi_heap_reset_minimum_free_bytes .text.multi_heap_restore_minimum_free_bytes .text.multi_heap_walk)
    *libheap.a:tlsf.*(.literal.control_construct .literal.default_walker .literal.integrity_walker .literal.tlsf_add_pool .literal.tlsf_check .literal.tlsf_check_pool .literal.tlsf_create .literal.tlsf_create_with_pool .literal.tlsf_fit_size .literal.tlsf_malloc_addr .literal.tlsf_remove_pool .literal.tlsf_walk_pool .text .text.control_construct .text.default_walker .text.integrity_walker .text.tlsf_add_pool .text.tlsf_check .text.tlsf_check_pool .text.tlsf_create .text.tlsf_create_with_pool .text.tlsf_destroy .text.tlsf_find_containing_block .text.tlsf_fit_size .text.tlsf_malloc_addr .text.tlsf_pool_overhead .text.tlsf_remove_pool .text.tlsf_walk_pool)
    *liblog.a:log_timestamp.*(.text)
    *liblog.a:log_write.*(.literal.esp_log_set_vprintf .text .text.esp_log_set_vprintf)
    *liblog.a:tag_log_level.*(.literal.esp_log_level_get .literal.esp_log_level_set .literal.log_level_get .literal.log_level_set .text .text.esp_log_level_get .text.esp_log_level_set .text.log_level_get .text.log_level_set)
    *liblwip.a:api_lib.*(.literal.netconn_accept .literal.netconn_bind .literal.netconn_bind_if .literal.netconn_close .literal.netconn_close_shutdown .literal.netconn_connect .literal.netconn_delete .literal.netconn_disconnect .literal.netconn_err .literal.netconn_getaddr .literal.netconn_gethostbyname_addrtype .literal.netconn_join_leave_group .literal.netconn_join_leave_group_netif .literal.netconn_listen_with_backlog .literal.netconn_new_with_proto_and_callback .literal.netconn_prepare_delete .literal.netconn_recv_tcp_pbuf .literal.netconn_recv_udp_raw_netbuf .literal.netconn_shutdown .literal.netconn_thread_cleanup .literal.netconn_thread_init .text .text.netconn_accept .text.netconn_bind .text.netconn_bind_if .text.netconn_close .text.netconn_close_shutdown .text.netconn_connect .text.netconn_delete .text.netconn_disconnect .text.netconn_err .text.netconn_getaddr .text.netconn_gethostbyname_addrtype .text.netconn_join_leave_group .text.netconn_join_leave_group_netif .text.netconn_listen_with_backlog .text.netconn_new_with_proto_and_callback .text.netconn_prepare_delete .text.netconn_recv_tcp_pbuf .text.netconn_recv_udp_raw_netbuf .text.netconn_shutdown .text.netconn_thread_cleanup .text.netconn_thread_init)
    *liblwip.a:api_msg.*(.literal.accept_function .literal.err_tcp .literal.lwip_netconn_do_accepted .literal.lwip_netconn_do_bind .literal.lwip_netconn_do_bind_if .literal.lwip_netconn_do_close .literal.lwip_netconn_do_close_internal .literal.lwip_netconn_do_connect .literal.lwip_netconn_do_connected .literal.lwip_netconn_do_delconn .literal.lwip_netconn_do_disconnect .literal.lwip_netconn_do_dns_found .literal.lwip_netconn_do_getaddr .literal.lwip_netconn_do_gethostbyname .literal.lwip_netconn_do_join_leave_group .literal.lwip_netconn_do_join_leave_group_netif .literal.lwip_netconn_do_listen .literal.lwip_netconn_do_newconn .literal.lwip_netconn_do_recv .literal.lwip_netconn_do_writemore .literal.lwip_netconn_err_to_msg .literal.lwip_netconn_is_deallocated_msg .literal.lwip_netconn_is_err_msg .literal.netconn_alloc .literal.netconn_drain .literal.netconn_free .literal.netconn_mark_mbox_invalid .literal.pcb_new .literal.poll_tcp .literal.recv_raw .literal.recv_tcp .literal.sent_tcp .literal.setup_tcp .literal.tcp_do_abort .text .text.accept_function .text.err_tcp .text.lwip_netconn_do_accepted .text.lwip_netconn_do_bind .text.lwip_netconn_do_bind_if .text.lwip_netconn_do_close .text.lwip_netconn_do_close_internal .text.lwip_netconn_do_connect .text.lwip_netconn_do_connected .text.lwip_netconn_do_delconn .text.lwip_netconn_do_disconnect .text.lwip_netconn_do_dns_found .text.lwip_netconn_do_getaddr .text.lwip_netconn_do_gethostbyname .text.lwip_netconn_do_join_leave_group .text.lwip_netconn_do_join_leave_group_netif .text.lwip_netconn_do_listen .text.lwip_netconn_do_newconn .text.lwip_netconn_do_recv .text.lwip_netconn_do_writemore .text.lwip_netconn_err_to_msg .text.lwip_netconn_is_deallocated_msg .text.lwip_netconn_is_err_msg .text.netconn_alloc .text.netconn_drain .text.netconn_free .text.netconn_mark_mbox_invalid .text.pcb_new .text.poll_tcp .text.recv_raw .text.recv_tcp .text.sent_tcp .text.setup_tcp .text.tcp_do_abort)
    *liblwip.a:etharp.*(.literal.etharp_acd_announce .literal.etharp_acd_probe .literal.etharp_add_static_entry .literal.etharp_cleanup_netif .literal.etharp_find_addr .literal.etharp_find_entry .literal.etharp_free_entry .literal.etharp_get_entry .literal.etharp_input .literal.etharp_query .literal.etharp_raw .literal.etharp_remove_static_entry .literal.etharp_request .literal.etharp_request_dst .literal.etharp_tmr .literal.etharp_update_arp_entry .literal.free_etharp_q .text .text.etharp_acd_announce .text.etharp_acd_probe .text.etharp_add_static_entry .text.etharp_cleanup_netif .text.etharp_find_addr .text.etharp_find_entry .text.etharp_free_entry .text.etharp_get_entry .text.etharp_input .text.etharp_query .text.etharp_raw .text.etharp_remove_static_entry .text.etharp_request .text.etharp_request_dst .text.etharp_tmr .text.etharp_update_arp_entry .text.free_etharp_q)
    *liblwip.a:ethernet.*(.text)
    *liblwip.a:inet_chksum.*(.literal.inet_chksum_pseudo_partial .literal.inet_cksum_pseudo_partial_base .literal.ip6_chksum_pseudo .literal.ip6_chksum_pseudo_partial .literal.ip_chksum_pseudo_partial .text .text.inet_chksum_pseudo_partial .text.inet_cksum_pseudo_partial_base .text.ip6_chksum_pseudo .text.ip6_chksum_pseudo_partial .text.ip_chksum_pseudo_partial)
    *liblwip.a:ip4.*(.literal.ip4_input_accept .literal.ip4_set_default_multicast_netif .text .text.ip4_input_accept .text.ip4_set_default_multicast_netif)
    *liblwip.a:ip4_addr.*(.literal.ip4_addr_netmask_valid .literal.ip4addr_aton .literal.ip4addr_ntoa .literal.ip4addr_ntoa_r .literal.ipaddr_addr .text .text.ip4_addr_netmask_valid .text.ip4addr_aton .text.ip4addr_ntoa .text.ip4addr_ntoa_r .text.ipaddr_addr)
    *liblwip.a:lwip_default_hooks.*(.literal.dhcp_append_extra_opts .literal.dhcp_parse_extra_opts .literal.ip4_netif_exist .literal.lwip_hook_ip6_input .text .text.dhcp_append_extra_opts .text.dhcp_parse_extra_opts .text.ip4_netif_exist .text.lwip_hook_ip6_input)
    *liblwip.a:mem.*(.literal.mem_calloc .literal.mem_free .text .text.mem_calloc .text.mem_free .text.mem_init .text.mem_trim)
    *liblwip.a:memp.*(.literal.do_memp_free_pool .literal.memp_free .literal.memp_free_pool .literal.memp_malloc_pool .text .text.do_memp_free_pool .text.memp_free .text.memp_free_pool .text.memp_init .text.memp_init_pool .text.memp_malloc_pool)
    *liblwip.a:netbuf.*(.literal.netbuf_chain .literal.netbuf_delete .literal.netbuf_new .literal.netbuf_ref .text .text.netbuf_chain .text.netbuf_data .text.netbuf_delete .text.netbuf_first .text.netbuf_new .text.netbuf_next .text.netbuf_ref)
    *liblwip.a:pbuf.*(.literal.pbuf_add_header_force .literal.pbuf_alloc_reference .literal.pbuf_cat .literal.pbuf_chain .literal.pbuf_coalesce .literal.pbuf_copy_partial .literal.pbuf_dechain .literal.pbuf_free_header .literal.pbuf_free_ooseq .literal.pbuf_free_ooseq_callback .literal.pbuf_get_at .literal.pbuf_get_contiguous .literal.pbuf_header_force .literal.pbuf_memcmp .literal.pbuf_memfind .literal.pbuf_pool_is_empty .literal.pbuf_put_at .literal.pbuf_realloc .literal.pbuf_ref .literal.pbuf_skip .literal.pbuf_strstr .literal.pbuf_take .literal.pbuf_take_at .literal.pbuf_try_get_at .text .text.pbuf_add_header_force .text.pbuf_alloc_reference .text.pbuf_cat .text.pbuf_chain .text.pbuf_clen .text.pbuf_coalesce .text.pbuf_copy_partial .text.pbuf_dechain .text.pbuf_free_header .text.pbuf_free_ooseq .text.pbuf_free_ooseq_callback .text.pbuf_get_at .text.pbuf_get_contiguous .text.pbuf_header_force .text.pbuf_memcmp .text.pbuf_memfind .text.pbuf_pool_is_empty .text.pbuf_put_at .text.pbuf_realloc .text.pbuf_ref .text.pbuf_skip .text.pbuf_skip_const .text.pbuf_strstr .text.pbuf_take .text.pbuf_take_at .text.pbuf_try_get_at)
    *liblwip.a:raw.*(.literal.raw_bind .literal.raw_connect .literal.raw_disconnect .literal.raw_netif_ip_addr_changed .literal.raw_new .literal.raw_new_ip_type .literal.raw_remove .literal.raw_send .literal.raw_sendto .literal.raw_sendto_if_src .text .text.raw_bind .text.raw_bind_netif .text.raw_connect .text.raw_disconnect .text.raw_netif_ip_addr_changed .text.raw_new .text.raw_new_ip_type .text.raw_recv .text.raw_remove .text.raw_send .text.raw_sendto .text.raw_sendto_if_src)
    *liblwip.a:sockets.*(.literal.alloc_socket .literal.free_socket .literal.free_socket_free_elements .literal.free_socket_locked .literal.lwip_accept .literal.lwip_bind .literal.lwip_close .literal.lwip_connect .literal.lwip_fcntl .literal.lwip_getaddrname .literal.lwip_getpeername .literal.lwip_getsockname .literal.lwip_getsockopt .literal.lwip_getsockopt_callback .literal.lwip_getsockopt_impl .literal.lwip_inet_ntop .literal.lwip_inet_pton .literal.lwip_ioctl .literal.lwip_link_select_cb .literal.lwip_listen .literal.lwip_poll .literal.lwip_poll_dec_sockets_used .literal.lwip_poll_inc_sockets_used .literal.lwip_pollscan .literal.lwip_read .literal.lwip_readv .literal.lwip_recv .literal.lwip_recvmsg .literal.lwip_select_dec_sockets_used .literal.lwip_select_inc_sockets_used .literal.lwip_select_inc_sockets_used_set .literal.lwip_selscan .literal.lwip_sendmsg .literal.lwip_setsockopt .literal.lwip_setsockopt_callback .literal.lwip_setsockopt_impl .literal.lwip_shutdown .literal.lwip_sock_make_addr .literal.lwip_socket .literal.lwip_socket_dbg_get_socket .literal.lwip_socket_drop_registered_memberships .literal.lwip_socket_drop_registered_mld6_memberships .literal.lwip_socket_register_membership .literal.lwip_socket_register_mld6_membership .literal.lwip_socket_thread_cleanup .literal.lwip_socket_thread_init .literal.lwip_socket_unregister_membership .literal.lwip_socket_unregister_mld6_membership .literal.lwip_sockopt_to_ipopt .literal.lwip_unlink_select_cb .literal.lwip_write .literal.lwip_writev .literal.sock_inc_used_locked .literal.sockaddr_to_ipaddr_port .text .text.alloc_socket .text.free_socket .text.free_socket_free_elements .text.free_socket_locked .text.lwip_accept .text.lwip_bind .text.lwip_close .text.lwip_connect .text.lwip_fcntl .text.lwip_getaddrname .text.lwip_getpeername .text.lwip_getsockname .text.lwip_getsockopt .text.lwip_getsockopt_callback .text.lwip_getsockopt_impl .text.lwip_inet_ntop .text.lwip_inet_pton .text.lwip_ioctl .text.lwip_link_select_cb .text.lwip_listen .text.lwip_poll .text.lwip_poll_dec_sockets_used .text.lwip_poll_inc_sockets_used .text.lwip_poll_should_wake .text.lwip_pollscan .text.lwip_read .text.lwip_readv .text.lwip_recv .text.lwip_recvmsg .text.lwip_select_dec_sockets_used .text.lwip_select_inc_sockets_used .text.lwip_select_inc_sockets_used_set .text.lwip_selscan .text.lwip_sendmsg .text.lwip_setsockopt .text.lwip_setsockopt_callback .text.lwip_setsockopt_impl .text.lwip_shutdown .text.lwip_sock_make_addr .text.lwip_socket .text.lwip_socket_dbg_get_socket .text.lwip_socket_drop_registered_memberships .text.lwip_socket_drop_registered_mld6_memberships .text.lwip_socket_register_membership .text.lwip_socket_register_mld6_membership .text.lwip_socket_thread_cleanup .text.lwip_socket_thread_init .text.lwip_socket_unregister_membership .text.lwip_socket_unregister_mld6_membership .text.lwip_sockopt_to_ipopt .text.lwip_unlink_select_cb .text.lwip_write .text.lwip_writev .text.sock_inc_used_locked .text.sockaddr_to_ipaddr_port)
    *liblwip.a:sys_arch.*(.literal.sys_arch_mbox_tryfetch .literal.sys_delay_ms .literal.sys_init .literal.sys_jiffies .literal.sys_mbox_free .literal.sys_mbox_new .literal.sys_mbox_trypost_fromisr .literal.sys_mutex_free .literal.sys_mutex_new .literal.sys_now .literal.sys_sem_free .literal.sys_sem_new .literal.sys_sem_signal_isr .literal.sys_thread_new .literal.sys_thread_sem_deinit .literal.sys_thread_sem_free .literal.sys_thread_sem_get .literal.sys_thread_sem_init .text .text.sys_arch_mbox_tryfetch .text.sys_delay_ms .text.sys_init .text.sys_jiffies .text.sys_mbox_free .text.sys_mbox_new .text.sys_mbox_trypost_fromisr .text.sys_mutex_free .text.sys_mutex_new .text.sys_now .text.sys_sem_free .text.sys_sem_new .text.sys_sem_signal_isr .text.sys_thread_new .text.sys_thread_sem_deinit .text.sys_thread_sem_free .text.sys_thread_sem_get .text.sys_thread_sem_init)
    *liblwip.a:tcp.*(.literal.tcp_abandon .literal.tcp_abort .literal.tcp_accept_null .literal.tcp_alloc .literal.tcp_backlog_accepted .literal.tcp_backlog_delayed .literal.tcp_bind .literal.tcp_close .literal.tcp_close_ext .literal.tcp_close_shutdown .literal.tcp_close_shutdown_fin .literal.tcp_connect .literal.tcp_debug_state_str .literal.tcp_eff_send_mss_netif .literal.tcp_err .literal.tcp_free .literal.tcp_free_listen .literal.tcp_free_ooseq .literal.tcp_handle_closepend .literal.tcp_init .literal.tcp_kill_prio .literal.tcp_kill_state .literal.tcp_kill_timewait .literal.tcp_listen_closed .literal.tcp_listen_with_backlog .literal.tcp_listen_with_backlog_and_err .literal.tcp_netif_ip_addr_changed .literal.tcp_netif_ip_addr_changed_pcblist .literal.tcp_new .literal.tcp_new_ip_type .literal.tcp_new_port .literal.tcp_next_iss .literal.tcp_pcb_purge .literal.tcp_pcb_remove .literal.tcp_poll .literal.tcp_recv .literal.tcp_remove_listener .literal.tcp_sent .literal.tcp_shutdown .literal.tcp_tcp_get_tcp_addrinfo .literal.tcp_txnow .text .text.tcp_abandon .text.tcp_abort .text.tcp_accept .text.tcp_accept_null .text.tcp_alloc .text.tcp_arg .text.tcp_backlog_accepted .text.tcp_backlog_delayed .text.tcp_bind .text.tcp_bind_netif .text.tcp_close .text.tcp_close_ext .text.tcp_close_shutdown .text.tcp_close_shutdown_fin .text.tcp_connect .text.tcp_debug_state_str .text.tcp_eff_send_mss_netif .text.tcp_err .text.tcp_free .text.tcp_free_listen .text.tcp_free_ooseq .text.tcp_handle_closepend .text.tcp_init .text.tcp_kill_prio .text.tcp_kill_state .text.tcp_kill_timewait .text.tcp_listen_closed .text.tcp_listen_with_backlog .text.tcp_listen_with_backlog_and_err .text.tcp_netif_ip_addr_changed .text.tcp_netif_ip_addr_changed_pcblist .text.tcp_new .text.tcp_new_ip_type .text.tcp_new_port .text.tcp_next_iss .text.tcp_pcb_purge .text.tcp_pcb_remove .text.tcp_poll .text.tcp_recv .text.tcp_remove_listener .text.tcp_sent .text.tcp_setprio .text.tcp_shutdown .text.tcp_tcp_get_tcp_addrinfo .text.tcp_txnow)
    *liblwip.a:tcp_in.*(.literal.tcp_get_next_optbyte .literal.tcp_listen_input .literal.tcp_timewait_input .literal.tcp_trigger_input_pcb_close .text .text.tcp_get_next_optbyte .text.tcp_listen_input .text.tcp_timewait_input .text.tcp_trigger_input_pcb_close)
    *liblwip.a:tcp_out.*(.literal.tcp_enqueue_flags .literal.tcp_keepalive .literal.tcp_output .literal.tcp_output_control_segment .literal.tcp_output_segment_busy .literal.tcp_rexmit_rto .literal.tcp_rexmit_rto_commit .literal.tcp_rst_common .literal.tcp_rst_netif .literal.tcp_send_fin .literal.tcp_zero_window_probe .text .text.tcp_enqueue_flags .text.tcp_keepalive .text.tcp_output .text.tcp_output_control_segment .text.tcp_output_segment_busy .text.tcp_rexmit_rto .text.tcp_rexmit_rto_commit .text.tcp_rst_common .text.tcp_rst_netif .text.tcp_send_fin .text.tcp_zero_window_probe)
    *liblwip.a:tcpip.*(.literal.mem_free_callback .literal.pbuf_free_callback .literal.pbuf_free_int .literal.tcpip_api_call .literal.tcpip_callback_wait .literal.tcpip_callbackmsg_delete .literal.tcpip_callbackmsg_new .literal.tcpip_callbackmsg_trycallback .literal.tcpip_callbackmsg_trycallback_fromisr .literal.tcpip_init .literal.tcpip_timeouts_mbox_fetch .text .text.mem_free_callback .text.pbuf_free_callback .text.pbuf_free_int .text.tcpip_api_call .text.tcpip_callback_wait .text.tcpip_callbackmsg_delete .text.tcpip_callbackmsg_new .text.tcpip_callbackmsg_trycallback .text.tcpip_callbackmsg_trycallback_fromisr .text.tcpip_init .text.tcpip_timeouts_mbox_fetch)
    *liblwip.a:udp.*(.literal.udp_bind .literal.udp_connect .literal.udp_disconnect .literal.udp_init .literal.udp_netif_ip_addr_changed .literal.udp_new .literal.udp_new_ip_type .literal.udp_new_port .literal.udp_remove .text .text.udp_bind .text.udp_bind_netif .text.udp_connect .text.udp_disconnect .text.udp_init .text.udp_netif_ip_addr_changed .text.udp_new .text.udp_new_ip_type .text.udp_new_port .text.udp_recv .text.udp_remove)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_app_disable_protect .literal.esp_flash_get_protectable_regions .literal.esp_flash_read_chip_id .literal.esp_flash_read_id .literal.esp_flash_read_unique_chip_id .literal.esp_flash_suspend_cmd_init .literal.find_region .text .text.esp_flash_app_disable_protect .text.esp_flash_get_protectable_regions .text.esp_flash_read_chip_id .text.esp_flash_read_id .text.esp_flash_read_unique_chip_id .text.esp_flash_suspend_cmd_init .text.find_region)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.esp_flash_app_enable_os_functions .literal.esp_flash_deinit_os_functions .literal.esp_flash_init_os_functions .text .text.esp_flash_app_enable_os_functions .text.esp_flash_deinit_os_functions .text.esp_flash_init_main_bus_lock .text.esp_flash_init_os_functions .text.esp_flash_set_dangerous_write_protection .text.use_bus_lock)
    *libspi_flash.a:spi_flash_os_func_noos.*(.text)
    *libxtensa.a:xt_trax.*(.literal .literal.* .text .text.*)
    *libxtensa.a:xtensa_intr.*(.literal .literal.* .text .text.*)
    *(.stub)
    *(.gnu.warning)
    *(.gnu.linkonce.literal.* .gnu.linkonce.t.*.literal .gnu.linkonce.t.*)
    *(.irom0.text) /* catch stray ICACHE_RODATA_ATTR */
    /**

     * CPU will try to prefetch up to 16 bytes of of instructions.

     * This means that any configuration (e.g. MMU, PMS) must allow

     * safe access to up to 16 bytes after the last real instruction, add

     * dummy bytes to ensure this

     */
    . += 16;
    _text_end = ABSOLUTE(.);
    /**

     * Mark the flash.text end.

     * This can be used for MMU driver to maintain virtual address.

     */
    _instruction_reserved_end = ABSOLUTE(.);
    _etext = .;
    /**

     * Similar to _iram_start, this symbol goes here so it is

     * resolved by addr2line in preference to the first symbol in

     * the flash.text segment.

     */
    _flash_cache_start = ABSOLUTE(0);
  } > default_code_seg
  /**

   * Dummy section represents the .flash.text section but in default_rodata_seg.

   * Thus, it must have its alignment and (at least) its size.

   */
  .flash_rodata_dummy (NOLOAD):
  {
    _flash_rodata_dummy_start = ABSOLUTE(.);
    . = ALIGN(ALIGNOF(.flash.text)) + SIZEOF(.flash.text);
    /* Add alignment of MMU page size + 0x20 bytes for the mapping header. */
    . = ALIGN(0x10000) + 0x20;
  } > default_rodata_seg
  .flash.appdesc : ALIGN(0x10)
  {
    /**

     * Mark flash.rodata start.

     * This can be used for mmu driver to maintain virtual address

     */
    _rodata_reserved_start = ABSOLUTE(.);
    _rodata_start = ABSOLUTE(.);
    /* !DO NOT PUT ANYTHING BEFORE THIS! */
    /* Should be the first.  App version info. */
    *(.rodata_desc .rodata_desc.*)
    /* Should be the second. Custom app version info. */
    *(.rodata_custom_desc .rodata_custom_desc.*)
    /**

     * Create an empty gap within this section. Thanks to this, the end of this

     * section will match .flah.rodata's begin address. Thus, both sections

     * will be merged when creating the final bin image.

     */
    . = ALIGN(ALIGNOF(.flash.rodata));
  } > default_rodata_seg
  ASSERT((ADDR(.flash.rodata) == ADDR(.flash.appdesc) + SIZEOF(.flash.appdesc)), "The gap between .flash.appdesc and .flash.rodata must not exist to produce the final bin image.")
  .flash.rodata : ALIGN(0x10)
  {
    _flash_rodata_start = ABSOLUTE(.);
    *(EXCLUDE_FILE(*libgcov.a *libphy.a *libclang_rt.builtins.a:_divsf3.* *libesp_hal_gpspi.a:spi_hal_iram.* *libesp_hal_gpspi.a:spi_slave_hal_iram.* *libesp_hal_mspi.a:spi_flash_encrypt_hal_iram.* *libesp_hal_mspi.a:spi_flash_hal_gpspi.* *libesp_hal_mspi.a:spi_flash_hal_iram.* *libesp_hal_wdt.a:rom_patch.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_by_mspi_delay.* *libesp_hw_support.a:mspi_timing_config.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:systimer.* *libesp_libc.a:abort.* *libesp_libc.a:assert.* *libesp_libc.a:heap.* *libesp_libc.a:stdatomic.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_rom.a:esp_rom_cache_esp32s2_esp32s3.* *libesp_rom.a:esp_rom_cache_writeback_esp32s3.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_rom.a:esp_rom_systimer.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:reset_reason.* *libesp_system.a:ubsan.* *libgcc.a:_divsf3.* *libhal.a:cache_hal.* *libhal.a:mmu_hal.* *libhal.a:systimer_hal.* *libheap.a:multi_heap.* *libheap.a:tlsf.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp_common.* *liblog.a:util.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_mxic_opi.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_hpm_enable.* *libspi_flash.a:spi_flash_oct_flash_init.* *libspi_flash.a:spi_flash_os_func_app.* *libspi_flash.a:spi_flash_wrap.*) .rodata EXCLUDE_FILE(*libgcov.a *libphy.a *libclang_rt.builtins.a:_divsf3.* *libesp_hal_gpspi.a:spi_hal_iram.* *libesp_hal_gpspi.a:spi_slave_hal_iram.* *libesp_hal_mspi.a:spi_flash_encrypt_hal_iram.* *libesp_hal_mspi.a:spi_flash_hal_gpspi.* *libesp_hal_mspi.a:spi_flash_hal_iram.* *libesp_hal_wdt.a:rom_patch.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_by_mspi_delay.* *libesp_hw_support.a:mspi_timing_config.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:systimer.* *libesp_libc.a:abort.* *libesp_libc.a:assert.* *libesp_libc.a:heap.* *libesp_libc.a:stdatomic.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_rom.a:esp_rom_cache_esp32s2_esp32s3.* *libesp_rom.a:esp_rom_cache_writeback_esp32s3.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_rom.a:esp_rom_systimer.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:reset_reason.* *libesp_system.a:ubsan.* *libgcc.a:_divsf3.* *libhal.a:cache_hal.* *libhal.a:mmu_hal.* *libhal.a:systimer_hal.* *libheap.a:multi_heap.* *libheap.a:tlsf.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp_common.* *liblog.a:util.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_mxic_opi.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_hpm_enable.* *libspi_flash.a:spi_flash_oct_flash_init.* *libspi_flash.a:spi_flash_os_func_app.* *libspi_flash.a:spi_flash_wrap.*) .rodata.* EXCLUDE_FILE(*libgcov.a *libphy.a *libclang_rt.builtins.a:_divsf3.* *libesp_hal_gpspi.a:spi_hal_iram.* *libesp_hal_gpspi.a:spi_slave_hal_iram.* *libesp_hal_mspi.a:spi_flash_encrypt_hal_iram.* *libesp_hal_mspi.a:spi_flash_hal_gpspi.* *libesp_hal_mspi.a:spi_flash_hal_iram.* *libesp_hal_wdt.a:rom_patch.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_by_mspi_delay.* *libesp_hw_support.a:mspi_timing_config.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:systimer.* *libesp_libc.a:abort.* *libesp_libc.a:assert.* *libesp_libc.a:heap.* *libesp_libc.a:stdatomic.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_rom.a:esp_rom_cache_esp32s2_esp32s3.* *libesp_rom.a:esp_rom_cache_writeback_esp32s3.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_rom.a:esp_rom_systimer.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:reset_reason.* *libesp_system.a:ubsan.* *libgcc.a:_divsf3.* *libhal.a:cache_hal.* *libhal.a:mmu_hal.* *libhal.a:systimer_hal.* *libheap.a:multi_heap.* *libheap.a:tlsf.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp_common.* *liblog.a:util.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_mxic_opi.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_hpm_enable.* *libspi_flash.a:spi_flash_oct_flash_init.* *libspi_flash.a:spi_flash_os_func_app.* *libspi_flash.a:spi_flash_wrap.*) .sdata2 EXCLUDE_FILE(*libgcov.a *libphy.a *libclang_rt.builtins.a:_divsf3.* *libesp_hal_gpspi.a:spi_hal_iram.* *libesp_hal_gpspi.a:spi_slave_hal_iram.* *libesp_hal_mspi.a:spi_flash_encrypt_hal_iram.* *libesp_hal_mspi.a:spi_flash_hal_gpspi.* *libesp_hal_mspi.a:spi_flash_hal_iram.* *libesp_hal_wdt.a:rom_patch.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_by_mspi_delay.* *libesp_hw_support.a:mspi_timing_config.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:systimer.* *libesp_libc.a:abort.* *libesp_libc.a:assert.* *libesp_libc.a:heap.* *libesp_libc.a:stdatomic.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_rom.a:esp_rom_cache_esp32s2_esp32s3.* *libesp_rom.a:esp_rom_cache_writeback_esp32s3.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_rom.a:esp_rom_systimer.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:reset_reason.* *libesp_system.a:ubsan.* *libgcc.a:_divsf3.* *libhal.a:cache_hal.* *libhal.a:mmu_hal.* *libhal.a:systimer_hal.* *libheap.a:multi_heap.* *libheap.a:tlsf.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp_common.* *liblog.a:util.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_mxic_opi.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_hpm_enable.* *libspi_flash.a:spi_flash_oct_flash_init.* *libspi_flash.a:spi_flash_os_func_app.* *libspi_flash.a:spi_flash_wrap.*) .sdata2.* EXCLUDE_FILE(*libgcov.a *libphy.a *libclang_rt.builtins.a:_divsf3.* *libesp_hal_gpspi.a:spi_hal_iram.* *libesp_hal_gpspi.a:spi_slave_hal_iram.* *libesp_hal_mspi.a:spi_flash_encrypt_hal_iram.* *libesp_hal_mspi.a:spi_flash_hal_gpspi.* *libesp_hal_mspi.a:spi_flash_hal_iram.* *libesp_hal_wdt.a:rom_patch.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_by_mspi_delay.* *libesp_hw_support.a:mspi_timing_config.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:systimer.* *libesp_libc.a:abort.* *libesp_libc.a:assert.* *libesp_libc.a:heap.* *libesp_libc.a:stdatomic.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_rom.a:esp_rom_cache_esp32s2_esp32s3.* *libesp_rom.a:esp_rom_cache_writeback_esp32s3.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_rom.a:esp_rom_systimer.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:reset_reason.* *libesp_system.a:ubsan.* *libgcc.a:_divsf3.* *libhal.a:cache_hal.* *libhal.a:mmu_hal.* *libhal.a:systimer_hal.* *libheap.a:multi_heap.* *libheap.a:tlsf.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp_common.* *liblog.a:util.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_mxic_opi.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_hpm_enable.* *libspi_flash.a:spi_flash_oct_flash_init.* *libspi_flash.a:spi_flash_os_func_app.* *libspi_flash.a:spi_flash_wrap.*) .srodata EXCLUDE_FILE(*libgcov.a *libphy.a *libclang_rt.builtins.a:_divsf3.* *libesp_hal_gpspi.a:spi_hal_iram.* *libesp_hal_gpspi.a:spi_slave_hal_iram.* *libesp_hal_mspi.a:spi_flash_encrypt_hal_iram.* *libesp_hal_mspi.a:spi_flash_hal_gpspi.* *libesp_hal_mspi.a:spi_flash_hal_iram.* *libesp_hal_wdt.a:rom_patch.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_by_mspi_delay.* *libesp_hw_support.a:mspi_timing_config.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:systimer.* *libesp_libc.a:abort.* *libesp_libc.a:assert.* *libesp_libc.a:heap.* *libesp_libc.a:stdatomic.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_rom.a:esp_rom_cache_esp32s2_esp32s3.* *libesp_rom.a:esp_rom_cache_writeback_esp32s3.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_rom.a:esp_rom_systimer.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:reset_reason.* *libesp_system.a:ubsan.* *libgcc.a:_divsf3.* *libhal.a:cache_hal.* *libhal.a:mmu_hal.* *libhal.a:systimer_hal.* *libheap.a:multi_heap.* *libheap.a:tlsf.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp_common.* *liblog.a:util.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_mxic_opi.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_hpm_enable.* *libspi_flash.a:spi_flash_oct_flash_init.* *libspi_flash.a:spi_flash_os_func_app.* *libspi_flash.a:spi_flash_wrap.*) .srodata.*)
    *(.rodata_wlog_error .rodata_wlog_error.*)
    *(.rodata_wlog_info .rodata_wlog_info.*)
    *(.rodata_wlog_warning .rodata_wlog_warning.*)
    *libesp_hw_support.a:cpu.*(.rodata.__func__$0 .rodata.__func__$1 .rodata.__func__$2)
    *libesp_system.a:reset_reason.*(.rodata.__func__$0)
    *libheap.a:multi_heap.*(.rodata.__func__$0 .rodata.__func__$1 .rodata.__func__$2 .rodata.__func__$3 .rodata.__func__$4 .rodata.__func__$5 .rodata.__func__$6 .rodata.__func__$7 .rodata.__func__$8 .rodata.multi_heap_dump.str1.4 .rodata.multi_heap_dump_tlsf.str1.4 .rodata.multi_heap_find_containing_block_impl.str1.4 .rodata.multi_heap_register_impl.str1.4)
    *libheap.a:tlsf.*(.rodata.__func__$0 .rodata.__func__$1 .rodata.__func__$10 .rodata.__func__$11 .rodata.__func__$12 .rodata.__func__$13 .rodata.__func__$14 .rodata.__func__$15 .rodata.__func__$16 .rodata.__func__$17 .rodata.__func__$19 .rodata.__func__$2 .rodata.__func__$20 .rodata.__func__$3 .rodata.__func__$5 .rodata.__func__$6 .rodata.__func__$7 .rodata.__func__$8 .rodata.__func__$9 .rodata.control_construct.str1.4 .rodata.default_walker.str1.4 .rodata.tlsf_add_pool.str1.4 .rodata.tlsf_check.str1.4 .rodata.tlsf_create.str1.4 .rodata.tlsf_remove_pool.str1.4)
    *libspi_flash.a:spi_flash_os_func_app.*(.rodata.__func__$0 .rodata.esp_flash_spi23_default_os_functions)
    *(.irom1.text) /* catch stray ICACHE_RODATA_ATTR */
    *(.gnu.linkonce.r.*)
    *(.rodata1)
    /* C++ exception handlers table. */
    
 . = ALIGN(4); 
 __XT_EXCEPTION_TABLE_ = ABSOLUTE(.);
    *(.xt_except_table)
    *(.gcc_except_table .gcc_except_table.*)
    *(.gnu.linkonce.e.*)
    
 . = ALIGN(4); 
 __XT_EXCEPTION_DESCS_ = ABSOLUTE(.);
    *(.xt_except_desc)
    *(.gnu.linkonce.h.*)
    __XT_EXCEPTION_DESCS_END__ = ABSOLUTE(.);
    *(.xt_except_desc_end)
    /**

     * C++ constructor tables.

     *

     * Excluding crtbegin.o/crtend.o since IDF doesn't use the toolchain crt.

     */
    
 . = ALIGN(4); 
 __preinit_array_start = ABSOLUTE(.);
    
 . = ALIGN(4); 
 __bothinit_array_start = ABSOLUTE(.);
    KEEP (*(.preinit_array))
    __preinit_array_end = ABSOLUTE(.);
    
 . = ALIGN(4); 
 __init_array_start = ABSOLUTE(.);
    KEEP (*(SORT_BY_INIT_PRIORITY(EXCLUDE_FILE (*crtend.* *crtbegin.*) .ctors.*)))
    KEEP (*(EXCLUDE_FILE (*crtend.* *crtbegin.*) .ctors))
    __init_array_end = ABSOLUTE(.);
    __bothinit_array_end = ABSOLUTE(.);
    /* Addresses of memory regions reserved via SOC_RESERVE_MEMORY_REGION() */
    
 . = ALIGN(4); 
 soc_reserved_memory_region_start = ABSOLUTE(.);
    KEEP (*(.reserved_memory_address))
    soc_reserved_memory_region_end = ABSOLUTE(.);
    /* System init functions registered via ESP_SYSTEM_INIT_FN */
    
 . = ALIGN(4); 
 _esp_system_init_fn_array_start = ABSOLUTE(.);
    KEEP (*(SORT_BY_INIT_PRIORITY(.esp_system_init_fn.*)))
    _esp_system_init_fn_array_end = ABSOLUTE(.);
    _rodata_end = ABSOLUTE(.);
    /* Literals are also RO data. */
    _lit4_start = ABSOLUTE(.);
    *(*.lit4)
    *(.lit4.*)
    *(.gnu.linkonce.lit4.*)
    _lit4_end = ABSOLUTE(.);
    /* TLS data. */
    
 . = ALIGN(4); 
 _thread_local_start = ABSOLUTE(.);
    _picolibc_reent_stub_start = ABSOLUTE(.);
    KEEP(*(.tdata.errno))
    /* Reproduce the public fields from struct _reent. */
    KEEP(*(.tdata.tls_stdin))
    KEEP(*(.tdata.tls_stdout))
    KEEP(*(.tdata.tls_stderr))
    _picolibc_reent_stub_end = ABSOLUTE(.);
    *(.tdata)
    *(.tdata.*)
    *(.tbss)
    *(.tbss.*)
    _thread_local_end = ABSOLUTE(.);
  } > default_rodata_seg
  ASSERT((_picolibc_reent_stub_end - _picolibc_reent_stub_start) == 16, "Newlib _reent stub have wrong size")
  _flash_rodata_align = ALIGNOF(.flash.rodata);
  /**

   * This section contains all the rodata that is not used

   * at runtime, helping to avoid an increase in binary size.

   */
  .flash.rodata_noload (NOLOAD) :
  {
    /**

     * This symbol marks the end of flash.rodata. It can be utilized by the MMU

     * driver to maintain the virtual address.

     * NOLOAD rodata may not be included in this section.

     */
    _rodata_reserved_end = ABSOLUTE(.);
    *(.rodata_wlog_debug .rodata_wlog_debug.*)
    *(.rodata_wlog_verbose .rodata_wlog_verbose.*)
  } > default_rodata_seg
  /**

   * Dummy section to skip flash rodata sections.

   * Because to `extern_ram_seg` and `drom0_0_seg` are on the same bus

   */
  .ext_ram.dummy (NOLOAD):
  {
    . = ORIGIN(extern_ram_seg);
    . = . + (_rodata_reserved_end - _flash_rodata_dummy_start);
    . = ALIGN (0x10000);
  } > extern_ram_seg
  /* Marks the end of IRAM code segment */
  .iram0.text_end (NOLOAD) :
  {
    /* Padding for possible CPU prefetch + alignment for PMS split lines */
    . += 16;
    . = ALIGN(256);
    /* iram_end_test section exists for use by memprot unit tests only */
    *(.iram_end_test)
    _iram_text_end = ABSOLUTE(.);
  } > iram0_0_seg
  .iram0.data :
  {
    
 . = ALIGN(4); 
 _iram_data_start = ABSOLUTE(.);
    *(.iram.data .iram.data.*)
    
 . = ALIGN(4); 
 _iram_data_end = ABSOLUTE(.);
  } > iram0_0_seg
  .iram0.bss (NOLOAD) :
  {
    
 . = ALIGN(4); 
 _iram_bss_start = ABSOLUTE(.);
    *(.iram.bss .iram.bss.*)
    _iram_bss_end = ABSOLUTE(.);
    
 . = ALIGN(4); 
 _iram_end = ABSOLUTE(.);
  } > iram0_0_seg
  /* Marks the end of data, bss and possibly rodata  */
  .dram0.heap_start (NOLOAD) :
  {
    /* Lowest possible start address for the heap */
    
 . = ALIGN(8); 
 _heap_low_start = ABSOLUTE(.);
  } > dram0_0_seg
  /**

   * This section is not included in the binary image; it is only present in the ELF file.

   * It is used to keep certain symbols in the ELF file.

   */
  .noload 0 (INFO) :
  {
    /* Reserve first 4 bytes as zero for vars pointed to NULL */
    . = 0;
    LONG(0);
    _noload_keep_in_elf_start = ABSOLUTE(.);
    KEEP(*(.noload_keep_in_elf .noload_keep_in_elf.*))
    _noload_keep_in_elf_end = ABSOLUTE(.);
  }
  /* DWARF 1 */
  .debug 0 : { *(.debug) }
  .line 0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo 0 : { *(.debug_srcinfo) }
  .debug_sfnames 0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges 0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info 0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev 0 : { *(.debug_abbrev) }
  .debug_line 0 : { *(.debug_line) }
  .debug_frame 0 : { *(.debug_frame) }
  .debug_str 0 : { *(.debug_str) }
  .debug_loc 0 : { *(.debug_loc) }
  .debug_macinfo 0 : { *(.debug_macinfo) }
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  /* DWARF 3 */
  .debug_ranges 0 : { *(.debug_ranges) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames 0 : { *(.debug_varnames) }
  /* GNU DWARF 2 extensions */
  .debug_gnu_pubnames 0 : { *(.debug_gnu_pubnames) }
  .debug_gnu_pubtypes 0 : { *(.debug_gnu_pubtypes) }
  /* DWARF 4 */
  .debug_types 0 : { *(.debug_types) }
  /* DWARF 5 */
  .debug_addr 0 : { *(.debug_addr) }
  .debug_line_str 0 : { *(.debug_line_str) }
  .debug_loclists 0 : { *(.debug_loclists) }
  .debug_macro 0 : { *(.debug_macro) }
  .debug_names 0 : { *(.debug_names) }
  .debug_rnglists 0 : { *(.debug_rnglists) }
  .debug_str_offsets 0 : { *(.debug_str_offsets) }
  .comment 0 : { *(.comment) }
  .note.GNU-stack 0: { *(.note.GNU-stack) }
/**

 * .xt.prop and .xt.lit sections will be used by the debugger and disassembler

 * to get more information about raw data present in the code.

 * Indeed, it may be required to add some padding at some points in the code

 * in order to align a branch/jump destination on a particular bound.

 * Padding these instructions will generate null bytes that shall be

 * interpreted as data, and not code by the debugger or disassembler.

 * This section will only be present in the ELF file, not in the final binary

 * For more details, check GCC-212

 */
  .xtensa.info 0: { *(.xtensa.info) }
  .xt.prop 0 : { *(.xt.prop .xt.prop.* .gnu.linkonce.prop.*) }
  .xt.lit 0 : { *(.xt.lit .xt.lit.* .gnu.linkonce.p.*) }
  /DISCARD/ :
  {
   *(.fini)
   *(.eh_frame_hdr)
   *(.eh_frame)
  }
}
ASSERT(((_iram_end - ORIGIN(iram0_0_seg)) <= LENGTH(iram0_0_seg)),
          "IRAM0 segment data does not fit.")
ASSERT(((_heap_low_start - ORIGIN(dram0_0_seg)) <= LENGTH(dram0_0_seg)),
          "DRAM segment data does not fit.")
