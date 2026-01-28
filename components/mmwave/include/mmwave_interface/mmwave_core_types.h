#pragma once
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "my_hal/hal_mmwave_types.h"

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
 * @typedef mmWaveFrame
 * @brief Struktura koja predstavlja cijeli mmWave frame.
 * 
 * Koristi se kod TX taska.
 * 
 */
typedef FrameData_t mmWaveFrameForTX;

/**
 * @typedef mmWaveFrameData
 * @brief Struktura koja se sastoji od samo semantički korisnih podataka frame-a.
 * 
 * Semantički korisni podatci su oni koje aplication sloj koristi:
 * -data[0] = control word
 * -data[1] = command word
 * -data[2, ...] = payload
 * -data_len = payload_len + 2
 * 
 */
typedef FrameData_t mmWaveFrameSemanticData;