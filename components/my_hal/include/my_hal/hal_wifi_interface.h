/**
 * @file hal_wifi_interface.h
 * @author Marko Fuček
 * @brief HAL sučelje koje pruža platform sloju callback za WiFi evente.
 * @version 0.1
 * @date 2026-04-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "stdio.h"
#include "stdint.h"
#include "platform/wifi_client.h"

/**
 * @brief Callback na HAL funkciju koju platform sloj poziva kada primi WiFi event da ga proslijedi HAL-u.
 * 
 */
typedef void (*on_wifi_event)(WiFiState);