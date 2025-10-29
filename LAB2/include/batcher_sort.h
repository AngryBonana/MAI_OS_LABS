#ifndef BATCHER_H
#define BATCHER_H
#include <stdint.h>
#include <pthread.h>

extern int max_threads;
extern int active_threads;
extern pthread_mutex_t mtx;

void batcher_sort(int32_t *arr, int32_t n, int32_t ascending);

void odd_even_merge(int32_t *arr, int32_t n, int32_t ascending);

void compare_and_swap(int32_t *a, int32_t *b, int32_t ascending);

#endif