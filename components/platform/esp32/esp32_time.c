#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

uint32_t platform_getNumOfTicks(void)
{
    return xTaskGetTickCount();
}

uint32_t platform_getNumOfMs(void)
{
    return  pdMS_TO_TICKS(xTaskGetTickCount());
}