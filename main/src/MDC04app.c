#include "MDC04IIC.h"
#include "MDC04.h"
#include "MDC04app.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static float Co = 15.0f;
static float cap1;

uint8_t MDC04_SysCfg(uint8_t hz, uint8_t Re)
{
    uint8_t repeat = CFG_Rep_Low;

    if (Re == 0x01) {
        repeat = CFG_Rep_Medium;
    } else if (Re == 0x02) {
        repeat = CFG_Rep_High;
    }

    MDC04_SetConfig(hz, repeat);
    return 1;
}

uint8_t MDC04_Set_Cap_FullScale(float Cr)
{
    if (!((Cr >= 0.0f) && (Cr <= 15.5f))) {
        return 0;
    }
    MDC04_CapConfigureFs(Cr);
    return 1;
}

uint8_t MDC04_Set_Cap_Offset(float Co_value)
{
    if (!((Co_value >= 0.0f) && (Co_value <= 103.5f))) {
        return 0;
    }
    MDC04_CapConfigureOffset(Co_value);
    return 1;
}

uint8_t MDC04_Set_Cap_Channel(int Ch)
{
    uint8_t Cap_Cfg;

    MDC04_SetCapChannel(Ch & 0x07);
    MDC04_GetCapChannel(&Cap_Cfg);
    return 1;
}

uint8_t MDC04_ConvertCap(void)
{
    MDC04_write_cmd(CONVERT_C);
    return 1;
}

uint8_t ReadCap(uint8_t ch, uint16_t *icap)
{
    uint8_t Cap_lsb;
    uint8_t Cap_msb;
    uint8_t scr_cap_lsb;
    uint8_t scr_cap_msb;

    switch (ch) {
    case 2:
        scr_cap_lsb = Cap2_lsb;
        scr_cap_msb = Cap2_msb;
        break;
    case 3:
        scr_cap_lsb = Cap3_lsb;
        scr_cap_msb = Cap3_msb;
        break;
    case 4:
        scr_cap_lsb = Cap4_lsb;
        scr_cap_msb = Cap4_msb;
        break;
    case 1:
    default:
        scr_cap_lsb = Cap1_lsb;
        scr_cap_msb = Cap1_msb;
        break;
    }

    MDC04_SingleByteRead(scr_cap_lsb, &Cap_lsb);
    MDC04_SingleByteRead(scr_cap_msb, &Cap_msb);
    *icap = (Cap_msb << 8) | Cap_lsb;
    return 1;
}

void Read_ChanelNum(uint8_t NUM, float *fcap1)
{
    uint16_t cap;
    float Co_temp;
    float Cr_temp;

    MDC04_ReadCapConfigure(&Co_temp, &Cr_temp);
    ReadCap(NUM, &cap);
    *fcap1 = MDC04_OutputtoCap(cap, Co_temp, Cr_temp);
}

void MDC04_Auto_Cmd(void)
{
    MDC04_write_cmd(AUTO_CALIBRATION);
}

void MDC04_Cfb_range_value(uint8_t range_value)
{
    uint8_t value_cfb;

    MDC04_SingleByteRead(Cfb, &value_cfb);
    switch (range_value) {
    case 0:
        value_cfb &= 0x3F;
        break;
    case 1:
        value_cfb = (value_cfb & 0x3F) | 0x40;
        break;
    case 2:
        value_cfb = (value_cfb & 0x3F) | 0x80;
        break;
    default:
        value_cfb |= 0xC0;
        break;
    }
    MDC04_SingleByteWrite(Cfb, value_cfb);
}

void MDC04_ResetStatus(void)
{
    MDC04_write_cmd(CLEAR_STATUS);
}

uint8_t MDCO4_ReadStatus(void)
{
    uint8_t Status_Value;

    MDC04_SingleByteRead(Status, &Status_Value);
    return Status_Value;
}

void MDC04_Software_Auto(float *valeu, float *Co_value)
{
    static float temp_Co1 = 25.0f;
    static float temp_Co2 = 0.0f;

    if (*valeu > temp_Co1) {
        temp_Co2 = temp_Co1;
        temp_Co1 += 10.0f;
        if (*Co_value == 95.0f) {
            *Co_value = 103.5f;
        } else {
            *Co_value += 10.0f;
        }
        MDC04_Set_Cap_Offset(*Co_value);
    } else if (*valeu < temp_Co2 && temp_Co1 > 25.0f) {
        temp_Co1 = temp_Co2;
        temp_Co2 -= 10.0f;
        if (*Co_value == 103.5f) {
            *Co_value = 95.0f;
        } else {
            *Co_value -= 10.0f;
        }
        MDC04_Set_Cap_Offset(*Co_value);
    }
}

void MDC04_Set_Co_Range(uint8_t Cos_Range)
{
    uint8_t cfb_reg;

    MDC04_SingleByteRead(Cfb, &cfb_reg);
    cfb_reg = (cfb_reg & 0x3F) | (Cos_Range << 6);
    MDC04_SingleByteWrite(Cfb, cfb_reg);
}

bool MDC04_ID(void)
{
    uint8_t Romcode2[3];

    MDC04_read(0x14, Romcode2);
    if (Romcode2[2] == MY_I2C_CRC8(Romcode2, 2)) {
        return true;
    }

    printf("MDC04 FALSE\n");
    printf("%x  %x  %x  %.1f  %.3f\n", Romcode2[0], Romcode2[1], Romcode2[2], Co, cap1);
    return false;
}

bool MDC04_init(void)
{
    float Cr = 15.5f;
    bool link = MDC04_ID();

    MDC04_SysCfg(CFG_MPS_Single, MDC04_REPEATABILITY_HIGH);
    MDC04_Set_Cap_FullScale(Cr);
    MDC04_Set_Cap_Offset(Co);
    MDC04_Set_Cap_Channel(CAP_CH1_SEL);
    return link;
}

float Get_MDC04_Data(void)
{
    float temp_data = 0.0f;

    for (int i = 0; i < 50; i++) {
        MDC04_ConvertCap();
        Read_ChanelNum(1, &cap1);
        MDC04_Software_Auto(&cap1, &Co);
        if (i >= 40) {
            temp_data += cap1;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    cap1 = temp_data / 10.0f;
    return cap1;
}

float Get_MDC04_Data1(void)
{
    MDC04_ConvertCap();
    Read_ChanelNum(1, &cap1);
    MDC04_Software_Auto(&cap1, &Co);
    return cap1;
}
