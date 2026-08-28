idf_component_register(SRCS "webdav.c" "miniz.c" "lua_port.c" "api.c" "sd_card.c"
                            "crash_log.c"
                            "wifi_module.c"
                            "http_server.c"
                            "myfat.c"
                            "LED.c"
                       INCLUDE_DIRS "."
                       REQUIRES fatfs
                                esp_wifi
                                esp_http_server
                                app_update
                                heap
                                nvs_flash
                                esp_driver_gpio
                       WHOLE_ARCHIVE)
