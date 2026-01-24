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