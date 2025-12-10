#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>

typedef float (*sin_integral_t)(float, float, float);
typedef int* (*sort_t)(int*, size_t);

int main()
{
    void* handle = dlopen("./libvar1.so", RTLD_LAZY);
    if (!handle) 
    {
        write(2, "dlopen failed\n", 14);
        return 1;
    }

    sin_integral_t sin_integral = dlsym(handle, "sin_integral");
    sort_t sort = dlsym(handle, "sort");

    int current = 1;
    char buf[512];

    while (1)
    {
        int n = read(0, buf, sizeof(buf) - 1);
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

        if (strcmp(args[0], "0") == 0) 
        {
            dlclose(handle);
            if (current == 1) 
            {
                handle = dlopen("./libvar2.so", RTLD_LAZY);
                current = 2;
            } 
            else 
            {
                handle = dlopen("./libvar1.so", RTLD_LAZY);
                current = 1;
            }
            if (!handle) 
            { 
                write(2, "dlopen failed\n", 14); return 1;
            }
            sin_integral = dlsym(handle, "sin_integral");
            sort = dlsym(handle, "sort");
            char msg[32];
            int len = snprintf(msg, sizeof(msg), "Switched to var %d\n", current);
            write(1, msg, len);
        }
        else if (strcmp(args[0], "1") == 0 && argc == 4)
        {
            float a = strtof(args[1], NULL);
            float b = strtof(args[2], NULL);
            float e = strtof(args[3], NULL);
            float res = sin_integral(a, b, e);
            char out[64];
            int len = snprintf(out, sizeof(out), "%.6f\n", res);
            write(1, out, len);
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

    dlclose(handle);
    return 0;
}