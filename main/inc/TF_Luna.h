#ifndef TF_LUNA_H
#define TF_LUNA_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "user_hal.h"


#define TF_Luna_addr                0x10                  //GD60914地址
#define TF_WRITE_BIT                   I2C_MASTER_WRITE      //写：0
#define TF_READ_BIT                    I2C_MASTER_READ       //读：1
#define TF_ACK_CHECK_EN                0x1                   //主机检查从机的ACK
#define TF_ACK_CHECK_DIS               0x0                   //主机不检查从机的ACK
#define TF_ACK_VAL                     0x0                   //应答
#define TF_NACK_VAL                    0x1                   //不应答
#define TF_TAG1                         "TF_I2C"
#define TF_IO_OUT                       10


#define DIST_LOW                       0x00
#define DIST_HIGH                      0x01
#define AMP_LOW                        0x02
#define AMP_HIGH                       0x03
#define TEMP_LOW                       0x04
#define TEMP_HIGH                      0x05
#define TICK_LOW                       0x06
#define TICK_HIGH                      0x07
#define ERROR_LOW                      0x08
#define ERROR_HIGH                     0x09
#define VERSION_REVISION               0x0A
#define VERSION_MINOR                  0x0B
#define VERSION_MAJOR                  0x0C
// 0x0D ~ 0x0F 保留或未定义

#define SN_BASE                        0x10  // SN是0x10~0x1D，一般作为数组首地址
#define ULTRA_LOW_POWER                0x1F
#define SAVE                           0x20
#define SHUTDOWN_REBOOT                0x21
#define SLAVE_ADDR                     0x22
#define MODE                           0x23
#define TRIG_ONE_SHOT                  0x24
#define ENABLE                         0x25
#define FPS_LOW                        0x26
#define FPS_HIGH                       0x27
#define LOW_POWER                      0x28
#define RESTORE_FACTORY_DEFAULTS       0x29
#define AMP_THR_LOW                    0x2A
#define AMP_THR_HIGH                   0x2B

#define DUMMY_DIST_LOW                 0x2C
#define DUMMY_DIST_HIGH                0x2D
#define MIN_DIST_LOW                   0x2E
#define MIN_DIST_HIGH                  0x2F
#define MAX_DIST_LOW                   0x30
#define MAX_DIST_HIGH                  0x31
// 0x32~0x3B 保留未用

#define SIGNATURE_BASE                 0x3C   // SIGNATURE 0x3C~0x3F, 用于数组首地址

#define TF_TAG                         "TF_I2C"

esp_err_t TF_Luna_read(uint8_t reg, uint8_t *data, uint8_t length);
esp_err_t TF_Luna_write(uint8_t reg, uint8_t *data, uint8_t length);
esp_err_t TF_Luna_write_byte(uint8_t reg, uint8_t data);
void TF_Luna_init(void);
void TF_Luna_task(void *pvParameters);

#endif