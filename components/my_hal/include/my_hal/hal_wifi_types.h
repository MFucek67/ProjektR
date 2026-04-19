/**
 * @file hal_wifi_types.h
 * @author Marko Fuček
 * @brief Definicije tipova i konstanti za WiFi HAL sloj.
 * 
 * Ovaj header definira osnovne tipove podataka, enumeracije i konfiguracijske strukture
 * koje WiFi HAL sloj koristi.
 * 
 * Svrha mu je pružiti tipove WiFi podataka koji su jedinstveni za HAL sloj i tako povećati
 * modularnost i apstrakciju WiFi konekcije.
 * @version 0.1
 * @date 2026-04-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "stdio.h"
#include "stdint.h"
#include "board.h"
#include "platform/platform.h"

/**
 * @enum HalWifiStatus
 * @brief Status operacije nad WiFi driverom.
 * 
 */
typedef enum {
    HAL_WIFI_OK, /**< Operacija uspješna */
    HAL_WIFI_INVALID_STATE, /**< Funkcija pozvana iz krivog stanja */
    HAL_WIFI_ERROR, /**< Operacija neuspješna */
    HAL_WIFI_TIMEOUT /**< Isteklo je vrijeme čekanja */
} HalWifiStatus;

/**
 * @enum HalWifiState
 * @brief Stanje u kojem se nalazi HAL WiFi driver.
 * 
 */
typedef enum {
    HAL_WIFI_UNINIT, /**< Modul nije inicijaliziran */
    HAL_WIFI_INIT, /**< Modul je inicijaliziran */
    HAL_WIFI_RUNNING, /**< Modul je započeo s radom */
    HAL_WIFI_STOPPED, /**< Modul je prestao s radom */
    HAL_WIFI_ERRORED /**< Greška u radu modula */
} HalWifiState;

/**
 * @typedef hal_wifi_config
 * @brief Tip za HAL WiFi konfiguracijsku strukturu mreže.
 * 
 */
typedef wifi_config hal_wifi_config;