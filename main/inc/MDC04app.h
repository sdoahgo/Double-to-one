#ifndef MDC04APP_H
#define MDC04APP_H

#include <stdbool.h>
#include <stdint.h>

#define CAP_CH1_SEL (0x01)
#define CAP_CH2_SEL (0x02)
#define CAP_CH3_SEL (0x03)
#define CAP_CH4_SEL (0x04)
#define CAP_CH1CH2_SEL (0x05)
#define CAP_CH1CH2CH3_SEL (0x06)
#define CAP_CH1CH2CH3CH4_SEL (0x07)

#define MDC04_REPEATABILITY_LOW (0x00 << 0)
#define MDC04_REPEATABILITY_MEDIUM (0x01 << 0)
#define MDC04_REPEATABILITY_HIGH (0x02 << 0)

uint8_t MDC04_SysCfg(uint8_t hz, uint8_t Re);
uint8_t MDC04_Set_Cap_FullScale(float Cr);
uint8_t MDC04_Set_Cap_Offset(float Co);
uint8_t MDC04_Set_Cap_Channel(int Ch);
uint8_t MDC04_ConvertCap(void);
uint8_t ReadCap(uint8_t ch, uint16_t *icap);
void Read_ChanelNum(uint8_t NUM, float *fcap1);
void MDC04_Auto_Cmd(void);
void MDC04_Cfb_range_value(uint8_t range_value);
uint8_t MDCO4_ReadStatus(void);
void MDC04_ResetStatus(void);
void MDC04_Software_Auto(float *valeu, float *Co);
void MDC04_Set_Co_Range(uint8_t Cos_Range);
bool MDC04_ID(void);
bool MDC04_init(void);
float Get_MDC04_Data(void);
float Get_MDC04_Data1(void);

#endif
