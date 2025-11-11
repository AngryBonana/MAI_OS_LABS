#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>


#define SHM_SIZE 4096

int main(int argc, char **argv) {
    if (argc != 5) {
        const char msg[] = "Usage: child <file> <shm> <sem_srv> <sem_chd>\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        _exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (fd == -1) {
        const char msg[] = "Can't open file\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        _exit(EXIT_FAILURE);
    }

    int shm_fd = shm_open(argv[2], O_RDWR, 0);
    if (shm_fd == -1) {
        const char msg[] = "Can't open shm\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        _exit(EXIT_FAILURE);
    }

    char *shm_buf = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_buf == MAP_FAILED) {
        const char msg[] = "Can't mmap\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        _exit(EXIT_FAILURE);
    }

    sem_t *sem_child = sem_open(argv[4], 0); 
    if (sem_child == SEM_FAILED) {
        const char msg[] = "Can't create sem\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        _exit(EXIT_FAILURE);
    }

    sem_t *sem_server = sem_open(argv[3], 0);
    if (sem_server == SEM_FAILED) {
        const char msg[] = "Can't create sem\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        _exit(EXIT_FAILURE);
    }

    uint32_t *len_ptr = (uint32_t *)shm_buf;
    char *text = shm_buf + sizeof(uint32_t);

    while (1) {
        sem_wait(sem_child);

        if (*len_ptr == UINT32_MAX) {
            break;
        }

        if (*len_ptr == 0) {
            sem_post(sem_server);
            continue;
        }

        text[*len_ptr] = '\0';

        char *buf = strdup(text); 
        if (!buf) {
            const char msg[] = "Out of memory\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            break;
        }

        int nums[100];
        int count = 0;
        char *token = strtok(buf, " \t");

        while (token && count < 100) {
            char *end;
            long val = strtol(token, &end, 10);
            if (*end != '\0') { 
                break;
            }
            nums[count++] = (int)val;
            token = strtok(NULL, " \t");
        }

        free(buf);

        if (count < 2) {
            const char msg[] = "Need at least two integers\n";
            write(STDOUT_FILENO, msg, sizeof(msg) - 1);
            *len_ptr = 0;
            sem_post(sem_server);
            continue;
        }

        for (int i = 1; i < count; i++) {
            if (nums[i] == 0) {
                const char err[] = "Division by zero! Terminating.\n";
                write(STDERR_FILENO, err, sizeof(err) - 1);
                *len_ptr = UINT32_MAX;
                sem_post(sem_server);
                close(fd);
                _exit(EXIT_FAILURE); 
            }
        }

        int first = nums[0];
        char result_buf[1024] = {0};
        int total_len = 0;

        for (int i = 1; i < count; i++) {
            int res = first / nums[i];
            int n = snprintf(result_buf + total_len,
                             sizeof(result_buf) - total_len,
                             "%d / %d = %d\n", first, nums[i], res);
            if (n < 0 || (size_t)(n + total_len) >= sizeof(result_buf)) break;
            total_len += n;

            char line[64];
            int len = snprintf(line, sizeof(line), "%d / %d = %d\n", first, nums[i], res);
            if (len > 0) {
                write(fd, line, len);
            }
        }

        if (total_len > 0) {
            memcpy(text, result_buf, total_len);
            *len_ptr = total_len;
        } else {
            const char ok[] = "Done.\n";
            memcpy(text, ok, sizeof(ok) - 1);
            *len_ptr = sizeof(ok) - 1;
        }

        sem_post(sem_server);
    }

    sem_close(sem_child);
    sem_close(sem_server);
    munmap(shm_buf, SHM_SIZE);
    close(shm_fd);
    close(fd);

    return 0;
}