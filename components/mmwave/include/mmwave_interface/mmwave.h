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
#include "mmwave_core_interface.h"

/**
 * @enum mmwave_status_t
 * @brief Status operacije nad mmWave core modulom.
 * 
 */
typedef enum {
    S_MMWAVE_OK, /**< Operacija uspješna */
    S_MMWAVE_MEMORY_PROBLEM, /**< Neuspješna alokacija memorije */
    S_MMWAVE_ERR_INVALID_PARAM, /**< Korišteni neispravni ulazni podatci */
    S_MMWAVE_ERR_TIMEOUT /**< Timeout tijekom operacije */
} mmwave_status_t;

/**
 * @enum mmwave_frame_status_t
 * @brief Status parsiranja ulaznih podataka.
 * 
 */
typedef enum {
    MMWAVE_FRAME_OK, /**< Nađen je barem jedan cijeli frame i stavljen u queue */
    MMWAVE_NO_FRAMES, /**< Nije pronađen niti jedan valjan frame (bajtovi su svi odbačeni) */
    MMWAVE_QUEUE_FULL, /**< Frame je valjan, ali queue je pun, odbacuje se okvir */
    MMWAVE_MEMORY_PROBLEM, /**< Nedovoljno memorije na heapu */
    MMWAVE_UNFINISHED_FRAME /**< Nije pronađen niti jedan cijeli frame, ali postoji jedan u izgradnji */
} mmwave_frame_status_t;

/**
 * @name Specifični dijelovi frame-a
 * @{
 */

/**
 * @brief Prvi bajt zaglavlja frame-a.
 */
#define HEADER1 0x53

/**
 * @brief Drugi bajt zaglavlja frame-a.
 */
#define HEADER2 0x59

/**
 * @brief Prvi bajt završetka frame-a.
 */
#define FOOTER1 0x54

/**
 * @brief Drugi bajt završetka frame-a.
 */
#define FOOTER2 0x43
/**
 * @}
 * 
 */

 /**
  * @brief Početna veličina internog parser buffera.
  * 
  */
#define STARTING_PARSER_BUFFER_SIZE 20

/**
 * @brief Maksimalna veličina internog parser buffera.
 * 
 * Maksimalna veličina payloada je 65535 bajtova, uz dodatne bajtove za
 * header, footer, control word, command word, checksum i bajtove koji određuju veličinu payloada. 
 * 
 */
#define MAX_PARSER_BUFFER_SIZE (65535 + 9)

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
mmWaveFrame* mmwave_build_frame(const uint8_t* payload, size_t payload_len, const uint8_t ctrl_w, const uint8_t cmd_w);

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
mmwave_status_t mmwave_init(void);

/**
 * @brief Zaustavlja rad mmWave core modula.
 * 
 * Oslobađa interne resurse, invalidira interne varijable i efektivno gasi parser.
 * 
 * @return Status operacije nad modulom
 */
mmwave_status_t mmwave_stop(void);