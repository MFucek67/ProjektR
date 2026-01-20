#pragma once
#include "stdio.h"
#include "stdint.h"
#include "board.h"
#include "mmwave_interface/mmwave.h"
#include "mmwave_interface/mmwave_core_interface.h"
#include "platform/platform.h"

#define MAX_FRAMES_IN_QUEUE 20

typedef PlatformEventHandle_t HalEventHandle_t;

//pravila konfiguracije - tu se wrappaju i postavljaju zvanjem platform sloj funkcija iz HAL sloja
typedef struct {
    BoardUartId id;
    uint32_t baudrate;
    uint8_t data_bits;
    uint8_t parity;
    uint8_t stop_bits;
    size_t rx_buff_size;
    size_t tx_buff_size;
    size_t rx_thresh;
    size_t event_queue_len;
} hal_mmwave_config;

typedef enum {
    HAL_MMWAVE_OK,
    HAL_MMWAVE_INVALID_STATE,
    HAL_ERROR,
    HAL_MMWAVE_TIMEOUT
} HalMmwaveStatus;

typedef enum {
    HAL_MMWAVE_UNINIT = 0,
    HAL_MMWAVE_INIT = 1,
    HAL_MMWAVE_RUNNING = 2,
    HAL_MMWAVE_STOPPED = 3,
    HAL_MMWAVE_ERROR = 4
} HalMmwaveState;

typedef QueueElement_t FrameData_t;

HalMmwaveStatus hal_mmwave_init(hal_mmwave_config* configuration, mmWave_core_interface* core_api);
HalMmwaveStatus hal_mmwave_start(void);
HalMmwaveStatus hal_mmwave_stop(void);
HalMmwaveStatus hal_mmwave_deinit(void);
HalMmwaveStatus hal_mmwave_send_frame(const uint8_t* data, size_t data_len, const uint8_t ctrl_w, const uint8_t cmd_w);
HalMmwaveStatus hal_mmwave_get_frame_from_queue(QueueElement_t* buffer, uint32_t timeout_in_ms);
void hal_mmwave_release_frame_memory(mmWaveFrameData* frame_data);