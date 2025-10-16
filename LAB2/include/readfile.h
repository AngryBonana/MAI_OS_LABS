#ifndef READFILE_H
#define READFILE_H
#include <stdint.h>

char *read_from_file (const int32_t fd, uint32_t *error_status);

int32_t *convert_to_int (char *data, uint32_t *error_status, uint32_t *array_size);

uint32_t write_nums_to_file (const int32_t fd, int32_t *nums, uint32_t size);


#endif