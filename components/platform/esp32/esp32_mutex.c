/**
 * @file esp32_mutex.c
 * @author Marko Fuček
 * @brief ESP32 implementacija platform_mutex API-ja.
 * 
 * Ovaj modul pruža implementaciju funkcija za upravljanje mutexom na razini platforme.
 * Implementacija se temelji na FreeRTOS Semaphore mutexima.
 * 
 * @version 0.1
 * @date 2026-01-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "platform/platform_mutex.h"

MutexHandle_t platform_create_mutex(void)
{
    return xSemaphoreCreateMutex();
}
MutexOperationStatus platform_lock_mutex(MutexHandle_t mutex, uint32_t timeout)
{
    if(xSemaphoreTake(mutex, pdMS_TO_TICKS(timeout)) == pdTRUE) {
        return MUTEX_OP_SUCCESSFUL;
    } else {
        return MUTEX_OP_UNSUCCESSFUL;
    }
}
MutexOperationStatus platform_unlock_mutex(MutexHandle_t mutex)
{
    if(xSemaphoreGive(mutex) == pdTRUE) {
        return MUTEX_OP_SUCCESSFUL;
    } else {
        return MUTEX_OP_UNSUCCESSFUL;
    }
}
void platform_delete_mutex(MutexHandle_t mutex)
{
    return vSemaphoreDelete(mutex);
}