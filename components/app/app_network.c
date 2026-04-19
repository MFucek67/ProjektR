/**
 * @file app_network.c
 * @author Marko Fuček
 * @brief Implementacija krovnog API-ja za upravljanje network komponentom sustava.
 * 
 * Implementacija funkcija zadanih u headeru app_network.h.
 * 
 * @version 0.1
 * @date 2026-04-11
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdint.h>
#include <stdbool.h>
#include "app/app_network.h"
#include "my_hal/system_monitor.h"
#include "platform/platform_task.h"
#include "app/app_mmwave.h"
#include "app/app_network_packet_serializer.h"

static volatile bool network_ready = false;
static volatile bool end_flag = false;
static volatile bool task_ended = false;
static AppNetworkState current_state = APP_NETWORK_UNINIT;
static task_handler send_task_h = NULL;
static volatile uint32_t sent_responses = 0;
static volatile uint32_t sent_reports = 0;

/**
 * @brief Prima network event od HAL sloja i povlači određene akcije.
 * 
 * Za NETWORK_EVENT_CONNECTED daje dozvolu aplikacijskom sloju da smije slati podatke preko mreže.
 * Za NETWORK_EVENT_DISCONNECTED daje zastavicu za zaustavljanje slanja podataka.
 * 
 * @note Drugi eventi (error itd.) - TO-DO
 * 
 * @param nw Tip network eventa koji se dogodio
 */
static void on_nw_event(NetworkEvent nw)
{
    if(nw == NETWORK_EVENT_CONNECTED) {
        network_ready = true;
    } else if(nw == NETWORK_EVENT_DISCONNECTED) {
        network_ready = false;
    } else {
        //TO-DO drugi eventi
    }
}

/**
 * @brief Task koji upravlja uzimanjem podataka iz cachea i slanjem preko mreže.
 * 
 * Ako je mreža spremna funkcija polla mmwave response i report objekte iz cache-a, serijalizira ih u paket,
 * te nakon provjere veličine šalje mrežom i zabilježava da su poslani.
 * 
 * Task će se sam ugasiti i osloboditi zauzete resurse kada dobije flag od sustava.
 * 
 * @param arg Ne koristi se
 */
static void network_send_task(void* arg)
{
    for(;;) {
        if(network_ready) {
            DecodedResponse response;
            uint8_t res_packet[sizeof(ResponsePacket_t)];
            while(mmwave_poll_response(&response, 0)) {
                int len = app_serialize_response(&response, res_packet, sizeof(res_packet));
                if(len > 0) {
                    hal_network_send(&res_packet, len); //poslali smo response HAL-u, a on na websocket
                    sent_responses++;
                }
            }

            DecodedReport report;
            uint8_t rep_packet[sizeof(ReportPacket_t)];
            while(mmwave_poll_report(&report, 0)) {
                int len = app_serialize_report(&report, rep_packet, sizeof(rep_packet));
                if(len > 0) {
                    hal_network_send(&rep_packet, len); //poslali smo response HAL-u, a on na websocket
                    sent_reports++;
                }
            }

            if(end_flag) {
                system_monitor_unregister_task(send_task_h);
                task_ended = true;
                send_task_h = NULL;
                platform_delete_task(NULL);
            }

            platform_delay_task(1); //dajemo vrijeme IDLE tasku da se watchdog ne aktivira
        } else {
            platform_delay_task(100); //ako nam mreža nije spremna, čekamo connect
            if(end_flag) {
                system_monitor_unregister_task(send_task_h);
                task_ended = true;
                send_task_h = NULL;
                platform_delete_task(NULL);
            }
        }
    }
}

AppNetworkStatus network_init(app_network_config conf)
{
    if(current_state != APP_NETWORK_UNINIT) {
        return APP_NETWORK_INVALID_STATE;
    }
    if(hal_network_init((hal_network_config*) &conf) != HAL_NETWORK_OK) {
        return APP_NETWORK_ERROR;
    }
    hal_bind_network_callback(&on_nw_event);
    current_state = APP_NETWORK_INIT;
    return APP_NETWORK_OK;
}

AppNetworkStatus network_start(void)
{
    if(current_state != APP_NETWORK_INIT && current_state != APP_NETWORK_STOPPED) {
        return APP_NETWORK_INVALID_STATE;
    }
    if(hal_network_start() != HAL_NETWORK_OK) {
        return APP_NETWORK_ERROR;
    }

    end_flag = false;
    task_ended = false;

    TaskConfig_t cfg = {
        .task_function = network_send_task,
        .task_name = "net",
        .task_parameters = NULL,
        .task_priority = 6,
        .task_stack = 10000
    };
    send_task_h = platform_create_task(&cfg);
    if(send_task_h == NULL) {
        return APP_NETWORK_ERROR;
    }

    system_monitor_register_task("net_sender", send_task_h);
    current_state = APP_NETWORK_RUNNING;
    return APP_NETWORK_OK;
}

AppNetworkStatus network_stop(void)
{
    if(current_state != APP_NETWORK_RUNNING) {
        return APP_NETWORK_INVALID_STATE;
    }

    end_flag = true;
    while(task_ended == false) {
        platform_delay_task(20);
    }

    if(hal_network_stop() != HAL_NETWORK_OK) {
        return APP_NETWORK_ERROR;
    }
    current_state = APP_NETWORK_STOPPED;
    return APP_NETWORK_OK;
}

AppNetworkStatus network_uninit(void)
{
    if(current_state != APP_NETWORK_INIT && current_state != APP_NETWORK_STOPPED) {
        return APP_NETWORK_INVALID_STATE;
    }
    if(hal_network_deinit() != HAL_NETWORK_OK) {
        return APP_NETWORK_ERROR;
    }

    end_flag = false;
    task_ended = false;
    network_ready = false;

    current_state = APP_NETWORK_UNINIT;
    return APP_NETWORK_OK;
}

void sent_via_network_statistics(uint32_t* reports, uint32_t* responses)
{
    *reports = sent_reports;
    *responses = sent_responses;
}