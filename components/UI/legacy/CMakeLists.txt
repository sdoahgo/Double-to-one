

file(GLOB UI_SRCS "ui_font_bold_60.c" "LEBREW.c" "ui_image_recharge.c" "ui_image_wifi.c" "bat5Percent.c" "bat20Percent.c" "bat40Percent.c" "bat60Percent.c" "bat80Percent.c" "bat_change.c" "ui_font_bold_16.c" "screens.c" "ui.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
idf_component_register(
    SRCS ${UI_SRCS}
    INCLUDE_DIRS "."
                 "../../main/inc"
    REQUIRES lvgl
             esp_lcd
)
