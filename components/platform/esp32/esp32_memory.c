#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "../include/platform/platform_memory.h"

void* platform_malloc(void** mem_location, uint32_t size)
{
    if(size > MAX_MEMORY_SIZE) {
        return MEM_ERR_INVALID_PARAM;
    }
    void* allocated_memory = malloc(size);

    if(allocated_memory == NULL) {
        return MEM_ERROR;
    } else {
        mem_location = &allocated_memory;
        return MEM_OK;
    }
}

void platform_free(void* memory)
{
    free(memory);
}