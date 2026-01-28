#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "platform/platform_task.h"

task_handler platform_create_task(TaskConfig_t* taskConfig)
{
    TaskHandle_t task_handle = NULL;
    xTaskCreate(taskConfig->task_function, taskConfig->task_name, taskConfig->task_stack,
         taskConfig->task_parameters, taskConfig->task_priority, &task_handle);
    return (task_handler)task_handle;
}

void platform_delete_task(task_handler task_handler)
{
    vTaskDelete((TaskHandle_t)task_handler);
}

void platform_delay_task(uint32_t ms_to_delay)
{
    vTaskDelay(pdMS_TO_TICKS(ms_to_delay));
}

void platform_set_task_priority(task_handler task_handler, uint32_t priority)
{
    vTaskPrioritySet((TaskHandle_t)task_handler, priority);
}

uint32_t platform_get_task_priority()
{
    return uxTaskPriorityGet(xTaskGetCurrentTaskHandle());
}