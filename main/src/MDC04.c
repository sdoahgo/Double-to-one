#include "MDC04.h"
#include "MDC04IIC.h"

#define POLYNOMIAL 0x131

static const float COS_Factor[8] = { 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 40.0f };
static const struct {
    float Cfb0;
    float Factor[6];
} CFB = { 2.0f, { 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 46.0f } };

uint8_t MY_I2C_CRC8(uint8_t data[], uint8_t nbrOfBytes)
{
    uint8_t bit;
    uint8_t crc = 0xFF;

    for (uint8_t byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++) {
        crc ^= data[byteCtr];
        for (bit = 8; bit > 0; --bit) {
            crc = (crc & 0x80) ? ((crc << 1) ^ POLYNOMIAL) : (crc << 1);
        }
    }
    return crc;
}

float MDC04_OutputtoCap(uint16_t out, float Co, float Cr)
{
    return (2.0f * (out / 65535.0f - 0.5f) * Cr + Co);
}

uint8_t MDC04_SetConfig(uint8_t mps, uint8_t repeatability)
{
    uint8_t scr_wr[3];
    uint8_t temp = (mps << 2) | repeatability;

    scr_wr[0] = temp;
    scr_wr[1] = 0xFF;
    scr_wr[2] = MY_I2C_CRC8(scr_wr, 2);
    MDC04_write(Config, scr_wr, 3);
    return 1;
}

void MDC04_CapConfigureOffset(float Coffset)
{
    uint8_t CosCfg = CaptoCoscfg(Coffset + 0.25f);
    MDC04_SingleByteWrite(Cos, CosCfg);
}

uint8_t CaptoCoscfg(float osCap)
{
    uint8_t CosCfg = 0x00;

    for (int i = 7; i >= 0; i--) {
        if (osCap >= COS_Factor[i]) {
            CosCfg |= (0x01 << i);
            osCap -= COS_Factor[i];
        }
    }
    return CosCfg;
}

uint32_t WriteCosConfig(uint8_t Coffset, uint8_t Cosbits)
{
    (void)Cosbits;
    MDC04_SingleByteWrite(Cos, Coffset);
    return 1;
}

uint32_t MDC04_SingleByteWrite(uint8_t reg, uint8_t value)
{
    uint8_t scr_wr_byte[3];

    scr_wr_byte[0] = value;
    scr_wr_byte[1] = 0xFF;
    scr_wr_byte[2] = MY_I2C_CRC8(scr_wr_byte, 2);
    MDC04_write(reg, scr_wr_byte, 3);
    return 1;
}

uint32_t MDC04_SingleByteRead(uint8_t reg, uint8_t *value)
{
    uint8_t scr_rd_byte[3];

    MDC04_read(reg, scr_rd_byte);
    if (scr_rd_byte[2] != MY_I2C_CRC8(scr_rd_byte, 2)) {
        return 0;
    }

    *value = scr_rd_byte[0];
    return 1;
}

uint8_t MDC04_CapConfigureFs(float Cr)
{
    uint8_t Cfbcfg;

    Cr = Cr + 0.1408f;
    Cfbcfg = CapRangetocfbCfg(Cr);
    WriteCfbConfig(Cfbcfg);
    return 1;
}

uint8_t CapRangetocfbCfg(float Cr)
{
    uint8_t CfbCfg = 0x00;

    Cr = Cr * (3.6f / 0.507f);
    Cr -= CFB.Cfb0;

    for (int8_t i = 5; i >= 0; i--) {
        if (Cr >= CFB.Factor[i]) {
            Cr -= CFB.Factor[i];
            CfbCfg |= (0x01 << i);
        }
    }
    return CfbCfg;
}

uint8_t WriteCfbConfig(uint8_t value)
{
    uint8_t reg_cfb;

    MDC04_SingleByteRead(Cfb, &reg_cfb);
    reg_cfb = 0xC0 | (value & 0x3F);
    MDC04_SingleByteWrite(Cfb, reg_cfb);
    return 1;
}

uint8_t MDC04_SetCapChannel(uint8_t channel)
{
    uint8_t reg_ChSel;

    MDC04_SingleByteRead(Ch_sel, &reg_ChSel);
    reg_ChSel = (reg_ChSel & ~0x07) | (channel & 0x07);
    MDC04_SingleByteWrite(Ch_sel, reg_ChSel);
    return 1;
}

uint8_t MDC04_GetCapChannel(uint8_t *chann)
{
    uint8_t reg_ChSel;

    MDC04_SingleByteRead(Ch_sel, &reg_ChSel);
    *chann = reg_ChSel & 0x07;
    return 1;
}

uint8_t MDC04_ReadCapConfigure(float *Co, float *Cr)
{
    MDC04_GetCfg_CapOffset(Co);
    MDC04_GetCfg_CapRange(Cr);
    return 1;
}

void MDC04_GetCfg_CapOffset(float *Co)
{
    uint8_t cfg;

    if (ReadCosConfig(&cfg)) {
        *Co = CoscfgtoCapOffset(cfg);
    }
}

bool ReadCosConfig(uint8_t *Coscfg)
{
    return MDC04_SingleByteRead(Cos, Coscfg) == 1;
}

float CoscfgtoCapOffset(uint8_t CosCfg)
{
    float osCap = 0.0f;

    for (int i = 0; i < 8; i++) {
        if (CosCfg & (0x01 << i)) {
            osCap += COS_Factor[i];
        }
    }
    return osCap - 0.25f;
}

uint8_t MDC04_GetCfg_CapRange(float *Crange)
{
    uint8_t cfb;

    if (!ReadCfbConfig(&cfb)) {
        return 0;
    }

    *Crange = CfbcfgtoCapRange(cfb);
    return 1;
}

uint8_t ReadCfbConfig(uint8_t *cfb)
{
    return MDC04_SingleByteRead(Cfb, cfb);
}

float CfbcfgtoCapRange(uint8_t fbCfg)
{
    float Cr = CFB.Cfb0;

    for (int i = 0; i < 6; i++) {
        if (fbCfg & (0x01 << i)) {
            Cr += CFB.Factor[i];
        }
    }

    Cr = Cr * (0.507f / 3.6f);
    return Cr - 0.1408f;
}
