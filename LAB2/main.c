#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "include/readfile.h"
#include "include/batcher_sort.h"
#include <time.h>



int main (int argc, char *argv[])
{
    if (argc != 4)
    {
        const char msg[] = "Using: ./program <num_of_threads> <input_file> <output_file>\n";
        write(STDOUT_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_SUCCESS);
    }
    const int32_t num_of_threads = atoi(argv[1]);
    if (num_of_threads < 1)
    {
        const char msg[] = "ERROR: Invalid num of threads\nNum of threads must be possitive integer\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    int32_t input_fd = open(argv[2], O_RDONLY);
    if (input_fd == -1)
    {
        const char msg[] = "ERROR: Invalid filename\nCan't open input file\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }
    int32_t output_fd = open(argv[3], O_WRONLY | O_CREAT);
    if (output_fd == -1)
    {
        const char msg[] = "ERROR: Invalid filename\nCan't open output file\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        close(input_fd);
        exit(EXIT_FAILURE);
    }

    uint32_t error_status;

    char *file_data = read_from_file(input_fd, &error_status);
    switch (error_status)
    {
        case 1:
        {
            const char msg[] = "ERROR: Memory Error\nCan't get memory to read file\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            close(input_fd);
            close(output_fd);
            exit(EXIT_FAILURE);
        }
        break;
        case 2:
        {
            const char msg[] = "ERROR: Descriptor Error\nDescriptor can't read from input file\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            close(input_fd);
            close(output_fd);
            exit(EXIT_FAILURE);
        }
        break;
    }
    close(input_fd);

    uint32_t nums_size = 0;
    int32_t *nums = convert_to_int(file_data, &error_status, &nums_size);
    
    switch (error_status)
    {
        case 1:
        {
            const char msg[] = "ERROR: Memory Error\nCan't get memory to convert data\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            close(output_fd);
            free(file_data);
            file_data = NULL;
            exit(EXIT_FAILURE);
        }
        break;
        case 2:
        {
            const char msg[] = "ERROR: Ptr Error\nFunction got wrong ptr\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            close(output_fd);
            free(file_data);
            file_data = NULL;
            exit(EXIT_FAILURE);
        }
        break;

    }
    free(file_data);
    file_data = NULL;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    batcher_sort(nums, nums_size, num_of_threads);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0
                   + (end.tv_nsec - start.tv_nsec) / 1e6;


    char buf[100] = {};
    snprintf(buf, 100, "Потоков: %u\tВремя: %.3f мс\n", num_of_threads, elapsed_ms);
    write(STDOUT_FILENO, buf, 100);

    write_nums_to_file(output_fd, nums, nums_size);
    free(nums);
    nums = NULL;

    return 0;
}