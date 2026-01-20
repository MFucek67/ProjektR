#pragma once
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"

typedef void* MutexHandle_t;

typedef enum {
    MUTEX_OP_SUCCESSFUL = 0,
    MUTEX_OP_UNSUCCESSFUL = 1
} MutexOperationStatus;

MutexHandle_t platform_create_mutex(void);
MutexOperationStatus platform_lock_mutex(MutexHandle_t mutex, uint32_t timeout);
MutexOperationStatus platform_unlock_mutex(MutexHandle_t mutex);
void platform_delete_mutex(MutexHandle_t mutex);