/**
 * @file hal_ws.h
 * @author Marko Fuček
 * @brief HAL API za upravljanje Web Socketom.
 * 
 * Implementira wrappere za inicijalizaciju, start, stop, deinicijalizaciju i slanje paketa preko
 * Web Socketa.
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
#include "my_hal/hal_ws_types.h"
#include "my_hal/hal_ws_interface.h"

/**
 * @brief Wrapper funkcija za slanje paketa preko Web Socketa.
 * 
 * @param packet Pokazivač na niz bajtova koji čine podatak koji se šalje
 * @param packet_len Duljina podatka koji se šalje u bajtovima
 */
void hal_send_packet(uint8_t* packet, size_t packet_len);

/**
 * @brief Wrapper funkcija za inicijalizaciju Web Socketa.
 * 
 * @param configuration Konfiguracija Web Socket konekcije
 * @return HAL status operacije nad Web Socketom 
 */
HalWebSocketStatus hal_ws_init(hal_ws_config* configuration);

/**
 * @brief Wrapper funkcija za pokretanje Web Socket konekcije.
 * 
 * @return HAL status operacije nad Web Socketom
 */
HalWebSocketStatus hal_ws_start(void);

/**
 * @brief Wrapper funkcija za zaustavljanje Web Socket konekcije.
 * 
 * @return HAL status operacije nad Web Socketom
 */
HalWebSocketStatus hal_ws_stop(void);

/**
 * @brief Wrapper funkcija za deinicijalizaciju Web Socketa.
 * 
 * @return HAL status operacije nad Web Socketom 
 */
HalWebSocketStatus hal_ws_deinit(void);