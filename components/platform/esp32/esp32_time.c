#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

uint32_t platform_getNumOfTicks(void)
{
    return xTaskGetTickCount();
}