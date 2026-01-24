/**
 * @file app_mmwave_hal_config.h
 * @author Marko Fuček
 * @brief Konfiguracija HAL-a i mmWave core interface-a za aplikacijski sloj.
 * 
 * Ovaj modul pruža konfiguracijske objekte potrebne aplikacijskom manager sloju mmWave modula da
 * inicijalizira i pokrene HAL sloj i mmWave core logiku.
 * 
 * Aplikacijski sloj ovdje djeluje kao poveznica između HAL sloja i mmWave core sloja,
 * bez da direktno upravlja njihovom implementacijom.
 *  
 * @version 0.1
 * @date 2026-01-23
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "HAL/hal_mmwave.h"
#include "mmwave_interface/mmwave_core_interface.h"

/**
 * @brief Dohvaća konfiguraciju HAL sloja.
 * 
 * Vlasništvo nad konfiguracijom je u aplication sloju, a pozivatelj ju ne smije mijenjati.
 * 
 * @return Pokazivač na const HAL konfiguraciju
 */
const hal_mmwave_config* app_mmwave_get_hal_config(void);

/**
 * @brief Dohvaća mmWave core interface.
 * 
 * Sučelje (interface) je definirano i povezano u aplication sloju te je njegovo vlasništvo
 * i pozivatelj ga ne smije mijenjati.
 * 
 * @return Pokazivač na const mmWave core interface
 */
const mmWave_core_interface* app_mmwave_get_core_interface(void);