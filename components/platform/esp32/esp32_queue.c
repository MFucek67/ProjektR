/**
 * @file esp32_queue.c
 * @author Marko Fuček
 * @brief ESP32 implementacija platform_queue API-ja.
 * 
 * Ovaj modul implementira funkcije za rad s queue-ovima na platform sloju. Modul je wrapper za
 * FreeRTOS queue API kako bi se omogućila apstrakcija i neovisnost ostatka sustava o platformi.
 * 
 * @note Sve timeout vrijednosti se automatski konvertiraju iz milisekundi u tickove koristeći
 * FreeRTOS pdMS_TO_TICKS makro.
 * 
 * @version 0.1
 * @date 2026-01-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

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