#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"

#include "esp_lcd_gc9307.h"
#include "lv_demos.h"


static const char *TAG = "example";

#define LCD_HOST  SPI2_HOST
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (40 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL  0
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_SCLK           6
#define EXAMPLE_PIN_NUM_MOSI           2
#define EXAMPLE_PIN_NUM_MISO           -1
#define EXAMPLE_PIN_NUM_LCD_DC         7
#define EXAMPLE_PIN_NUM_LCD_RST        8
#define EXAMPLE_PIN_NUM_LCD_CS         -1
#define EXAMPLE_PIN_NUM_BK_LIGHT       3

#define ON_OFF 0

// 水平和垂直方向的像素数
#define EXAMPLE_LCD_H_RES              320//172
#define EXAMPLE_LCD_V_RES              172//320

// Bit number used to represent command and parameter
#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8

#define EXAMPLE_LVGL_TICK_PERIOD_MS    2


extern void example_lvgl_demo_ui(lv_disp_t *disp);

static bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // 将缓冲区的内容复制到显示器的特定区域
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

/* 在 LVGL 中旋转屏幕时。驱动程序参数更新时调用。 */
static void example_lvgl_port_update_callback(lv_disp_drv_t *drv)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;

    switch (drv->rotated) {
    case LV_DISP_ROT_NONE:
        // 旋转液晶显示屏
        esp_lcd_panel_swap_xy(panel_handle, false);
        esp_lcd_panel_mirror(panel_handle, true, false);
        break;
    case LV_DISP_ROT_90:
        // 旋转液晶显示屏
        esp_lcd_panel_swap_xy(panel_handle, true);
        esp_lcd_panel_mirror(panel_handle, true, true);
        break;
    case LV_DISP_ROT_180:
        // 旋转液晶显示屏
        esp_lcd_panel_swap_xy(panel_handle, false);
        esp_lcd_panel_mirror(panel_handle, false, true);
        break;
    case LV_DISP_ROT_270:
        // 旋转液晶显示屏
        esp_lcd_panel_swap_xy(panel_handle, true);
        esp_lcd_panel_mirror(panel_handle, false, false);
        break;
    }
}

static void example_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

void app_main(void)
{
    // 配置 ON_OFF 引脚为输出模式，并初始化为高电平（打开某设备电源或主控电源）
    gpio_config_t on_off_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << ON_OFF
    };
    ESP_ERROR_CHECK(gpio_config(&on_off_gpio_config));
    gpio_set_level(ON_OFF, 1); // 打开 ON_OFF 电源（假定高电平有效）

    // 定义 LVGL 的绘制缓冲区和显示驱动结构体（静态，生命周期和主函数一致）
    static lv_disp_draw_buf_t disp_buf; // LVGL 的绘图缓冲区对象
    static lv_disp_drv_t disp_drv;      // LVGL 的显示驱动结构体

    ESP_LOGI(TAG, "Turn off LCD backlight");
    // 配置 LCD 背光引脚为输出，准备后续点亮背光
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    ESP_LOGI(TAG, "Initialize SPI bus");
    // SPI 总线配置（连接 LCD）
    spi_bus_config_t buscfg = {
        .sclk_io_num = EXAMPLE_PIN_NUM_SCLK,   // SPI 时钟线
        .mosi_io_num = EXAMPLE_PIN_NUM_MOSI,   // SPI MOSI 线
        .miso_io_num = EXAMPLE_PIN_NUM_MISO,   // SPI MISO 线
        .quadwp_io_num = -1,                   // 不使用 WP/HD
        .quadhd_io_num = -1,
        .max_transfer_sz = EXAMPLE_LCD_H_RES * 80 * sizeof(uint16_t), // 最大单次传输字节数
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO)); // 初始化 SPI 总线

    ESP_LOGI(TAG, "Install panel IO");
    // SPI panel IO 配置
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = EXAMPLE_PIN_NUM_LCD_DC,       // 数据/命令切换引脚
        .cs_gpio_num = EXAMPLE_PIN_NUM_LCD_CS,       // SPI 片选引脚
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,       // SPI 时钟频率
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,        // 命令位宽
        .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS,    // 参数位宽
        .spi_mode = 0,                              // SPI 模式0
        .trans_queue_depth = 10,                     // 传输队列深度
        .on_color_trans_done = example_notify_lvgl_flush_ready, // 颜色传输完成回调
        .user_ctx = &disp_drv,                       // 用户上下文，传递 LVGL 显示驱动
    };
    // 挂载 LCD 到 SPI 总线
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install GC9307 panel driver");
    // 创建 LCD 面板驱动实例（GC9307 驱动 IC）
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,  // 复位引脚
        .rgb_endian = LCD_RGB_ENDIAN_RGB,           // RGB 色彩顺序
        .bits_per_pixel = 16,                       // 每像素 16 位
    };
    //创建/初始化 GC9307 面板的驱动实例，为后续的 LCD 显示操作（如画图、刷新、配置方向等）做好准备。
    ESP_ERROR_CHECK(esp_lcd_new_panel_gc9307(io_handle, &panel_config, &panel_handle));

    // 初始化 LCD 面板
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));                // 复位 LCD
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));                 // 初始化 LCD
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 0, 34));       // 设置 LCD 画面偏移 gap（某些屏幕实际显示有像素偏移）
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, false));  // 镜像（X轴正向，Y轴不镜像）
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, 0));      // 反转颜色（有些屏幕显示颜色和代码逻辑相反）

    // 可选：在开背光/打开显示之前先显示图案，避免开机一瞬间花屏
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));    // 打开 LCD 显示

    ESP_LOGI(TAG, "Turn on LCD backlight");
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL); // 点亮背光

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init(); // 初始化 LVGL 核心库

    // 分配 LVGL 使用的双缓冲区（DMA 内存），建议至少为屏幕的 1/10 大小
    lv_color_t *buf1 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1);
    lv_color_t *buf2 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2);
    // 初始化 LVGL 绘图缓冲区，支持双缓冲（提升性能）
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * 20);

    ESP_LOGI(TAG, "Register display driver to LVGL");
    // 初始化并注册 LVGL 的显示驱动（重要！）
    lv_disp_drv_init(&disp_drv);               // 初始化结构体为默认值
    disp_drv.hor_res = EXAMPLE_LCD_H_RES;      // 水平分辨率
    disp_drv.ver_res = EXAMPLE_LCD_V_RES;      // 垂直分辨率
    disp_drv.flush_cb = example_lvgl_flush_cb; // 刷新回调函数，将 LVGL 的画面数据写到屏幕
    disp_drv.drv_update_cb = example_lvgl_port_update_callback; // 可选，驱动更新回调
    disp_drv.draw_buf = &disp_buf;             // LVGL 绘图缓冲区
    disp_drv.user_data = panel_handle;         // 传递 panel_handle 作为用户数据
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv); // 注册显示驱动

    ESP_LOGI(TAG, "Install LVGL tick timer");
    // 配置 LVGL 的 Tick 计时器（LVGL 需要定期更新 tick，驱动动画/刷新）
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,              // 回调函数，每隔一段时间调用一次
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000)); // 定时器单位是微秒

    ESP_LOGI(TAG, "Display LVGL Meter Widget");
    // 显示 LVGL 界面，可以切换不同的 LVGL 界面 demo
    example_lvgl_demo_ui(disp); // 可选：自定义 demo
    // lv_demo_music(); // LVGL 自带音乐播放器 demo

    // 主循环，不断调用 LVGL 任务处理函数（定时驱动 LVGL 刷新和事件处理）
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10)); // 延时 10ms，降低本任务优先级
        lv_timer_handler();            // 处理 LVGL 相关任务（刷新、动画等）
    }
}
