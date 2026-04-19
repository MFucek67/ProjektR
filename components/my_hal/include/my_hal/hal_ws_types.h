/**
 * @file hal_ws_types.h
 * @author Marko Fuček
 * @brief Definicije tipova i konstanti za Web Socket HAL sloj.
 * 
 * Ovaj header definira osnovne tipove podataka, enumeracije i konfiguracijske strukture
 * koje Web Socket HAL sloj koristi.
 * 
 * Svrha mu je pružiti tipove Web Socket podataka koji su jedinstveni za HAL sloj i tako povećati
 * modularnost i apstrakciju samog Web Socketa.
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
#include "board.h"
#include "platform/platform.h"

/**
 * @enum HalWebSocketStatus
 * @brief Status operacije nad Web Socketom na HAL sloju.
 * 
 */
typedef enum {
    HAL_WS_OK, /**< Operacija uspješna */
    HAL_WS_INVALID_STATE, /**< Funkcija pozvana iz krivog stanja */
    HAL_WS_ERROR, /**< Operacija neuspješna */
    HAL_WS_TIMEOUT /**< Isteklo je vrijeme čekanja */
} HalWebSocketStatus;

/**
 * @enum HalWebSocketState
 * @brief Stanje u kojem se nalazi HAL Web Socket modul.
 * 
 */
typedef enum {
    HAL_WS_UNINIT, /**< Modul nije inicijaliziran */
    HAL_WS_INIT, /**< Modul je inicijaliziran */
    HAL_WS_RUNNING, /**< Modul je započeo s radom */
    HAL_WS_STOPPED, /**< Modul je prestao s radom */
    HAL_WS_ERRORED /**< Greška u radu modula */
} HalWebSocketState;

/**
 * @typedef hal_ws_config
 * @brief Tip za HAL Web Socket konfiguracijsku strukturu. 
 * 
 */
typedef ws_config hal_ws_config;