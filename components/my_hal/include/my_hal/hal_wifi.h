/**
 * @file hal_wifi.h
 * @author Marko Fuček
 * @brief HAL modul za upravljanje WiFi konekcijom i driverom.
 * 
 * Ovaj modul uvodi wrapper funkcije za WiFi funkcije s platform sloja na HAL sloj.
 * 
 * @version 0.1
 * @date 2026-04-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "stdio.h"
#include "stdint.h"
#include "my_hal/hal_wifi_types.h"
#include "my_hal/hal_network_interface.h"

/**
 * @brief Inicijalizira WiFi driver s HAL sloja.
 * 
 * @param configuration Konfiguracija WiFi konekcije
 * @return Status operacije nad WiFi driverom
 */
HalWifiStatus hal_wifi_init(hal_wifi_config* configuration);

/**
 * @brief Pokreće WiFi driver s HAL sloja.
 * 
 * @return Status operacije nad WiFi driverom
 */
HalWifiStatus hal_wifi_start(void);

/**
 * @brief Zaustavlja rad WiFi drivera s HAL sloja.
 * 
 * @return Status operacije nad WiFi driverom
 */
HalWifiStatus hal_wifi_stop(void);

/**
 * @brief Deinicijalizira WiFi driver s HAL sloja.
 * 
 * @return Status operacije nad WiFi driverom
 */
HalWifiStatus hal_wifi_deinit(void);