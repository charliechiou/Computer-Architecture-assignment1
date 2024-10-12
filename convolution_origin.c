#include <stdio.h>
#include <stdint.h>

int main()
{
    // Initialize two sequences
    float x[10] = {1.0, 2.0, 0.0}; // x: 1.0, 2.0, 0.0
    float h[10] = {3.0, 2.0, 1.0}; // y: 3.0, 4.0, 1.0
    float y[20];
    // initialize two sequence
    int i, j, m = 3, n = 3; // Lengths of sequences x and h are both 3

    for (i = 0; i < m + n - 1; i++) // Loop through each position in the output sequence y
    {
        y[i] = 0;                // Initialize the current output sample y[i] to 0
        for (j = 0; j <= i; j++) // Loop through each element of sequences x and h
        {
            // Ensure the indices are within bounds for sequences x and h
            if (j < m && (i - j) < n)
            {
                y[i] += x[j] * h[i - j]; // calculate
            }
        }
    }

    // Display the output
    printf("Convoluted sequence is :\n");
    for (i = 0; i < m + n - 1; i++)
    {
        printf("y[%d] = %f\n", i, y[i]);
    }

    return 0;
}
