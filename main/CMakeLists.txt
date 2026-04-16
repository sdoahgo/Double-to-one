file(GLOB USER_SRC "src/*.c")
list(FILTER USER_SRC EXCLUDE REGEX "src[/\\\\]GD60914\\.c$")
idf_component_register(
    SRCS "main.c" "lvgl_demo_ui.c" "src/adc_filter.c" ${USER_SRC}
    INCLUDE_DIRS "." "inc"
    REQUIRES
        driver
        esp_adc
        esp_timer
        esp_lcd
        esp_lcd_st7789v
        nvs_flash
        bt
        UI
        espressif__button
        esp_driver_usb_serial_jtag
    PRIV_REQUIRES
        spiffs
        app_update
)
idf_component_get_property(lvgl_lib lvgl__lvgl COMPONENT_LIB)
if(lvgl_lib)
    target_compile_options(${lvgl_lib} PRIVATE -Wno-error=format-truncation)
endif()

#spiffs_create_partition_image(storage storage FLASH_IN_PROJECT)
