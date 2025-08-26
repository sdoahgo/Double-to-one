#include <stdio.h>
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "nvs_flash.h"
#include <math.h>
#include "user_key.h"
#include "GD60914.h"
#include "TF_Luna.h"
#include "esp_lcd_st7789v.h"
#include "lv_demos.h"
#include "user_bat.h"
#include "user_hal.h"
#include "user_ble.h"
#include "images.h"

sys_data SYS_DATA;
ui_icon_t UI_icon;
QueueHandle_t ui_msg_queue = NULL; 
static const char *TAG = "example";

#define LCD_HOST  SPI2_HOST
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (40 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL  0
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL


#define EXAMPLE_PIN_NUM_SCLK           8
#define EXAMPLE_PIN_NUM_MOSI           7
#define EXAMPLE_PIN_NUM_MISO           -1
#define EXAMPLE_PIN_NUM_LCD_DC         6
#define EXAMPLE_PIN_NUM_LCD_RST        2
#define EXAMPLE_PIN_NUM_LCD_CS         -1
#define EXAMPLE_PIN_NUM_BK_LIGHT       3

#define ON_OFF 0
#define Power_CTRL 5

// 水平和垂直方向的像素数
#define EXAMPLE_LCD_H_RES              320//172
#define EXAMPLE_LCD_V_RES              172//320

// Bit number used to represent command and parameter
#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8

#define EXAMPLE_LVGL_TICK_PERIOD_MS    2
void gui_task_key_callback(uint8_t *event);
void gui_task_UI_callback(ui_msg_t *msg);

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
lv_disp_drv_t disp_drv;      // LVGL 的显示驱动结构体
void app_main(void)
{
    // 配置 ON_OFF 引脚为输出模式，并初始化为高电平（打开某设备电源或主控电源）
    gpio_config_t on_off_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << ON_OFF | 1ULL << Power_CTRL | 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT 
    };
    ESP_ERROR_CHECK(gpio_config(&on_off_gpio_config));
    gpio_set_level(ON_OFF, 1); 
    gpio_set_level(Power_CTRL, 1);
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, 0); 

    int ret=nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    user_device_init();

    UI_icon.bat_icon = false;
    UI_icon.bat_icon = 0;
    UI_icon.charge_icon = false;

    // 定义 LVGL 的绘制缓冲区和显示驱动结构体（静态，生命周期和主函数一致）
    static lv_disp_draw_buf_t disp_buf; // LVGL 的绘图缓冲区对象
    // static lv_disp_drv_t disp_drv;      // LVGL 的显示驱动结构体
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
    // 挂载 LCD 到 SPI 总线
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install GC9307 panel driver");
    // 创建 LCD 面板驱动实例（GC9307 驱动 IC）
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,  // 复位引脚
        // .reset_gpio_num = -1,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,           // RGB 色彩顺序
        .bits_per_pixel = 16,                       // 每像素 16 位
    };
    //创建/初始化 GC9307 面板的驱动实例，为后续的 LCD 显示操作（如画图、刷新、配置方向等）做好准备。
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789v(io_handle, &panel_config, &panel_handle));

    // 初始化 LCD 面板
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));                
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));                
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 0, 34));       
    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, true, false);
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, false)); 

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init(); // 初始化 LVGL 核心库

    lv_color_t *buf1 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1);
    lv_color_t *buf2 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2);
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * 20);

    ESP_LOGI(TAG, "Register display driver to LVGL");
    // 初始化并注册 LVGL 的显示驱动（重要！）
    lv_disp_drv_init(&disp_drv);               
    disp_drv.hor_res = EXAMPLE_LCD_H_RES;     
    disp_drv.ver_res = EXAMPLE_LCD_V_RES;      
    disp_drv.flush_cb = example_lvgl_flush_cb; 
    disp_drv.drv_update_cb = example_lvgl_port_update_callback; 
    disp_drv.draw_buf = &disp_buf;            
    disp_drv.user_data = panel_handle;         
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv); 

    ESP_LOGI(TAG, "Install LVGL tick timer");
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,             
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000)); // 定时器单位是微秒


    user_key_init();
    user_ble_init();
    ui_msg_queue = xQueueCreate(10, sizeof(ui_msg_t));
    ui_msg_t msg;
    myi2c_Init();

    #ifdef USE_TDS
        driver_TF_Luna_init();
    #endif
    bool flags = false;
    xTaskCreate(GD60914_task, "GD60914_task", (1024 * 2), (void *)NULL, (tskIDLE_PRIORITY+4), NULL);
    xTaskCreate(bat_task, "bat_task", (1024 * 2), (void *)NULL, (tskIDLE_PRIORITY+3), NULL);
    ESP_LOGI(TAG, "Display LVGL Meter Widget"); 

    ui_init();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10)); // 延时 10ms，降低本任务优先级
        if(flags == false)
        {
            ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));    // 打开 LCD 显示
            flags = true;
        }
        if (xQueueReceive(KeyQueue, &key_event, 0)) {
            gui_task_key_callback(&key_event);
        }
        if(xQueueReceive(ui_msg_queue, &msg, 0)){
            gui_task_UI_callback(&msg);
        }
        lv_timer_handler();            // 处理 LVGL 相关任务（刷新、动画等）
    }
}
volatile uint8_t Screens_ID = 1;
void gui_task_key_callback(uint8_t *event)
{
    switch (*event)
    {
    case USER_KEY_SHORT_IN_EVT:
        if(GIF_end_flag)
        {
            Screens_ID = Screens_ID +1;
            if(Screens_ID >= 5)
            {
                Screens_ID = 1;
            }
            loadScreen(Screens_ID);
            switch (Screens_ID)
            {
            case SCREEN_ID_CALIBRATION_PAGE:
                lv_obj_set_style_bg_color(objects.calibration_bt, lv_color_hex(0xffa851ff), LV_PART_MAIN | LV_STATE_DEFAULT);
                break;
            
            default:
                break;
            }
        }
        break;
    case USER_KEY_LONG_IN_EVT:
        
        break; 
    case USER_KEY_DOUBLE_CLICK_IN_EVT:
        printf("USER_KEY_DOUBLE_CLICK_IN_EVT %d\n",Screens_ID);
        switch (Screens_ID)
        {
        case SCREEN_ID_LANGUAGE_PAGE:
            SYS_DATA.language_id ++ ;
            if(SYS_DATA.language_id>1){
                SYS_DATA.language_id = 0;
            }
            user_save_sys_evn(&SYS_DATA);
            if(SYS_DATA.language_id){//英文
                lv_obj_set_style_bg_color(objects.chinese_bt, lv_color_hex(0xff505050), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_color(objects.engilsh_bt, lv_color_hex(0xffa851ff), LV_PART_MAIN | LV_STATE_DEFAULT);
            }   
            else{//中文
                lv_obj_set_style_bg_color(objects.chinese_bt, lv_color_hex(0xffa851ff), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_color(objects.engilsh_bt, lv_color_hex(0xff505050), LV_PART_MAIN | LV_STATE_DEFAULT);
            }
            #ifdef USE_TEMP
            lv_label_set_text(lv_obj_get_child(objects.temp_container,5), UI_STRING[0][SYS_DATA.language_id]);
            #endif
            lv_label_set_text(objects.language_switch_prompt, UI_STRING[1][SYS_DATA.language_id]);
            lv_label_set_text(objects.calibration_prompt, UI_STRING[2][SYS_DATA.language_id]);
            lv_label_set_text(lv_obj_get_child(objects.calibration_bt,0), UI_STRING[3][SYS_DATA.language_id]);
            lv_label_set_text(lv_obj_get_child(objects.product_name_panel,1), UI_STRING[6][SYS_DATA.language_id]);
            lv_label_set_text(lv_obj_get_child(objects.model_panel,1), UI_STRING[7][SYS_DATA.language_id]);
            lv_label_set_text(lv_obj_get_child(objects.software_version_panel,1), UI_STRING[9][SYS_DATA.language_id]);
            lv_label_set_text(lv_obj_get_child(objects.hardware_version_panel,1), UI_STRING[8][SYS_DATA.language_id]);
            break;
        case SCREEN_ID_CALIBRATION_PAGE:
            lv_event_send(objects.calibration_bt, LV_EVENT_CLICKED, NULL);
            break;
        default:
            break;
        }
        break; 
    case USER_KEY_MULTIPLE_CLICK_IN_EVT:
        switch_ota_and_reboot();
        break;
    default:
        break;
    }

}

void gui_task_UI_callback(ui_msg_t *msg){
    switch (msg->type)
    {
        case UI_MSG_UPDATE_TEMP:
        int temp_f_times100 = msg->temp_value * 18 + 3200;//将摄氏度的10倍，转换成华氏度并100倍
        int temp_f1 = temp_f_times100/100;
        int temp_f2 = temp_f_times100%100;
        switch (Screens_ID)
        {
            #ifdef USE_TDS
            case SCREEN_ID_MEASURE_TDS:
                lv_obj_t *child1 = lv_obj_get_child(objects.tds_temp_container, 1);
                lv_obj_t *child2 = lv_obj_get_child(objects.tds_temp_container, 2);
                lv_obj_t *child3 = lv_obj_get_child(objects.tds_temp_container, 3);
                lv_obj_t *child4 = lv_obj_get_child(objects.tds_temp_container, 4);
                // if(msg->temp_value >=  1000)
                // {
                //     lv_label_set_text_fmt(child1,"%d.",msg->temp_value/10);
                // }
                // else{
                //     lv_label_set_text_fmt(child1,"0%d.",msg->temp_value/10);
                // }
                // lv_label_set_text_fmt(child2,"%d0℃",msg->temp_value%10);
                // if(temp_f1 >= 100){
                //     lv_label_set_text_fmt(child3,"%d.",temp_f1);    
                // }
                // else{
                //     lv_label_set_text_fmt(child3,"0%d.",temp_f1);
                // }
                // if(temp_f2 == 0){
                //     lv_label_set_text(child4, "00℉");
                // }
                // else{
                //     lv_label_set_text_fmt(child4,"%d℉",temp_f2);
                // }
                if(msg->temp_value >=  1000)
                {
                    lv_obj_set_pos(child2, 57, 4);
                    lv_obj_set_pos(child4, 57, 36);
                    lv_label_set_text_fmt(child1,
                    "%s%d.", msg->temp_value < 0 ? "-" : "",
                    (msg->temp_value < 0 ? -msg->temp_value : msg->temp_value) / 10);
                }
                else if(msg->temp_value < 1000 && msg->temp_value >= 100)
                {
                    lv_obj_set_pos(child2, 45, 4);
                    lv_obj_set_pos(child4, 45, 36);
                    lv_label_set_text_fmt(child1,
                    "%s%d.", msg->temp_value < 0 ? "-" : "",
                    (msg->temp_value < 0 ? -msg->temp_value : msg->temp_value) / 10);
                }
                else if(msg->temp_value < 100 && msg->temp_value >= 0)
                {
                    lv_obj_set_pos(child2, 45, 4);
                    lv_obj_set_pos(child4, 45, 36);
                    lv_label_set_text_fmt(child1,
                    "%s0%d.", msg->temp_value < 0 ? "-" : "",
                    (msg->temp_value < 0 ? -msg->temp_value : msg->temp_value) / 10);
                }
                else if(msg->temp_value < 0 && msg->temp_value > -100)
                {
                    lv_obj_set_pos(child2, 57, 4);
                    lv_obj_set_pos(child4, 57, 36);
                    lv_label_set_text_fmt(child1,
                    "%s0%d.", msg->temp_value < 0 ? "-" : "",
                    (msg->temp_value < 0 ? -msg->temp_value : msg->temp_value) / 10);
                }
                else
                {
                    lv_obj_set_pos(child2, 57, 4);
                    lv_obj_set_pos(child4, 57, 36);
                    lv_label_set_text_fmt(child1,
                    "%s%d.", msg->temp_value < 0 ? "-" : "",
                    (msg->temp_value < 0 ? -msg->temp_value : msg->temp_value) / 10);
                }
                lv_label_set_text_fmt(child2,"%d0℃",(msg->temp_value % 10 + 10) % 10);
                 lv_label_set_text_fmt(child3,"%d.",temp_f1);
                 lv_label_set_text_fmt(child4,"%d℉",temp_f2);

            break;
            #endif
            #ifdef USE_TEMP
            case SCREEN_ID_MEASURE_TEMP:
                // if(msg->temp_value >=  1000){
                //     lv_label_set_text_fmt(lv_obj_get_child(objects.temp_container,2),"%d.",msg->temp_value/10);    
                // }
                // else{
                //     lv_label_set_text_fmt(lv_obj_get_child(objects.temp_container,2),"0%d.",msg->temp_value/10);
                // }
                // if(temp_f_times100>=10000){
                //     lv_label_set_text_fmt(lv_obj_get_child(objects.temp_container,3),"%.2f℉",(float)temp_f_times100 / 100.0f);     
                // }
                // else{
                //     lv_label_set_text_fmt(lv_obj_get_child(objects.temp_container,3),"0%.2f℉",(float)temp_f_times100 / 100.0f);
                // } 
                if(msg->temp_value >=  1000)
                {
                    lv_obj_set_pos(lv_obj_get_child(objects.temp_container,0), 192, 59);
                    lv_obj_set_pos(lv_obj_get_child(objects.temp_container,1), 144, 43);
                    lv_obj_set_pos(lv_obj_get_child(objects.temp_container,2), 20, 24);
                    lv_obj_set_pos(lv_obj_get_child(objects.temp_container,3), 132, 1);
                    lv_label_set_text_fmt(lv_obj_get_child(objects.temp_container,2),
                                    "%s%d.", msg->temp_value < 0 ? "-" : "",
                                    (msg->temp_value < 0 ? -msg->temp_value : msg->temp_value) / 10);
                }
                else if(msg->temp_value < 1000 && msg->temp_value >= 100)
                {
                    lv_obj_set_pos(lv_obj_get_child(objects.temp_container,0), 156, 59);
                    lv_obj_set_pos(lv_obj_get_child(objects.temp_container,1), 108, 43);
                    if(temp_f_times100>=10000)
                    {
                        lv_obj_set_pos(lv_obj_get_child(objects.temp_container,3), 96, 1);
                    }
                    else{
                        lv_obj_set_pos(lv_obj_get_child(objects.temp_container,3), 108, 1);
                    }
                    lv_label_set_text_fmt(lv_obj_get_child(objects.temp_container,2),
                                    "%s%d.", msg->temp_value < 0 ? "-" : "",
                                    (msg->temp_value < 0 ? -msg->temp_value : msg->temp_value) / 10);
                }
                else if(msg->temp_value < 100 && msg->temp_value >= 0)
                {
                    lv_obj_set_pos(lv_obj_get_child(objects.temp_container,0), 156, 59);
                    lv_obj_set_pos(lv_obj_get_child(objects.temp_container,1), 108, 43);
                    lv_obj_set_pos(lv_obj_get_child(objects.temp_container,3), 108, 1);
                    lv_label_set_text_fmt(lv_obj_get_child(objects.temp_container,2),
                                    "%s0%d.", msg->temp_value < 0 ? "-" : "",
                                    (msg->temp_value < 0 ? -msg->temp_value : msg->temp_value) / 10);
                }
                else if(msg->temp_value < 0 && msg->temp_value > -100)
                {
                    lv_obj_set_pos(lv_obj_get_child(objects.temp_container,0), 192, 59);
                    lv_obj_set_pos(lv_obj_get_child(objects.temp_container,1), 144, 43);
                    lv_obj_set_pos(lv_obj_get_child(objects.temp_container,3), 144, 1);
                    lv_label_set_text_fmt(lv_obj_get_child(objects.temp_container,2),
                                    "%s0%d.", msg->temp_value < 0 ? "-" : "",
                                    (msg->temp_value < 0 ? -msg->temp_value : msg->temp_value) / 10);
                }
                else{
                    lv_obj_set_pos(lv_obj_get_child(objects.temp_container,0), 192, 59);
                    lv_obj_set_pos(lv_obj_get_child(objects.temp_container,1), 144, 43);
                    lv_obj_set_pos(lv_obj_get_child(objects.temp_container,2), 20, 24);
                    lv_obj_set_pos(lv_obj_get_child(objects.temp_container,3), 144, 1);
                    lv_label_set_text_fmt(lv_obj_get_child(objects.temp_container,2),
                                    "%s%d.", msg->temp_value < 0 ? "-" : "",
                                    (msg->temp_value < 0 ? -msg->temp_value : msg->temp_value) / 10);
                }
                lv_label_set_text_fmt(lv_obj_get_child(objects.temp_container,1),"%d0",(msg->temp_value % 10 + 10) % 10);
                lv_label_set_text_fmt(lv_obj_get_child(objects.temp_container,3),"%.2f℉",(float)temp_f_times100 / 100.0f);
            break;
            #endif
            default:
            break;
        }
    break;
    #ifdef USE_TDS
    case UI_MSG_UPDATE_850:
        uint8_t hundreds = msg->TF_DIST_value / 100;   // 包含多少个100
        uint8_t remainder = msg->TF_DIST_value % 100;  // 百位以下是多少
        if(remainder < 10)
        {
            lv_label_set_text_fmt(lv_obj_get_child(objects.tds_container,3), "0%d",remainder);
        }
        else
        {
            lv_label_set_text_fmt(lv_obj_get_child(objects.tds_container,3), "%d", remainder);
        }
        if(hundreds < 10){
            lv_label_set_text_fmt(lv_obj_get_child(objects.tds_container,2), "0%d.", hundreds);
        }else{
            lv_label_set_text_fmt(lv_obj_get_child(objects.tds_container,2), "%d.", hundreds);
        }
        break;
    #endif
    case UI_MSG_UPDATE_LANUAGE:
        /* code */
        break;
    case UI_MSG_UPDATE_STATE:
        /* code */
        break;
    case UI_MSG_UPDATA_WIFI_BLE_ICON:
        if(UI_icon.ble_icon){
            #ifdef USE_TEMP
            lv_obj_clear_flag(objects.wifi,LV_OBJ_FLAG_HIDDEN);
            #endif
            lv_obj_clear_flag(objects.wifi_1,LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(objects.wifi_2,LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(objects.wifi_3,LV_OBJ_FLAG_HIDDEN);
            #ifdef USE_TDS
            lv_obj_clear_flag(objects.wifi_4,LV_OBJ_FLAG_HIDDEN);
            #endif
        }
        else{
            #ifdef USE_TEMP
            lv_obj_add_flag(objects.wifi,LV_OBJ_FLAG_HIDDEN);
            #endif
            lv_obj_add_flag(objects.wifi_1,LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(objects.wifi_2,LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(objects.wifi_3,LV_OBJ_FLAG_HIDDEN);
            #ifdef USE_TDS
            lv_obj_add_flag(objects.wifi_4,LV_OBJ_FLAG_HIDDEN);
            #endif
        }
        break;
    case UI_MSG_UPDATA_BAT_ICON:
        switch (UI_icon.bat_icon)
        {
        case 0:
            #ifdef USE_TEMP
            lv_img_set_src(objects.bat,&bat80Percent);
            #endif
            lv_img_set_src(objects.bat_1,&bat80Percent);
            lv_img_set_src(objects.bat_2,&bat80Percent);
            lv_img_set_src(objects.bat_3,&bat80Percent);
            #ifdef USE_TDS
            lv_img_set_src(objects.bat_4,&bat80Percent);
            #endif
            break;
        case 1:
            #ifdef USE_TEMP
            lv_img_set_src(objects.bat,&bat60Percent);
            #endif
            lv_img_set_src(objects.bat_1,&bat60Percent);
            lv_img_set_src(objects.bat_2,&bat60Percent);
            lv_img_set_src(objects.bat_3,&bat60Percent);
            #ifdef USE_TDS
            lv_img_set_src(objects.bat_4,&bat60Percent);
            #endif
            break;
        case 2:
            #ifdef USE_TEMP
            lv_img_set_src(objects.bat,&bat40Percent);
            #endif
            lv_img_set_src(objects.bat_1,&bat40Percent);
            lv_img_set_src(objects.bat_2,&bat40Percent);
            lv_img_set_src(objects.bat_3,&bat40Percent);
            #ifdef USE_TDS
            lv_img_set_src(objects.bat_4,&bat40Percent);
            #endif
            break;
        case 3:
            #ifdef USE_TEMP
            lv_img_set_src(objects.bat,&bat20Percent);
            #endif
            lv_img_set_src(objects.bat_1,&bat40Percent);
            lv_img_set_src(objects.bat_2,&bat40Percent);
            lv_img_set_src(objects.bat_3,&bat40Percent);
            #ifdef USE_TDS
            lv_img_set_src(objects.bat_4,&bat40Percent);
            #endif
            break;
        case 4:
            #ifdef USE_TEMP
            lv_img_set_src(objects.bat,&bat20Percent);
            #endif
            lv_img_set_src(objects.bat_1,&bat20Percent);
            lv_img_set_src(objects.bat_2,&bat20Percent);
            lv_img_set_src(objects.bat_3,&bat20Percent);
            #ifdef USE_TDS
            lv_img_set_src(objects.bat_4,&bat20Percent);
            #endif    
            break;
        case 5:
            #ifdef USE_TEMP
            lv_img_set_src(objects.bat,&bat5Percent);
            #endif
            lv_img_set_src(objects.bat_1,&bat5Percent);
            lv_img_set_src(objects.bat_2,&bat5Percent);
            lv_img_set_src(objects.bat_3,&bat5Percent);
            #ifdef USE_TDS
            lv_img_set_src(objects.bat_4,&bat5Percent);
            #endif
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}