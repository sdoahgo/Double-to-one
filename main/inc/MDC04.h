#ifndef MDC04_H
#define MDC04_H

#include <stdbool.h>
#include <stdint.h>

#define CFG_MPS_Single (0x00)
#define CFG_MPS_Half (0x04)
#define CFG_MPS_1 (0x08)
#define CFG_MPS_2 (0x0C)
#define CFG_MPS_4 (0x10)
#define CFG_MPS_10 (0x14)

#define CFG_Rep_Low (0x00)
#define CFG_Rep_Medium (0x01)
#define CFG_Rep_High (0x02)

#define Temp_lsb 0x00
#define Temp_msb 0x01
#define Cap1_lsb 0x02
#define Cap1_msb 0x03
#define Config 0x06
#define Status 0x07
#define Cap2_lsb 0x0E
#define Cap2_msb 0x0F
#define Cap3_lsb 0x10
#define Cap3_msb 0x11
#define Cap4_lsb 0x12
#define Cap4_msb 0x13
#define Ch_sel 0x1C
#define Cos 0x1D
#define Cfb 0x22

#define CONVERT_T 0xCC44
#define CONVERT_C 0xCC66
#define READ_ONE_BYTE 0xD200
#define WRITE_ONE_BYTE 0x5200
#define WRITE_CONFIG 0x5206
#define READ_STATUSCONFIG 0xF32D
#define CLEAR_STATUS 0x3041
#define BREAK 0x3093
#define AUTO_CALIBRATION 0xA187
#define SOFT_RESET 0x30A2
#define COPY_PAGE0 0xCC48

uint8_t MY_I2C_CRC8(uint8_t data[], uint8_t nbrOfBytes);
float MDC04_OutputtoCap(uint16_t out, float Co, float Cr);
uint8_t MDC04_SetConfig(uint8_t mps, uint8_t repeatability);
void MDC04_CapConfigureOffset(float Coffset);
uint8_t CaptoCoscfg(float osCap);
uint32_t WriteCosConfig(uint8_t Coffset, uint8_t Cosbits);
uint32_t MDC04_SingleByteWrite(uint8_t reg, uint8_t value);
uint32_t MDC04_SingleByteRead(uint8_t reg, uint8_t *value);
uint8_t MDC04_CapConfigureFs(float Cr);
uint8_t CapRangetocfbCfg(float Cr);
uint8_t WriteCfbConfig(uint8_t value);
uint8_t MDC04_SetCapChannel(uint8_t channel);
uint8_t MDC04_GetCapChannel(uint8_t *chann);
uint8_t MDC04_ReadCapConfigure(float *Co, float *Cr);
void MDC04_GetCfg_CapOffset(float *Co);
bool ReadCosConfig(uint8_t *Coscfg);
float CoscfgtoCapOffset(uint8_t CosCfg);
uint8_t MDC04_GetCfg_CapRange(float *Crange);
uint8_t ReadCfbConfig(uint8_t *cfb);
float CfbcfgtoCapRange(uint8_t fbCfg);

#endif
