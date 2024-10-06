#include <stdio.h>
#include <stdint.h>

int main()
{
    // 固定的 FP16 輸入值
    float x[10] = {1.0, 2.0, 0.0}; // FP16: 1.0, 2.0, 0.0
    float h[10] = {3.0, 2.0, 1.0}; // FP16: 3.0, 4.0, 1.0
    float y[20];
    // float x[30] = {0}, h[30] = {0}, y[60] = {0}; // 假設序列長度不超過 30
    int i, j, m = 3, n = 3;

    // 計算卷積
    for (i = 0; i < m + n - 1; i++)
    {
        y[i] = 0;
        for (j = 0; j <= i; j++)
        {
            if (j < m && (i - j) < n)
            { // 確保不超出 x 和 h 的範圍
                y[i] += x[j] * h[i - j];
            }
        }
    }

    // 輸出卷積結果
    printf("Convoluted sequence is :\n");
    for (i = 0; i < m + n - 1; i++)
    {
        printf("y[%d] = %f\n", i, y[i]);
    }

    return 0;
}
