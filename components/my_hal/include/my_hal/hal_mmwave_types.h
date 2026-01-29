/**
 * @file hal_mmwave_types.h
 * @author Marko Fuček
 * @brief Definicije tipova i konstanti za mmWave HAL sloj.
 * 
 * Ovaj header definira osnovne tipove podataka, enumeracija i konfiguracijske strukture
 * koje HAL sloj koristi.
 * 
 * Svrha ovog modula je objediniti sve tipove koji se koriste na HAL sloju te tako olakšati
 * portabilnost i održavanje koda i povećatai apstrakciju istog.
 * 
 * @version 0.1
 * @date 2026-01-29
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
 * @brief Maksimalan broj frame-ova u internom queue-u.
 * 
 */
#define MAX_FRAMES_IN_QUEUE 40

/**
 * @typedef HalEventHandle_t
 * @brief Tip handle-a (pokazivača) za HAL evente.
 * 
 * HAL interno koristi platform evente, ali ima svoje tipove handlera
 * zbog čitljivosti i apstrakcije.
 * 
 */
typedef PlatformEventHandle_t HalEventHandle_t;

/**
 * @struct hal_mmwave_config
 * @brief Konfiguracijska struktura za za mmWave HAL modul.
 * 
 * Struktura sadrži sve važne parametre potrebne za UART komunikaciju i
 * interni event sustav.
 * 
 * HAL ovu strukutru prilagođava i prosljeđuje platform sloju.
 * 
 */
typedef struct {
    BoardUartId id; /**< logički UART id */
    uint32_t baudrate; /**< Baud rate UART-a */
    uint8_t data_bits; /**< Broj podatkovnih bitova */
    uint8_t parity; /**< Paritet */
    uint8_t stop_bits; /**< Broj stop bitova */
    size_t rx_buff_size; /**< Veličina RX buffera */
    size_t tx_buff_size; /**< Veličina TX buffera */
    size_t rx_thresh; /**< RX threshold za generiranje UART_DATA eventa */
    size_t event_queue_len; /**< Veličina internog event queue */
} hal_mmwave_config;

/**
 * @enum HalMmwaveStatus
 * @brief Povratni statusi operacija nad HAL-om.
 * 
 */
typedef enum {
    HAL_MMWAVE_OK, /**< Operacija uspješna */
    HAL_MMWAVE_INVALID_STATE, /**< Funkcija pozvana iz krivog stanja */
    HAL_ERROR, /**< Operacija neuspješna */
    HAL_MMWAVE_TIMEOUT /**< Isteklo je vrijeme čekanja */
} HalMmwaveStatus;

/**
 * @enum HalMmwaveState
 * @brief Interna stanja mmWave HAL modula.
 * 
 */
typedef enum {
    HAL_MMWAVE_UNINIT, /**< Modul nije inicijaliziran */
    HAL_MMWAVE_INIT, /**< Modul je inicijaliziran */
    HAL_MMWAVE_RUNNING, /**< Modul je započeo s radom */
    HAL_MMWAVE_STOPPED, /**< Modul je prestao s radom */
    HAL_MMWAVE_ERROR /**< Greška u radu modula */
} HalMmwaveState;

/**
 * @typedef FrameData_t
 * @brief Tip podatka za pohranu mmWave frame-ova u queue na HAL sloju.
 * 
 */
typedef QueueElement_t FrameData_t;
