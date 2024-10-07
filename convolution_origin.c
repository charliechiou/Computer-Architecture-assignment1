#include <stdio.h>
#include <stdint.h>

int main()
{
    float x[10] = {1.0, 2.0, 0.0}; // FP16: 1.0, 2.0, 0.0
    float h[10] = {3.0, 2.0, 1.0}; // FP16: 3.0, 4.0, 1.0
    float y[20];
    // float x[30] = {0}, h[30] = {0}, y[60] = {0};
    int i, j, m = 3, n = 3;

    for (i = 0; i < m + n - 1; i++)
    {
        y[i] = 0;
        for (j = 0; j <= i; j++)
        {
            if (j < m && (i - j) < n)
            { 
                y[i] += x[j] * h[i - j];
            }
        }
    }

    printf("Convoluted sequence is :\n");
    for (i = 0; i < m + n - 1; i++)
    {
        printf("y[%d] = %f\n", i, y[i]);
    }

    return 0;
}
