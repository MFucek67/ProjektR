/**
 * @file system_monitor.h
 * @author Marko Fuček
 * @brief Monitoring sučelje čija je uloga nadzor taskova i vraćanje statistika računalnih resursa koje taskovi koriste.
 * 
 * Ovaj header definira osnovne tipove podataka za nadziranje taskova i vraćanje podataka o
 * računalnim resursima koje zabilježeni taskovi koriste.
 * 
 * Svrha ovog modula je imati registar taskova u sustavu, koji nadzire korištenje resursa koji
 * su taskovima dani u vremenu, te nudi podatke o resursima taskova u vremenu.
 * 
 * @note Pošto osnovna zadaća taska nije registrirati sam sebe, za to, ali i deregistraciju odgovoran je
 * onaj sloj čiji je task sastavni dio, po završetku njegova životnog tijeka.
 * 
 * @version 0.1
 * @date 2026-03-14
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "stdio.h"
#include "stdint.h"

/**
 * @brief Maksimalan broj taskova koji se može pohraniti u system monitor.
 * 
 */
#define MAX_TASKS 10

/**
 * @struct monitor_task_t
 * @brief Struktura koja predstavlja jedan task u monitor registru.
 * 
 */
typedef struct {
    const char* name; /**< Naziv taska*/
    void* task_handle; /**< Pokazivač na task*/
} monitor_task_t;

/**
 * @struct monitor_task_stats_t
 * @brief Struktura koja predstavlja podatke o iskorištenim računalnim resursima određenog taska.
 * 
 */
typedef struct {
    const char* name; /**< Naziv taska*/
    uint32_t remaining_stack; /**< Količina slobodnog stacka u bajtovima za taj task*/
} monitor_task_stats_t;

/**
 * @brief Registrira task u registar.
 * 
 * @param name Naziv taska
 * @param handle Pokazivač na task
 */
void system_monitor_register_task(const char* name, void* handle);

/**
 * @brief Vraća informacije o računalnim resursima svih registriranih taskova.
 * 
 * @return Lista s monitor statsima
 */
monitor_task_stats_t* get_all_tasks_stats(void);

/**
 * @brief Deregistrira task iz registra.
 * 
 * @param handle Pokazivač na task.
 */
void system_monitor_unregister_task(void* handle);

/**
 * @brief Vraća broj koliko postoji taskova u registru.
 * 
 * @return Broj taskova u registru
 */
int system_monitor_task_count(void);