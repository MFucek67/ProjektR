/**
 * @file platform_time.h
 * @author Marko Fuček
 * @brief Platform API za dobivanje podataka o vremenu izvođenja.
 * 
 * Ovaj modul pruža API za dohat broja sistemskih tickova i mjerenje
 * vremena izvođenja taskova.
 * 
 * @version 0.1
 * @date 2026-01-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"

/**
 * @brief Dohvaća broj tickova procesora od pokretanja taska.
 * 
 * @return Broj tickova (uint32_t)
 */
uint32_t platform_getNumOfTicks(void);

/**
 * @brief Dohvaća broj milisekundi procesora od pokretanja taska.
 * 
 * @return Broj milisekundi (uint32_t)
 */
uint32_t platform_getNumOfMs(void);