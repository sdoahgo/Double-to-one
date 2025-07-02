file(GLOB USER_SRC "src/*.c")
idf_component_register(
    SRCS "main.c" "lvgl_demo_ui.c" ${USER_SRC}
    INCLUDE_DIRS "." "inc"
)
