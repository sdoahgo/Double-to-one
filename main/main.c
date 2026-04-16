#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "lvgl.h"
#include "nvs_flash.h"

#include "user_key.h"
#include "TF_Luna.h"
#include "esp_lcd_st7789v.h"
#include "lv_demos.h"
#include "user_bat.h"
#include "user_hal.h"
#include "user_ble.h"
#include "images.h"
#include "fonts.h"
#include "MDC04IIC.h"
#include "user_uart.h"
#include "user_sensor.h"

sys_data SYS_DATA;
NVS_data_t NVS_data;

ui_icon_t UI_icon;
QueueHandle_t ui_msg_queue = NULL;
static const char *TAG = "example";

#define LCD_HOST  SPI2_HOST
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (40 * 1000 * 1000)

#define EXAMPLE_PIN_NUM_SCLK           8
#define EXAMPLE_PIN_NUM_MOSI           7
#define EXAMPLE_PIN_NUM_MISO           -1
#define EXAMPLE_PIN_NUM_LCD_DC         6
#define EXAMPLE_PIN_NUM_LCD_RST        2
#define EXAMPLE_PIN_NUM_LCD_CS         -1
#define EXAMPLE_PIN_NUM_LCD_POWER      GPIO_NUM_4
#define EXAMPLE_PIN_NUM_KEY            GPIO_NUM_1

#define ON_OFF                         GPIO_NUM_0
#define USER_KEY_SHORT_CLICK_MAX_MS    500
#define USER_KEY_LONG_PRESS_MS         1000

#define EXAMPLE_LCD_H_RES              320
#define EXAMPLE_LCD_V_RES              172
#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8
#define EXAMPLE_LVGL_TICK_PERIOD_MS    2

typedef struct {
    esp_lcd_panel_handle_t panel_handle;
    bool display_ready;
} app_context_t;

static lv_disp_draw_buf_t s_disp_buf;
lv_disp_drv_t disp_drv;

void gui_task_key_callback(const uint8_t *event);
void gui_task_UI_callback(ui_msg_t *msg);
void ui_calibration_success(ui_msg_t *msg);

extern void example_lvgl_demo_ui(lv_disp_t *disp);

static void app_start_uart_task(void);
static void app_init_power_outputs(void);
static void app_init_storage(void);
static void app_init_ui_state(void);
static void app_log_board_validation(void);
static void app_init_display(app_context_t *app);
static void app_init_lvgl(void);
static void app_register_lvgl_display(app_context_t *app);
static void app_start_runtime_services(void);
static void app_deferred_runtime_task(void *arg);
static void app_scan_i2c_devices(void);
static void app_enable_display_once(app_context_t *app);
static void app_process_key_queue(void);
static void app_process_ui_queue(void);
static void app_run_loop(app_context_t *app);
static void app_set_display_enabled(app_context_t *app, bool enabled);
static void app_power_off(void);

static void ui_handle_short_click(void);
static void ui_handle_double_click(void);
static void ui_apply_language_mode(bool save_to_nvs);
static void ui_switch_language(void);
static void ui_apply_setting_language_images(void);
static void ui_apply_localized_texts(void);
static void ui_show_main_page(size_t main_page_index);
static void ui_enter_sub_page(void);
static void ui_exit_sub_page(void);
static void ui_cycle_sub_page(void);
static void ui_advance_zerong_flow(void);
static void ui_show_measure_variant(size_t variant_index);
static void ui_update_measure_hold_bar(void);
static void ui_update_setting_hold_bar(void);
static void ui_reset_zerong_bar(uint8_t screen_id);
static void ui_update_zerong_hold_flow(void);
static void ui_update_sensor_labels(const ui_msg_t *msg);
static void ui_update_ble_icons(void);
static void ui_update_battery_icons(void);
static void ui_init_battery_percent_labels(void);
static void ui_update_battery_percent_labels(void);
static void ui_update_charge_icons(void);
static void ui_apply_battery_image(const lv_img_dsc_t *img_src);
static int ui_get_battery_percent(void);
static void ui_handle_temp_update(const ui_msg_t *msg);
static void ui_set_ble_icon_visibility_recursive(lv_obj_t *root, bool visible);
static bool s_setting_cn_images_applied;
typedef struct {
    lv_obj_t *icon;
    lv_obj_t *label;
} battery_indicator_t;

static battery_indicator_t s_battery_indicators[32];
static size_t s_battery_indicator_count;
static int s_display_battery_percent = -1;
static bool s_battery_percent_reinit_on_show = true;
static int64_t s_battery_percent_show_after_us = 0;
#ifdef USE_TDS
static void ui_handle_tds_update(const ui_msg_t *msg);
static void ui_update_tds_temp_labels(const ui_msg_t *msg, int temp_f_times100);
#endif
#ifdef USE_TEMP
static void ui_update_temp_measure_labels(float temp_c, float humidity, bool valid);
#endif

static bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io,
                                            esp_lcd_panel_io_event_data_t *edata,
                                            void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    (void)panel_io;
    (void)edata;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
    esp_lcd_panel_draw_bitmap(panel_handle,
                              area->x1,
                              area->y1,
                              area->x2 + 1,
                              area->y2 + 1,
                              color_map);
}

static void example_lvgl_port_update_callback(lv_disp_drv_t *drv)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;

    switch (drv->rotated) {
    case LV_DISP_ROT_NONE:
        esp_lcd_panel_swap_xy(panel_handle, false);
        esp_lcd_panel_mirror(panel_handle, true, false);
        break;
    case LV_DISP_ROT_90:
        esp_lcd_panel_swap_xy(panel_handle, true);
        esp_lcd_panel_mirror(panel_handle, true, true);
        break;
    case LV_DISP_ROT_180:
        esp_lcd_panel_swap_xy(panel_handle, false);
        esp_lcd_panel_mirror(panel_handle, false, true);
        break;
    case LV_DISP_ROT_270:
        esp_lcd_panel_swap_xy(panel_handle, true);
        esp_lcd_panel_mirror(panel_handle, false, false);
        break;
    }
}

static void example_increase_lvgl_tick(void *arg)
{
    (void)arg;
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

static esp_err_t example_panel_disp_on_off(esp_lcd_panel_handle_t panel, bool on_off)
{
    if (!panel) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!panel->disp_on_off) {
        return ESP_ERR_NOT_SUPPORTED;
    }
    return panel->disp_on_off(panel, on_off);
}

static void app_start_uart_task(void)
{
    xTaskCreate(user_uart_task, "user_uart_task", 1024 * 4, NULL, tskIDLE_PRIORITY + 5, NULL);
}

static void app_init_power_outputs(void)
{
    gpio_config_t on_off_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << ON_OFF) | (1ULL << EXAMPLE_PIN_NUM_LCD_POWER),
    };

    gpio_deep_sleep_hold_dis();
    gpio_hold_dis(ON_OFF);
    gpio_hold_dis(EXAMPLE_PIN_NUM_LCD_POWER);

    ESP_ERROR_CHECK(gpio_config(&on_off_gpio_config));
    gpio_set_level(ON_OFF, 1);
    gpio_set_level(EXAMPLE_PIN_NUM_LCD_POWER, 1);
}

static void app_init_storage(void)
{
    int ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);
    user_device_init();
}

static void app_init_ui_state(void)
{
    UI_icon.bat_icon = 0;
    UI_icon.ble_icon = false;
    UI_icon.charge_icon = false;
}

static const char *app_reset_reason_to_string(esp_reset_reason_t reason)
{
    switch (reason) {
    case ESP_RST_UNKNOWN:
        return "unknown";
    case ESP_RST_POWERON:
        return "power-on";
    case ESP_RST_EXT:
        return "external";
    case ESP_RST_SW:
        return "software";
    case ESP_RST_PANIC:
        return "panic";
    case ESP_RST_INT_WDT:
        return "interrupt-wdt";
    case ESP_RST_TASK_WDT:
        return "task-wdt";
    case ESP_RST_WDT:
        return "other-wdt";
    case ESP_RST_DEEPSLEEP:
        return "deepsleep";
    case ESP_RST_BROWNOUT:
        return "brownout";
    case ESP_RST_SDIO:
        return "sdio";
    case ESP_RST_USB:
        return "usb";
    case ESP_RST_JTAG:
        return "jtag";
    case ESP_RST_EFUSE:
        return "efuse";
    case ESP_RST_PWR_GLITCH:
        return "power-glitch";
    case ESP_RST_CPU_LOCKUP:
        return "cpu-lockup";
    default:
        return "unhandled";
    }
}

static app_context_t *s_app_context = NULL;

static void app_log_board_validation(void)
{
    esp_reset_reason_t reset_reason = esp_reset_reason();
    float battery_mv = 0.0f;
    esp_err_t bat_ret;

    ESP_LOGI(TAG, "Board validation start");
    ESP_LOGI(TAG, "Main power (indirect): reset reason = %s", app_reset_reason_to_string(reset_reason));
    if (reset_reason == ESP_RST_BROWNOUT || reset_reason == ESP_RST_PWR_GLITCH) {
        ESP_LOGW(TAG, "Main power may be unstable because the last reset reason indicates a power event");
    }

    bat_ret = user_bat_read_voltage_once(&battery_mv);
    if (bat_ret == ESP_OK) {
        ESP_LOGI(TAG, "Battery path (ADC): %.2f mV", battery_mv);
    } else {
        ESP_LOGW(TAG, "Battery path (ADC) read failed: %s", esp_err_to_name(bat_ret));
    }

    ESP_LOGW(TAG, "Battery management chip cannot be directly probed in current code; validate it by USB plug/unplug and battery voltage trend");
    ESP_LOGW(TAG, "Level shifter cannot be directly probed in current code; validate it indirectly through the downstream bus/device behavior");
}

static void app_init_display(app_context_t *app)
{
    spi_bus_config_t buscfg = {
        .sclk_io_num = EXAMPLE_PIN_NUM_SCLK,
        .mosi_io_num = EXAMPLE_PIN_NUM_MOSI,
        .miso_io_num = EXAMPLE_PIN_NUM_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = EXAMPLE_LCD_H_RES * 80 * sizeof(uint16_t),
    };
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = EXAMPLE_PIN_NUM_LCD_DC,
        .cs_gpio_num = EXAMPLE_PIN_NUM_LCD_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,
        .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS,
        .spi_mode = 3,
        .trans_queue_depth = 10,
        .on_color_trans_done = example_notify_lvgl_flush_ready,
        .user_ctx = &disp_drv,
    };
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = 16,
    };

    ESP_LOGI(TAG, "Initialize SPI bus");
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    ESP_LOGI(TAG, "Install panel IO");
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install GC9307 panel driver");
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789v(io_handle, &panel_config, &app->panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(app->panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(app->panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(app->panel_handle, 0, 34));
    esp_lcd_panel_swap_xy(app->panel_handle, true);
    esp_lcd_panel_mirror(app->panel_handle, true, false);
    ESP_ERROR_CHECK(example_panel_disp_on_off(app->panel_handle, false));
}

static void app_init_lvgl(void)
{
    lv_color_t *buf1 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    lv_color_t *buf2 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();

    assert(buf1);
    assert(buf2);
    lv_disp_draw_buf_init(&s_disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * 20);

    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = EXAMPLE_LCD_H_RES;
    disp_drv.ver_res = EXAMPLE_LCD_V_RES;
    disp_drv.flush_cb = example_lvgl_flush_cb;
    disp_drv.drv_update_cb = example_lvgl_port_update_callback;
    disp_drv.draw_buf = &s_disp_buf;
}

static void app_register_lvgl_display(app_context_t *app)
{
    disp_drv.user_data = app->panel_handle;
    lv_disp_drv_register(&disp_drv);
}

static void app_start_runtime_services(void)
{
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;

    ESP_LOGI(TAG, "Install LVGL tick timer");
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

    ui_msg_queue = xQueueCreate(10, sizeof(ui_msg_t));
    ESP_LOGI(TAG, "Display LVGL Meter Widget");
    ui_init();
    ui_init_battery_percent_labels();
    s_setting_cn_images_applied = (SYS_DATA.language_id == 0);
    ui_apply_language_mode(false);
    ui_update_ble_icons();
    ui_update_battery_icons();
    ui_update_charge_icons();
    lv_timer_handler();
    lv_refr_now(NULL);
    app_enable_display_once(s_app_context);

    user_key_init();
    xTaskCreate(app_deferred_runtime_task, "app_deferred_runtime", 1024 * 4, NULL, tskIDLE_PRIORITY + 2, NULL);
}

static void app_deferred_runtime_task(void *arg)
{
    (void)arg;

    vTaskDelay(pdMS_TO_TICKS(120));
    user_ble_init();
    user_sensor_init();
    app_scan_i2c_devices();
#ifdef USE_TDS
    driver_TF_Luna_init();
#endif
    xTaskCreate(bat_task, "bat_task", 1024 * 4, NULL, tskIDLE_PRIORITY + 3, NULL);
    vTaskDelete(NULL);
}

static void app_scan_i2c_devices(void)
{
    bool found_device = false;

    if (i2c_mutex && xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(2000)) != pdTRUE) {
        ESP_LOGW(TAG, "Skip I2C scan because bus is busy");
        return;
    }

    ESP_LOGI(TAG, "Start scanning I2C devices on bus %d", I2C_MASTER_NUM);
    for (int address = 0x03; address <= 0x77; address++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        if (!cmd) {
            ESP_LOGE(TAG, "Failed to create I2C command link");
            break;
        }

        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);

        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(100));
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK) {
            found_device = true;
            switch (address) {
            case 0x44:
                ESP_LOGI(TAG, "Found I2C device at 0x%02X (SHT45)", address);
                break;
            case 0x45:
                ESP_LOGI(TAG, "Found I2C device at 0x%02X (MDC04)", address);
                break;
            default:
                ESP_LOGI(TAG, "Found I2C device at 0x%02X", address);
                break;
            }
        }
    }

    if (i2c_mutex) {
        xSemaphoreGive(i2c_mutex);
    }

    if (!found_device) {
        ESP_LOGW(TAG, "No I2C devices found");
    }
}

static void app_enable_display_once(app_context_t *app)
{
    if (!app->display_ready) {
        app_set_display_enabled(app, true);
    }
}

static void app_set_display_enabled(app_context_t *app, bool enabled)
{
    if (!app || !app->panel_handle) {
        return;
    }

    ESP_ERROR_CHECK(example_panel_disp_on_off(app->panel_handle, enabled));
    app->display_ready = enabled;
    ESP_LOGI(TAG, "Display %s", enabled ? "on" : "off");
}

static void app_power_off(void)
{
    esp_err_t err;

    if (s_app_context && s_app_context->panel_handle) {
        err = example_panel_disp_on_off(s_app_context->panel_handle, false);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "Failed to turn display off before power off: %s", esp_err_to_name(err));
        } else {
            s_app_context->display_ready = false;
        }
    }

    gpio_set_level(EXAMPLE_PIN_NUM_LCD_RST, 0);
    gpio_set_level(EXAMPLE_PIN_NUM_LCD_POWER, 0);
    gpio_set_level(ON_OFF, 0);

    ESP_LOGI(TAG, "Power hold released; press the key again to power on");
    fflush(stdout);
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void app_process_key_queue(void)
{
    uint8_t key_event_value = 0;

    if (xQueueReceive(KeyQueue, &key_event_value, 0)) {
        key_event = key_event_value;
        gui_task_key_callback(&key_event_value);
    }
}

static void app_process_ui_queue(void)
{
    ui_msg_t msg;

    if (xQueueReceive(ui_msg_queue, &msg, 0)) {
        if (calibration_flags) {
            ui_calibration_success(&msg);
        } else {
            gui_task_UI_callback(&msg);
        }
    }
}

static void app_run_loop(app_context_t *app)
{
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        app_enable_display_once(app);
        ui_update_measure_hold_bar();
        ui_update_setting_hold_bar();
        ui_update_zerong_hold_flow();
        app_process_key_queue();
        app_process_ui_queue();
        lv_timer_handler();
    }
}

void app_main(void)
{
    app_context_t app = {0};
    s_app_context = &app;

    app_start_uart_task();
    app_init_power_outputs();
    app_init_storage();
    app_init_ui_state();
    app_log_board_validation();
    app_init_display(&app);
    app_init_lvgl();
    app_register_lvgl_display(&app);
    app_start_runtime_services();
    app_run_loop(&app);
}

typedef enum {
    UI_MAIN_PAGE_MEASURE = 0,
    UI_MAIN_PAGE_SETTING,
    UI_MAIN_PAGE_ZERONG,
    UI_MAIN_PAGE_ABOUT,
} ui_main_page_t;

typedef enum {
    UI_NAV_LEVEL_MAIN = 0,
    UI_NAV_LEVEL_SUB,
} ui_nav_level_t;

typedef enum {
    UI_ZERONG_FLOW_M_D = 0,
    UI_ZERONG_FLOW_AW,
} ui_zerong_flow_t;

volatile uint8_t Screens_ID = UI_FIRST_CYCLING_SCREEN;
static ui_main_page_t s_ui_main_page = UI_MAIN_PAGE_MEASURE;
static ui_nav_level_t s_ui_nav_level = UI_NAV_LEVEL_MAIN;
static size_t s_ui_sub_page_index = 0;
static bool s_measure_hold_active = false;
static int64_t s_measure_hold_start_us = 0;
static bool s_measure_hold_block_next_short_click = false;
static bool s_measure_small_bar_latched = false;
static bool s_measure_wait_release = false;
static bool s_measure_in_progress = false;
static int64_t s_measure_start_us = 0;
static float s_measure_last_moisture = 0.0f;
static float s_measure_last_density = 0.0f;
static float s_measure_last_aw = 0.0f;
static bool s_measure_last_valid = false;
static uint8_t s_measure_stable_count = 0;
static bool s_setting_hold_active = false;
static int64_t s_setting_hold_start_us = 0;
static bool s_setting_hold_block_next_short_click = false;
static bool s_setting_hold_triggered = false;
static bool s_setting_cn_images_applied;
static bool s_setting_selected_cn;
static ui_zerong_flow_t s_ui_zerong_flow = UI_ZERONG_FLOW_M_D;
static bool s_zerong_hold_active = false;
static int64_t s_zerong_hold_start_us = 0;
static bool s_zerong_hold_block_next_short_click = false;
static bool s_zerong_hold_triggered = false;
static bool s_zerong_wait_release = false;
static uint8_t s_zerong_last_ing_screen = 0;
static int64_t s_zerong_ing_start_us = 0;

static void ui_set_charge_icon_visibility_recursive(lv_obj_t *root, bool visible)
{
    if (!root) {
        return;
    }

    if (lv_obj_check_type(root, &lv_img_class) && lv_img_get_src(root) == &img_light) {
        if (visible) {
            lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
        }
    }

    uint32_t child_count = lv_obj_get_child_cnt(root);
    for (uint32_t i = 0; i < child_count; ++i) {
        ui_set_charge_icon_visibility_recursive(lv_obj_get_child(root, i), visible);
    }
}

static void ui_set_ble_icon_visibility_recursive(lv_obj_t *root, bool visible)
{
    if (!root) {
        return;
    }

    if (lv_obj_check_type(root, &lv_img_class) &&
        (lv_img_get_src(root) == &img_wifi || lv_img_get_src(root) == &img_wifi_2)) {
        if (visible) {
            lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
        }
    }

    uint32_t child_count = lv_obj_get_child_cnt(root);
    for (uint32_t i = 0; i < child_count; ++i) {
        ui_set_ble_icon_visibility_recursive(lv_obj_get_child(root, i), visible);
    }
}

static bool ui_is_battery_image_src(const void *src)
{
    return src == &img_bat ||
           src == &img_bat_2 ||
           src == &bat80Percent ||
           src == &bat60Percent ||
           src == &bat40Percent ||
           src == &bat20Percent ||
           src == &bat5Percent;
}

static battery_indicator_t *ui_find_battery_indicator_by_parent(lv_obj_t *parent)
{
    for (size_t i = 0; i < s_battery_indicator_count; ++i) {
        if (s_battery_indicators[i].icon && lv_obj_get_parent(s_battery_indicators[i].icon) == parent) {
            return &s_battery_indicators[i];
        }
    }
    return NULL;
}

static void ui_position_battery_label(battery_indicator_t *indicator)
{
    if (!indicator || !indicator->icon || !indicator->label) {
        return;
    }

    lv_obj_update_layout(indicator->icon);
    lv_coord_t x = lv_obj_get_x(indicator->icon) + 4;
    lv_coord_t y = lv_obj_get_y(indicator->icon) + 4;
    lv_obj_set_pos(indicator->label, x, y);
}

static void ui_collect_battery_percent_labels_recursive(lv_obj_t *root)
{
    if (!root) {
        return;
    }

    if (lv_obj_check_type(root, &lv_img_class)) {
        const void *src = lv_img_get_src(root);

        if (ui_is_battery_image_src(src)) {
            lv_obj_t *parent = lv_obj_get_parent(root);
            battery_indicator_t *indicator = ui_find_battery_indicator_by_parent(parent);
            if (!indicator && s_battery_indicator_count < (sizeof(s_battery_indicators) / sizeof(s_battery_indicators[0]))) {
                indicator = &s_battery_indicators[s_battery_indicator_count++];
                indicator->icon = root;
                indicator->label = NULL;
            } else if (indicator && !indicator->icon) {
                indicator->icon = root;
            }
            if (indicator) {
                indicator->icon = root;
            }
        } else if (src == &img_100) {
            battery_indicator_t *indicator = ui_find_battery_indicator_by_parent(lv_obj_get_parent(root));
            if (!indicator && s_battery_indicator_count < (sizeof(s_battery_indicators) / sizeof(s_battery_indicators[0]))) {
                indicator = &s_battery_indicators[s_battery_indicator_count++];
                indicator->icon = NULL;
                indicator->label = NULL;
            }

            if (indicator && !indicator->label) {
                lv_obj_t *parent = lv_obj_get_parent(root);
                lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);

                indicator->label = lv_label_create(parent);
                lv_obj_set_size(indicator->label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                lv_obj_set_style_text_font(indicator->label, &ui_font_bold_10, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(indicator->label, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_label_set_text(indicator->label, "100");
                ui_position_battery_label(indicator);
            }
        }
    } else if (lv_obj_check_type(root, &lv_label_class)) {
        const char *text = lv_label_get_text(root);
        if (text && strcmp(text, "100") == 0) {
            battery_indicator_t *indicator = ui_find_battery_indicator_by_parent(lv_obj_get_parent(root));
            if (!indicator && s_battery_indicator_count < (sizeof(s_battery_indicators) / sizeof(s_battery_indicators[0]))) {
                indicator = &s_battery_indicators[s_battery_indicator_count++];
                indicator->icon = NULL;
                indicator->label = NULL;
            }
            if (indicator) {
                indicator->label = root;
                lv_obj_set_style_text_font(root, &ui_font_bold_10, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(root, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                ui_position_battery_label(indicator);
            }
        }
    }

    uint32_t child_count = lv_obj_get_child_cnt(root);
    for (uint32_t i = 0; i < child_count; ++i) {
        ui_collect_battery_percent_labels_recursive(lv_obj_get_child(root, i));
    }
}

static void ui_init_battery_percent_labels(void)
{
    lv_obj_t *screens[] = {
        objects.measure,
        objects.measure_roasted,
        objects.measure_green,
        objects.measure_parchment,
        objects.measure_dry_cherry,
        objects.setting,
        objects.setting_language,
        objects.zerong,
        objects.zerong_m_d,
        objects.zerong_aw,
        objects.zerong_m_d_ing,
        objects.zerong_aw_ing,
        objects.zerong_m_d_ok,
        objects.zerong_aw_ok,
        objects.zerong_aw_warm,
        objects.zerong_m_d_success,
        objects.zerong_aw_success,
        objects.about,
        objects.about_message,
        objects.setting_language_cn,
    };

    s_battery_indicator_count = 0;
    for (size_t i = 0; i < sizeof(screens) / sizeof(screens[0]); ++i) {
        ui_collect_battery_percent_labels_recursive(screens[i]);
    }

    for (size_t i = 0; i < s_battery_indicator_count; ++i) {
        ui_position_battery_label(&s_battery_indicators[i]);
    }
}

static void ui_apply_battery_image_recursive(lv_obj_t *root, const lv_img_dsc_t *img_src)
{
    if (!root) {
        return;
    }

    if (lv_obj_check_type(root, &lv_img_class) && ui_is_battery_image_src(lv_img_get_src(root))) {
        lv_img_set_src(root, img_src);
    }

    uint32_t child_count = lv_obj_get_child_cnt(root);
    for (uint32_t i = 0; i < child_count; ++i) {
        ui_apply_battery_image_recursive(lv_obj_get_child(root, i), img_src);
    }
}


static const uint8_t s_ui_main_screens[] = {
    SCREEN_ID_MEASURE,
    SCREEN_ID_SETTING,
    SCREEN_ID_ZERONG,
    SCREEN_ID_ABOUT,
};

static const uint8_t s_ui_measure_sub_screens[] = {
    SCREEN_ID_MEASURE_ROASTED,
    SCREEN_ID_MEASURE_GREEN,
    SCREEN_ID_MEASURE_PARCHMENT,
    SCREEN_ID_MEASURE_DRY_CHERRY,
};

typedef struct {
    const lv_img_dsc_t *image_src;
    const char *label_text_en;
    const char *label_text_cn;
    lv_coord_t label_x_en;
    lv_coord_t label_x_cn;
    uint8_t active_point_index;
} ui_measure_variant_t;

static const ui_measure_variant_t s_ui_measure_variants[] = {
    { &img_roasted, "Roasted", "烘焙豆", 229, 232, 0 },
    { &img_green, "Green", "生豆", 236, 243, 1 },
    { &img_parchment, "Parchment", "带壳豆", 221, 232, 2 },
    { &img_drycherry, "DryCherry", "干果", 221, 243, 3 },
};

static const uint8_t s_ui_setting_sub_screens[] = {
    SCREEN_ID_SETTING_LANGUAGE,
};

static const uint8_t s_ui_zerong_sub_screens[] = {
    SCREEN_ID_ZERONG_M_D,
    SCREEN_ID_ZERONG_AW,
    SCREEN_ID_ZERONG_M_D_ING,
    SCREEN_ID_ZERONG_AW_ING,
    SCREEN_ID_ZERONG_M_D_OK,
    SCREEN_ID_ZERONG_AW_OK,
    SCREEN_ID_ZERONG_AW_WARM,
    SCREEN_ID_ZERONG_M_D_SUCCESS,
    SCREEN_ID_ZERONG_AW_SUCCESS,
};

static const uint8_t s_ui_about_sub_screens[] = {
    SCREEN_ID_ABOUT_MESSAGE,
};

static const uint8_t *ui_get_sub_screens(size_t *count)
{
    switch (s_ui_main_page) {
    case UI_MAIN_PAGE_MEASURE:
        *count = sizeof(s_ui_measure_sub_screens) / sizeof(s_ui_measure_sub_screens[0]);
        return s_ui_measure_sub_screens;
    case UI_MAIN_PAGE_SETTING:
        *count = sizeof(s_ui_setting_sub_screens) / sizeof(s_ui_setting_sub_screens[0]);
        return s_ui_setting_sub_screens;
    case UI_MAIN_PAGE_ZERONG:
        *count = sizeof(s_ui_zerong_sub_screens) / sizeof(s_ui_zerong_sub_screens[0]);
        return s_ui_zerong_sub_screens;
    case UI_MAIN_PAGE_ABOUT:
        *count = sizeof(s_ui_about_sub_screens) / sizeof(s_ui_about_sub_screens[0]);
        return s_ui_about_sub_screens;
    default:
        *count = 0;
        return NULL;
    }
}

static void ui_show_main_page(size_t main_page_index)
{
    size_t main_screen_count = sizeof(s_ui_main_screens) / sizeof(s_ui_main_screens[0]);

    if (main_screen_count == 0) {
        return;
    }

    s_ui_main_page = (ui_main_page_t)(main_page_index % main_screen_count);
    s_ui_nav_level = UI_NAV_LEVEL_MAIN;
    s_ui_sub_page_index = 0;
    Screens_ID = s_ui_main_screens[s_ui_main_page];
    if (s_ui_main_page == UI_MAIN_PAGE_ABOUT) {
        loadScreenNoAnim(Screens_ID);
    } else {
        loadScreen(Screens_ID);
    }
}

static void ui_enter_sub_page(void)
{
    size_t sub_screen_count = 0;
    const uint8_t *sub_screens = ui_get_sub_screens(&sub_screen_count);

    if (!sub_screens || sub_screen_count == 0) {
        return;
    }

    s_ui_nav_level = UI_NAV_LEVEL_SUB;
    s_ui_sub_page_index = 0;
    if (s_ui_main_page == UI_MAIN_PAGE_ZERONG) {
        s_ui_zerong_flow = UI_ZERONG_FLOW_M_D;
    }
    Screens_ID = sub_screens[s_ui_sub_page_index];
    if (s_ui_main_page == UI_MAIN_PAGE_ABOUT) {
        loadScreenNoAnim(Screens_ID);
    } else {
        loadScreen(Screens_ID);
    }
    ui_reset_zerong_bar(Screens_ID);

    if (s_ui_main_page == UI_MAIN_PAGE_MEASURE) {
        ui_show_measure_variant(s_ui_sub_page_index);
    } else if (s_ui_main_page == UI_MAIN_PAGE_SETTING) {
        s_setting_selected_cn = s_setting_cn_images_applied;
        s_setting_hold_active = false;
        s_setting_hold_start_us = 0;
        s_setting_hold_block_next_short_click = false;
        s_setting_hold_triggered = false;
        if (objects.obj28) {
            lv_bar_set_value(objects.obj28, 0, LV_ANIM_OFF);
        }
        ui_apply_setting_language_images();
    }
}

static void ui_exit_sub_page(void)
{
    ui_show_main_page((size_t)s_ui_main_page);
}

static void ui_cycle_sub_page(void)
{
    size_t sub_screen_count = 0;
    const uint8_t *sub_screens = ui_get_sub_screens(&sub_screen_count);

    if (!sub_screens || sub_screen_count == 0) {
        return;
    }

    if (s_ui_main_page == UI_MAIN_PAGE_ZERONG) {
        if (Screens_ID == SCREEN_ID_ZERONG_M_D || Screens_ID == SCREEN_ID_ZERONG_AW) {
            if (Screens_ID == SCREEN_ID_ZERONG_M_D) {
                s_ui_sub_page_index = 1;
                s_ui_zerong_flow = UI_ZERONG_FLOW_AW;
                Screens_ID = SCREEN_ID_ZERONG_AW;
            } else {
                s_ui_sub_page_index = 0;
                s_ui_zerong_flow = UI_ZERONG_FLOW_M_D;
                Screens_ID = SCREEN_ID_ZERONG_M_D;
            }
            loadScreen(Screens_ID);
            ui_reset_zerong_bar(Screens_ID);
        }
        return;
    }

    if (s_ui_main_page == UI_MAIN_PAGE_SETTING) {
        ui_switch_language();
        return;
    }

    s_ui_sub_page_index = (s_ui_sub_page_index + 1) % sub_screen_count;
    Screens_ID = sub_screens[s_ui_sub_page_index];
    if (s_ui_main_page == UI_MAIN_PAGE_MEASURE) {
        ui_show_measure_variant(s_ui_sub_page_index);
    } else if (s_ui_main_page == UI_MAIN_PAGE_ABOUT) {
        loadScreenNoAnim(Screens_ID);
    } else {
        loadScreen(Screens_ID);
    }
}

static void ui_show_measure_variant(size_t variant_index)
{
    const lv_img_dsc_t *inactive_point = &img_piont_black;
    const lv_img_dsc_t *active_point = &img_point_white;
    lv_obj_t *points[] = {
        objects.measure_variant_point_1,
        objects.measure_variant_point_2,
        objects.measure_variant_point_3,
        objects.measure_variant_point_4,
    };

    if (variant_index >= (sizeof(s_ui_measure_variants) / sizeof(s_ui_measure_variants[0]))) {
        return;
    }

    lv_img_set_src(objects.measure_variant_image, s_ui_measure_variants[variant_index].image_src);
    lv_label_set_text(objects.measure_variant_label,
                      s_setting_cn_images_applied ? s_ui_measure_variants[variant_index].label_text_cn
                                                  : s_ui_measure_variants[variant_index].label_text_en);
    lv_obj_set_x(objects.measure_variant_label,
                 s_setting_cn_images_applied ? s_ui_measure_variants[variant_index].label_x_cn
                                             : s_ui_measure_variants[variant_index].label_x_en);

    for (size_t i = 0; i < (sizeof(points) / sizeof(points[0])); ++i) {
        lv_img_set_src(points[i], i == s_ui_measure_variants[variant_index].active_point_index ? active_point
                                                                                               : inactive_point);
    }

    s_measure_hold_active = false;
    s_measure_hold_start_us = 0;
    s_measure_hold_block_next_short_click = false;
    s_measure_small_bar_latched = false;
    s_measure_wait_release = false;
    s_measure_in_progress = false;
    s_measure_start_us = 0;
    s_measure_last_valid = false;
    s_measure_stable_count = 0;

    if (objects.obj4) {
        lv_bar_set_value(objects.obj4, 0, LV_ANIM_OFF);
    }

    if (objects.obj6) {
        lv_bar_set_value(objects.obj6, 0, LV_ANIM_OFF);
    }

    if (objects.measure_moisture_value) {
        lv_label_set_text(objects.measure_moisture_value, "00.0");
    }
    if (objects.measure_density_value) {
        lv_label_set_text(objects.measure_density_value, "000g/");
    }
    if (objects.measure_aw_value) {
        lv_label_set_text(objects.measure_aw_value, "0.000");
    }
    if (objects.measure_time_value) {
        lv_label_set_text(objects.measure_time_value, "00:00");
    }
}

static void ui_advance_zerong_flow(void)
{
    if (s_ui_nav_level != UI_NAV_LEVEL_SUB || s_ui_main_page != UI_MAIN_PAGE_ZERONG) {
        return;
    }

    switch (Screens_ID) {
    case SCREEN_ID_ZERONG_M_D:
        s_ui_zerong_flow = UI_ZERONG_FLOW_M_D;
        s_ui_sub_page_index = 4;
        Screens_ID = SCREEN_ID_ZERONG_M_D_OK;
        break;
    case SCREEN_ID_ZERONG_M_D_OK:
        s_ui_zerong_flow = UI_ZERONG_FLOW_M_D;
        s_ui_sub_page_index = 6;
        Screens_ID = SCREEN_ID_ZERONG_AW_WARM;
        break;
    case SCREEN_ID_ZERONG_AW_WARM:
        if (s_ui_zerong_flow == UI_ZERONG_FLOW_M_D) {
            s_ui_sub_page_index = 2;
            Screens_ID = SCREEN_ID_ZERONG_M_D_ING;
        } else {
            s_ui_sub_page_index = 3;
            Screens_ID = SCREEN_ID_ZERONG_AW_ING;
        }
        break;
    case SCREEN_ID_ZERONG_M_D_ING:
        s_ui_zerong_flow = UI_ZERONG_FLOW_M_D;
        s_ui_sub_page_index = 7;
        Screens_ID = SCREEN_ID_ZERONG_M_D_SUCCESS;
        break;
    case SCREEN_ID_ZERONG_AW:
        s_ui_zerong_flow = UI_ZERONG_FLOW_AW;
        s_ui_sub_page_index = 5;
        Screens_ID = SCREEN_ID_ZERONG_AW_OK;
        break;
    case SCREEN_ID_ZERONG_AW_OK:
        s_ui_zerong_flow = UI_ZERONG_FLOW_AW;
        s_ui_sub_page_index = 6;
        Screens_ID = SCREEN_ID_ZERONG_AW_WARM;
        break;
    case SCREEN_ID_ZERONG_AW_ING:
        s_ui_zerong_flow = UI_ZERONG_FLOW_AW;
        s_ui_sub_page_index = 8;
        Screens_ID = SCREEN_ID_ZERONG_AW_SUCCESS;
        break;
    default:
        return;
    }

    loadScreen(Screens_ID);
    ui_reset_zerong_bar(Screens_ID);
    ui_apply_localized_texts();
}

static lv_obj_t *ui_get_zerong_hold_bar_for_screen(uint8_t screen_id)
{
    switch (screen_id) {
    case SCREEN_ID_ZERONG_M_D:
        return objects.obj30;
    case SCREEN_ID_ZERONG_AW:
        return objects.obj32;
    case SCREEN_ID_ZERONG_M_D_OK:
        return objects.obj35;
    case SCREEN_ID_ZERONG_AW_OK:
        return objects.obj37;
    case SCREEN_ID_ZERONG_AW_WARM:
        return objects.obj39;
    case SCREEN_ID_ZERONG_M_D_SUCCESS:
        return objects.obj41;
    case SCREEN_ID_ZERONG_AW_SUCCESS:
        return objects.obj43;
    default:
        return NULL;
    }
}

static bool ui_is_zerong_ing_screen(uint8_t screen_id)
{
    return screen_id == SCREEN_ID_ZERONG_M_D_ING || screen_id == SCREEN_ID_ZERONG_AW_ING;
}

static void ui_reset_zerong_bar(uint8_t screen_id)
{
    lv_obj_t *bar = ui_get_zerong_hold_bar_for_screen(screen_id);
    if (bar) {
        lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    }
}

static void ui_reset_zerong_hold_state(void)
{
    s_zerong_hold_active = false;
    s_zerong_hold_start_us = 0;
    s_zerong_hold_triggered = false;
}

static void ui_update_zerong_hold_flow(void)
{
    lv_obj_t *bar = NULL;
    int32_t progress = 0;

    if (!(GIF_end_flag && s_ui_nav_level == UI_NAV_LEVEL_SUB && s_ui_main_page == UI_MAIN_PAGE_ZERONG)) {
        ui_reset_zerong_hold_state();
        s_zerong_wait_release = false;
        s_zerong_hold_block_next_short_click = false;
        s_zerong_last_ing_screen = 0;
        s_zerong_ing_start_us = 0;
        return;
    }

    if (ui_is_zerong_ing_screen(Screens_ID)) {
        if (s_zerong_last_ing_screen != Screens_ID) {
            s_zerong_last_ing_screen = Screens_ID;
            s_zerong_ing_start_us = esp_timer_get_time();
        } else if ((esp_timer_get_time() - s_zerong_ing_start_us) >= 1500 * 1000) {
            if (Screens_ID == SCREEN_ID_ZERONG_M_D_ING) {
                s_ui_zerong_flow = UI_ZERONG_FLOW_M_D;
                s_ui_sub_page_index = 7;
                Screens_ID = SCREEN_ID_ZERONG_M_D_SUCCESS;
            } else {
                s_ui_zerong_flow = UI_ZERONG_FLOW_AW;
                s_ui_sub_page_index = 8;
                Screens_ID = SCREEN_ID_ZERONG_AW_SUCCESS;
            }
            loadScreen(Screens_ID);
            ui_reset_zerong_bar(Screens_ID);
            ui_apply_localized_texts();
            s_zerong_last_ing_screen = 0;
            s_zerong_ing_start_us = 0;
            s_zerong_wait_release = true;
            ui_reset_zerong_hold_state();
        }
        return;
    }

    s_zerong_last_ing_screen = 0;
    s_zerong_ing_start_us = 0;

    bar = ui_get_zerong_hold_bar_for_screen(Screens_ID);
    if (!bar) {
        ui_reset_zerong_hold_state();
        return;
    }

    if (s_zerong_wait_release) {
        if (gpio_get_level(EXAMPLE_PIN_NUM_KEY) != 0) {
            s_zerong_wait_release = false;
        }
    } else if (gpio_get_level(EXAMPLE_PIN_NUM_KEY) == 0) {
        int64_t now_us = esp_timer_get_time();

        if (!s_zerong_hold_active) {
            s_zerong_hold_active = true;
            s_zerong_hold_start_us = now_us;
            s_zerong_hold_triggered = false;
        }

        int64_t held_ms = (now_us - s_zerong_hold_start_us) / 1000;
        if (held_ms >= USER_KEY_SHORT_CLICK_MAX_MS) {
            progress = (int32_t)(((held_ms - USER_KEY_SHORT_CLICK_MAX_MS) * 100) /
                                 (USER_KEY_LONG_PRESS_MS - USER_KEY_SHORT_CLICK_MAX_MS));
        } else {
            progress = 0;
        }
        if (progress > 100) {
            progress = 100;
        }

        if (progress >= 100 && !s_zerong_hold_triggered) {
            s_zerong_hold_triggered = true;
            s_zerong_hold_block_next_short_click = true;

            if (Screens_ID == SCREEN_ID_ZERONG_M_D_SUCCESS) {
                s_ui_zerong_flow = UI_ZERONG_FLOW_M_D;
                s_ui_sub_page_index = 0;
                Screens_ID = SCREEN_ID_ZERONG_M_D;
                loadScreen(Screens_ID);
                ui_reset_zerong_bar(Screens_ID);
                ui_apply_localized_texts();
            } else if (Screens_ID == SCREEN_ID_ZERONG_AW_SUCCESS) {
                s_ui_zerong_flow = UI_ZERONG_FLOW_AW;
                s_ui_sub_page_index = 1;
                Screens_ID = SCREEN_ID_ZERONG_AW;
                loadScreen(Screens_ID);
                ui_reset_zerong_bar(Screens_ID);
                ui_apply_localized_texts();
            } else {
                ui_advance_zerong_flow();
            }

            s_zerong_wait_release = true;
            ui_reset_zerong_hold_state();
            progress = 0;
            bar = ui_get_zerong_hold_bar_for_screen(Screens_ID);
        }
    } else {
        int64_t held_ms = (esp_timer_get_time() - s_zerong_hold_start_us) / 1000;
        if (s_zerong_hold_active && held_ms >= USER_KEY_SHORT_CLICK_MAX_MS) {
            s_zerong_hold_block_next_short_click = true;
        }
        ui_reset_zerong_hold_state();
    }

    if (bar && lv_bar_get_value(bar) != progress) {
        lv_bar_set_value(bar, progress, LV_ANIM_OFF);
    }
}

static float ui_interp_linear(const float *x, const float *y, size_t count, float value)
{
    if (count == 0) {
        return 0.0f;
    }
    if (value <= x[0]) {
        float dx = x[1] - x[0];
        return y[0] + (value - x[0]) * (y[1] - y[0]) / dx;
    }
    for (size_t i = 1; i < count; ++i) {
        if (value <= x[i]) {
            float dx = x[i] - x[i - 1];
            return y[i - 1] + (value - x[i - 1]) * (y[i] - y[i - 1]) / dx;
        }
    }
    float dx = x[count - 1] - x[count - 2];
    return y[count - 2] + (value - x[count - 2]) * (y[count - 1] - y[count - 2]) / dx;
}

static float ui_calculate_measure_moisture(float cap_pf, float temp_c)
{
    static const float md_x[] = {
        4.1550101f, 4.459793f, 4.953665f, 5.231326f, 5.439820f, 6.973031f,
        9.856157f, 13.982872f, 16.840042f, 17.035393f, 20.530336f
    };
    static const float green_y[] = {
        4.1f, 4.3f, 5.2f, 5.9f, 6.6f, 8.7f, 10.3f, 12.3f, 15.4f, 16.1f, 17.8f
    };
    static const float roasted_x[] = { 3.2f, 3.3f, 3.92f, 5.8f };
    static const float roasted_y[] = { 1.5f, 1.6f, 2.3f, 2.6f };

    if (cap_pf > 118.0f) {
        return 99.9f;
    }

    float moisture = 0.0f;
    switch (s_ui_sub_page_index) {
    case 0: // Roasted
        moisture = ui_interp_linear(roasted_x, roasted_y, sizeof(roasted_x) / sizeof(roasted_x[0]), cap_pf);
        break;
    case 1: { // Green
        float temp_comp = -0.000066f * temp_c * temp_c * temp_c +
                          0.008673f * temp_c * temp_c -
                          0.097408f * temp_c -
                          2.04322f;
        moisture = ui_interp_linear(md_x, green_y, sizeof(md_x) / sizeof(md_x[0]), cap_pf - temp_comp);
        break;
    }
    case 2: // Parchment
        moisture = 0.0066540959f * cap_pf * cap_pf + 1.4002391865f * cap_pf + 2.7631594894f;
        break;
    case 3: // DryCherry
        moisture = 1.0120714060f * cap_pf * cap_pf - 23.0250720579f * cap_pf + 142.2996681948f;
        break;
    default:
        break;
    }

    if (moisture < 0.0f) {
        moisture = 0.0f;
    }
    if (moisture > 99.9f) {
        moisture = 99.9f;
    }
    return moisture;
}

static float ui_calculate_measure_density(int32_t cs1237_raw)
{
    float weight = 0.3657126f * (float)cs1237_raw * 0.01f - 304.1618f;
    float density = weight * 1000000.0f / 184380.0f * 1.15426f;

    if (density < 0.0f) {
        density = 0.0f;
    }
    if (density > 999.0f) {
        density = 999.0f;
    }
    return density;
}

static float ui_calculate_measure_aw(float humidity)
{
    float aw = 0.00000349528f * humidity * humidity * humidity -
               0.000624506f * humidity * humidity +
               0.04605f * humidity -
               0.70156f;

    if (aw < 0.0f) {
        aw = 0.0f;
    }
    if (aw > 1.0f) {
        aw = 1.0f;
    }
    return aw;
}

static void ui_start_measurement(void)
{
    s_measure_in_progress = true;
    s_measure_start_us = esp_timer_get_time();
    s_measure_last_valid = false;
    s_measure_stable_count = 0;
    if (objects.obj6) {
        lv_bar_set_value(objects.obj6, 0, LV_ANIM_OFF);
    }
    if (objects.measure_time_value) {
        lv_label_set_text(objects.measure_time_value, "00:00");
    }
}

static void ui_finish_measurement(float moisture, float density, float aw)
{
    int64_t elapsed_ms = (esp_timer_get_time() - s_measure_start_us) / 1000;
    int seconds = (int)(elapsed_ms / 1000);
    int centiseconds = (int)((elapsed_ms % 1000) / 10);

    if (objects.measure_moisture_value) {
        lv_label_set_text_fmt(objects.measure_moisture_value, "%04.1f", moisture);
    }
    if (objects.measure_density_value) {
        lv_label_set_text_fmt(objects.measure_density_value, "%03dg/", (int)lroundf(density));
    }
    if (objects.measure_aw_value) {
        lv_label_set_text_fmt(objects.measure_aw_value, "%.3f", aw);
    }
    if (objects.measure_time_value) {
        lv_label_set_text_fmt(objects.measure_time_value, "%02d:%02d", seconds, centiseconds);
    }

    s_measure_in_progress = false;
    s_measure_small_bar_latched = true;
    s_measure_hold_block_next_short_click = false;
    s_measure_wait_release = false;
    s_measure_hold_active = false;
    s_measure_hold_start_us = 0;
    if (objects.obj4) {
        lv_bar_set_value(objects.obj4, 0, LV_ANIM_OFF);
    }
    if (objects.obj6) {
        lv_bar_set_value(objects.obj6, 100, LV_ANIM_OFF);
    }
}

static void ui_update_measurement_result(const ui_msg_t *msg)
{
    if (!s_measure_in_progress || !msg || !msg->MDC04_link || !msg->SHT45_link) {
        return;
    }

    if ((esp_timer_get_time() - s_measure_start_us) < 500 * 1000) {
        return;
    }

    float moisture = ui_calculate_measure_moisture(msg->MDC04_value, msg->SHT45_TEMP_value);
    float density = ui_calculate_measure_density(msg->CS1237_value);
    float aw = ui_calculate_measure_aw(msg->SHT45_HUMIDITY_value);

    if (s_measure_last_valid) {
        float moisture_delta = fabsf(moisture - s_measure_last_moisture);
        float density_delta = fabsf(density - s_measure_last_density);
        float aw_delta = fabsf(aw - s_measure_last_aw);

        if (moisture_delta <= 0.2f && density_delta <= 2.0f && aw_delta <= 0.005f) {
            s_measure_stable_count++;
        } else {
            s_measure_stable_count = 0;
        }
    }

    s_measure_last_moisture = moisture;
    s_measure_last_density = density;
    s_measure_last_aw = aw;
    s_measure_last_valid = true;

    if (s_measure_stable_count >= 2) {
        ui_finish_measurement(moisture, density, aw);
    }
}

static void ui_update_measure_hold_bar(void)
{
    int32_t progress = 0;
    int32_t small_bar_progress = 0;

    if (!objects.obj4) {
        return;
    }

    if (GIF_end_flag && s_ui_nav_level == UI_NAV_LEVEL_SUB && s_ui_main_page == UI_MAIN_PAGE_MEASURE) {
        if (s_measure_in_progress) {
            progress = 100;
        } else if (s_measure_wait_release) {
            if (gpio_get_level(EXAMPLE_PIN_NUM_KEY) != 0) {
                s_measure_wait_release = false;
            }
        } else if (gpio_get_level(EXAMPLE_PIN_NUM_KEY) == 0) {
            int64_t now_us = esp_timer_get_time();
            int64_t held_ms = 0;

            if (!s_measure_hold_active) {
                s_measure_hold_active = true;
                s_measure_hold_start_us = now_us;
            }

            held_ms = (now_us - s_measure_hold_start_us) / 1000;
            if (held_ms >= USER_KEY_SHORT_CLICK_MAX_MS) {
                progress = (int32_t)(((held_ms - USER_KEY_SHORT_CLICK_MAX_MS) * 100) /
                                     (USER_KEY_LONG_PRESS_MS - USER_KEY_SHORT_CLICK_MAX_MS));
            }
            if (progress > 100) {
                progress = 100;
            }

            if (progress >= 100 && !s_measure_in_progress) {
                s_measure_hold_block_next_short_click = true;
                s_measure_wait_release = true;
                s_measure_hold_active = false;
                s_measure_hold_start_us = 0;
                s_measure_small_bar_latched = false;
                ui_start_measurement();
                progress = 100;
            }
        } else {
            int64_t held_ms = (esp_timer_get_time() - s_measure_hold_start_us) / 1000;

            if (s_measure_hold_active && held_ms >= USER_KEY_SHORT_CLICK_MAX_MS && held_ms < USER_KEY_LONG_PRESS_MS) {
                s_measure_hold_block_next_short_click = true;
            }
            s_measure_hold_active = false;
            s_measure_hold_start_us = 0;
        }
    } else {
        s_measure_hold_active = false;
        s_measure_hold_start_us = 0;
        s_measure_wait_release = false;
        s_measure_in_progress = false;
        if (!(GIF_end_flag && s_ui_nav_level == UI_NAV_LEVEL_SUB && s_ui_main_page == UI_MAIN_PAGE_MEASURE)) {
            s_measure_hold_block_next_short_click = false;
        }
    }

    if (lv_bar_get_value(objects.obj4) != progress) {
        lv_bar_set_value(objects.obj4, progress, LV_ANIM_OFF);
    }

    if (objects.obj6) {
        small_bar_progress = s_measure_small_bar_latched ? 100 : 0;
        if (lv_bar_get_value(objects.obj6) != small_bar_progress) {
            lv_bar_set_value(objects.obj6, small_bar_progress, LV_ANIM_OFF);
        }
    }
}

static void ui_apply_setting_language_images(void)
{
    if (objects.setting_language_chinese_img) {
        lv_img_set_src(objects.setting_language_chinese_img,
                       s_setting_selected_cn ? &img_cn : &img_chinese);
    }

    if (objects.setting_language_english_img) {
        lv_img_set_src(objects.setting_language_english_img,
                       s_setting_selected_cn ? &img_en : &img_english);
    }
}

static void ui_apply_localized_texts(void)
{
    bool chinese_mode = s_setting_cn_images_applied;
    lv_obj_t *label = NULL;

    label = lv_obj_get_child(objects.measure, 3);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "测量" : "Measure");
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 130);
    }

    label = lv_obj_get_child(objects.measure_roasted, 1);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "水分" : "Moisture");
    }

    if (objects.obj0) {
        lv_label_set_text(objects.obj0, chinese_mode ? "水分" : "Moisture");
        lv_obj_set_x(objects.obj0, chinese_mode ? 56 : 37);
    }
    if (objects.obj1) {
        lv_label_set_text(objects.obj1, chinese_mode ? "密度" : "Density");
        lv_obj_set_x(objects.obj1, chinese_mode ? 146 : 142);
    }
    if (objects.obj2) {
        lv_label_set_text(objects.obj2, chinese_mode ? "水活" : "AW");
    }
    if (objects.obj3) {
        lv_label_set_text(objects.obj3, chinese_mode ? "时间" : "Time");
    }
    if (objects.obj5) {
        lv_label_set_text(objects.obj5, chinese_mode ? "检测" : "Measure");
        lv_obj_align(objects.obj5, LV_ALIGN_CENTER, 0, 0);
    }

    label = lv_obj_get_child(objects.setting, 3);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "设置" : "Setting");
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 130);
    }

    label = lv_obj_get_child(objects.setting_language, 1);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "语言" : "Language");
    }

    label = lv_obj_get_child(objects.about, 3);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "关于" : "About");
        lv_obj_set_style_text_font(label, chinese_mode ? &ui_font_bold_20 : &ui_font_bold_20,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 130);
    }

    label = lv_obj_get_child(objects.about_message, 1);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "关于" : "About");
        lv_obj_set_style_text_font(label, chinese_mode ? &ui_font_bold_16 : &ui_font_bold_16,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    if (objects.about_message) {
        lv_obj_t *about_content = lv_obj_get_child(objects.about_message, 7);

        if (about_content) {
            lv_obj_t *name_panel = lv_obj_get_child(about_content, 1);
            lv_obj_t *model_panel = lv_obj_get_child(about_content, 2);
            lv_obj_t *hardware_panel = lv_obj_get_child(about_content, 3);
            lv_obj_t *software_panel = lv_obj_get_child(about_content, 4);

            if (name_panel) {
                lv_obj_t *field_label = lv_obj_get_child(name_panel, 0);
                if (field_label) {
                    lv_label_set_text(field_label, chinese_mode ? "产品名" : "Name");
                    lv_obj_set_style_text_font(field_label, chinese_mode ? &ui_font_bold_12 : &lv_font_montserrat_12,
                                               LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }

            if (model_panel) {
                lv_obj_t *field_label = lv_obj_get_child(model_panel, 0);
                if (field_label) {
                    lv_label_set_text(field_label, chinese_mode ? "型号" : "Model");
                    lv_obj_set_style_text_font(field_label, chinese_mode ? &ui_font_bold_12 : &lv_font_montserrat_12,
                                               LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }

            if (hardware_panel) {
                lv_obj_t *field_label = lv_obj_get_child(hardware_panel, 0);
                if (field_label) {
                    lv_label_set_text(field_label, chinese_mode ? "硬件版本" : "Hardware Version");
                    lv_obj_set_style_text_font(field_label, chinese_mode ? &ui_font_bold_12 : &lv_font_montserrat_12,
                                               LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }

            if (software_panel) {
                lv_obj_t *field_label = lv_obj_get_child(software_panel, 0);
                if (field_label) {
                    lv_label_set_text(field_label, chinese_mode ? "软件版本" : "Software Version");
                    lv_obj_set_style_text_font(field_label, chinese_mode ? &ui_font_bold_12 : &lv_font_montserrat_12,
                                               LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
    }

    label = lv_obj_get_child(objects.zerong, 3);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "校准" : "Zerong");
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 130);
    }

    label = lv_obj_get_child(objects.zerong_m_d, 9);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "水分&密度\n校准中" : "Moisture&Density\nZeroing");
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 91);
    }

    label = lv_obj_get_child(objects.zerong_m_d, 1);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "水分&密度校准" : "M&D Zerong");
    }

    label = lv_obj_get_child(objects.zerong_aw, 1);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "水活度校准" : "AW Zerong");
    }

    if (objects.obj34) {
        lv_label_set_text(objects.obj34, chinese_mode ? "水活度\n校准中" : "AW\nZeroing");
        lv_obj_align(objects.obj34, LV_ALIGN_TOP_MID, 0, 91);
    }

    label = lv_obj_get_child(objects.zerong_m_d_ing, 1);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "水分&密度校准" : "M&D Zerong");
    }

    label = lv_obj_get_child(objects.zerong_m_d_ing, 7);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "水分&密度校准中..." : "M&D Zeroing...");
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 122);
    }

    label = lv_obj_get_child(objects.zerong_aw_ing, 1);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "水活度校准" : "AW Zerong");
    }

    label = lv_obj_get_child(objects.zerong_aw_ing, 7);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "水活度校准中..." : "A Zeroing...");
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 122);
    }

    label = lv_obj_get_child(objects.zerong_m_d_ok, 1);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "水分&密度校准" : "M&D Zerong");
    }

    label = lv_obj_get_child(objects.zerong_m_d_ok, 8);
    if (label) {
        lv_label_set_text(label, chinese_mode
                                     ? "您即将开始校准流程，请在开始前务必完全理解校准操作步骤。"
                                     : "You will be starting the calibration process, please make sure that you have fully understood the calibration procedure before starting the calibration process.");
        lv_obj_set_style_text_font(label, chinese_mode ? &ui_font_regular_16 : &lv_font_montserrat_16,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, chinese_mode ? 48 : 12);
    }

    label = lv_obj_get_child(objects.zerong_aw_ok, 1);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "水活度校准" : "AW Zerong");
    }

    label = lv_obj_get_child(objects.zerong_aw_ok, 8);
    if (label) {
        lv_label_set_text(label, chinese_mode
                                     ? "您即将开始校准流程，请在开始前务必完全理解校准操作步骤。"
                                     : "You will be starting the calibration process, please make sure that you have fully understood the calibration procedure before starting the calibration process.");
        lv_obj_set_style_text_font(label, chinese_mode ? &ui_font_regular_16 : &lv_font_montserrat_16,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, chinese_mode ? 48 : 12);
    }

    label = lv_obj_get_child(objects.zerong_aw_warm, 1);
    if (label) {
        lv_label_set_text(label,
                          s_ui_zerong_flow == UI_ZERONG_FLOW_M_D
                              ? (chinese_mode ? "水分&密度校准" : "M&D Zerong")
                              : (chinese_mode ? "水活度校准" : "AW Zerong"));
    }

    label = lv_obj_get_child(objects.zerong_aw_warm, 9);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "确定要立即开始校准吗？" : "Are you sure you want to zeroing now?");
        lv_obj_set_style_text_font(label, chinese_mode ? &ui_font_regular_16 : LV_FONT_DEFAULT,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 89);
    }

    label = lv_obj_get_child(objects.zerong_m_d_success, 1);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "水分&密度校准" : "M&D Zerong");
    }

    label = lv_obj_get_child(objects.zerong_m_d_success, 8);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "校准成功" : "Success");
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 105);
    }

    label = lv_obj_get_child(objects.zerong_aw_success, 1);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "水活度校准" : "AW Zerong");
    }

    label = lv_obj_get_child(objects.zerong_aw_success, 8);
    if (label) {
        lv_label_set_text(label, chinese_mode ? "校准成功" : "Success");
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 105);
    }

    ui_show_measure_variant(s_ui_sub_page_index);
}

static void ui_apply_language_mode(bool save_to_nvs)
{
    SYS_DATA.language_id = s_setting_cn_images_applied ? 0 : 1;

    if (save_to_nvs) {
        user_save_sys_evn(&SYS_DATA);
    }

    if (objects.chinese_bt && objects.engilsh_bt) {
        if (s_setting_cn_images_applied) {
            lv_obj_set_style_bg_color(objects.chinese_bt, lv_color_hex(0xffa851ff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(objects.engilsh_bt, lv_color_hex(0xff505050), LV_PART_MAIN | LV_STATE_DEFAULT);
        } else {
            lv_obj_set_style_bg_color(objects.chinese_bt, lv_color_hex(0xff505050), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(objects.engilsh_bt, lv_color_hex(0xffa851ff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }

    ui_apply_setting_language_images();
    ui_apply_localized_texts();
}

static void ui_reset_setting_hold_state(void)
{
    s_setting_hold_active = false;
    s_setting_hold_start_us = 0;
    s_setting_hold_triggered = false;
}

static void ui_update_setting_hold_bar(void)
{
    int32_t progress = 0;

    if (!objects.obj28) {
        return;
    }

    if (GIF_end_flag && s_ui_nav_level == UI_NAV_LEVEL_SUB && s_ui_main_page == UI_MAIN_PAGE_SETTING) {
        if (gpio_get_level(EXAMPLE_PIN_NUM_KEY) == 0) {
            int64_t now_us = esp_timer_get_time();
            int64_t held_ms = 0;

            if (!s_setting_hold_active) {
                s_setting_hold_active = true;
                s_setting_hold_start_us = now_us;
                s_setting_hold_triggered = false;
            }

            held_ms = (now_us - s_setting_hold_start_us) / 1000;
            if (held_ms >= USER_KEY_SHORT_CLICK_MAX_MS) {
                progress = (int32_t)(((held_ms - USER_KEY_SHORT_CLICK_MAX_MS) * 100) /
                                     (USER_KEY_LONG_PRESS_MS - USER_KEY_SHORT_CLICK_MAX_MS));
            }
            if (progress > 100) {
                progress = 100;
            }

            if (progress >= 100 && !s_setting_hold_triggered) {
                s_setting_hold_triggered = true;
                s_setting_hold_block_next_short_click = true;
                s_setting_cn_images_applied = s_setting_selected_cn;
                ui_apply_language_mode(true);
            }
        } else {
            int64_t held_ms = (esp_timer_get_time() - s_setting_hold_start_us) / 1000;

            if (s_setting_hold_active && held_ms >= USER_KEY_SHORT_CLICK_MAX_MS) {
                s_setting_hold_block_next_short_click = true;
            }
            ui_reset_setting_hold_state();
        }
    } else {
        ui_reset_setting_hold_state();
        if (!(GIF_end_flag && s_ui_nav_level == UI_NAV_LEVEL_SUB && s_ui_main_page == UI_MAIN_PAGE_SETTING)) {
            s_setting_hold_block_next_short_click = false;
        }
    }

    if (lv_bar_get_value(objects.obj28) != progress) {
        lv_bar_set_value(objects.obj28, progress, LV_ANIM_OFF);
    }
}

static void ui_handle_short_click(void)
{
    size_t main_screen_count = sizeof(s_ui_main_screens) / sizeof(s_ui_main_screens[0]);

    if (!lv_obj_has_flag(objects.low_power_page, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(objects.low_power_page, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    if (s_ui_nav_level == UI_NAV_LEVEL_SUB) {
        ui_cycle_sub_page();
        return;
    }

    ui_show_main_page(((size_t)s_ui_main_page + 1) % main_screen_count);
}

static void ui_handle_double_click(void)
{
    if (s_ui_nav_level == UI_NAV_LEVEL_SUB) {
        ui_exit_sub_page();
    } else {
        ui_enter_sub_page();
    }
}

static void ui_switch_language(void)
{
    s_setting_selected_cn = !s_setting_selected_cn;
    ui_apply_setting_language_images();
}

void gui_task_key_callback(const uint8_t *event)
{
    switch (*event) {
    case USER_KEY_SHORT_IN_EVT:
        if (GIF_end_flag) {
            if (s_measure_hold_block_next_short_click) {
                s_measure_hold_block_next_short_click = false;
                ESP_LOGI(TAG, "Ignore short click after measure hold attempt");
                break;
            }
            if (s_setting_hold_block_next_short_click) {
                s_setting_hold_block_next_short_click = false;
                ESP_LOGI(TAG, "Ignore short click after setting hold attempt");
                break;
            }
            if (s_zerong_hold_block_next_short_click) {
                s_zerong_hold_block_next_short_click = false;
                ESP_LOGI(TAG, "Ignore short click after zerong hold attempt");
                break;
            }
            ui_handle_short_click();
            ESP_LOGI(TAG, "Current screen_id=%d", Screens_ID);
        }
        break;
    case USER_KEY_LONG_IN_EVT:
        if (GIF_end_flag) {
            ESP_LOGI(TAG, "Confirm long press triggered, screen_id=%d", Screens_ID);
        }
        break;
    case USER_KEY_POWER_LONG_IN_EVT:
        if (GIF_end_flag) {
            app_power_off();
            ESP_LOGI(TAG, "Current screen_id=%d", Screens_ID);
        }
        break;
    case USER_KEY_DOUBLE_CLICK_IN_EVT:
        printf("USER_KEY_DOUBLE_CLICK_IN_EVT %d\n", Screens_ID);
        ui_handle_double_click();
        ESP_LOGI(TAG, "Current screen_id=%d", Screens_ID);
        break;
    case USER_KEY_MULTIPLE_CLICK_IN_EVT:
        break;
    default:
        break;
    }
}

void ui_calibration_success(ui_msg_t *msg)
{
    SYS_DATA.temp_calibration_value = msg->temp_value;
    user_save_sys_evn(&SYS_DATA);
    calibration_flags = false;
    lv_obj_set_style_bg_color(objects.calibration_bt, lv_color_hex(0xff28f641), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(lv_obj_get_child(objects.calibration_bt, 0), UI_STRING[5][SYS_DATA.language_id]);
}

#ifdef USE_TDS
static void ui_update_tds_temp_labels(const ui_msg_t *msg, int temp_f_times100)
{
    int temp_f1 = temp_f_times100 / 100;
    int temp_f2 = temp_f_times100 >= 0 ? temp_f_times100 % 100 : -(temp_f_times100 % 100);
    lv_obj_t *child1 = lv_obj_get_child(objects.tds_temp_container, 1);
    lv_obj_t *child2 = lv_obj_get_child(objects.tds_temp_container, 2);
    lv_obj_t *child3 = lv_obj_get_child(objects.tds_temp_container, 3);
    lv_obj_t *child4 = lv_obj_get_child(objects.tds_temp_container, 4);

    if (msg->temp_value >= 1000) {
        lv_obj_set_pos(child2, 57, 4);
        lv_obj_set_pos(child4, 57, 36);
        lv_label_set_text_fmt(child1, "%s%d.",
                              msg->temp_value < 0 ? "-" : "",
                              (msg->temp_value < 0 ? -msg->temp_value : msg->temp_value) / 10);
    } else if (msg->temp_value < 1000 && msg->temp_value >= 100) {
        lv_obj_set_pos(child2, 45, 4);
        lv_obj_set_pos(child4, temp_f_times100 >= 10000 ? 57 : 45, 36);
        lv_label_set_text_fmt(child1, "%s%d.",
                              msg->temp_value < 0 ? "-" : "",
                              (msg->temp_value < 0 ? -msg->temp_value : msg->temp_value) / 10);
    } else if (msg->temp_value < 100 && msg->temp_value >= 0) {
        lv_obj_set_pos(child2, 45, 4);
        lv_obj_set_pos(child4, 45, 36);
        lv_label_set_text_fmt(child1, "%s0%d.",
                              msg->temp_value < 0 ? "-" : "",
                              (msg->temp_value < 0 ? -msg->temp_value : msg->temp_value) / 10);
    } else {
        lv_obj_set_pos(child2, 57, 4);
        lv_obj_set_pos(child4, 57, 36);
        if (msg->temp_value > -100) {
            lv_label_set_text_fmt(child1, "%s0%d.",
                                  msg->temp_value < 0 ? "-" : "",
                                  (msg->temp_value < 0 ? -msg->temp_value : msg->temp_value) / 10);
        } else {
            lv_label_set_text_fmt(child1, "%s%d.",
                                  msg->temp_value < 0 ? "-" : "",
                                  (msg->temp_value < 0 ? -msg->temp_value : msg->temp_value) / 10);
        }
    }

    lv_label_set_text_fmt(child2, "%d0F", (msg->temp_value % 10 + 10) % 10);
    lv_label_set_text_fmt(child3, "%d.", temp_f1);
    lv_label_set_text_fmt(child4, "%dF", temp_f2);
}

static void ui_handle_tds_update(const ui_msg_t *msg)
{
    int32_t integer = (int32_t)msg->TF_TDS_value;
    uint32_t fractional_part = (uint32_t)lroundf((msg->TF_TDS_value - (float)integer) * 100.0f);

    if (fractional_part == 100) {
        fractional_part = 0;
        integer += 1;
    }

    lv_label_set_text_fmt(lv_obj_get_child(objects.tds_container, 3), "%02u", (unsigned)fractional_part);
    if (integer < 10) {
        lv_label_set_text_fmt(lv_obj_get_child(objects.tds_container, 2), "0%u.", (unsigned)integer);
    } else {
        lv_label_set_text_fmt(lv_obj_get_child(objects.tds_container, 2), "%u.", (unsigned)integer);
    }
    if (integer >= 100) {
        lv_label_set_text(lv_obj_get_child(objects.tds_container, 3), "99");
        lv_label_set_text(lv_obj_get_child(objects.tds_container, 2), "99.");
    }
}
#endif

#ifdef USE_TEMP
static void ui_update_temp_measure_labels(float temp_c, float humidity, bool valid)
{
    lv_obj_t *degree_label = lv_obj_get_child(objects.temp_container, 0);
    lv_obj_t *decimal_label = lv_obj_get_child(objects.temp_container, 1);
    lv_obj_t *integer_label = lv_obj_get_child(objects.temp_container, 2);
    lv_obj_t *humidity_label = lv_obj_get_child(objects.temp_container, 3);
    int16_t temp_tenths;

    if (!valid) {
        lv_obj_set_pos(degree_label, 156, 59);
        lv_obj_set_pos(decimal_label, 108, 43);
        lv_obj_set_pos(integer_label, 20, 24);
        lv_obj_set_pos(humidity_label, 108, 1);
        lv_label_set_text(integer_label, "--.");
        lv_label_set_text(decimal_label, "-");
        lv_label_set_text(humidity_label, "--.- %RH");
        return;
    }

    temp_tenths = (int16_t)lroundf(temp_c * 10.0f);

    if (temp_tenths >= 1000) {
        lv_obj_set_pos(degree_label, 192, 59);
        lv_obj_set_pos(decimal_label, 144, 43);
        lv_obj_set_pos(integer_label, 20, 24);
    } else if (temp_tenths >= 100) {
        lv_obj_set_pos(degree_label, 156, 59);
        lv_obj_set_pos(decimal_label, 108, 43);
    } else if (temp_tenths >= 0) {
        lv_obj_set_pos(degree_label, 156, 59);
        lv_obj_set_pos(decimal_label, 108, 43);
    } else if (temp_tenths > -100) {
        lv_obj_set_pos(degree_label, 192, 59);
        lv_obj_set_pos(decimal_label, 144, 43);
    } else {
        lv_obj_set_pos(degree_label, 192, 59);
        lv_obj_set_pos(decimal_label, 144, 43);
        lv_obj_set_pos(integer_label, 20, 24);
    }

    if (temp_tenths < 1000 && temp_tenths >= 100) {
        lv_label_set_text_fmt(integer_label, "%s%d.",
                              temp_tenths < 0 ? "-" : "",
                              (temp_tenths < 0 ? -temp_tenths : temp_tenths) / 10);
    } else if (temp_tenths < 100 && temp_tenths >= 0) {
        lv_label_set_text_fmt(integer_label, "0%d.", temp_tenths / 10);
    } else if (temp_tenths < 0 && temp_tenths > -100) {
        lv_label_set_text_fmt(integer_label, "-0%d.", (-temp_tenths) / 10);
    } else {
        lv_label_set_text_fmt(integer_label, "%s%d.",
                              temp_tenths < 0 ? "-" : "",
                              (temp_tenths < 0 ? -temp_tenths : temp_tenths) / 10);
    }

    lv_label_set_text_fmt(decimal_label, "%d0", (temp_tenths % 10 + 10) % 10);
    lv_obj_set_pos(humidity_label, 100, 1);
    lv_label_set_text_fmt(humidity_label, "%.1f %%RH", humidity);
}
#endif

static void ui_handle_temp_update(const ui_msg_t *msg)
{
#if !defined(USE_TEMP) && !defined(USE_TDS)
    (void)msg;
#else
    int16_t temp_casual = msg->temp_value - SYS_DATA.temp_calibration_value;
    int temp_f_times100 = temp_casual * 18 + 3200;

    switch (Screens_ID) {
#ifdef USE_TDS
    case SCREEN_ID_MEASURE_TDS:
        ui_update_tds_temp_labels(msg, temp_f_times100);
        break;
#endif
#ifdef USE_TEMP
    case SCREEN_ID_MEASURE_TEMP:
        ui_update_temp_measure_labels((float)temp_casual / 10.0f, 0.0f, true);
        break;
#endif
    default:
        break;
    }
#endif
}

static void ui_update_sensor_labels(const ui_msg_t *msg)
{
#ifdef USE_TEMP
    ui_update_temp_measure_labels(msg->SHT45_TEMP_value, msg->SHT45_HUMIDITY_value, msg->SHT45_link);
#endif
    ui_update_measurement_result(msg);
}

static void ui_update_ble_icons(void)
{
    lv_obj_t *screens[] = {
        objects.measure,
        objects.measure_roasted,
        objects.measure_green,
        objects.measure_parchment,
        objects.measure_dry_cherry,
        objects.setting,
        objects.setting_language,
        objects.zerong,
        objects.zerong_m_d,
        objects.zerong_aw,
        objects.zerong_m_d_ing,
        objects.zerong_aw_ing,
        objects.zerong_m_d_ok,
        objects.zerong_aw_ok,
        objects.zerong_aw_warm,
        objects.zerong_m_d_success,
        objects.zerong_aw_success,
        objects.about,
        objects.about_message,
        objects.setting_language_cn,
    };

    for (size_t i = 0; i < sizeof(screens) / sizeof(screens[0]); ++i) {
        ui_set_ble_icon_visibility_recursive(screens[i], UI_icon.ble_icon);
    }
}

static void ui_apply_battery_image(const lv_img_dsc_t *img_src)
{
    lv_obj_t *screens[] = {
        objects.measure,
        objects.measure_roasted,
        objects.measure_green,
        objects.measure_parchment,
        objects.measure_dry_cherry,
        objects.setting,
        objects.setting_language,
        objects.zerong,
        objects.zerong_m_d,
        objects.zerong_aw,
        objects.zerong_m_d_ing,
        objects.zerong_aw_ing,
        objects.zerong_m_d_ok,
        objects.zerong_aw_ok,
        objects.zerong_aw_warm,
        objects.zerong_m_d_success,
        objects.zerong_aw_success,
        objects.about,
        objects.about_message,
        objects.setting_language_cn,
    };

    for (size_t i = 0; i < sizeof(screens) / sizeof(screens[0]); ++i) {
        ui_apply_battery_image_recursive(screens[i], img_src);
    }
}

static int ui_get_battery_percent(void)
{
    static const float voltage_points[] = { 3450.0f, 3630.0f, 3730.0f, 3830.0f, 3980.0f, 4200.0f };
    static const int percent_points[] = { 5, 20, 40, 60, 80, 100 };
    float voltage = bat_value;

    if (voltage >= voltage_points[sizeof(voltage_points) / sizeof(voltage_points[0]) - 1]) {
        return 100;
    }
    if (voltage <= voltage_points[0]) {
        return 5;
    }

    for (size_t i = 1; i < sizeof(voltage_points) / sizeof(voltage_points[0]); ++i) {
        if (voltage <= voltage_points[i]) {
            float start_v = voltage_points[i - 1];
            float end_v = voltage_points[i];
            int start_percent = percent_points[i - 1];
            int end_percent = percent_points[i];
            float ratio = (voltage - start_v) / (end_v - start_v);
            int percent = start_percent + (int)lroundf(ratio * (end_percent - start_percent));

            if (percent < 5) {
                return 5;
            }
            if (percent > 100) {
                return 100;
            }
            return percent;
        }
    }

    return 80;
}

int app_get_battery_percent_for_log(void)
{
    return ui_get_battery_percent();
}

static void ui_update_battery_percent_labels(void)
{
    char text[8];
    int raw_percent = ui_get_battery_percent();
    int64_t now_us = esp_timer_get_time();

    if (UI_icon.charge_icon) {
        s_battery_percent_reinit_on_show = true;
        s_battery_percent_show_after_us = now_us + 3000 * 1000;
        for (size_t i = 0; i < s_battery_indicator_count; ++i) {
            if (s_battery_indicators[i].label) {
                lv_obj_add_flag(s_battery_indicators[i].label, LV_OBJ_FLAG_HIDDEN);
            }
        }
        return;
    }

    if (bat_value <= 0.0f) {
        return;
    }

    if (s_battery_percent_show_after_us > 0 && now_us < s_battery_percent_show_after_us) {
        for (size_t i = 0; i < s_battery_indicator_count; ++i) {
            if (s_battery_indicators[i].label) {
                lv_obj_add_flag(s_battery_indicators[i].label, LV_OBJ_FLAG_HIDDEN);
            }
        }
        return;
    }

    if (s_display_battery_percent < 0 || s_battery_percent_reinit_on_show) {
        s_display_battery_percent = raw_percent;
        s_battery_percent_reinit_on_show = false;
        s_battery_percent_show_after_us = 0;
    } else if (raw_percent < s_display_battery_percent) {
        s_display_battery_percent = raw_percent;
    }
    snprintf(text, sizeof(text), "%d", s_display_battery_percent);

    for (size_t i = 0; i < s_battery_indicator_count; ++i) {
        if (s_battery_indicators[i].label) {
            lv_obj_clear_flag(s_battery_indicators[i].label, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(s_battery_indicators[i].label, text);
            ui_position_battery_label(&s_battery_indicators[i]);
        }
    }
}

static void ui_update_battery_icons(void)
{
    switch (UI_icon.bat_icon) {
    case 0:
        ui_apply_battery_image(&img_bat_2);
        break;
    case 1:
        ui_apply_battery_image(&bat80Percent);
        break;
    case 2:
        ui_apply_battery_image(&bat60Percent);
        break;
    case 3:
        ui_apply_battery_image(&bat40Percent);
        break;
    case 4:
        ui_apply_battery_image(&bat20Percent);
        break;
    case 5:
        ui_apply_battery_image(&bat5Percent);
        lv_obj_clear_flag(objects.low_power_page, LV_OBJ_FLAG_HIDDEN);
        left_or_right_Animation(lv_obj_get_child(objects.low_power_page, 0), -118, 0, 500, 1, 50);
        left_or_right_Animation(lv_obj_get_child(objects.low_power_page, 1), 181, 118, 500, 1, 50);
        break;
    default:
        break;
    }

    ui_update_battery_percent_labels();
}

static void ui_update_charge_icons(void)
{
    lv_obj_t *screens[] = {
        objects.measure,
        objects.measure_roasted,
        objects.measure_green,
        objects.measure_parchment,
        objects.measure_dry_cherry,
        objects.setting,
        objects.setting_language,
        objects.zerong,
        objects.zerong_m_d,
        objects.zerong_aw,
        objects.zerong_m_d_ing,
        objects.zerong_aw_ing,
        objects.zerong_m_d_ok,
        objects.zerong_aw_ok,
        objects.zerong_aw_warm,
        objects.zerong_m_d_success,
        objects.zerong_aw_success,
        objects.about,
        objects.about_message,
        objects.setting_language_cn,
    };

    for (size_t i = 0; i < sizeof(screens) / sizeof(screens[0]); ++i) {
        ui_set_charge_icon_visibility_recursive(screens[i], UI_icon.charge_icon);
    }
}

void gui_task_UI_callback(ui_msg_t *msg)
{
    switch (msg->type) {
    case UI_MSG_UPDATE_TEMP:
        ui_handle_temp_update(msg);
        break;
#ifdef USE_TDS
    case UI_MSG_UPDATE_850:
        ui_handle_tds_update(msg);
        break;
#endif
    case UI_MSG_UPDATE_SENSOR:
        ui_update_sensor_labels(msg);
        break;
    case UI_MSG_UPDATE_LANUAGE:
        break;
    case UI_MSG_UPDATE_STATE:
        break;
    case UI_MSG_UPDATA_WIFI_BLE_ICON:
        ui_update_ble_icons();
        break;
    case UI_MSG_UPDATA_BAT_ICON:
        ui_update_battery_icons();
        ui_update_charge_icons();
        break;
    case UI_MSG_UPDATE_CHARGE_STATE:
        ui_update_charge_icons();
        break;
    default:
        break;
    }
}
