/**
 * @file app_network.h
 * @author Marko Fuček
 * @brief Krovni API aplikacijskog sloja za upravljanje povezivanjem na mrežu, upravljanje konfiguracijom mreže
 * i prekidanjem povezanosti na mrežu.
 * 
 * Ovaj modul pruža funkcije za inicijalizaciju mreže (konfiguracija kompletnog sustava), pokretanje spajanja na mrežu,
 * prekidanje konekcije te deinicijalizaciju mreže i uništavanje objekata koji čine mrežu i klijenta u njoj.
 * 
 * @version 0.1
 * @date 2026-04-10
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "app/app_network_packet_serializer.h"
#include "app/app_types.h"
#include "my_hal/hal_network_interface.h"

/**
 * @typedef app_network_config
 * @brief Definira tip konfiguracije mreže na aplikacijskom sloju.
 * 
 */
typedef hal_network_config app_network_config;

/**
 * @brief Mapira konfiguraciju klijenta koja se kasnije koristi za spajanje na mrežu i inicijalizira network
 * sustav klijenta.
 * 
 * @param conf Konfiguracija mreže na aplikacijskom sloju.
 * @return Status operacije nad mrežnim driverom.
 */
AppNetworkStatus network_init(app_network_config conf);

/**
 * @brief Pokreće kompletni network sustav klijenta.
 * 
 * @return Status operacije nad mrežnim driverom.
 */
AppNetworkStatus network_start(void);

/**
 * @brief Zaustavlja rad drivera network sustava klijenta.
 * 
 * @return Status operacije nad mrežnim driverom.
 */
AppNetworkStatus network_stop(void);

/**
 * @brief Deinicijalizira network sustav klijenta.
 * 
 * @return Status operacije nad mrežnim driverom. 
 */
AppNetworkStatus network_uninit(void);

/**
 * @brief Daje statistiku reportova i responseova poslanih preko mreže.
 * 
 * @param reports Pokazivač na broj poslanih reportova
 * @param responses Pokazivač na broj poslanih responseova
 */
void sent_via_network_statistics(uint32_t* reports, uint32_t* responses);