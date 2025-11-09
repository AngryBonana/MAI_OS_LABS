#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>


typedef struct {
    int32_t *a;
    uint32_t start;
    uint32_t end;
    uint32_t j;
    uint32_t k;
} thread_task_t;


static inline void compare_and_swap(int32_t *a, uint32_t i, uint32_t j, int dir) 
{
    if (dir == (a[i] > a[j])) 
    {
        int32_t tmp = a[i];
        a[i] = a[j];
        a[j] = tmp;
    }
}


static void *worker_thread(void *arg) 
{
    thread_task_t *task = (thread_task_t *)arg;
    uint32_t *a = (uint32_t *)task->a; 
    uint32_t j = task->j;
    uint32_t k = task->k;

    for (uint32_t i = task->start; i < task->end; i++) 
    {
        uint32_t ij = i ^ j;
        if (ij > i) 
        {
            int dir = ((i & k) == 0) ? 1 : 0;
            compare_and_swap(task->a, i, ij, dir);
        }
    }
    return NULL;
}


static void apply_layer(int32_t *a, uint32_t n, uint32_t k, uint32_t j, uint32_t max_threads) 
{

    const uint32_t MIN_WORK_PER_THREAD = 131032;

    if (n < MIN_WORK_PER_THREAD * max_threads || max_threads <= 1) 
    {

        for (uint32_t i = 0; i < n; i++) 
        {
            uint32_t ij = i ^ j;
            if (ij > i) 
            {
                int dir = ((i & k) == 0) ? 1 : 0;
                compare_and_swap(a, i, ij, dir);
            }
        }
        return;
    }

    uint32_t num_threads = (n + MIN_WORK_PER_THREAD - 1) / MIN_WORK_PER_THREAD;
    if (num_threads > max_threads) num_threads = max_threads;

    pthread_t *threads = calloc(num_threads, sizeof(pthread_t));
    thread_task_t *tasks = calloc(num_threads, sizeof(thread_task_t));

    if (!threads || !tasks) 
    {
        
        free(threads);
        free(tasks);
        for (uint32_t i = 0; i < n; i++) 
        {
            uint32_t ij = i ^ j;
            if (ij > i) 
            {
                int dir = ((i & k) == 0) ? 1 : 0;
                compare_and_swap(a, i, ij, dir);
            }
        }
        return;
    }

    uint32_t chunk = (n + num_threads - 1) / num_threads;

    int32_t ok = 1;
    for (uint32_t t = 0; t < num_threads; t++) 
    {
        uint32_t start = t * chunk;
        uint32_t end = (t == num_threads - 1) ? n : start + chunk;
        if (start >= end) break;

        tasks[t] = (thread_task_t)
        {
            .a = a,
            .start = start,
            .end = end,
            .j = j,
            .k = k
        };

        if (pthread_create(&threads[t], NULL, worker_thread, &tasks[t]) != 0) 
        {
            ok = 0;
            break;
        }
    }

    if (ok) 
    {
        for (uint32_t t = 0; t < num_threads; t++) 
        {
            pthread_join(threads[t], NULL);
        }
        free(threads);
        free(tasks);
        return;
    }

    free(threads);
    free(tasks);
    for (uint32_t i = 0; i < n; i++) 
    {
        uint32_t ij = i ^ j;
        if (ij > i) 
        {
            int dir = ((i & k) == 0) ? 1 : 0;
            compare_and_swap(a, i, ij, dir);
        }
    }
}


void batcher_sort(int32_t *a, uint32_t n, uint32_t num_threads)
{
    if (n <= 1) return;

    if ((n & (n - 1)) != 0) 
    {
        const char msg[] = "Error: n must be a power of two (1, 2, 4, 8, ...)\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        return;
    }

    if (num_threads == 0) num_threads = 1;

    for (uint32_t k = 2; k <= n; k <<= 1) 
    {          
        for (uint32_t j = k >> 1; j > 0; j >>= 1) 
        { 
            apply_layer(a, n, k, j, num_threads);
        }
    }
}