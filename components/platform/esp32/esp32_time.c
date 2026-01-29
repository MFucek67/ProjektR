/**
 * @file esp32_time.c
 * @author Marko Fuček
 * @brief ESP32 implementacija platform_time API-ja.
 * 
 * Ovaj modul implementira funkcije za dohvaćanje broja tickova i milisekundi od početka
 * rada procesora za ESP-IDF framework.
 * 
 * @version 0.1
 * @date 2026-01-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "platform/platform_time.h"

uint32_t platform_getNumOfTicks(void)
{
    return xTaskGetTickCount();
}

uint32_t platform_getNumOfMs(void)
{
    return  pdMS_TO_TICKS(xTaskGetTickCount());
}