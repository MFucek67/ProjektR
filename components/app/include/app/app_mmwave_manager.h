#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "app_mmwave_decoder.h"
#include "app_mmwave_constants.h"
#include "app_mmwave.h"

#define APP_EVENT_QUEUE_LEN 20

typedef enum {
    APP_SENSOR_OK,
    APP_SENSOR_ERROR,
    APP_SENSOR_INVALID_STATE,
    APP_SENSOR_BUSY,
    APP_SENSOR_BAD_ARGUMENT,
    APP_SENSOR_BAD_MODE
} AppSensorStatus;

typedef enum {
    APP_SENSOR_UNINIT,
    APP_SENSOR_INIT,
    APP_SENSOR_RUNNING,
    APP_SENSOR_STOPPED
} AppSensorState;

typedef enum {
    SENSOR_MODE_STANDARD,
    SENSOR_MODE_UNDERLYING_OPEN
} SensorOperationMode;

typedef enum {
    NO_TYPE,
    HEARTBEAT,
    MODULE_RESET,
    PRODUCT_MODEL,
    PRODUCT_ID,
    INIT_STATUS,
    HARDWARE_MODEL,
    FIRMWARE_VERSION,
    SCENE_SETTINGS,
    SCENE_SETTINGS_I,
    SENSITIVITY,
    SENSITIVITY_I,
    PRESENCE,
    MOTION,
    BMP,
    TIME_FOR_NO_PERSON,
    TIME_FOR_NO_PERSON_I,
    PROXIMITY,
    OUTPUT_SWITCH,
    OUTPUT_SWITCH_I,
    EXISTENCE_ENERGY,
    MOTION_ENERGY,
    STATIC_DISTANCE,
    MOTION_DISTANCE,
    MOTION_SPEED,
    CUSTOM_MODE,
    CUSTOM_MODE_END,
    CUSTOM_MODE_I,
    EXISTENCE_JUDGMENT_THRESH,
    MOTION_TRIGGER_THRESH,
    EXISTENCE_PERCEPTION_BOUND,
    MOTION_TRIGGER_BOUND,
    MOTION_TRIGGER_TIME,
    MOTION_TO_STILL_TIME
} AppInquiryType;

typedef enum {
    MMWAVE_EVENT_REPORT,
    MMWAVE_EVENT_RESPONSE
} MmwaveEventType;

typedef struct {
    MmwaveEventType type;
    union
    {
        DecodedReport report;
        DecodedResponse response;
    };
} MmwaveEvent;

AppSensorStatus app_start_sys(void);
AppSensorStatus app_init_sys(void);
AppSensorStatus app_stop_sys(void);
SensorOperationMode app_get_mode(void);
AppSensorStatus app_set_mode(SensorOperationMode mode);
void mmwave_register_event_callback(MMwaveEventCallback cb); //poziva app mmwave manager da registrira funkciju koja obavlja slanje signala za event u queue
bool app_get_event(MmwaveEvent* out_event, uint32_t timeout_ms); //omogućuje da imamo i polling
void onResponse(DecodedResponse* response);
void onReport(DecodedReport* report);
AppSensorStatus app_send_inquiry(const uint8_t* data, size_t data_len, const uint8_t ctrl_w, const uint8_t cmd_w);