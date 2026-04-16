#include <stdlib.h>
#include "adc_filter.h"

MyFilter_t myFilter;

void filterInit(void)
{
    myFilter.Current = 0;
    myFilter.LastCurrent = 0;
    myFilter.Result = 0;
    myFilter.LastMedian = 0;
    myFilter.MedianCount = 0;
    myFilter.FastCount = 0;

    for (uint8_t i = 0; i < FILTER_BUFF_SIZE; i++) {
        myFilter.Buff[i] = 0;
    }
}

int32_t get_filter(int32_t ADC_Value)
{
    int32_t Current = ADC_Value >> 2;

    if (abs(myFilter.Current - Current) > 50) {
        myFilter.LastCurrent = Current;
        myFilter.Current = Current;
        myFilter.FastCount = 0;
        for (uint8_t i = 0; i < FILTER_BUFF_SIZE; i++) {
            myFilter.Buff[i] = Current;
        }

        myFilter.LastMedian = Current;
        myFilter.Result = Current;
        myFilter.MedianCount = 0;
    } else {
        if (myFilter.FastCount < 6) {
            myFilter.FastCount++;
            myFilter.Current = (myFilter.Current + myFilter.LastCurrent + Current) / 3;
            myFilter.LastCurrent = Current;

            myFilter.LastMedian = myFilter.Current;
            myFilter.Result = myFilter.Current;
            myFilter.MedianCount = 0;
        } else {
            myFilter.Current = (8 * (Current + myFilter.LastCurrent) + 112 * myFilter.Current) / 128;
            myFilter.LastCurrent = Current;
        }
    }

    for (uint8_t i = 0; i < FILTER_BUFF_SIZE - 1; i++) {
        myFilter.Buff[i] = myFilter.Buff[i + 1];
    }
    myFilter.Buff[FILTER_BUFF_SIZE - 1] = myFilter.Current;

    int32_t buff[FILTER_BUFF_SIZE];
    for (uint8_t i = 0; i < FILTER_BUFF_SIZE; i++) {
        buff[i] = myFilter.Buff[i];
    }

    for (uint8_t i = 0; i < FILTER_BUFF_SIZE - 1; i++) {
        for (uint8_t j = i + 1; j < FILTER_BUFF_SIZE; j++) {
            if (buff[i] < buff[j]) {
                int32_t temp = buff[i];
                buff[i] = buff[j];
                buff[j] = temp;
            }
        }
    }

    int32_t median = buff[3];

    if (myFilter.MedianCount < 6) {
        myFilter.Result = ((112 * median) + (8 * myFilter.LastMedian) + (8 * myFilter.Result)) / 128;
        myFilter.MedianCount++;
    } else {
        if (abs(median - myFilter.Result) < 5) {
            myFilter.Result = ((8 * median) + (8 * myFilter.LastMedian) + (112 * myFilter.Result)) / 128;
        } else if (abs(median - myFilter.Result) < 10) {
            myFilter.Result = ((20 * median) + (8 * myFilter.LastMedian) + (100 * myFilter.Result)) / 128;
        } else {
            myFilter.Result = ((30 * median) + (8 * myFilter.LastMedian) + (90 * myFilter.Result)) / 128;
        }
    }
    myFilter.LastMedian = median;

    return myFilter.Result;
}
