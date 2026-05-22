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
#include "platform/platform.h"
#include "app/app_mmwave.h"
#include "app/app_network_packet_serializer.h"

static volatile bool network_ready = false;
static volatile bool end_flag = false;
static volatile bool task_ended = false;
static AppNetworkState current_state = APP_NETWORK_UNINIT;
static task_handler send_task_h = NULL;
static volatile uint32_t sent_responses = 0;
static volatile uint32_t sent_reports = 0;
static PlatformQueueHandle rx_queue = NULL;

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

static void on_nw_data_received(const uint8_t* data, size_t len)
{
    if(!data || len == 0) return;

    NetworkRxPacket packet;
    MemoryOperationStatus status = platform_malloc(&(packet.data), len);
    if(status != MEM_OK) {
        return;
    }

    memcpy(packet.data, data, len);
    packet.len = len;

    if(platform_queue_send(rx_queue, &packet, 10) != QUEUE_OK) {
        platform_free(packet.data);
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
            if(app_get_mmwave_state() == APP_SENSOR_RUNNING) {
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
            }

            if(end_flag) {
                system_monitor_unregister_task(send_task_h);
                task_ended = true;
                send_task_h = NULL;
                platform_delete_task(NULL);
            }

            platform_delay_task(100); //dajemo vrijeme IDLE tasku da se watchdog ne aktivira
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
    rx_queue = platform_queue_create(RX_QUEUE_LEN, sizeof(NetworkRxPacket));
    hal_bind_network_callback(&on_nw_event, &on_nw_data_received);
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

    NetworkRxPacket p;
    while(platform_get_num_of_queue_elements(rx_queue) != 0) {
        platform_queue_get(rx_queue, &p, 0);
        platform_free(&p.data);
    }
    platform_queue_delete(rx_queue);

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

//korisnik (test) mora osigurati stack buffer - ako je duljina buffera koji da manja od duljine poruke sa servera
//poruka se reže na veličinu buffera!
bool network_poll_rx(uint8_t* out_buf, size_t buf_len, size_t* out_len, uint32_t timeout_in_ms)
{
    if(!rx_queue || !out_buf || !out_len) {
        return false;
    }

    NetworkRxPacket packet;
    if(platform_queue_get(rx_queue, &packet, timeout_in_ms) != QUEUE_OK) {
        return false;
    }

    //ako je ono što je server poslao preveliko - odrezat ćemo "višak"
    size_t copy_len = 0;
    if(packet.len > buf_len) {
        copy_len = buf_len;
    } else {
        copy_len = packet.len;
    }

    memcpy(out_buf, packet.data, copy_len);
    *out_len = copy_len;

    platform_free(packet.data); //free paketa s heapa
    return true;
}