#include <math.h>
#include "../include/libfunc.h"


float sin_integral(float a, float b, float e) 
{
    float sum = 0;
    for (float x = a; x < b; x += e) 
    {
        sum += sinf(x) * e;
    }
    return sum;
}


int* sort(int* array, size_t n) 
{
    for (size_t i = 0; i < n - 1; ++i) 
    {
        for (size_t j = 0; j < n - i - 1; ++j) 
        {
            if (array[j] > array[j + 1]) 
            {
                int tmp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = tmp;
            }
        }
    }
    return array;
}