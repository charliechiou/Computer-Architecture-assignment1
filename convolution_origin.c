#include <stdio.h>

int main()
{
    // 增加陣列大小以確保足夠空間，並考慮卷積結果的最大長度
    float x[30] = {0}, h[30] = {0}, y[60] = {0}; // 假設序列長度不超過 30
    int i, j, m, n;

    // 輸入兩個序列的長度
    printf("Enter the length of the first sequence, m = ");
    scanf("%d", &m);
    printf("Enter the length of the second sequence, n = ");
    scanf("%d", &n);

    // 輸入第一個序列
    printf("Enter the first sequence x[n]: ");
    for (i = 0; i < m; i++)
    {
        scanf("%f", &x[i]);
    }

    // 輸入第二個序列
    printf("Enter the second sequence h[n]: ");
    for (i = 0; i < n; i++)
    {
        scanf("%f", &h[i]);
    }

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

