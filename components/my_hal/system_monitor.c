/**
 * @file system_monitor.c
 * @author Marko Fuček
 * @brief Implementacija monitornig sučelja.
 * 
 * Implementira funkcije koje vraćaju podatke o iskorištenju računalnih resursa određenih taskova, funkcije za registraciju i
 * deregistraciju taskova u monitoring sučelje, te funkciju koja vraća broj registriranih taskova u sučelju.
 * 
 * @version 0.1
 * @date 2026-04-10
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "stdio.h"
#include "stdint.h"
#include "my_hal/system_monitor.h"
#include "platform/platform_task.h"
#include "platform/platform_memory.h"

static monitor_task_t monitor_task_list[MAX_TASKS];
static monitor_task_stats_t task_stats_list[MAX_TASKS];
static int registered_tasks = 0;

void system_monitor_register_task(const char* name, void* handle)
{
    if(registered_tasks == MAX_TASKS) {
        return;
    }

    monitor_task_t task = {
        .name = name,
        .task_handle = handle
    };
    monitor_task_list[registered_tasks] = task;
    registered_tasks++;
}

monitor_task_stats_t* get_all_tasks_stats(void)
{
    for(int i = 0; i < registered_tasks; i++) {
        monitor_task_stats_t task = {
            .name = monitor_task_list[i].name,
            .remaining_stack = platform_get_remaining_stack((task_handler) monitor_task_list[i].task_handle)
        };
        task_stats_list[i] = task;
    }
    return task_stats_list;
}

void system_monitor_unregister_task(void* handle)
{
    for(int i = 0; i < registered_tasks; i++) {
        if(monitor_task_list[i].task_handle == (task_handler)handle) {
            for(int j = i; j < (registered_tasks - 1); j++) {
                monitor_task_list[j] = monitor_task_list[j + 1];
            }
            registered_tasks--;
            break;
        }
    }
}

int system_monitor_task_count(void)
{
    return registered_tasks;
}