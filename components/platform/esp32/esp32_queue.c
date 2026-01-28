#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "platform/platform_queue.h"

PlatformQueueHandle platform_queue_create(size_t queue_length, size_t element_size)
{
    return (PlatformQueueHandle) xQueueCreate(queue_length, element_size);
}

QueueOperationStatus platform_queue_send(PlatformQueueHandle queue, QueueElement_t* item, uint32_t timeout_in_ms)
{
    if(xQueueSend((QueueHandle_t)queue, item, pdMS_TO_TICKS(timeout_in_ms)) == pdTRUE) {
        return QUEUE_OK;
    } else {
        return QUEUE_FULL;
    }
}

QueueOperationStatus platform_queue_get(PlatformQueueHandle queue, QueueElement_t* buffer, uint32_t timeout_in_ms)
{
    if(xQueueReceive((QueueHandle_t)queue, buffer, pdMS_TO_TICKS(timeout_in_ms)) == pdTRUE) {
        return QUEUE_OK;
    } else {
        return QUEUE_EMPTY;
    }
}

void platform_queue_delete(PlatformQueueHandle queue)
{
    if(queue != NULL) {
        vQueueDelete((QueueHandle_t)queue);
    }
}

uint32_t platform_get_num_of_queue_elements(PlatformQueueHandle queue)
{
    if(queue != NULL) {
        return uxQueueMessagesWaiting((QueueHandle_t)queue);
    } else {
        return -1;
    }
}

QueueOperationStatus platform_queue_reset(PlatformQueueHandle queue)
{
    if(queue != NULL) {
        xQueueReset(queue);
        return QUEUE_OK;
    } else {
        return QUEUE_ERROR;
    }
}