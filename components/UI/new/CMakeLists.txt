file(GLOB UI_SRCS "ui_image_en.c" "ui_image_cn.c" "images.c" "ui_image_zerong_1.c" "actions.c" "screens.c" "ui.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

idf_component_register(
    SRCS ${UI_SRCS}
    INCLUDE_DIRS "." "../../main/inc"
    REQUIRES lvgl esp_lcd
)

)

