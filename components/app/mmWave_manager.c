/**
 * @file mmWave_manager.c
 * @author Marko Fuček
 * @brief Implementacija managera za upravljanje aplikacijskim slojem.
 * 
 * Ovaj modul implementira aplikacijski manager. Zadaća managera je povezivanje svih djelova aplikacijskog sloja,
 * komunikacija s HAL-om, davanje callbackova decoderu i upravljanje internim stanjem sustava, kako bi se rasteretio
 * vršni aplikacijski sloj koji nudi API prema vanjskim programima.
 * 
 * Modul je ovisan o platformi i koristi platform funkcije (platform-specific).
 *  
 * @version 0.1
 * @date 2026-01-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "app/app_mmwave_manager.h"
#include "platform/platform_queue.h"
#include "platform/platform_task.h"
#include "platform/platform_memory.h"
#include "my_hal/hal_mmwave.h"
#include "app/app_mmwave_hal_config.h"

static AppSensorState current_state = APP_SENSOR_UNINIT;
static PlatformQueueHandle app_report_queue; //event queue za Report
static PlatformQueueHandle app_response_queue; //event queue za Response
static AppInquiryType pending_inquiry = NO_TYPE; //trenutno poslani inquiry
static SensorOperationMode current_mode = SENSOR_MODE_STANDARD; //mode senzora
static task_handler decoder_task_handler;
static MMwaveReportCallback higher_app_report_callback;
static MMwaveResponseCallback higher_app_response_callback;
static bool end_flag = false;
static bool task_ended = false;

static AppDecoderContext decoder_ctx = {
        .sendReportCallback = onReport,
        .sendResponseCallback = onResponse
};

/**
 * @brief Task za obradu (dekodiranje) primljenih parsiranih podataka.
 * 
 *  Uzima podatke iz HAL queue-a, preuzima ownership frame-a na aplikacijski sloj, dekodira podatke, te
 *  nakon slanja reporta/requesta oslobađa memoriju koju je frame bio zauzimao.
 * 
 * @param arg Ne koristi se
 */
static void decoder_task(void* arg)
{
    QueueElement_t buffer;
    while(true) {
        if(hal_mmwave_get_frame_from_queue(&buffer, 10) != HAL_MMWAVE_OK) {
            if(end_flag) {
                task_ended = true;
                break;
            }
            platform_delay_task(20);
            continue;
        }

        FrameData_t frame = {
            .data = buffer.data,
            .len = buffer.len
        };

        if(buffer.data == NULL || buffer.len == 0) {
            printf("[decoder_task] ERROR: empty frame (data=%p len=%zu)\n",
            buffer.data, buffer.len);
            continue;
        } //KASNIJE MAKNUTI

        printf("[decoder_task] Got frame len=%zu\n", buffer.len); //KASNIJE MAKNUTI

        app_mmwave_decoder_process_frame(frame.data, frame.len);
        //tek sada kada smo uzeli podatke i protumačili frame
        //oslobađamo memoriju koju je mmwave_core zauzeo za spremanje frame-a
        printf("[decoder_task] Releasing frame memory: frame.data=%p, frame.len=%zu\n",
            frame.data, frame.len);
        hal_mmwave_release_frame_memory(&frame);
    }
}

AppSensorStatus app_init_sys(void)
{
    if(current_state != APP_SENSOR_UNINIT) {
        return APP_SENSOR_INVALID_STATE;
    }

    hal_mmwave_config* hal_conf = app_mmwave_get_hal_config();
    mmWave_core_interface* core_call = app_mmwave_get_core_interface();
    if(!hal_conf || !core_call) {
        printf("[app_init] ERROR: NULL HAL config or core interface\n");
        return APP_SENSOR_ERROR;
    } //KASNIJE MAKNUTI

    if(!core_call->mmwave_parse_data || !core_call->mmwave_build_frame) {
        printf("[app_init] ERROR: core interface incomplete\n");
        return APP_SENSOR_ERROR;
    } //KASNIJE MAKNUTI
    
    if(hal_mmwave_init(hal_conf, core_call) != HAL_MMWAVE_OK) {
        return APP_SENSOR_ERROR;
    }

    app_mmwave_decoder_init(&decoder_ctx);

    app_report_queue = platform_queue_create(APP_EVENT_QUEUE_LEN, sizeof(DecodedReport));
    app_response_queue = platform_queue_create(APP_EVENT_QUEUE_LEN, sizeof(DecodedResponse));
    if(!app_report_queue || !app_response_queue) {
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

    TaskConfig_t task_conf = {decoder_task, "decoder_task", 8000, NULL, 6};
    decoder_task_handler = platform_create_task(&task_conf);
    if(!decoder_task_handler) {
        printf("[app_start] ERROR: decoder task not created\n"); //KASNIJE MAKNUTI
        return APP_SENSOR_ERROR;
    }

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

    end_flag = true;
    while(!task_ended) {
        platform_delay_task(10);
    }
    platform_delete_task(decoder_task_handler);
    decoder_task_handler = NULL;

    platform_queue_delete(app_report_queue);
    app_report_queue = NULL;

    platform_queue_delete(app_response_queue);
    app_response_queue = NULL;

    current_state = APP_SENSOR_STOPPED;
    return APP_SENSOR_OK;
}

AppSensorStatus app_deinit_sys(void)
{
    if(current_state != APP_SENSOR_STOPPED) {
        return APP_SENSOR_INVALID_STATE;
    }

    if(hal_mmwave_deinit() != HAL_MMWAVE_OK) {
        return APP_SENSOR_ERROR;
    }
    app_mmwave_decoder_deinit();

    end_flag = false;
    task_ended = false;

    current_state = APP_SENSOR_UNINIT;
    return APP_SENSOR_OK;
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

void mmwave_register_event_callback(MMwaveResponseCallback res_cb, MMwaveReportCallback rep_cb)
{
    higher_app_response_callback = res_cb;
    higher_app_report_callback = rep_cb;
}

bool app_get_response(DecodedResponse* out_response, uint32_t timeout_ms)
{
    QueueElement_t q_el;
    if(platform_queue_get(app_response_queue, &q_el, timeout_ms) != QUEUE_OK) {
        return false;
    }

    DecodedResponse* response_ptr = (DecodedResponse*)q_el.data;
    *out_response = *response_ptr; //kopiramo cijelu strukturu u korisnički buffer

    //oslobađamo memoriju zauzetu kod stvaranja RESPONSE eventa (dodavanje u queue)
    platform_free(response_ptr);

    return true;
}

bool app_get_report(DecodedReport* out_report, uint32_t timeout_ms)
{
    QueueElement_t q_el;
    if(platform_queue_get(app_report_queue, &q_el, timeout_ms) != QUEUE_OK) {
        return false;
    }

    DecodedReport* report_ptr = (DecodedReport*) q_el.data;
    *out_report = *report_ptr;

    //sad treba osloboditi memoriju zauzetu kod stvaranja REPORT-a (dodavanje u queue)
    platform_free(report_ptr);

    return true;
}

void onResponse(DecodedResponse* response)
{
    if(!response) return;

    // Provjera veličine response podataka - odbijamo ako prelazi maximum
    if(response->data_l > MAX_RESPONSE_DATA_LEN) {
        return;
    }

    //za queue trebamo alocirani event (jer se dijeli među slojevima)
    //moramo raditi kopiju jer će task odmah nakon izbrisati pravi frame i vratiti HAL-u memoriju
    DecodedResponse* ev_queue;
    if(platform_malloc((void**)&ev_queue, sizeof(DecodedResponse)) != MEM_OK) {
        printf("[onResponse] ERROR: malloc failed\n");
        return;
    }
    printf("[onResponse] Kopiranje response strukture na heap\n");
    memcpy(ev_queue, response, sizeof(DecodedResponse));

    QueueElement_t q_el = {
        .data = (uint8_t*)ev_queue,
        .len = sizeof(DecodedResponse)
    };

    //šaljemo event u queue
    printf("[onResponse] Slanje u queue\n");
    if(platform_queue_send(app_response_queue, &q_el, 10) != QUEUE_OK) {
        printf("[onResponse] ERROR: queue_send failed\n");
        platform_free(ev_queue);
        return;
    }
    printf("[onResponse] Uspješno poslano u queue\n");
    if(higher_app_response_callback) {
        higher_app_response_callback(response);
    }
}

void onReport(DecodedReport* report)
{
    if(!report) {
        printf("[onReport] ERROR: report is NULL\n");
        return;
    }

    printf("[onReport] Primljen report\n");

    //za queue trebamo alocirani event
    DecodedReport* ev_queue;
    if (platform_malloc((void**)&ev_queue, sizeof(DecodedReport)) != MEM_OK) {
        printf("[onReport] ERROR: malloc failed\n");
        return;
    }
    printf("[onReport] Kopiranje report strukture na heap\n");
    memcpy(ev_queue, report, sizeof(DecodedReport));

    QueueElement_t q_el = {
        .data = (uint8_t*)ev_queue,
        .len = sizeof(DecodedReport)
    };

    //šaljemo event u queue
    printf("[onReport] Slanje u queue\n");
    if(platform_queue_send(app_report_queue, &q_el, 10) != QUEUE_OK) {
        printf("[onReport] ERROR: queue_send failed\n");
        platform_free(ev_queue);
        return;
    }
    printf("[onReport] Uspješno poslano u queue\n");
    if(higher_app_report_callback) {
        higher_app_report_callback(report);
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