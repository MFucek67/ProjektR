/**
 * @file platform_mutex.h
 * @author Marko Fuček
 * @brief Platform API za mutex.
 * 
 * Ovaj modul pruža funkcije za upravljanje mutex mehanizama za sinkronizaciju taskova.
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
 * @typedef MutexHandle_t
 * @brief Pokazivač (handle) na mutex objekt.
 * 
 */
typedef void* MutexHandle_t;

/**
 * @enum MutexOperationStatus
 * @brief Status operacije nad mutexom.
 * 
 */
typedef enum {
    MUTEX_OP_SUCCESSFUL, /**< Operacija nad mutexom uspješna */
    MUTEX_OP_UNSUCCESSFUL /**< Operacija nad mutexom neuspješna */
} MutexOperationStatus;

/**
 * @brief Stvara mutex objekt.
 * 
 * @return Pokazivač na mutex ili NULL (kod neuspješnog stvaranja)
 */
MutexHandle_t platform_create_mutex(void);

/**
 * @brief Zauzima mutex.
 * 
 * @param mutex Pokazivač na mutex
 * @param timeout Vrijeme čekanja na mutex u ms
 * @return Status operacije
 */
MutexOperationStatus platform_lock_mutex(MutexHandle_t mutex, uint32_t timeout);

/**
 * @brief Oslobađa mutex.
 * 
 * @param mutex Pokazivač na mutex
 * @return Status operacije
 */
MutexOperationStatus platform_unlock_mutex(MutexHandle_t mutex);

/**
 * @brief Briše mutex i oslobađa resurse.
 * 
 * @param mutex Pokazivač na mutex
 */
void platform_delete_mutex(MutexHandle_t mutex);