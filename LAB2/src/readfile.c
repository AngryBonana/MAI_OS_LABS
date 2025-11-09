#include "../include/readfile.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

char *read_from_file(const int32_t fd, uint32_t *error_status)
{
    uint32_t size = 256;
    uint32_t counter = 0;
    char *buf = (char *)malloc(sizeof(char) * size);
    if (buf == NULL)
    {
        *error_status = 1;
        return NULL;
    }

    int32_t bytes;
    while ((bytes = read(fd, buf + counter, size - counter - 1)) > 0)
    {
        if (bytes == -1)
        {
            *error_status = 2;
            free(buf);
            return NULL;
        }
        
        counter += bytes;
        
        if (counter == size - 1)
        {
            size *= 2;
            char *tmp = (char *)realloc(buf, sizeof(char) * size);
            if (tmp == NULL)
            {
                *error_status = 1;
                free(buf);
                return NULL;
            }
            buf = tmp;
        }
    }

    if (counter < size) {
        buf[counter] = '\0';
    } else {
        char *tmp = (char *)realloc(buf, sizeof(char) * (size + 1));
        if (tmp == NULL)
        {
            *error_status = 1;
            free(buf);
            return NULL;
        }
        buf = tmp;
        buf[counter] = '\0';
    }

    *error_status = 0;
    return buf;
}


int32_t *convert_to_int (char *data, uint32_t *error_status, uint32_t *array_size)
{
    if (data == NULL)
    {
        *array_size = 0;
        *error_status = 2;
        return NULL;
    }
    uint32_t size = 64;
    uint32_t counter = 0;
    int32_t *nums = (int32_t *)malloc(sizeof(int32_t) * size);
    if (nums == NULL)
    {
        *array_size = 0;
        *error_status = 1;
        return NULL;
    }

    char *tmp;
    tmp = strtok(data, " \t\n");
    while (tmp != NULL)
    {
        nums[counter] = atoi(tmp);
        counter++;
        if (counter == size)
        {
            size *= 2;
            
            int32_t *p_tmp = (int32_t *)realloc(nums, sizeof(int32_t) * size);
            if (p_tmp == NULL)
            {
                *array_size = 0;
                *error_status = 1;
                free(nums);
                return NULL;
            }
            nums = p_tmp;
            p_tmp = NULL;
        }
        tmp = strtok(NULL, " \t\n");
    }
    

    *array_size = counter;
    *error_status = 0;
    return nums;
}


uint32_t write_nums_to_file(const int32_t fd, int32_t *nums, uint32_t size)
{
    char buf[50];
    int8_t counter = 0;
    int32_t bytes;
    
    for (uint32_t i = 0; i < size; i++)
    {
        int len = snprintf(buf, sizeof(buf), "%d ", nums[i]);
        if (len < 0 || len >= sizeof(buf)) {
            return 1;
        }
        
        bytes = write(fd, buf, len);
        if (bytes == -1 || bytes != len)
        {
            return 1;
        }
        
        counter++;
        if (counter == 10)
        {
            bytes = write(fd, "\n", 1);
            if (bytes == -1 || bytes != 1)
            {
                return 1;
            }
            counter = 0;
        }
        
    }

    return 0;
}