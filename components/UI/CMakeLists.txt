

file(GLOB UI_SRCS "ui_font_bold_16.c" "screens.c" "ui.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
idf_component_register(
    SRCS ${UI_SRCS}
    INCLUDE_DIRS "."
                 "../../main/inc"
    REQUIRES lvgl
             esp_lcd
)
