#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "../include/platform/platform_queue.h"

QueueHandle_t platform_queue_create(size_t queue_length, size_t element_size)
{
    return xQueueCreate(queue_length, element_size);
}

QueueOperationStatus platform_queue_send(QueueHandle_t queue, QueueElement_t* item, uint32_t timeout_in_ms)
{
    if(xQueueSend(queue, item, pdMS_TO_TICKS(timeout_in_ms)) == pdTRUE) {
        return QUEUE_OK;
    } else {
        return QUEUE_FULL;
    }
}

QueueOperationStatus platform_queue_get(QueueHandle_t queue, QueueElement_t* buffer, uint32_t timeout_in_ms)
{
    if(xQueueReceive(queue, buffer, pdMS_TO_TICKS(timeout_in_ms)) == pdTRUE) {
        return QUEUE_OK;
    } else {
        return QUEUE_EMPTY;
    }
}

void platform_queue_delete(QueueHandle_t queue)
{
    if(queue != NULL) {
        vQueueDelete(queue);
    }
}

uint32_t platform_get_num_of_queue_elements(QueueHandle_t queue)
{
    if(queue != NULL) {
        return uxQueueMessagesWaiting(queue);
    } else {
        return -1;
    }
}