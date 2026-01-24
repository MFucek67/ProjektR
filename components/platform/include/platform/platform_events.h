/**
 * @file platform_events.h
 * @author Marko Fuček
 * @brief Event queue API platform sloja
 * 
 * Ovaj modul koristi se za event queue mehanizam.
 * 
 * Svrha mu je pružiti komunikaciju između ISR-a, drivera i viših slojeva.
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
#include "platform_queue.h"

/**
 * @enum EventStatus
 * @brief Statusi operacija nad event queue-om.
 * 
 */
typedef enum {
    PLATFORM_EVENT_OK, /**< Operacija uspješna */
    PLATFORM_EVENT_TIMEOUT, /**< Istek vremena kod izvođenja operacije */
    PLATFORM_EVENT_ERROR /**< Operacija neuspješna */
} EventStatus;

/**
 * @enum PlatformEvent_type
 * @brief Tipovi događaja koje platform event može prikazati.
 * 
 */
typedef enum {
    PLATFORM_EVENT_NONE, /**< Nema događaja */
    PLATFORM_EVENT_RX_DATA, /**< Primljeni podaci */
    PLATFORM_EVENT_TX_DONE, /**< Završeno slanje podataka */
    PLATFORM_EVENT_ERR, /**< Greška */
    PLATFORM_EVENT_FIFO_OVF, /**< Overflow event queue-a */
    PLATFORM_EVENT_BUFFER_FULL /**< Ring buffer popunjen */
} PlatformEvent_type;

/**
 * @typedef PlatformEvent_t
 * @brief Pokazivač (handle) na platform event queue.
 * 
 */
typedef QueueHandle_t PlatformEventHandle_t;

/**
 * @enum PlatformEvent_t
 * @brief Struktura koja opisuje jedan event.
 * 
 */
typedef struct {
    PlatformEvent_type type; /**< Tip događaja */
    void* data; /**< Pokazivač na podatke koje event prenosi (opcionalno - ne mora prenositi podatke) */
    size_t len; /**< Duljina podataka poslanih preko eventa (ako nema podataka poslanih, = 0) */
} PlatformEvent_t;

/**
 * @brief Kreira event queue.
 * 
 * @param size Maksimalan broj eventova koji stane u queue
 * @return Pokazivač na queue ili NULL (kod neuspješnog stvaranja)
 */
PlatformEventHandle_t platform_create_event_queue(size_t max_size);

/**
 * @brief Čekanje na event iz event queue (blokirajuće).
 * 
 * @param event_queue Pokazivač na queue
 * @param event Pokazivač na strukturu u koju se pohranjuje event koji se pročita
 * @param timeout Vrijeme čekanja u ms
 * @return Status operacije
 */
EventStatus platform_event_wait(PlatformEventHandle_t event_queue, PlatformEvent_t* event, uint32_t timeout_in_ms);

/**
 * @brief Slanje eventa u queue (blokirajuće).
 * 
 * @param event_queue Pokazivač na queue
 * @param event Pokazivač na događaj koji se sprema u queue
 * @param timeout Vrijeme čekanja u ms
 * @return Status operacije
 */
EventStatus platform_event_post(PlatformEventHandle_t event_queue, PlatformEvent_t* event, uint32_t timeout_in_ms);

/**
 * @brief Šalje event u event queue iz ISR (Interrupt Service Routine) (neblokirajuće).
 * 
 * @param event_queue Pokazivač na queue
 * @param event Pokazivač na događaj koji se sprema u queue
 * @return Status operacije
 */
EventStatus platform_event_post_from_ISR(PlatformEventHandle_t event_queue, PlatformEvent_t* event);

/**
 * @brief Briše event queue i oslobađa resurse.
 * 
 * @param event_queue Pokazivač na queue
 */
void platform_event_queue_delete(PlatformEventHandle_t event_queue);

/**
 * @brief Resetira sadržaj event queue (postaje prazna).
 * 
 * @param event_queue Pokazivač na queue
 */
void platform_event_queue_reset(PlatformEventHandle_t event_queue);