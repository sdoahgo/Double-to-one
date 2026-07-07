#include "cs1237.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "adc_filter.h"

static const char *TAG = "CS1237";

#define CS1237_DOUT_PIN 5
#define CS1237_SCLK_PIN 9
#define CS1237_BATCH_SAMPLE_COUNT 10    //每批采集 10 个 CS1237 值
#define CS1237_BATCH_VALID_COUNT 3  //10个CS1237值中取中间 3 个值作为有效值
#define CS1237_TARGET_SAMPLE_COUNT 9    //重复次啊杨，直到累计9个有效值
#define CS1237_SAMPLE_DELAY_MS 50

static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
static int32_t sample_buff[CS1237_TARGET_SAMPLE_COUNT];
static int32_t batch_buff[CS1237_BATCH_SAMPLE_COUNT];

static void delay_us(uint32_t delay)
{
    esp_rom_delay_us(delay);
}

static void cs1237_send_clk(const struct CS1237_HANDLER *handle, uint8_t clk_num)
{
    for (uint8_t i = 0; i < clk_num; i++) {
        gpio_set_level(handle->sclk_pin, 1);
        delay_us(1);
        gpio_set_level(handle->sclk_pin, 0);
        delay_us(1);
    }
}

static void cs1237_send_bit(const struct CS1237_HANDLER *handle, uint8_t bit, uint8_t time_us)
{
    gpio_set_direction(handle->dout_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(handle->sclk_pin, 1);
    delay_us(time_us);
    gpio_set_level(handle->dout_pin, bit ? 1 : 0);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(time_us);
}

static uint8_t cs1237_check(const struct CS1237_HANDLER *handle)
{
    uint32_t retry = 0;
    gpio_config_t io_conf = { 0 };

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << handle->sclk_pin) | (1ULL << handle->dout_pin);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);

    delay_us(10);
    gpio_set_direction(handle->sclk_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(handle->sclk_pin, 0);
    gpio_set_direction(handle->dout_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(handle->dout_pin, 1);
    gpio_set_direction(handle->dout_pin, GPIO_MODE_INPUT);

    while (gpio_get_level(handle->dout_pin) == 1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        retry++;
        if (retry > 3000) {
            gpio_set_direction(handle->dout_pin, GPIO_MODE_OUTPUT);
            gpio_set_level(handle->dout_pin, 1);
            gpio_set_level(handle->sclk_pin, 1);
            return 1;
        }
    }

    return 0;
}

uint8_t cs1237_check_data(const struct CS1237_HANDLER *handle)
{
    uint32_t retry = 0;

    gpio_set_direction(handle->sclk_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(handle->sclk_pin, 0);
    gpio_set_direction(handle->dout_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(handle->dout_pin, 1);
    gpio_set_direction(handle->dout_pin, GPIO_MODE_INPUT);

    while (gpio_get_level(handle->dout_pin) == 1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        retry++;
        if (retry > 100) {
            gpio_set_direction(handle->dout_pin, GPIO_MODE_OUTPUT);
            gpio_set_level(handle->dout_pin, 1);
            gpio_set_level(handle->sclk_pin, 1);
            return 1;
        }
    }

    return 0;
}

static void cs1237_send_byte(const struct CS1237_HANDLER *handle, uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++) {
        gpio_set_level(handle->sclk_pin, 1);
        delay_us(1);
        gpio_set_level(handle->dout_pin, (byte & 0x80) ? 1 : 0);
        byte <<= 1;
        gpio_set_level(handle->sclk_pin, 0);
        delay_us(1);
    }
}

static esp_err_t cs1237_init(const struct CS1237_HANDLER *handle, cs1237_dev_frequency_em frequency, cs1237_dev_pga_em pga, cs1237_dev_ch_em ch)
{
    uint8_t cs1237_config = 0;

    if (cs1237_check(handle) != 0) {
        ESP_LOGE(TAG, "cs1237_check failed");
        return ESP_FAIL;
    }

    switch (frequency) {
    case DEV_FREQUENCY_40:
        setbit(cs1237_config, 4);
        break;
    case DEV_FREQUENCY_640:
        setbit(cs1237_config, 5);
        break;
    case DEV_FREQUENCY_1280:
        setbit(cs1237_config, 4);
        setbit(cs1237_config, 5);
        break;
    case DEV_FREQUENCY_10:
    default:
        break;
    }

    switch (pga) {
    case DEV_PGA_2:
        setbit(cs1237_config, 2);
        break;
    case DEV_PGA_64:
        setbit(cs1237_config, 3);
        break;
    case DEV_PGA_128:
        setbit(cs1237_config, 2);
        setbit(cs1237_config, 3);
        break;
    case DEV_PGA_1:
    default:
        break;
    }

    switch (ch) {
    case DEV_CH_SAVE:
        setbit(cs1237_config, 0);
        break;
    case DEV_CH_TEMPERERATURE:
        setbit(cs1237_config, 1);
        break;
    case DEV_CH_SHORT:
        setbit(cs1237_config, 1);
        setbit(cs1237_config, 0);
        break;
    case DEV_CH_A:
    default:
        break;
    }

    gpio_set_direction(handle->sclk_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(handle->dout_pin, GPIO_MODE_OUTPUT);

    for (uint8_t i = 0; i < 29; i++) {
        gpio_set_level(handle->sclk_pin, 1);
        delay_us(40);
        gpio_set_level(handle->sclk_pin, 0);
        delay_us(40);
    }

    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->dout_pin, 1);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);

    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->dout_pin, 1);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);

    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->dout_pin, 0);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);

    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->dout_pin, 0);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);

    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->dout_pin, 1);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);

    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->dout_pin, 0);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);

    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->dout_pin, 1);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);

    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);

    cs1237_send_byte(handle, cs1237_config);
    gpio_set_level(handle->dout_pin, 1);
    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);

    ESP_LOGI(TAG, "adc config = 0x%x", cs1237_config);
    return ESP_OK;
}

static uint8_t cs1237_read_config(const struct CS1237_HANDLER *handle)
{
    unsigned char dat = 0;

    gpio_set_direction(handle->sclk_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(handle->dout_pin, GPIO_MODE_OUTPUT);
    for (uint8_t i = 0; i < 29; i++) {
        gpio_set_level(handle->sclk_pin, 1);
        delay_us(40);
        gpio_set_level(handle->sclk_pin, 0);
        delay_us(40);
    }

    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->dout_pin, 1);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);

    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->dout_pin, 0);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);

    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->dout_pin, 1);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);

    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->dout_pin, 0);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);

    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->dout_pin, 1);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);

    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->dout_pin, 1);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);

    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->dout_pin, 0);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);

    gpio_set_level(handle->dout_pin, 1);
    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);

    gpio_set_direction(handle->dout_pin, GPIO_MODE_INPUT);
    for (unsigned char i = 0; i < 8; i++) {
        gpio_set_level(handle->sclk_pin, 1);
        delay_us(1);
        gpio_set_level(handle->sclk_pin, 0);
        delay_us(1);
        dat <<= 1;
        if (gpio_get_level(handle->dout_pin) == 1) {
            dat++;
        }
    }

    gpio_set_level(handle->sclk_pin, 1);
    delay_us(1);
    gpio_set_level(handle->sclk_pin, 0);
    delay_us(1);
    gpio_set_direction(handle->dout_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(handle->dout_pin, 1);
    return dat;
}

static int32_t cs1237_read_data(const struct CS1237_HANDLER *handle)
{
    uint32_t dat = 0;
    int32_t adc_dat = 0;
    unsigned char retry = 0;

    gpio_set_direction(handle->dout_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(handle->dout_pin, 1);
    gpio_set_direction(handle->dout_pin, GPIO_MODE_INPUT);
    while (gpio_get_level(handle->dout_pin) == 1) {
        esp_rom_delay_us(1000);
        retry++;
        if (retry > 200) {
            gpio_set_direction(handle->dout_pin, GPIO_MODE_OUTPUT);
            gpio_set_level(handle->dout_pin, 1);
            gpio_set_level(handle->sclk_pin, 1);
            return 0;
        }
    }

    for (unsigned char i = 0; i < 24; i++) {
        gpio_set_level(handle->sclk_pin, 1);
        delay_us(1);
        dat <<= 1;
        if (gpio_get_level(handle->dout_pin) == 1) {
            dat++;
        }
        gpio_set_level(handle->sclk_pin, 0);
        delay_us(1);
    }

    for (unsigned char i = 0; i < 3; i++) {
        cs1237_send_bit(handle, 1, 1);
    }

    gpio_set_direction(handle->dout_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(handle->dout_pin, 1);

    if (dat & 0x00800000) {
        adc_dat = -(((~dat) & 0x007FFFFF) + 1);
    } else {
        adc_dat = dat;
    }

    return adc_dat;
}

cs1237_handler_t cs1237_handler1 = {
    .init = cs1237_init,
    .sclk_pin = CS1237_SCLK_PIN,
    .dout_pin = CS1237_DOUT_PIN,
    .send_multi_clk = cs1237_send_clk,
    .read_data = cs1237_read_data,
    .check = cs1237_check,
    .read_config = cs1237_read_config,
};

static int32_t process_and_average(int32_t *buff, uint8_t len, char flags, uint8_t number)
{
    int32_t temp = 0;
    int64_t sum = 0;
    uint8_t count;
    uint8_t i;
    uint8_t j;

    for (j = 0; j < len - 1; j++) {
        for (i = 0; i < len - j - 1; i++) {
            if (buff[i] > buff[i + 1]) {
                temp = buff[i];
                buff[i] = buff[i + 1];
                buff[i + 1] = temp;
            }
        }
    }

    if (flags) {
        for (count = number; count < len - number; count++) {
            sum += buff[count];
        }
        temp = (int32_t)(sum / (len - (number * 2)));
    } else {
        temp = buff[(len - 1) / 2];
    }

    return temp;
}

static int32_t cs1237_read_filtered_value(void)
{
    int32_t ad_val = 0;

    taskENTER_CRITICAL(&mux);
    ad_val = cs1237_handler1.read_data(&cs1237_handler1);
    taskEXIT_CRITICAL(&mux);
    return get_filter(ad_val);
}

static void cs1237_collect_middle_samples(int32_t *out_buff, uint8_t *out_count)
{
    enum {
        FIRST_MIDDLE_INDEX = (CS1237_BATCH_SAMPLE_COUNT - CS1237_BATCH_VALID_COUNT) / 2,
    };

    for (uint8_t i = 0; i < CS1237_BATCH_SAMPLE_COUNT; ++i) {
        batch_buff[i] = cs1237_read_filtered_value();
        vTaskDelay(pdMS_TO_TICKS(CS1237_SAMPLE_DELAY_MS));
    }

    process_and_average(batch_buff, CS1237_BATCH_SAMPLE_COUNT, 1, 0);

    for (uint8_t i = 0; i < CS1237_BATCH_VALID_COUNT && *out_count < CS1237_TARGET_SAMPLE_COUNT; ++i) {
        out_buff[(*out_count)++] = batch_buff[FIRST_MIDDLE_INDEX + i];
    }
}

int32_t get_cs1237_val(void)
{
    uint8_t valid_count = 0;

    while (valid_count < CS1237_TARGET_SAMPLE_COUNT) {
        cs1237_collect_middle_samples(sample_buff, &valid_count);
    }

    return process_and_average(sample_buff, CS1237_TARGET_SAMPLE_COUNT, 1, 0);
}

int32_t get_cs1237_raw(void)
{
    int32_t val = 0;

    taskENTER_CRITICAL(&mux);
    val = cs1237_handler1.read_data(&cs1237_handler1);
    taskEXIT_CRITICAL(&mux);
    return val;
}

bool driver_cs1237_init(void)
{
    esp_err_t init_status = cs1237_handler1.init(&cs1237_handler1, DEV_FREQUENCY_40, DEV_PGA_128, DEV_CH_A);
    if (init_status == ESP_OK) {
        ESP_LOGI(TAG, "CS1237 initialization successful");
    } else {
        ESP_LOGE(TAG, "CS1237 initialization failed");
    }
    filterInit();
    ESP_LOGI(TAG, "read_sta:%d", cs1237_handler1.read_config(&cs1237_handler1));
    return init_status == ESP_OK;
}
