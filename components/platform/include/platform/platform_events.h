#pragma once
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "platform_queue.h"

typedef enum {
    PLATFORM_EVENT_OK = 0,
    PLATFORM_EVENT_TIMEOUT = 1,
    PLATFORM_EVENT_ERROR = 2
} EventStatus;

typedef enum {
    PLATFORM_EVENT_NONE = 0,
    PLATFORM_EVENT_RX_DATA = 1,
    PLATFORM_EVENT_TX_DONE = 2,
    PLATFORM_EVENT_ERR = 3,
    PLATFORM_EVENT_FIFO_OVF = 4,
    PLATFORM_EVENT_BUFFER_FULL = 5
} PlatformEvent_type;

typedef QueueHandle_t PlatformEventHandle_t;

typedef struct {
    PlatformEvent_type type;
    void* data;
    size_t len;
} PlatformEvent_t;

PlatformEventHandle_t platform_create_event_queue(size_t size);
EventStatus platform_event_wait(PlatformEventHandle_t event_queue, PlatformEvent_t* event, uint32_t timeout);
EventStatus platform_event_post(PlatformEventHandle_t event_queue, PlatformEvent_t* event, uint32_t timeout);
EventStatus platform_event_post_from_ISR(PlatformEventHandle_t event_queue, PlatformEvent_t* event);
void platform_event_queue_delete(PlatformEventHandle_t event_queue);
void platform_event_queue_reset(PlatformEventHandle_t event_queue);