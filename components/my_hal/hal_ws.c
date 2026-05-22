/**
 * @file hal_ws.c
 * @author Marko Fuček
 * @brief Implementacija HAL sloja za Web Socket modul.
 * 
 * Implementacija se sastoji od poziva prema platform Web Socket API-ju za inicijalizaciju, pokretanje, zaustavljanje
 * i deinicijalizaciju Web Socketa. Dodatno, implementacija ima funkcije koje kao callback prosljeđuje platform sloju.
 * Preko tih funkcija teče komunikacija platform WS -> HAL WS -> application WS. Konačno, implementacija funkcije
 * hal_bind_network_callback služi za dodjelu app callbackova HAL-u.
 * 
 * @warning Implementacija ima nedovršenih funkcija za dvosmjernu komunikaciju i error handling.
 * 
 * @version 0.1
 * @date 2026-04-10
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "stdio.h"
#include "stdint.h"
#include "my_hal/hal_ws.h"
#include "my_hal/hal_ws_interface.h"
#include "my_hal/hal_network_interface.h"
#include "platform/websocket.h"

static HalWebSocketState current_hal_ws_state = HAL_WS_UNINIT;
static on_network_event network_cb = NULL;
static on_network_data network_data_cb = NULL;

/**
 * @brief Handla podatke koje platform Web Socket primi preko Web Socketa.
 * 
 * @note TO-DO
 * 
 * @param data Pokazivač na primljene podatke
 * @param len Duljina primljenih podataka u bajtovima
 */
static void hal_on_ws_data(const uint8_t* data, size_t len)
{
    if((len > 0) && data) {
        if(network_data_cb) {
            network_data_cb(data, len);
        }
    }
    platform_free(data); //free memorije rezervirane na platform sloju
}

/**
 * @brief Handla pogreške Web Socket sustava na HAL sloju.
 * 
 * @note TO-DO
 * 
 */
static void hal_on_ws_error(void)
{
 //TO-DO ako bude potrebe
}

/**
 * @brief Handla pokretanje i zaustavljanje Web Socket drivera.
 * 
 * @param started Signal koji nam govori je li sustav upaljen ili ugašen (true - upaljen, false - ugašen)
 */
static void hal_on_ws_start_stop(bool started)
{
    if(started) { //started = true -> konekcija ostvarena
        current_hal_ws_state = HAL_WS_RUNNING;
        if(network_cb) {
            network_cb(NETWORK_EVENT_CONNECTED);
        }
    } else { //started = false -> konekcija pukla
        current_hal_ws_state = HAL_WS_STOPPED;
        if(network_cb) {
            network_cb(NETWORK_EVENT_DISCONNECTED);
        }
    }
}

void hal_send_packet(uint8_t* packet, size_t packet_len)
{
    if(packet && (packet_len > 0) && (current_hal_ws_state == HAL_WS_RUNNING)) {
        ws_send(packet, packet_len, 1000);
    }
}

HalWebSocketStatus hal_ws_init(hal_ws_config* configuration)
{
    if(current_hal_ws_state != HAL_WS_UNINIT) {
        return HAL_WS_INVALID_STATE;
    }
    if(ws_client_init((ws_config*) configuration) != WS_OK) {
        return HAL_WS_ERROR;
    }
    if(ws_register_hal_callback(&hal_on_ws_data, &hal_on_ws_error, &hal_on_ws_start_stop) != WS_OK) {
        return HAL_WS_ERROR;
    }
    current_hal_ws_state = HAL_WS_INIT;
    return HAL_WS_OK;
}

HalWebSocketStatus hal_ws_start(void)
{
    if(current_hal_ws_state != HAL_WS_INIT && current_hal_ws_state != HAL_WS_STOPPED) {
        return HAL_WS_INVALID_STATE;
    }
    if(ws_client_start() != WS_OK) {
        return HAL_WS_ERROR;
    }
    return HAL_WS_OK;
}

HalWebSocketStatus hal_ws_stop(void)
{
    if(current_hal_ws_state != HAL_WS_RUNNING) {
        return HAL_WS_INVALID_STATE;
    }
    if(ws_client_stop() != WS_OK) {
        return HAL_WS_ERROR;
    }
    return HAL_WS_OK;
}

HalWebSocketStatus hal_ws_deinit(void)
{
    if(current_hal_ws_state != HAL_WS_STOPPED && current_hal_ws_state != HAL_WS_INIT) {
        return HAL_WS_INVALID_STATE;
    }
    if(ws_client_uninit() != WS_OK) {
        return HAL_WS_ERROR;
    }
    current_hal_ws_state = HAL_WS_UNINIT;
    return HAL_WS_OK;
}

void hal_bind_network_callback(on_network_event app_cb, on_network_data app_nw_cb)
{
    if(app_cb) {
        network_cb = app_cb;
    }
    if(app_nw_cb) {
        network_data_cb = app_nw_cb;
    }
}