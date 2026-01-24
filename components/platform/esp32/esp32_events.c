/**
 * @file esp32_events.c
 * @author Marko Fuček
 * @brief ESP32 implementacija platform generic event API-ja.
 * 
 * Ovaj modul implementira generički platform event API koji se bazira na FreeRTOS queue.
 * 
 * Modul ukljućuje:
 * Stvaranje i brisanje queue-ova za spremanje eventova.
 * Blokirajuće i neblokirajuće čeknje na event.
 * Slanje eventa u queue iz taska i ISR konteksta.
 * Resetiranje event queue.
 * 
 * @version 0.1
 * @date 2026-01-21
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "platform/platform_events.h"

PlatformEventHandle_t platform_create_event_queue(size_t max_size)
{
    if(max_size <= 0) {
        return NULL;
    } else {
        return xQueueCreate(max_size, sizeof(PlatformEvent_t));
    }
}

/**
 * @note Funkcija blokira task koji ju poziva sve dok se u queue ne pojavi novi event ili zadani
 * timeout ne istekne.
 *  
 */
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

/**
 * @note Funkcija pokušava poslati event u queue i blokira do isteka zadanog timeouta ako ne uspije.
 * 
 */
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

/**
 * @note Funkcija koristi FreeRTOS ISR API za slanje eventa u queue izravno iz ISR konteksta,
 * bez blokiranja. Po potrebi (ako je higher_woken task pdTRUE) funkcija budi task većeg prioriteta, ako je
 * on bio u redu čekanja na taj queue. Iako je on probuđen, tad je stavljen u listu taskova koji čekaju na
 * procesorsko vrijeme. On mora odmah krenuti s izvršavanjem, te preko schedulera tom tasku dajemo procesorsko vrijeme.
 * 
 */
EventStatus platform_event_post_from_ISR(PlatformEventHandle_t event_queue, PlatformEvent_t* event)
{
    if(event_queue == NULL || event == NULL) {
        return PLATFORM_EVENT_ERROR;
    }

    //Ovo je samo flag koji označava jesmo li probudili task većeg prioriteta (onaj koji blokirajuće čeka na event u event_queue)
    BaseType_t higher_woken = pdFALSE;

    if(xQueueSendFromISR(event_queue, event, &higher_woken) != pdTRUE) {
        return PLATFORM_EVENT_ERROR;
    }
    portYIELD_FROM_ISR(higher_woken);
    return PLATFORM_EVENT_OK;
}

/**
 * @note Funkcija briše FreeRTOS queue.
 * @warning Pozivatelj mora garantirati da niti jedan task više ne čeka na queue,
 * te da niti jedan ISR više ne šalje evente u queue.
 * 
 */
void platform_event_queue_delete(PlatformEventHandle_t event_queue)
{
    if(event_queue != NULL) {
        vQueueDelete(event_queue);
    }
}

void platform_event_queue_reset(PlatformEventHandle_t event_queue)
{
    xQueueReset(event_queue);
}