/**
 * @file platform_queue.h
 * @author Marko Fuček
 * @brief Platform API za upravljanje queue-om.
 * 
 * Ovaj modul pruža funkcije za stvaranje queue-a, spremanje i čitanje podataka iz queue-a i
 * brisanje queue-a na platform sloju.
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
 * @enum QueueOperationStatus
 * @brief Statusi operacija nad queue-om.
 * 
 */
typedef enum {
    QUEUE_OK, /**< Operacija uspješna */
    QUEUE_TIMEOUT, /**< Istek vremena čekanja */
    QUEUE_ERROR, /**< Operacija neuspješna */
    QUEUE_FULL, /**< Queue je pun */
    QUEUE_EMPTY /**< Queue je prazan */
} QueueOperationStatus;

/**
 * @struct QueueElement_t
 * @brief Element koji se sprema u queue.
 * 
 */
typedef struct {
    uint8_t* data; /**< Pokazivač na podatke */
    size_t len; /**< Duljina poslanih podataka u bajtovima */
} QueueElement_t;

/**
 * @typedef QueueHandle_t
 * @brief Pokazivač (handle) na queue.
 * 
 */
typedef void* QueueHandle_t;

/**
 * @brief Kreira queue.
 * 
 * @param queue_length Maksimalan broj elemenata queue-a koje može spremiti
 * @param element_size Veličina jednog queue elementa u bajtovima
 * @return Pokazivač na queue ili NULL (kod neuspješnog stvaranja)
 */
QueueHandle_t platform_queue_create(size_t queue_length, size_t element_size);

/**
 * @brief Šalje element u queue.
 * 
 * @param queue Pokazivač na queue
 * @param item Pokazivač na queue element koji se šalje
 * @param timeout Vrijeme čekanja u ms
 * @return Status operacije
 */
QueueOperationStatus platform_queue_send(QueueHandle_t queue, QueueElement_t* item, uint32_t timeout_in_ms);

/**
 * @brief Dohvaća element iz queue.
 * 
 * @param queue Pokazivač na queue
 * @param buffer Pokazivač na strukturu u koju se sprema dohvaćeni element
 * @param timeout_in_ms Vrijeme čekanja u ms
 * @return Status operacije
 */
QueueOperationStatus platform_queue_get(QueueHandle_t queue, QueueElement_t* buffer, uint32_t timeout_in_ms);

/**
 * @brief Briše queue i oslobađa resurse.
 * 
 * @param queue Pokazivač na queue
 */
void platform_queue_delete(QueueHandle_t queue);

/**
 * @brief Vraća trenutni broj elemenata spremljenih u queue.
 * 
 * @param queue Pokazivač na queue
 * @return Broj elemenata u queue-u
 */
uint32_t platform_get_num_of_queue_elements(QueueHandle_t queue);