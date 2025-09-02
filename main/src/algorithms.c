
#include "algorithms.h"


static const char *TAG = "algorithms";

//单个块的内存
float A_block[BLOCK_SIZE * BLOCK_SIZE]; 


// 快速排序的分区函数
int partition(int16_t *array, int low, int high) 
{
    int16_t pivot = array[high];  // 选取数组的最后一个元素作为pivot
    int i = (low - 1);  // 最小元素的索引

    // 遍历数组，将小于 pivot 的元素移到数组的左侧
    for (int j = low; j < high; j++) 
    {
        if (array[j] < pivot) 
        {
            i++;
            // 交换 array[i] 和 array[j]
            int16_t temp = array[i];
            array[i] = array[j];
            array[j] = temp;
        }
    }

    // 将 pivot 元素放置到正确的位置
    int16_t temp = array[i + 1];
    array[i + 1] = array[high];
    array[high] = temp;

    return (i + 1);
}

// 快速排序的递归函数
void quickSort(int16_t *array, int low, int high) 
{
    if (low < high) 
    {
        // 分区索引，数组被分成两部分
        int pi = partition(array, low, high);

        // 递归排序左右两部分
        quickSort(array, low, pi - 1);
        quickSort(array, pi + 1, high);
    }
}

// 快速排序，封装函数接口，调用时直接使用
void sortArray_int16(int16_t *array, int size) 
{
    quickSort(array, 0, size - 1);
}


// 排序数组
void sortArray_float(float *array, int size) 
{
    for (int i = 0; i < size - 1; i++) 
    {
        for (int j = 0; j < size - i - 1; j++) 
        {
            if (array[j] > array[j + 1]) 
            {
                float temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}

/*****************************************************************************************************************************************/
/************************************************************滑窗************************************************************************/
/*****************************************************************************************************************************************/
//初始化滑动窗口
void initSlidingWindow(SlidingWindow *window) 
{
    for (int i = 0; i < WINDOW_SIZE; i++) 
    {
        window->buffer[i] = 0;  //初始化缓冲区
    }
    window->head = 0;           //初始化写入位置
    window->count = 0;          //有效数据数量初始化为 0
}

//初始化滑动窗口
void initSlidingWindow_filter(SlidingWindow_filter *window) 
{
    for (int i = 0; i < WINDOW_SIZE_Filter; i++) 
    {
        window->buffer[i] = 0;  //初始化缓冲区
    }
    window->head = 0;           //初始化写入位置
    window->count = 0;          //有效数据数量初始化为 0
}

//添加数据到滑动窗口
void addToSlidingWindow(SlidingWindow *window, float value) 
{
    //将新值写入环形缓冲区的当前位置
    window->buffer[window->head] = value;

    //更新环形缓冲区的写入位置
    window->head = (window->head + 1) % WINDOW_SIZE;

    //如果窗口未满，增加有效数据计数
    if (window->count < WINDOW_SIZE) 
    {
        window->count++;
    }
}

//添加数据到滑动窗口
void addToSlidingWindow4Filter(SlidingWindow_filter *window, float value) 
{
    //将新值写入环形缓冲区的当前位置
    window->buffer[window->head] = value;

    //更新环形缓冲区的写入位置
    window->head = (window->head + 1) % WINDOW_SIZE_Filter;

    //如果窗口未满，增加有效数据计数
    if (window->count < WINDOW_SIZE_Filter) 
    {
        window->count++;
    }
}



//计算滑动窗口中位加权均值
float calculateWeightedMedianMean_window(SlidingWindow_filter *window)    ///no use
{
    if (window->count == 0) 
    {
        return 0.0f;  //如果窗口为空，返回0
    }

    // 1. 复制缓冲区到临时数组进行排序
    float tempBuffer[WINDOW_SIZE_Filter];
    for (int i = 0; i < window->count; i++) 
    {
        tempBuffer[i] = window->buffer[i];
    }
    sortArray_float(tempBuffer, window->count);

    // 2. 计算中位数
    float median;
    if (window->count % 2 == 0) 
    {
        median = (tempBuffer[window->count / 2 - 1] + tempBuffer[window->count / 2]) / 2.0f;
    } else 
    {
        median = tempBuffer[window->count / 2];
    }

    // 3. 加权计算
    float weightedSum = 0.0f;
    float weightSum = 0.0f;
    for (int i = 0; i < window->count; i++) 
    {
        float weight = 1.0f / (1.0f + abs((int)(tempBuffer[i] - median)));  //权重公式
        weightedSum += tempBuffer[i] * weight;
        weightSum += weight;
    }

    return weightedSum / weightSum;  //返回加权均值
}

//计算滑动窗口中位加权均值
float calculateWeightedMedianMean_float(float *buff,int length)     //
{
    // 1. 排序
    sortArray_float(buff, length);
    // 2. 计算中位数
    float median;
    if (length % 2 == 0) 
    {
        median = (buff[length / 2 - 1] + buff[length / 2]) / 2.0f;
    } 
    else 
    {
        median = buff[length / 2];
    }

    // 3. 加权计算
    float weightedSum = 0.0f;
    float weightSum = 0.0f;
    for (int i = 0; i < length; i++) 
    {
        float weight = 1.0f / (1.0f + abs((int)(buff[i] - median)));  //权重公式
        weightedSum += buff[i] * weight;
        weightSum += weight;
    }

    return weightedSum / weightSum;  //返回加权均值
}

//计算中位位加权均值
float calculateWeightedMedianMean_int16(int16_t *buff,int length) 
{
    //1.排序
    sortArray_int16(buff, length);

    //2.计算中位数
    float median;
    if (length % 2 == 0) 
    {
        median = (buff[length / 2 - 1] + buff[length / 2]) / 2.0f;
    } 
    else 
    {
        median = buff[length / 2];
    }

    //3.加权计算
    float weightedSum = 0.0;
    float weightSum = 0.0;
    for (int i = 0; i < length; i++) 
    {
        float weight = 1.0f / (1.0f + abs((int)(buff[i] - median)));  //权重
        weightedSum += buff[i] * weight;
        weightSum += weight;
    }

    return weightedSum / weightSum;  //返回加权均中位均值
}



 
//箱线图异常值剔除  算法实现：计算 IQR 并剔除异常值
void calculateIQR(float *data, int size, float *filtered_data, int *filtered_size)                   
{
    // 1. 排序数据（简单冒泡排序，适合小规模数据）
    for (int i = 0; i < size - 1; i++) 
    {
        for (int j = 0; j < size - i - 1; j++) 
        {
            if (data[j] > data[j + 1]) 
            {
                float temp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = temp;
            }
        }
    }

    // 2. 计算 Q1 和 Q3
    int Q1_index = size * 0.15f; // 第一四分位数索引  0.25
    int Q3_index = size * 0.85f; // 第三四分位数索引  0.75
    float Q1 = data[Q1_index];
    float Q3 = data[Q3_index];

    // 3. 计算 IQR 和上下限
    float IQR = Q3 - Q1;
    float lower_limit = Q1 - 1.5f * IQR;
    float upper_limit = Q3 + 1.5f * IQR;

    // 4. 筛选非异常值数据
    *filtered_size = 0;
    for (int i = 0; i < size; i++) 
    {
        if (data[i] >= lower_limit && data[i] <= upper_limit) 
        {
            filtered_data[*filtered_size] = data[i];
            (*filtered_size)++;
        }
    }
}


 
//箱线图异常值剔除  算法实现：计算 IQR 并剔除异常值
void calculateIQR4Live(float *data, int size, float *filtered_data, int *filtered_size)                   
{
    // 1. 排序数据（简单冒泡排序，适合小规模数据）
    for (int i = 0; i < size - 1; i++) 
    {
        for (int j = 0; j < size - i - 1; j++) 
        {
            if (data[j] > data[j + 1]) 
            {
                float temp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = temp;
            }
        }
    }

    // 2. 计算 Q1 和 Q3
    int Q1_index = size * 0.3f; // 第一四分位数索引  0.25
    int Q3_index = size * 0.7f; // 第三四分位数索引  0.75
    float Q1 = data[Q1_index];
    float Q3 = data[Q3_index];

    // 3. 计算 IQR 和上下限
    float IQR = Q3 - Q1;
    float lower_limit = Q1 - 1.5f * IQR;
    float upper_limit = Q3 + 1.5f * IQR;

    // 4. 筛选非异常值数据
    *filtered_size = 0;
    for (int i = 0; i < size; i++) 
    {
        if (data[i] >= lower_limit && data[i] <= upper_limit) 
        {
            filtered_data[*filtered_size] = data[i];
            (*filtered_size)++;
        }
    }
}










/*****************************************************************************************************************************************/
/****************************************************二维双调和样条插值算法*****************************************************************/
/*****************************************************************************************************************************************/
//径向基函数 (薄板样条)
float radial_basis_function(float r) 
{
    if (r == 0.0) return 0.0;
    return r * r * log(r);
}

//分块计算插值矩阵
void compute_interpolation_matrix_block(int n, const float *x, const float *y, float *C_block, int row_start, int col_start, int block_size) 
{
    for (int i = 0; i < block_size; i++) 
    {
        for (int j = 0; j < block_size; j++) 
        {
            int row = row_start + i;
            int col = col_start + j;
            if (row < n && col < n) 
            {
                if (row == col) 
                {
                    C_block[i * block_size + j] = 0.0f; //对角线设置为 0
                } 
                else 
                {
                    float dx = x[row] - x[col];
                    float dy = y[row] - y[col];
                    float r = sqrt(dx * dx + dy * dy);
                    C_block[i * block_size + j] = radial_basis_function(r);
                    //C_block[i * block_size + j] = gaussian_basis_function(r,0.5);
                }
            } 
            else 
            {
                C_block[i * block_size + j] = 0.0f; //超出矩阵范围的部分填充 0
            }
        }
    }
}

//分块构造插值矩阵
void compute_interpolation_matrix(int n, const float *x, const float *y, float *C) 
{
    int num_blocks = (n + BLOCK_SIZE - 1) / BLOCK_SIZE; //计算块的数量
    for (int block_row = 0; block_row < num_blocks; block_row++) 
    {
        for (int block_col = 0; block_col < num_blocks; block_col++) 
        {
            int row_start = block_row * BLOCK_SIZE;
            int col_start = block_col * BLOCK_SIZE;
            compute_interpolation_matrix_block(n, x, y, A_block, row_start, col_start, BLOCK_SIZE);

            //将块数据写回全局矩阵 C
            for (int i = 0; i < BLOCK_SIZE; i++) 
            {
                for (int j = 0; j < BLOCK_SIZE; j++) 
                {
                    int row = row_start + i;
                    int col = col_start + j;
                    if (row < n && col < n) 
                    {
                        C[row * n + col] = A_block[i * BLOCK_SIZE + j];
                    }
                }
            }
        }
    }
}

//分块高斯消元法
void gaussian_elimination(int n, float *C, float *B, float *X) 
{
    for (int i = 0; i < n; i++) 
    {
        //查找主元
        int max_row = i;
        for (int k = i + 1; k < n; k++) 
        {
            if (fabs(C[k * n + i]) > fabs(C[max_row * n + i])) 
            {
                max_row = k;
            }
        }

        //交换当前行和主元行
        for (int k = i; k < n; k++) 
        {
            float temp = C[i * n + k];
            C[i * n + k] = C[max_row * n + k];
            C[max_row * n + k] = temp;
        }
        float temp = B[i];
        B[i] = B[max_row];
        B[max_row] = temp;

        //如果主元接近零，矩阵可能是奇异的
        if (fabs(C[i * n + i]) < 1e-10) 
        {
            ESP_LOGI(TAG,"Matrix is singular or nearly singular\n");
        }

        //消元过程
        for (int k = i + 1; k < n; k++) 
        {
            float factor = C[k * n + i] / C[i * n + i];
            for (int j = i; j < n; j++) 
            {
                C[k * n + j] -= factor * C[i * n + j];
            }
            B[k] -= factor * B[i];
        }
    }

    //回代过程
    for (int i = n - 1; i >= 0; i--) 
    {
        X[i] = B[i];
        for (int j = i + 1; j < n; j++) 
        {
            X[i] -= C[i * n + j] * X[j];
        }
        X[i] /= C[i * n + i];
    }
}

//插值计算
float interpolate(int n, const float *x, const float *y, const float *lambda, float x0, float y0) 
{
    float result = 0.0f;
    for (int i = 0; i < n; i++) 
    {
        float dx = x0 - x[i];
        float dy = y0 - y[i];
        float r = sqrt(dx * dx + dy * dy);
        result += lambda[i] * radial_basis_function(r);
    }
    return result;
}



//一维三次样条插值
void computeSplineCoeffs(float x[], float y[], SplineCoeffs* spline) 
{
    int i;
    float h[N_spline], alpha[N_spline], l[N_spline], mu[N_spline], z[N_spline];

    for (i = 0; i < N_spline; i++) 
    {
        spline->x[i] = x[i];
        spline->a[i] = y[i];
    }

    for (i = 0; i < N_spline - 1; i++)
        h[i] = x[i + 1] - x[i];

    for (i = 1; i < N_spline - 1; i++)
        alpha[i] = (3.0f / h[i]) * (spline->a[i + 1] - spline->a[i]) - (3.0f / h[i - 1]) * (spline->a[i] - spline->a[i - 1]);

    l[0] = 1.0f; mu[0] = 0.0f; z[0] = 0.0f;

    for (i = 1; i < N_spline - 1; i++) 
    {
        l[i] = 2.0f * (x[i + 1] - x[i - 1]) - h[i - 1] * mu[i - 1];
        mu[i] = h[i] / l[i];
        z[i] = (alpha[i] - h[i - 1] * z[i - 1]) / l[i];
    }

    l[N_spline - 1] = 1.0f; z[N_spline - 1] = 0.0f; spline->c[N_spline - 1] = 0.0f;

    for (i = N_spline - 2; i >= 0; i--) 
    {
        spline->c[i] = z[i] - mu[i] * spline->c[i + 1];
        spline->b[i] = (spline->a[i + 1] - spline->a[i]) / h[i] - h[i] * (spline->c[i + 1] + 2.0f * spline->c[i]) / 3.0f;
        spline->d[i] = (spline->c[i + 1] - spline->c[i]) / (3.0f * h[i]);
    }
}


float evalSpline(SplineCoeffs* spline, float x_val) 
{
    int i;

    // 外推：小于最小值
    if (x_val < spline->x[0]) 
    {
        // 使用第一个区间的系数进行外推
        float dx = x_val - spline->x[0];
        return spline->a[0] + spline->b[0] * dx + spline->c[0] * dx * dx + spline->d[0] * dx * dx * dx;
    }

    // 外推：大于最大值
    if (x_val > spline->x[N_spline - 1]) 
    {
        // 使用最后一个区间的系数进行外推
        float dx = x_val - spline->x[N_spline - 2]; // 使用倒数第二个点的系数进行外推
        return spline->a[N_spline - 2] + spline->b[N_spline - 2] * dx + spline->c[N_spline - 2] * dx * dx + spline->d[N_spline - 2] * dx * dx * dx;
    }

    // 正常插值
    for (i = N_spline - 2; i >= 0; i--) 
    {
        if (x_val >= spline->x[i]) break;
    }

    float dx = x_val - spline->x[i];
    return spline->a[i] + spline->b[i] * dx + spline->c[i] * dx * dx + spline->d[i] * dx * dx * dx;
}



    // SplineCoeffs spline;
    // computeSplineCoeffs(MD_X, MD_GY, &spline);

    // // 测试插值输出
    // float test_x[] = {5.0, 6.0, 10.0, 17.0};
    // for (int i = 0; i < 4; i++) {
    //     float y = evalSpline(&spline, test_x[i]);
    //     printf("Spline(%.2f) = %.4f\n", test_x[i], y);
    // }


