/**
 * @file esp32_memory.c
 * @author Marko Fuček
 * @brief ESP32 implementacija platform_memory API-ja.
 * 
 * Ovaj modul implementira funkcije za sigurnu alokaciju i oslobađanje alocirane memorije
 * u ESP-IDF frameworku.
 * 
 * @version 0.1
 * @date 2026-01-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "platform/platform_memory.h"

MemoryOperationStatus platform_malloc(void** mem_location, uint32_t size)
{
    if(size > MAX_MEMORY_SIZE) {
        return MEM_ERR_INVALID_PARAM;
    }
    void* allocated_memory = malloc(size);

    if(allocated_memory == NULL) {
        return MEM_ERROR;
    } else {
        *mem_location = allocated_memory;
        return MEM_OK;
    }
}

void platform_free(void* memory)
{
    free(memory);
}