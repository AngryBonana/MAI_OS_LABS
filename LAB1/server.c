#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


int main(int argc, char **argv) {
    if (argc != 2) {
        const char msg[] = "usage: server output_filename\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    int32_t file = open(argv[1], O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (file == -1) {
        const char msg[] = "error: failed to open requested file\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    char line[4096];

    while (1) {
        ssize_t bytes = read(STDIN_FILENO, line, sizeof(line) - 1);
        if (bytes <= 0)
            break;

        line[bytes] = '\0';
        if (line[0] == '\n')
            break;

        int32_t nums[100];
        int32_t count = 0;
        char *token = strtok(line, " \t\n");
        while (token && count < 1024) {
            nums[count++] = atoi(token);
            token = strtok(NULL, " \t\n");
        }

        if (count < 2) {
            const char msg[] = "error: need at least two numbers\n";
            write(STDOUT_FILENO, msg, sizeof(msg) - 1);
            continue;
        }

        int32_t first = nums[0];
        char outbuf[1024];
        for (uint32_t i = 1; i < count; i++) {
            if (nums[i] == 0) {
                const char msg[] = "error: zero division\n";
                write(STDOUT_FILENO, msg, sizeof(msg) - 1);
                close(file);
                exit(EXIT_FAILURE);
            }
            int32_t result = first / nums[i];
            int32_t len = snprintf(outbuf, sizeof(outbuf), "%d // %d = %d\n", first, nums[i], result);
            write(file, outbuf, len);
        }

        const char okmsg[] = "Done!\n";
        write(STDOUT_FILENO, okmsg, sizeof(okmsg) - 1);
    }

    close(file);
    return 0;
}
