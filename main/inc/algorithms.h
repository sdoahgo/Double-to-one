
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"


#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************/
//插值使用
#define N  (16*12)   //(39*11)  //(31*9)                   //插值数组大小 116
#define BLOCK_SIZE 10           // 块大小，根据内存限制调整 50
//滑窗使用
#define WINDOW_SIZE 100         //滑动窗口---色值直方图峰值加权滤波
#define WINDOW_SIZE_Filter 50   //滑动窗口---色值箱线图有效振幅检测
// //滤波使用
// #define WAVELET_LEVEL 3
// #define SIGNAL_LENGTH 20  

//滑动窗口结构体
typedef struct 
{
    float buffer[WINDOW_SIZE];     // 环形缓冲区存储滑动窗口数据
    int head;                      // 当前写入位置
    int count;                     // 当前窗口内有效数据数量
} SlidingWindow;


//滑动窗口结构体
typedef struct 
{
    float buffer[WINDOW_SIZE_Filter];  // 环形缓冲区存储滑动窗口数据
    int head;                          // 当前写入位置
    int count;                         // 当前窗口内有效数据数量
} SlidingWindow_filter;

//三次样条插值函数
// 数据点数量
#define N_spline 13 //12 6


// 三次样条结构
typedef struct 
{
    float a[N_spline], b[N_spline], c[N_spline], d[N_spline], x[N_spline];
} SplineCoeffs;



//冒泡排序
int partition(int16_t *array, int low, int high) ;
void quickSort(int16_t *array, int low, int high) ;
void sortArray_int16(int16_t *array, int size);
void sortArray_float(float *array, int size);
float compare(const void *a, const void *b);

//滑动窗口使用
void initSlidingWindow(SlidingWindow *window) ;
void initSlidingWindow_filter(SlidingWindow_filter *window);
void addToSlidingWindow(SlidingWindow *window, float value);
void addToSlidingWindow4Filter(SlidingWindow_filter *window, float value) ;

//小波变换和中位加权滤波算法使用
float calculateWeightedMedianMean_int16(int16_t *buff,int length);
float calculateWeightedMedianMean_float(float *buff,int length) ;
float calculateWeightedMedianMean_window(SlidingWindow_filter *window);


//箱线图底噪识别和异常值剔除算法使用
void calculateIQR4Live(float *data, int size, float *filtered_data, int *filtered_size) ;      
void calculateIQR(float *data, int size, float *filtered_data, int *filtered_size) ;


//插值算法使用
void compute_interpolation_matrix_block(int n, const float *x, const float *y, float *C_block, int row_start, int col_start, int block_size);
void compute_interpolation_matrix(int n, const float *x, const float *y, float *C) ;
void gaussian_elimination(int n, float *C, float *B, float *X)  ;
float radial_basis_function(float r);
float interpolate(int n, const float *x, const float *y, const float *lambda, float x0, float y0);

void computeSplineCoeffs(float x[], float y[], SplineCoeffs* spline);
float evalSpline(SplineCoeffs* spline, float x_val);

/*********************************************************************************************/

#ifdef __cplusplus
}
#endif
 