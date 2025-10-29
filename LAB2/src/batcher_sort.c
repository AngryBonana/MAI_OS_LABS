#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../include/batcher_sort.h"



typedef struct {
    int32_t *arr;
    int32_t n;
    int32_t ascending;
} task_t;

void *sort_worker(void *arg) {
    task_t *t = (task_t *)arg;
    batcher_sort(t->arr, t->n, t->ascending);
    free(t);

    pthread_mutex_lock(&mtx);
    active_threads--;
    pthread_mutex_unlock(&mtx);

    return NULL;
}


void merge(int32_t *arr, int32_t n, int32_t ascending) {
    if (n <= 1) return;

    int32_t mid = n / 2;
    int32_t *left = malloc(mid * sizeof(int32_t));
    int32_t *right = malloc((n - mid) * sizeof(int32_t));

    memcpy(left, arr, mid * sizeof(int32_t));
    memcpy(right, arr + mid, (n - mid) * sizeof(int32_t));

    int32_t i = 0, j = 0, k = 0;

    while (i < mid && j < n - mid) {
        if ((ascending && left[i] <= right[j]) ||
            (!ascending && left[i] >= right[j])) {
            arr[k++] = left[i++];
        } else {
            arr[k++] = right[j++];
        }
    }

    while (i < mid) arr[k++] = left[i++];
    while (j < n - mid) arr[k++] = right[j++];

    free(left);
    free(right);
}

void batcher_sort(int32_t *arr, int32_t n, int32_t ascending) {
    if (n <= 1) 
    {
        return;
    }
    int32_t mid = n / 2;
    pthread_t t1 = 0, t2 = 0;
    int32_t t1_created = 0, t2_created = 0;


    pthread_mutex_lock(&mtx);
    if (active_threads < max_threads) {
        active_threads++;
        pthread_mutex_unlock(&mtx);

        task_t *args = malloc(sizeof(task_t));
        args->arr = arr;
        args->n = mid;
        args->ascending = ascending;

        if (pthread_create(&t1, NULL, sort_worker, args) == 0) {
            t1_created = 1;
        } else {
            pthread_mutex_lock(&mtx);
            active_threads--;
            pthread_mutex_unlock(&mtx);
            free(args);
            batcher_sort(arr, mid, ascending);
        }
    } else {
        pthread_mutex_unlock(&mtx);
        batcher_sort(arr, mid, ascending);
    }


    pthread_mutex_lock(&mtx);
    if (active_threads < max_threads) {
        active_threads++;
        pthread_mutex_unlock(&mtx);

        task_t *args = malloc(sizeof(task_t));
        args->arr = arr + mid;
        args->n = n - mid;
        args->ascending = ascending;

        if (pthread_create(&t2, NULL, sort_worker, args) == 0) {
            t2_created = 1;
        } else {
            pthread_mutex_lock(&mtx);
            active_threads--;
            pthread_mutex_unlock(&mtx);
            free(args);
            batcher_sort(arr + mid, n - mid, ascending);
        }
    } else {
        pthread_mutex_unlock(&mtx);
        batcher_sort(arr + mid, n - mid, ascending);
    }

    
    if (t1_created)
    {
        pthread_join(t1, NULL);
    }
    if (t2_created) 
    {
        pthread_join(t2, NULL);
    }
    
    merge(arr, n, ascending);
}