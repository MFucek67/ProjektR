#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "../include/platform/platform_events.h"

PlatformEventHandle_t platform_create_event_queue(size_t size)
{
    if(size <= 0) {
        return NULL;
    } else {
        return xQueueCreate(size, sizeof(PlatformEvent_t));
    }
}

EventStatus platform_event_wait(PlatformEventHandle_t event_queue, PlatformEvent_t* event, uint32_t timeout_in_ms)
{
    if(event_queue == NULL || event == NULL) {
        return PLATFORM_EVENT_ERROR;
    }

    if(xQueueReceive(event_queue, event, pdMS_TO_TICKS(timeout_in_ms)) == pdTRUE) {
        return PLATFORM_EVENT_OK;
    }
    return PLATFORM_EVENT_TIMEOUT;
}

EventStatus platform_event_post(PlatformEventHandle_t event_queue, PlatformEvent_t* event, uint32_t timeout_in_ms)
{
    if(event_queue == NULL || event == NULL) {
        return PLATFORM_EVENT_ERROR;
    }

    if(xQueueSend(event_queue, event, pdMS_TO_TICKS(timeout_in_ms)) == pdTRUE) {
        return PLATFORM_EVENT_OK;
    }
    return PLATFORM_EVENT_TIMEOUT;
}

EventStatus platform_event_post_from_ISR(PlatformEventHandle_t event_queue, PlatformEvent_t* event)
{
    if(event_queue == NULL || event == NULL) {
        return PLATFORM_EVENT_ERROR;
    }

    BaseType_t higher_woken = pdFALSE;
    //Ovo je samo flag koji označava jesmo li probudili task većeg prioriteta (onaj koji blokirajuće čeka na event u event_queue)

    if(xQueueSendFromISR(event_queue, event, &higher_woken) != pdTRUE) {
        /*Scheduler automatski provjerava koji taskovi su blokirani u redu taskova queue-a event_queue i ako ima neki većeg
        prioriteta od taska koji se trenutno izvodi budi ga, tj. stavlja u pripravne taskove, te postavlja higher_woken flag
        u pdTRUE!*/
        return PLATFORM_EVENT_ERROR;
    }
    /*E sad, iako je onaj task većeg prioriteta probuđen, on nije odmah dobio CPU time, što mi ne želimo, jer event može biti
    jako bitan i želimo da se odmah krene izvršavati potrebna radnja oko eventa - zato pokrećemo funkciju portYIELD_FROM_ISR koja
    ako dobije pdTRUE pokreće scheduler i tasku većeg prioriteta daje CPU time!*/
    portYIELD_FROM_ISR(higher_woken);
    return PLATFORM_EVENT_OK;
}

void platform_event_queue_delete(PlatformEventHandle_t event_queue)
{
    if(event_queue != NULL) {
        vQueueDelete(event_queue);
    }
    //Moram garantirati da nitko više ne čeka u redu tog queue i da ISR neće više slati u njega
    //To rješavam u HAL layeru
}

void platform_event_queue_reset(PlatformEventHandle_t event_queue)
{
    xQueueReset(event_queue);
}