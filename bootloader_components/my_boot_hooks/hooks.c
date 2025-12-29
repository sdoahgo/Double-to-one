#include "bootloader_common.h"
#include "esp_rom_gpio.h"
#include "soc/gpio_reg.h"
#include "soc/soc.h"
#include "esp_log.h"


/* Function used to tell the linker to include this file
 * with all its symbols.
 */
void bootloader_hooks_include(void){
}

// bootloader里设置GPIO输出并写电平（只用寄存器/ROM）
static inline void boot_gpio_output_set(int pin, int level)
{
    // 1) 选择这个 pad 为 GPIO 功能
    esp_rom_gpio_pad_select_gpio(pin);

    // 2) 使能输出（W1TS：write-1-to-set）
    REG_WRITE(GPIO_ENABLE_W1TS_REG, BIT(pin));

    // 3) 设置电平
    if (level) {
        REG_WRITE(GPIO_OUT_W1TS_REG, BIT(pin)); // 拉高
    } else {
        REG_WRITE(GPIO_OUT_W1TC_REG, BIT(pin)); // 拉低（W1TC：write-1-to-clear）
    }
}
//// bootloader 还没 init flash/heap 的最早阶段
void bootloader_before_init(void) {
    // // 只能做非常低层/ROM级别的事，比如简单 GPIO 设置
    // gpio_config_t io_conf = {
    //     .pin_bit_mask = 1ULL << GPIO_NUM_1,
    //     .mode = GPIO_MODE_OUTPUT,
    //     .pull_up_en = 0,
    //     .pull_down_en = 0,
    //     .intr_type = GPIO_INTR_DISABLE
    // };
    // gpio_config(&io_conf);
    // gpio_set_level(GPIO_NUM_1, 1);  // 例：拉高一个电源使能脚
    //     // 日志可以打，但别依赖堆内存/printf 大格式化
    // 选择该管脚为 GPIO 功能
    boot_gpio_output_set(0, 1); // 举例：GPIO1 拉高
    ESP_LOGI("HOOK", "before init========================================");
}

void bootloader_after_init(void) {
    ESP_LOGI("HOOK", "This hook is called AFTER bootloader initialization");
}