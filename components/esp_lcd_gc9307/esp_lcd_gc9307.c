/*
 * SPDX-FileCopyrightText: 2021-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <sys/cdefs.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_check.h"

#include "esp_lcd_gc9307.h"

static const char *TAG = "gc9307";

static esp_err_t panel_gc9307_del(esp_lcd_panel_t *panel);
static esp_err_t panel_gc9307_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_gc9307_init(esp_lcd_panel_t *panel);
static esp_err_t panel_gc9307_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data);
static esp_err_t panel_gc9307_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t panel_gc9307_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t panel_gc9307_swap_xy(esp_lcd_panel_t *panel, bool swap_axes);
static esp_err_t panel_gc9307_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap);
static esp_err_t panel_gc9307_disp_on_off(esp_lcd_panel_t *panel, bool off);

typedef struct {
    esp_lcd_panel_t base;                   // LCD 面板的基础操作接口（多态实现，包含一组函数指针）
    esp_lcd_panel_io_handle_t io;           // LCD 面板的 SPI（或其它总线）通信句柄
    int reset_gpio_num;                     // 复位引脚编号（-1表示不用硬件复位）
    bool reset_level;                       // 复位信号有效电平（true=高电平复位，false=低电平复位）
    int x_gap;                              // X 方向显示偏移（部分屏幕需要调整显示起始位置）
    int y_gap;                              // Y 方向显示偏移
    uint8_t fb_bits_per_pixel;              // frame buffer 每像素占用的位数（如16/18/24）
    uint8_t madctl_val;                     // 记录 LCD_CMD_MADCTL（方向/镜像）寄存器当前值，便于后续方向控制
    uint8_t colmod_val;                     // 记录 LCD_CMD_COLMOD（色深模式）寄存器当前值，便于后续数据格式控制
    const gc9307_lcd_init_cmd_t *init_cmds; // 指向屏厂/供应商提供的初始化命令序列数组（通常为寄存器设置列表）
    uint16_t init_cmds_size;                // 初始化命令数组的长度
} gc9307_panel_t;


esp_err_t esp_lcd_new_panel_gc9307(
    const esp_lcd_panel_io_handle_t io,
    const esp_lcd_panel_dev_config_t *panel_dev_config,
    esp_lcd_panel_handle_t *ret_panel)
{
    esp_err_t ret = ESP_OK;
    gc9307_panel_t *gc9307 = NULL;
    gpio_config_t io_conf = { 0 }; // GPIO 配置结构体，初始化为 0

    // 1. 参数有效性检查，任何参数为 NULL 都返回 ESP_ERR_INVALID_ARG
    ESP_GOTO_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");

    // 2. 分配 gc9307_panel_t 结构体内存，calloc 自动初始化为 0
    gc9307 = (gc9307_panel_t *)calloc(1, sizeof(gc9307_panel_t));
    //检查内存分配是否失败
    ESP_GOTO_ON_FALSE(gc9307, ESP_ERR_NO_MEM, err, TAG, "no mem for gc9307 panel");

    // 3. 配置并初始化 LCD reset 引脚（如果使用了外部硬件复位引脚）
    if (panel_dev_config->reset_gpio_num >= 0) {
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num;
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
    }

    // 4. 设置 MADCTL 控制字节中的 RGB/BGR 顺序
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    // 旧 IDF 版本用 color_space 字段
    switch (panel_dev_config->color_space) {
    case ESP_LCD_COLOR_SPACE_RGB:
        gc9307->madctl_val = 0; // 默认 RGB 顺序
        break;
    case ESP_LCD_COLOR_SPACE_BGR:
        gc9307->madctl_val |= LCD_CMD_BGR_BIT; // BGR 顺序，设置 BGR 位
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported color space");
        break;
    }
#else
    // 新 IDF 版本用 rgb_endian 字段
    switch (panel_dev_config->rgb_endian) {
    case LCD_RGB_ENDIAN_RGB:
        gc9307->madctl_val = 0; // RGB 顺序
        break;
    case LCD_RGB_ENDIAN_BGR:
        gc9307->madctl_val |= LCD_CMD_BGR_BIT; // BGR 顺序
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported rgb endian");
        break;
    }
#endif

    // 5. 设置每像素位宽和 COLMOD 寄存器值（屏幕数据格式）
    switch (panel_dev_config->bits_per_pixel) {
    case 16: // RGB565 模式
        gc9307->colmod_val = 0x55;
        gc9307->fb_bits_per_pixel = 16;
        break;
    case 18: // RGB666 模式
        gc9307->colmod_val = 0x66;
        // RGB666，每像素3字节，实际每个色彩分量占6高位
        gc9307->fb_bits_per_pixel = 24;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported pixel width");
        break;
    }

    // 6. 填充结构体剩余参数和函数指针（驱动接口多态）
    gc9307->io = io;                                         // SPI IO handle
    gc9307->reset_gpio_num = panel_dev_config->reset_gpio_num;
    gc9307->reset_level = panel_dev_config->flags.reset_active_high; // 复位高电平有效还是低电平有效
    if (panel_dev_config->vendor_config) {
        // 厂商私有扩展配置
        gc9307->init_cmds = ((gc9307_vendor_config_t *)panel_dev_config->vendor_config)->init_cmds;
        gc9307->init_cmds_size = ((gc9307_vendor_config_t *)panel_dev_config->vendor_config)->init_cmds_size;
    }
    // 赋值 LCD 面板标准接口，实现多态
    gc9307->base.del = panel_gc9307_del;                         // 删除/释放
    gc9307->base.reset = panel_gc9307_reset;                     // 复位操作
    gc9307->base.init = panel_gc9307_init;                       // 初始化
    gc9307->base.draw_bitmap = panel_gc9307_draw_bitmap;         // 画图
    gc9307->base.invert_color = panel_gc9307_invert_color;       // 颜色反转
    gc9307->base.set_gap = panel_gc9307_set_gap;                 // 设置显示偏移
    gc9307->base.mirror = panel_gc9307_mirror;                   // 镜像显示
    gc9307->base.swap_xy = panel_gc9307_swap_xy;                 // 行列互换
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    gc9307->base.disp_off = panel_gc9307_disp_on_off;            // 旧版本接口
#else
    gc9307->base.disp_on_off = panel_gc9307_disp_on_off;         // 新版本接口
#endif

    // 7. 返回面板对象 handle，供上层调用
    *ret_panel = &(gc9307->base);

    ESP_LOGD(TAG, "new gc9307 panel @%p", gc9307);

    return ESP_OK; // 初始化成功

err:
    // 错误处理，释放已分配资源
    if (gc9307) {
        if (panel_dev_config->reset_gpio_num >= 0) {
            gpio_reset_pin(panel_dev_config->reset_gpio_num); // 释放 GPIO 资源
        }
        free(gc9307); // 释放内存
    }
    return ret; // 返回错误码
}


static esp_err_t panel_gc9307_del(esp_lcd_panel_t *panel)
{
    gc9307_panel_t *gc9307 = __containerof(panel, gc9307_panel_t, base);

    if (gc9307->reset_gpio_num >= 0) {
        gpio_reset_pin(gc9307->reset_gpio_num);
    }
    ESP_LOGD(TAG, "del gc9307 panel @%p", gc9307);
    free(gc9307);
    return ESP_OK;
}

static esp_err_t panel_gc9307_reset(esp_lcd_panel_t *panel)
{
    gc9307_panel_t *gc9307 = __containerof(panel, gc9307_panel_t, base);
    esp_lcd_panel_io_handle_t io = gc9307->io;

    // perform hardware reset
    if (gc9307->reset_gpio_num >= 0) {
        gpio_set_level(gc9307->reset_gpio_num, gc9307->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(gc9307->reset_gpio_num, !gc9307->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
    } else { // perform software reset
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(20)); // spec, wait at least 5ms before sending new command
    }

    return ESP_OK;
}

static const gc9307_lcd_init_cmd_t vendor_specific_init_default[] = {
//  {cmd, { data }, data_size, delay_ms}
    {0xfe, (uint8_t []){0x00}, 0, 0},
    {0xef, (uint8_t []){0x00}, 0, 0},
    {0x36, (uint8_t []){0x68}, 1, 0},
    {0x3a, (uint8_t []){0x05}, 1, 0},
    {0x85, (uint8_t []){0xc0}, 1, 0},
    {0x86, (uint8_t []){0x98}, 1, 0},
    {0x87, (uint8_t []){0x28}, 1, 0},
    {0x89, (uint8_t []){0x33}, 1, 0},
    {0x8B, (uint8_t []){0x84}, 1, 0},
    {0x8D, (uint8_t []){0x3B}, 1, 0},
    {0x8E, (uint8_t []){0x0f}, 1, 0},
    {0x8F, (uint8_t []){0x70}, 1, 0},
    {0xe8, (uint8_t []){0x13,0x17}, 2, 0},
    {0xec, (uint8_t []){0x57,0x07,0xff}, 3, 0},
    {0xed, (uint8_t []){0x18,0x09}, 2, 0},
    {0xc9, (uint8_t []){0x10}, 1, 0},
    {0xff, (uint8_t []){0x61}, 1, 0},
    {0x99, (uint8_t []){0x3A}, 1, 0},
    {0x9d, (uint8_t []){0x43}, 1, 0},
    {0x98, (uint8_t []){0x3e}, 1, 0},
    {0x9c, (uint8_t []){0x4b}, 1, 0},
    {0xF0, (uint8_t []){0x06,0x08,0x08,0x06,0x05,0x1d}, 6, 0},
    {0xF2, (uint8_t []){0x00,0x01,0x09,0x07,0x04,0x23}, 6, 0},
    {0xF1, (uint8_t []){0x3b,0x68,0x66,0x36,0x35,0x2f}, 6, 0},
    {0xF3, (uint8_t []){0x37,0x6a,0x66,0x37,0x35,0x35}, 6, 0},
    {0xFA, (uint8_t []){0x80,0x0f}, 2, 0},
    {0xBE, (uint8_t []){0x11}, 1, 0},
    {0xCB, (uint8_t []){0x02}, 1, 0},
    {0xCD, (uint8_t []){0x22}, 1, 0},
    {0x9B, (uint8_t []){0xFF}, 1, 0},
    {0x35, (uint8_t []){0x00}, 1, 0},
    {0x44, (uint8_t []){0x00,0x0a}, 2, 0},
    {0x11, (uint8_t []){0x00}, 0, 200},
    {0x29, (uint8_t []){0x00}, 0, 0},
    {0x2c, (uint8_t []){0x00}, 0, 0},
};

static esp_err_t panel_gc9307_init(esp_lcd_panel_t *panel)
{
    // 1. 通过 base 指针获取完整的 gc9307_panel_t 对象（面向对象多态实现）
    gc9307_panel_t *gc9307 = __containerof(panel, gc9307_panel_t, base);
    esp_lcd_panel_io_handle_t io = gc9307->io; // IO 句柄，后续用于发送命令

    // 2. 首先退出睡眠模式（很多屏上电默认在 sleep，需先唤醒）
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_SLPOUT, NULL, 0), TAG, "send command failed");
    vTaskDelay(pdMS_TO_TICKS(100)); // 等待屏幕稳定醒来

    // 3. 设置屏幕内存访问控制方向（MADCTL寄存器）和色深（COLMOD寄存器）
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
        gc9307->madctl_val,
    }, 1), TAG, "send command failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_COLMOD, (uint8_t[]) {
        gc9307->colmod_val,
    }, 1), TAG, "send command failed");

    // 4. 获取初始化命令序列和长度（优先用厂商自定义命令，否则用默认内置）
    const gc9307_lcd_init_cmd_t *init_cmds = NULL;
    uint16_t init_cmds_size = 0;
    if (gc9307->init_cmds) {
        // 如果用户传入了厂商初始化命令，则优先使用
        init_cmds = gc9307->init_cmds;
        init_cmds_size = gc9307->init_cmds_size;
    } else {
        // 否则使用驱动库自带的默认命令序列
        init_cmds = vendor_specific_init_default;
        init_cmds_size = sizeof(vendor_specific_init_default) / sizeof(gc9307_lcd_init_cmd_t);
    }

    // 5. 遍历所有初始化命令，逐条写入屏幕
    bool is_cmd_overwritten = false;
    for (int i = 0; i < init_cmds_size; i++) {
        // 检查是否有初始化命令和之前内部已设置的 MADCTL/COLMOD 冲突，避免重复配置
        switch (init_cmds[i].cmd) {
        case LCD_CMD_MADCTL: // 内存访问方向控制寄存器
            is_cmd_overwritten = true;
            gc9307->madctl_val = ((uint8_t *)init_cmds[i].data)[0]; // 更新缓存的值
            break;
        case LCD_CMD_COLMOD: // 像素色深寄存器
            is_cmd_overwritten = true;
            gc9307->colmod_val = ((uint8_t *)init_cmds[i].data)[0]; // 更新缓存的值
            break;
        default:
            is_cmd_overwritten = false;
            break;
        }

        // 如果有重复设置，打印警告（优先使用用户初始化命令，覆盖库默认值）
        if (is_cmd_overwritten) {
            ESP_LOGW(TAG, "The %02Xh command has been used and will be overwritten by external initialization sequence", init_cmds[i].cmd);
        }

        // 发送当前初始化命令（一般是写寄存器+参数）
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, init_cmds[i].cmd, init_cmds[i].data, init_cmds[i].data_bytes), TAG, "send command failed");
        // 部分命令后需延时（防止屏幕处理不过来或命令要求有间隔）
        vTaskDelay(pdMS_TO_TICKS(init_cmds[i].delay_ms));
    }
    ESP_LOGD(TAG, "send init commands success"); // 打印调试信息

    return ESP_OK; // 初始化成功
}


static esp_err_t panel_gc9307_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    gc9307_panel_t *gc9307 = __containerof(panel, gc9307_panel_t, base);
    assert((x_start < x_end) && (y_start < y_end) && "start position must be smaller than end position");
    esp_lcd_panel_io_handle_t io = gc9307->io;

    x_start += gc9307->x_gap;
    x_end += gc9307->x_gap;
    y_start += gc9307->y_gap;
    y_end += gc9307->y_gap;

    // define an area of frame memory where MCU can access
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_CASET, (uint8_t[]) {
        (x_start >> 8) & 0xFF,
        x_start & 0xFF,
        ((x_end - 1) >> 8) & 0xFF,
        (x_end - 1) & 0xFF,
    }, 4), TAG, "send command failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_RASET, (uint8_t[]) {
        (y_start >> 8) & 0xFF,
        y_start & 0xFF,
        ((y_end - 1) >> 8) & 0xFF,
        (y_end - 1) & 0xFF,
    }, 4), TAG, "send command failed");
    // transfer frame buffer
    size_t len = (x_end - x_start) * (y_end - y_start) * gc9307->fb_bits_per_pixel / 8;
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_color(io, LCD_CMD_RAMWR, color_data, len), TAG, "send color failed");

    return ESP_OK;
}

static esp_err_t panel_gc9307_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
    gc9307_panel_t *gc9307 = __containerof(panel, gc9307_panel_t, base);
    esp_lcd_panel_io_handle_t io = gc9307->io;
    int command = 0;
    if (invert_color_data) {
        command = LCD_CMD_INVON;
    } else {
        command = LCD_CMD_INVOFF;
    }
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, command, NULL, 0), TAG, "send command failed");
    return ESP_OK;
}

static esp_err_t panel_gc9307_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    gc9307_panel_t *gc9307 = __containerof(panel, gc9307_panel_t, base);
    esp_lcd_panel_io_handle_t io = gc9307->io;
    if (mirror_x) {
        gc9307->madctl_val |= LCD_CMD_MX_BIT;
    } else {
        gc9307->madctl_val &= ~LCD_CMD_MX_BIT;
    }
    if (mirror_y) {
        gc9307->madctl_val |= LCD_CMD_MY_BIT;
    } else {
        gc9307->madctl_val &= ~LCD_CMD_MY_BIT;
    }
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
        gc9307->madctl_val
    }, 1), TAG, "send command failed");
    return ESP_OK;
}

static esp_err_t panel_gc9307_swap_xy(esp_lcd_panel_t *panel, bool swap_axes)
{
    gc9307_panel_t *gc9307 = __containerof(panel, gc9307_panel_t, base);
    esp_lcd_panel_io_handle_t io = gc9307->io;
    if (swap_axes) {
        gc9307->madctl_val |= LCD_CMD_MV_BIT;
    } else {
        gc9307->madctl_val &= ~LCD_CMD_MV_BIT;
    }
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
        gc9307->madctl_val
    }, 1), TAG, "send command failed");
    return ESP_OK;
}

static esp_err_t panel_gc9307_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    gc9307_panel_t *gc9307 = __containerof(panel, gc9307_panel_t, base);
    gc9307->x_gap = x_gap;
    gc9307->y_gap = y_gap;
    return ESP_OK;
}

static esp_err_t panel_gc9307_disp_on_off(esp_lcd_panel_t *panel, bool on_off)
{
    gc9307_panel_t *gc9307 = __containerof(panel, gc9307_panel_t, base);
    esp_lcd_panel_io_handle_t io = gc9307->io;
    int command = 0;

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    on_off = !on_off;
#endif

    if (on_off) {
        command = LCD_CMD_DISPON;
    } else {
        command = LCD_CMD_DISPOFF;
    }
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, command, NULL, 0), TAG, "send command failed");
    return ESP_OK;
}
