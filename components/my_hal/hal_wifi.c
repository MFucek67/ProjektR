/**
 * @file hal_wifi.c
 * @author Marko Fuček
 * @brief Implementacija HAL sloja za WiFi modul.
 * 
 * Implementacija se sastoji od pozivanja platform API-ja za inicijalizaciju, pokretanje, zaustavljanje i 
 * deinicijalizaciju WiFi drivera. Također, implementira se static funkcija koja upravlja wifi eventima.
 * 
 * @version 0.1
 * @date 2026-04-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "stdio.h"
#include "stdint.h"
#include "my_hal/hal_wifi.h"
#include "my_hal/hal_ws.h"
#include "platform/wifi_client.h"

static HalWifiState current_hal_wifi_state = HAL_WIFI_UNINIT;

/**
 * @brief Handler funkcija koja na određeni WiFi event uspostavlja vezu na aplikacijskom sloju (konkretno, 
 * websocket) i upravlja stanjem HAL-a.
 * 
 * @param st Stanje WiFi drivera na platform sloju
 */
static void hal_on_wifi_event(WiFiState st)
{
    switch (st)
    {
    case WIFI_STATE_STARTED:
        current_hal_wifi_state = HAL_WIFI_RUNNING;
        break;
    case WIFI_STATE_CONNECTING:
        break;
    case WIFI_STATE_DISCONNECTED:
        if(get_ws_client_state() == WS_STARTED) {
            hal_ws_stop(); //izgubili smo konekciju - zaustavljamo WS prijenos
        }
        break;
    case WIFI_STATE_CONFIGURED:
        current_hal_wifi_state = HAL_WIFI_STOPPED;
        break;
    case WIFI_STATE_CONNECTED:
        hal_ws_start(); //imamo IP adresu i sad pokrećemo WS prijenos preko HAL sučelja
        break;
    default:
        current_hal_wifi_state = HAL_WIFI_ERRORED;
        break;
    }
}

HalWifiStatus hal_wifi_init(hal_wifi_config* configuration)
{
    if(current_hal_wifi_state != HAL_WIFI_UNINIT) {
        return HAL_WIFI_INVALID_STATE;
    }
    if(wifi_client_init() != WIFI_OK) {
        return HAL_WIFI_ERROR;
    }
    if(wifi_client_configure_network((wifi_config*) configuration) != WIFI_OK) {
        return HAL_WIFI_ERROR;
    }
    if(wifi_register_hal_callback(&hal_on_wifi_event) != WIFI_OK) {
        return HAL_WIFI_ERROR;
    }
    current_hal_wifi_state = HAL_WIFI_INIT;
    return HAL_WIFI_OK;
}

HalWifiStatus hal_wifi_start(void)
{
    if(current_hal_wifi_state != HAL_WIFI_INIT && current_hal_wifi_state != HAL_WIFI_STOPPED) {
        return HAL_WIFI_INVALID_STATE;
    }
    if(wifi_client_start_and_connect() != WIFI_OK) {
        return HAL_WIFI_ERROR;
    }
    return HAL_WIFI_OK;
}

HalWifiStatus hal_wifi_stop(void)
{
    if(current_hal_wifi_state != HAL_WIFI_RUNNING) {
        return HAL_WIFI_INVALID_STATE;
    }
    if(wifi_client_disconnect() != WIFI_OK) {
        return HAL_WIFI_ERROR;
    }
    return HAL_WIFI_OK;
}

HalWifiStatus hal_wifi_deinit(void)
{
    if(current_hal_wifi_state != HAL_WIFI_INIT && current_hal_wifi_state != HAL_WIFI_STOPPED) {
        return HAL_WIFI_INVALID_STATE;
    }
    if(wifi_client_uninit() != WIFI_OK) {
        return HAL_WIFI_ERROR;
    }
    current_hal_wifi_state = HAL_WIFI_UNINIT;
    return HAL_WIFI_OK;
}