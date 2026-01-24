/**
 * @file platform_task.h
 * @author Marko Fuček
 * @brief Platform API za upravljanje taskovima.
 * 
 * Ovaj modul omogućuje stvaranje taskova, njihovo brisanje i upravljanje istima na platform sloju.
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
 * @typedef task_function
 * @brief Tip pokazivač na funkciju koju task izvršava.
 * 
 */
typedef void (*task_function)(void*);

/**
 * @typedef task_handler
 * @brief Tip pokazivač (handler) na task.
 * 
 */
typedef void* task_handler;

/**
 * @struct TaskConfig_t
 * @brief Konfiguracija taska.
 * 
 * Sadrži sve potrebne parametre za stvaranje taska.
 * 
 */
typedef struct {
    task_function task_function;
    const char* task_name;
    uint32_t task_stack;
    void* task_parameters;
    uint32_t task_priority;
} TaskConfig_t;

/**
 * @brief Kreira novi task.
 * 
 * @param taskConfig Pokazivač na konfiguraciju taska
 * @return Pokazivač na task ili NULL (kod neuspješnog stvaranja)
 */
task_handler platform_create_task(TaskConfig_t* taskConfig);

/**
 * @brief Briše task.
 * 
 * @param task_handler Pokazivač na task
 */
void platform_delete_task(task_handler task_handler);

/**
 * @brief Pauzira izvršavanje pozivajućeg taska.
 * 
 * @param ms_to_delay Vrijeme u ms na koje se task pauzira
 */
void platform_delay_task(uint32_t ms_to_delay);

/**
 * @brief Postavlja prioritet pozivajućeg taska.
 * 
 * @param task_handler Pokazivač na task
 * @param priority Prioritet
 */
void platform_set_task_priority(task_handler task_handler, uint32_t priority);

/**
 * @brief Dohvaća prioritet pozivajućeg taska.
 * 
 * @return Broj pozivajućeg taska
 */
uint32_t platform_get_task_priority();