include("${CMAKE_CURRENT_LIST_DIR}/variant.cmake" OPTIONAL)

if(NOT DEFINED PROJECT_UI_VARIANT)
    set(PROJECT_UI_VARIANT "new")
endif()

if(DEFINED UI_VARIANT)
    set(PROJECT_UI_VARIANT "${UI_VARIANT}")
endif()

if(PROJECT_UI_VARIANT STREQUAL "new")
    set(UI_VARIANT_DIR "${CMAKE_CURRENT_LIST_DIR}/new")
elseif(PROJECT_UI_VARIANT STREQUAL "legacy")
    set(UI_VARIANT_DIR "${CMAKE_CURRENT_LIST_DIR}/legacy")
else()
    message(FATAL_ERROR "Unsupported PROJECT_UI_VARIANT='${PROJECT_UI_VARIANT}'. Use 'new' or 'legacy'.")
endif()

file(GLOB UI_SRCS "${UI_VARIANT_DIR}/*.c")

if(PROJECT_UI_VARIANT STREQUAL "new")
    list(APPEND UI_SRCS "${CMAKE_CURRENT_LIST_DIR}/legacy/LEBREW.c")
    list(APPEND UI_SRCS
        "${CMAKE_CURRENT_LIST_DIR}/legacy/bat5Percent.c"
        "${CMAKE_CURRENT_LIST_DIR}/legacy/bat20Percent.c"
        "${CMAKE_CURRENT_LIST_DIR}/legacy/bat40Percent.c"
        "${CMAKE_CURRENT_LIST_DIR}/legacy/bat60Percent.c"
        "${CMAKE_CURRENT_LIST_DIR}/legacy/bat80Percent.c"
    )
endif()

idf_component_register(
    SRCS ${UI_SRCS}
    INCLUDE_DIRS "${UI_VARIANT_DIR}" "../../main/inc"
    REQUIRES lvgl esp_lcd
)

message(STATUS "Using UI variant: ${PROJECT_UI_VARIANT}")
