#ifndef MDC04IIC_H
#define MDC04IIC_H

#include <stdio.h>
#include "driver/i2c.h"
#include "esp_err.h"

#define I2C_MASTER_SCL_IO 21
#define I2C_MASTER_SDA_IO 20
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000
#define MDC04_ADDR 0x45

#define WRITE_BIT I2C_MASTER_WRITE
#define READ_BIT I2C_MASTER_READ
#define ACK_CHECK_EN 0x1
#define ACK_VAL 0x0
#define NACK_VAL 0x1

void MDC04_write_cmd(uint16_t comder);
esp_err_t myi2c_master_init(void);
esp_err_t MDC04_read(uint8_t reg, uint8_t *data_rd);
esp_err_t MDC04_write(uint8_t reg, uint8_t *data_wr, size_t size);

#endif
