#ifndef _ADC_FILTER_H_
#define _ADC_FILTER_H_

#include <stdint.h>

#define FILTER_BUFF_SIZE 7

typedef struct {
    int32_t Current;
    int32_t LastCurrent;
    int32_t Result;
    int32_t LastMedian;
    int32_t Buff[FILTER_BUFF_SIZE];
    uint8_t MedianCount;
    uint8_t FastCount;
} MyFilter_t;

void filterInit(void);
int32_t get_filter(int32_t ADC_Value);

#endif
