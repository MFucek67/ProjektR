#pragma once
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"

typedef enum {
    QUEUE_OK = 0,
    QUEUE_TIMEOUT = 1,
    QUEUE_ERROR = 2,
    QUEUE_FULL = 3,
    QUEUE_EMPTY = 4
} QueueOperationStatus;

typedef struct {
    uint8_t* data;
    size_t len;
} QueueElement_t;

typedef void* QueueHandle_t;

QueueHandle_t platform_queue_create(size_t queue_length, size_t element_size);
QueueOperationStatus platform_queue_send(QueueHandle_t queue, QueueElement_t* item, uint32_t timeout);
QueueOperationStatus platform_queue_get(QueueHandle_t queue, QueueElement_t* buffer, uint32_t timeout);
void platform_queue_delete(QueueHandle_t queue);
uint32_t platform_get_num_of_queue_elements(QueueHandle_t queue);

//Stvaranje queue i upravljanje njima