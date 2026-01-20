#pragma once
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"

#define MAX_MEMORY_SIZE 1024

typedef enum {
    MEM_OK = 0,
    MEM_ERROR = 1,
    MEM_ERR_INVALID_PARAM = 2
} MemoryOperationStatus;

MemoryOperationStatus platform_malloc(void** mem_location, uint32_t size);
void platform_free(void* memory);