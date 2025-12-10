#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>  
#include "include/libfunc.h"

int main()
{
    char buf[512];
    while (1)
    {
        int n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
        if (n <= 0) 
        {
            break;
        }
        buf[n] = '\0';
        if (buf[n-1] == '\n') 
        {
            buf[n-1] = '\0';
        }
        char* args[64];
        int argc = 0;
        char* p = buf;
        while (*p)
        {
            while (*p == ' ') 
            {
                p++;
            }
            if (!*p) 
            {
                break;
            }
            args[argc++] = p;
            while (*p && *p != ' ') 
            {
                p++;
            }
            if (*p) 
            {
                *p++ = '\0';
            }
        }

        if (argc == 0) 
        {
            continue;
        }
        if (strcmp(args[0], "1") == 0 && argc == 4) 
        {
            float a = strtof(args[1], NULL);
            float b = strtof(args[2], NULL);
            float e = strtof(args[3], NULL);
            float res = sin_integral(a, b, e);
            char out[64];
            int len = snprintf(out, sizeof(out), "%.6f\n", res);
            write(STDOUT_FILENO, out, len);
        }
        else if (strcmp(args[0], "2") == 0 && argc >= 2)
        {
            int n = (int)strtol(args[1], NULL, 10);
            if (n <= 0) 
            {
                continue;
            }
            int* arr = malloc(n * sizeof(int));
            for (int i = 0; i < n && i + 2 < argc; ++i)
            {
                arr[i] = (int)strtol(args[2 + i], NULL, 10);
            }
            sort(arr, n);
            char out[1024];
            int pos = 0;
            for (int i = 0; i < n; ++i)
            {
                pos += snprintf(out + pos, sizeof(out) - pos, "%d ", arr[i]);
            }
            if (pos > 0) 
            {
                out[pos - 1] = '\n';
            } 
            else 
            {
                out[pos++] = '\n';
            }
            write(STDOUT_FILENO, out, pos);
            free(arr);
        }
        else 
        {
            const char msg[] = "Unsupported command\n";
            write(STDOUT_FILENO, msg, sizeof(msg) - 1);
        }
    }
    return 0;
}