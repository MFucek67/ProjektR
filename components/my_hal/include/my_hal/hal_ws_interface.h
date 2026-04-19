/**
 * @file hal_ws_interface.h
 * @author Marko Fuček
 * @brief HAL sučelje koje pruža platform sloju callbackove na funkcije ovisno o Web Socket eventima.
 * @version 0.1
 * @date 2026-04-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "stdio.h"
#include "stdint.h"
#include "platform/websocket.h"

/**
 * @brief Pokazivač na funkciju koja se zove kada Web Socket primi podatke.
 * 
 * @param data Pokazivač na array charactera koji predstavljaju primljene podatke
 * @param len Duljina arraya primljenih podataka
 */
typedef void (*on_ws_data)(const char* data, size_t len);

/**
 * @brief Pokazivač na funkciju koja se zove kada Web Socket doživi error.
 * 
 */
typedef void (*on_ws_error)(void);

/**
 * @brief Pokazivač na funkciju koja se zove kad Web Socket starta/stopa.
 * 
 * @param started true - Web Socket startao, false - Web Skocket stoppao
 */
typedef void (*on_ws_start_stop)(bool started);