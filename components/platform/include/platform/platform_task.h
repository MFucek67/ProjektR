#pragma once
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"

typedef void (*task_function)(void*);
//definiramo tip podatka pokazivač na void funkciju (naziv tipa task_funcktion)
//koji prima generički argument (bilo koji tip, ali samo jedan podatak)

typedef void* task_handler;

typedef struct {
    task_function task_function;
    const char* task_name;
    uint32_t task_stack;
    void* task_parameters;
    uint32_t task_priority;
} TaskConfig_t;

task_handler platform_create_task(TaskConfig_t* taskConfig);
void platform_delete_task(task_handler task_handler);
void platform_delay_task(uint32_t ms_to_delay);
void platform_set_task_priority(task_handler task_handler, uint32_t priority);
uint32_t platform_get_task_priority();

//Stvaranje taskova i upravljanje njima