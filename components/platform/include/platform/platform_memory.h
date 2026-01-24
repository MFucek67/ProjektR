/**
 * @file platform_memory.h
 * @author Marko Fuček
 * @brief Platform API za upravljanje dinamičkom memorijom
 * 
 * Ovaj modul pruža kontrolirani pristup dinamičkoj memoriji iz viših slojeva.
 * 
 * Dopušta alokaciju do maksimalne veličine i kontrolirano otpuštanje zauzete memorije.
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
 * @brief Maksimalna dozvoljena veličina alocirane memorije (u bajtovima).
 * 
 */
#define MAX_MEMORY_SIZE 1024

/**
 * @enum MemoryOperationStatus
 * @brief Statusi operacija nad dinamičkom memorijom.
 * 
 */
typedef enum {
    MEM_OK, /**< Operacija uspješna */
    MEM_ERROR, /**< Greška */
    MEM_ERR_INVALID_PARAM /**< Poslani neispravni parametri */
} MemoryOperationStatus;

/**
 * @brief Alocira dinamičku memoriju.
 * 
 * Funkcija alocira dinamičku memoriju i provjerava valjanost poslanih parametara te
 * je li poslana veličina u granicama dopuštene.
 * 
 * @param mem_location Pokazivač na adresu u koju se sprema pokazivač na alociranu memoriju
 * @param size Veličina memorije u bajtovima
 * @return Status operacije
 */
MemoryOperationStatus platform_malloc(void** mem_location, uint32_t size);

/**
 * @brief Oslobađa dinamički zauzetu memoriju.
 * 
 * @param memory Pokazivač na memoriju koju se oslobađa
 */
void platform_free(void* memory);