#ifndef _CS1237_H_
#define _CS1237_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define setbit(x, y) x |= (1 << y)
#define clrbit(x, y) x &= ~(1 << y)

typedef enum {
    DEV_CH_NONE = 0,
    DEV_CH_A,
    DEV_CH_SAVE,
    DEV_CH_TEMPERERATURE,
    DEV_CH_SHORT,
    DEV_CH_TOTAL_STATE
} cs1237_dev_ch_em;

typedef enum {
    DEV_PGA_NONE = 0,
    DEV_PGA_1,
    DEV_PGA_2,
    DEV_PGA_64,
    DEV_PGA_128,
    DEV_PGA_TOTAL_STATE
} cs1237_dev_pga_em;

typedef enum {
    DEV_FREQUENCY_NONE = 0,
    DEV_FREQUENCY_10,
    DEV_FREQUENCY_40,
    DEV_FREQUENCY_640,
    DEV_FREQUENCY_1280,
    DEV_FREQUENCY_TOTAL_STATE
} cs1237_dev_frequency_em;

typedef struct CS1237_HANDLER {
    uint8_t sclk_pin;
    uint8_t dout_pin;
    esp_err_t (*init)(const struct CS1237_HANDLER *handle, cs1237_dev_frequency_em frequency, cs1237_dev_pga_em pga, cs1237_dev_ch_em ch);
    void (*send_multi_clk)(const struct CS1237_HANDLER *handle, uint8_t clk_num);
    int32_t (*read_data)(const struct CS1237_HANDLER *handle);
    uint8_t (*check)(const struct CS1237_HANDLER *handle);
    uint8_t (*read_config)(const struct CS1237_HANDLER *handle);
} cs1237_handler_t;

extern cs1237_handler_t cs1237_handler1;

bool driver_cs1237_init(void);
int32_t get_cs1237_val(void);
int32_t get_cs1237_raw(void);
uint8_t cs1237_check_data(const struct CS1237_HANDLER *handle);

#ifdef __cplusplus
}
#endif

#endif
