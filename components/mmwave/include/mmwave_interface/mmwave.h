/**
 * @file mmwave.h
 * @author Marko Fuček
 * @brief Javni API mmWave core parsera i frame buildera.
 * 
 * Ovaj modul implementira:
 * Parsiranje dolaznih podataka u frame-ove i spremanje semantički korisnih podataka frame-a u queue.
 * Izgradnju frame-ova koji se šalju na TX.
 * Osnovni state parsera.
 * 
 * Modul ne upravlja memorijom (za to koristi HAL callbackove), ne zna za UART, taskove niti FreeRTOS,
 * te ne komunicira izravno s aplikacijskim slojem.
 * 
 * @note Modul se koristi isključivo preko HAL sloja.
 * 
 * @version 0.1
 * @date 2026-01-22
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "mmwave_interface/mmwave_core_types.h"
#include "mmwave_interface/mmwave_core_interface.h"

/**
 * @brief Gradi kompletan mmWave frame za slanje.
 * 
 * @note Memorija za frame se alocira preko HAL callbacka.
 * 
 * @param payload Pokazivač na payload podatke
 * @param payload_len Duljina payloada u bajtovima
 * @param ctrl_w Control word
 * @param cmd_w Command word
 * @return Pokazivač na strukturu stvorenog okvira ili NULL
 */
bool mmwave_build_frame(mmWaveFrameForTX* out,
    const uint8_t* payload, size_t payload_len, const uint8_t ctrl_w, const uint8_t cmd_w);

/**
 * @brief Parsira ulazne RX bajtove.
 * 
 * Funkcija obrađuje ulazne RX bajtove, bajt po bajt, i interno gradi frame.
 * Kada se validan frame prepozna, alocira se memorija za njegove semantički korisne
 * podatke (preko alloc_mem HAL callbacka), te se poziva mmwave_save_frame callback iz
 * HAL sloja za spremanje tih podataka.
 * 
 * @param data Pokazivač na ulazne bajtove
 * @param data_len Duljina ulaznih bajtova
 * @return Status parsiranja ulaznih podataka
 */
mmwave_frame_status_t mmwave_parse_data(const uint8_t* data, size_t data_len);

/**
 * @brief Inicijalizira i resetira mmWave parser.
 * 
 * Briše i postavlja u početno stanje interne buffere, ponovno postavlja interne varijable
 * i resetira parser state.
 * 
 * @return Status operacije nad modulom
 */
mmwave_status_t mmwave_core_init(void);

/**
 * @brief Zaustavlja rad mmWave core modula.
 * 
 * Oslobađa interne resurse, invalidira interne varijable i efektivno gasi parser.
 * 
 * @return Status operacije nad modulom
 */
mmwave_status_t mmwave_core_stop(void);