#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>

#define SHM_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 2) {
        const char msg[] = "usage: ./client <filename>";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    char shm_name[64], sem_server_name[64], sem_child_name[64];
    pid_t mypid = getpid();

    snprintf(shm_name, sizeof(shm_name), "/myapp-%d-shm", mypid);
    snprintf(sem_server_name, sizeof(sem_server_name), "/myapp-%d-srv", mypid);
    snprintf(sem_child_name, sizeof(sem_child_name), "/myapp-%d-chd", mypid);

    // Создаём SHM
    int shm_fd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (shm_fd == -1) {
        const char msg[] = "error: can't open shm\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {

        const char msg[] = "error: can't resize shm\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    char *shm_buf = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_buf == MAP_FAILED) {
        const char msg[] = "error: mmap failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    
    sem_t *sem_server = sem_open(sem_server_name, O_CREAT | O_EXCL, 0600, 1);
    if (sem_server == SEM_FAILED) {
        const char msg[] = "error: can't create sem\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    sem_t *sem_child = sem_open(sem_child_name, O_CREAT | O_EXCL, 0600, 0);
    if (sem_child == SEM_FAILED) {
        const char msg[] = "error: can't create sem\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    pid_t child = fork();
    if (child == -1) {
        const char msg[] = "error: can't create child\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    if (child == 0) {
        // Дочерний процесс
        char *args[] = {
            "./child",
            argv[1],
            shm_name,
            sem_server_name,
            sem_child_name,
            NULL
        };
        int32_t status = execv("./child", args);
        if (status == -1)
        {
            const char msg[] = "error: can't execv\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            _exit(EXIT_FAILURE);

        }
    }

    //Родительский процесс
    {
        char buf[128];
        snprintf(buf, 128, "%d: I'm parent, child PID = %d\n", (int)mypid, (int)child);
        write(STDOUT_FILENO, buf, strlen(buf));
        const char msg[] = "Enter numbers (like '10 2 5'), blank line to exit.\n";
        write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    }

    bool running = true;
    uint32_t *len_ptr = (uint32_t *)shm_buf;
    char *text = shm_buf + sizeof(uint32_t);
    const size_t max_text_len = SHM_SIZE - sizeof(uint32_t) - 1; 

    while (running) {
        sem_wait(sem_server);

        if (*len_ptr == UINT32_MAX) {
            const char msg[] = "Child requested termination\n";
            write(STDOUT_FILENO, msg, sizeof(msg) - 1);
            running = false;
            break;
        }

        if (*len_ptr > 0) {
            write(STDOUT_FILENO, "Result: ", 8);
            write(STDOUT_FILENO, text, *len_ptr);
            *len_ptr = 0;
            sem_post(sem_server); 
            continue;
        }


        write(STDOUT_FILENO, "Input> ", 7);

        char input[max_text_len + 2];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            *len_ptr = UINT32_MAX;
            sem_post(sem_child);
            break;
        }

        size_t input_len = strlen(input);
        if (input_len == 0 || (input_len == 1 && input[0] == '\n')) {

            *len_ptr = UINT32_MAX;
            sem_post(sem_child);
            break;
        }


        if (input[input_len - 1] == '\n') {
            input[--input_len] = '\0';
        }

        if (input_len == 0) {
            *len_ptr = UINT32_MAX;
            sem_post(sem_child);
            break;
        }

        if (input_len > max_text_len) {
            const char msg[] = "Input too long!\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            continue;
        }

        *len_ptr = (uint32_t)input_len;
        memcpy(text, input, input_len);
        text[input_len] = '\0';

        sem_post(sem_child);
    }

    waitpid(child, NULL, 0);
    

    sem_close(sem_server);
    sem_close(sem_child);
    sem_unlink(sem_server_name);
    sem_unlink(sem_child_name);

    munmap(shm_buf, SHM_SIZE);
    close(shm_fd);
    shm_unlink(shm_name);

    return 0;
}