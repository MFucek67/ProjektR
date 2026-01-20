#include <stdio.h>
#include <stdint.h>
#include "app/app_mmwave_manager.h"
#include "platform/platform_queue.h"
#include "platform/platform_task.h"
#include "platform/platform_memory.h"
#include "HAL/hal_mmwave.h"
#include "app/app_mmwave_hal_config.h"

static AppSensorState current_state = APP_SENSOR_UNINIT; //za pamćenje stanja sustava
static QueueHandle_t app_event_queue; //event queue za Response/Report
static AppInquiryType pending_inquiry = NO_TYPE; //trenutno poslani inquiry (smije samo jedan prije dobivenog odgovora - čeka se neko vrijeme)
static SensorOperationMode current_mode = SENSOR_MODE_STANDARD; //mode senzora
static task_handler decoder_task_handler;
static MMwaveEventCallback higher_app_callback;
static bool end_flag = false;
static bool task_ended = false;

static void decoder_task(void* arg)
{
    QueueElement_t buffer;
    while(true) {
        hal_mmwave_get_frame_from_queue(&buffer, 10);
        mmWaveFrameData frame = {
            .data = buffer.data,
            .data_len = buffer.len
        };

        app_mmwave_decoder_process_frame(frame.data, frame.data_len);
        //tek sada kada smo uzeli podatke i protumačili frame
        //oslobađamo memoriju koju je mmwave_core zauzeo za spremanje frame-a
        hal_mmwave_release_frame_memory(&frame);
        if(end_flag) {
            task_ended = true;
            break;
        }
    }
}

AppSensorStatus app_init_sys(void)
{
    if(current_state != APP_SENSOR_UNINIT) {
        return APP_SENSOR_INVALID_STATE;
    }

    hal_mmwave_config* hal_conf = app_mmwave_get_hal_config();
    mmWave_core_interface* core_call = app_mmwave_get_core_interface();
    if(hal_mmwave_init(hal_conf, core_call) != HAL_MMWAVE_OK) {
        return APP_SENSOR_ERROR;
    }

    AppDecoderContext decoder_ctx = {
        .sendReportCallback = onReport,
        .sendResponseCallback = onResponse
    };
    app_mmwave_decoder_init(&decoder_ctx);

    app_event_queue = platform_queue_create(APP_EVENT_QUEUE_LEN, sizeof(MmwaveEvent));
    if(!app_event_queue) {
        return APP_SENSOR_ERROR;
    }

    end_flag = false;
    task_ended = false;
    current_state = APP_SENSOR_INIT;
    return APP_SENSOR_OK;
}

AppSensorStatus app_start_sys(void)
{
    if(current_state != APP_SENSOR_INIT && current_state != APP_SENSOR_STOPPED) {
        return APP_SENSOR_INVALID_STATE;
    }

    if(hal_mmwave_start() != HAL_MMWAVE_OK) {
        return APP_SENSOR_ERROR;
    }

    TaskConfig_t task_conf = {decoder_task, "decoder_task", 4000, NULL, 6};
    decoder_task_handler = platform_create_task(&task_conf);

    current_state = APP_SENSOR_RUNNING;
    return APP_SENSOR_OK;
}

AppSensorStatus app_stop_sys(void)
{
    if(current_state != APP_SENSOR_RUNNING) {
        return APP_SENSOR_INVALID_STATE;
    }

    if(hal_mmwave_stop() != HAL_MMWAVE_OK) {
        return APP_SENSOR_ERROR;
    }

    app_mmwave_decoder_deinit();
    end_flag = true;

    while(!task_ended) {
        platform_delay_task(10);
    }
    platform_delete_task(decoder_task_handler);
    decoder_task_handler = NULL;

    platform_queue_delete(app_event_queue);
    app_event_queue = NULL;

    current_state = APP_SENSOR_UNINIT;
}

SensorOperationMode app_get_mode(void)
{
    return current_mode;
}

AppSensorStatus app_set_mode(SensorOperationMode mode)
{
    if(current_state != APP_SENSOR_RUNNING) {
        return APP_SENSOR_INVALID_STATE;
    }
    if(mode != SENSOR_MODE_STANDARD && mode != SENSOR_MODE_UNDERLYING_OPEN) {
        return APP_SENSOR_ERROR;
    }
    current_mode = mode;
    return APP_SENSOR_OK;
}

void mmwave_register_event_callback(MMwaveEventCallback cb)
{
    higher_app_callback = cb;
}

bool app_get_event(MmwaveEvent* out_event, uint32_t timeout_ms)
{
    QueueElement_t q_el;
    if(platform_queue_get(app_event_queue, &q_el, 10) != QUEUE_OK) {
        return false;
    }

    MmwaveEvent* event_ptr = (MmwaveEvent*)q_el.data;
    *out_event = *event_ptr; //kopiramo cijelu strukturu u korisnički buffer
    //sad treba osloboditi memoriju zauzetu kod stvaranja eventa (dodavanje u queue)
    platform_free(event_ptr);

    return true;
}

void onResponse(DecodedResponse* response)
{
    MmwaveEvent* ev;
    if(platform_malloc(&ev, sizeof(MmwaveEvent)) != MEM_OK) {
        return;
    }
    ev->type = MMWAVE_EVENT_RESPONSE;
    ev->response = *response;

    QueueElement_t q_el = {
        .data = (uint8_t*)ev,
        .len = sizeof(MmwaveEvent)
    };

    //šaljemo event u queue
    platform_queue_send(app_event_queue, &q_el, 10);
    //ako je callback zadan, šaljemo "notification" i preko njega
    if(higher_app_callback) {
        higher_app_callback(ev);
    }
}

void onReport(DecodedReport* report)
{
    MmwaveEvent* ev;
    if(platform_malloc(&ev, sizeof(MmwaveEvent)) != MEM_OK) {
        return;
    }
    ev->type = MMWAVE_EVENT_REPORT;
    ev->report = *report;

    QueueElement_t q_el = {
        .data = (uint8_t*)ev,
        .len = sizeof(MmwaveEvent)
    };

    //šaljemo event u queue
    platform_queue_send(app_event_queue, &q_el, 10);
    //ako je callback zadan, šaljemo "notification" i preko njega
    if(higher_app_callback) {
        higher_app_callback(ev);
    }
}

AppSensorStatus app_send_inquiry(const uint8_t* data, size_t data_len, const uint8_t ctrl_w, const uint8_t cmd_w)
{
    if(current_state != APP_SENSOR_RUNNING) {
        return APP_SENSOR_INVALID_STATE;
    }
    HalMmwaveStatus status = hal_mmwave_send_frame(data, data_len, ctrl_w, cmd_w);
    if(status != HAL_MMWAVE_OK) {
        return APP_SENSOR_ERROR;
    }
    return APP_SENSOR_OK;
}