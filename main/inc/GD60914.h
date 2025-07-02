#ifndef GD60914_H
#define GD60914_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "user_hal.h"

#define I2C_MASTER_SCL_IO           19                     //SCL
#define I2C_MASTER_SDA_IO           18                     //SDA
#define I2C_MASTER_NUM              I2C_NUM_0             //I2C0
#define I2C_MASTER_FREQ_HZ          100000                //频率100K
#define GD60914_addr                0x18                  //GD60914地址

#define WRITE_BIT                   I2C_MASTER_WRITE      //写：0
#define READ_BIT                    I2C_MASTER_READ       //读：1
#define ACK_CHECK_EN                0x1                   //主机检查从机的ACK
#define ACK_CHECK_DIS               0x0                   //主机不检查从机的ACK
#define ACK_VAL                     0x0                   //应答
#define NACK_VAL                    0x1                   //不应答
#define TAG1                         "I2C"

void GD60914_Init(void);
void GD60914_task(void *pvParameters);

#endif