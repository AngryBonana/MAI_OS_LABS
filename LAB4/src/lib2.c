#include <math.h>
#include "../include/libfunc.h"


float sin_integral(float a, float b, float e) 
{
    float sum = 0;
    for (float x = a; x < b; x += e) 
    {
        sum += (sinf(x) + sinf(x + e)) * e * 0.5;
    }
    return sum;
}


void quicksort(int* arr, int low, int high) 
{
    if (low >= high) 
    {
        return;
    }
    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; ++j) 
    {
        if (arr[j] <= pivot) 
        {
            ++i;
            int tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
        }
    }
    arr[high] = arr[i + 1];
    arr[i + 1] = pivot;
    quicksort(arr, low, i);
    quicksort(arr, i + 2, high);
}

int* sort(int* array, size_t n) 
{
    if (n > 0) 
    {
        quicksort(array, 0, (int)n - 1);
    }
    return array;
}