/**
 * @file hal_network.c
 * @author Marko Fuček
 * @brief Implementacija HAL sloja za network modul.
 * 
 * Implementacija modula vrši se pozivanjem HAL wifi funkcija i HAL websocket funkcija.
 * Kod hal_network_init i hal_network_deinit pozivaju se inicijalizacijska/deinicijalizacijska
 * funkcija i za wifi i za websocket, a kod hal_start i hal_stop samo za wifi, jer WiFi platoform
 * modul povlači pokretanje i zaustavljanje websocketa putem eventova.
 * 
 * @version 0.1
 * @date 2026-04-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "my_hal/hal_network_interface.h"

void hal_network_send(uint8_t* packet, size_t packet_len)
{
    hal_send_packet(packet, packet_len);
}

HalNetworkStatus hal_network_init(hal_network_config* configuration)
{
    hal_wifi_config wifi_c;
    strncpy(wifi_c.ssid, configuration->ssid, (sizeof(wifi_c.ssid) - 1));
    strncpy(wifi_c.pass, configuration->pass, (sizeof(wifi_c.pass) - 1));
    HalWifiStatus res1 = hal_wifi_init(&wifi_c);
    switch (res1)
    {
    case HAL_WIFI_INVALID_STATE:
        return HAL_NETWORK_INVALID_STATE;
        break;
    case HAL_WIFI_ERROR:
        return HAL_NETWORK_ERROR;
        break;
    case HAL_WIFI_TIMEOUT:
        return HAL_NETWORK_TIMEOUT;
        break;
    default:
        hal_ws_config ws_c;
        strncpy(ws_c.uri, configuration->uri, (sizeof(ws_c.uri) - 1));
        ws_c.port = configuration->port;
        HalWebSocketStatus res2 = hal_ws_init(&ws_c);
        switch (res2)
        {
        case HAL_WS_INVALID_STATE:
            return HAL_NETWORK_INVALID_STATE;
            break;
        case HAL_WS_ERROR:
            return HAL_NETWORK_ERROR;
            break;
        case HAL_WS_TIMEOUT:
            return HAL_NETWORK_TIMEOUT;
            break;
        default:
            return HAL_NETWORK_OK;
            break;
        }
    }  
}

HalNetworkStatus hal_network_start(void)
{
    HalWifiStatus res = hal_wifi_start();
    switch (res)
    {
    case HAL_WIFI_INVALID_STATE:
        return HAL_NETWORK_INVALID_STATE;
        break;
    case HAL_WIFI_ERROR:
        return HAL_NETWORK_ERROR;
        break;
    case HAL_WIFI_TIMEOUT:
        return HAL_NETWORK_TIMEOUT;
        break;
    default:
        return HAL_NETWORK_OK; 
        break;
    }
}

HalNetworkStatus hal_network_stop(void)
{
    HalWifiStatus res = hal_wifi_stop();
    switch (res)
    {
    case HAL_WIFI_INVALID_STATE:
        return HAL_NETWORK_INVALID_STATE;
        break;
    case HAL_WIFI_ERROR:
        return HAL_NETWORK_ERROR;
        break;
    case HAL_WIFI_TIMEOUT:
        return HAL_NETWORK_TIMEOUT;
        break;
    default:
        return HAL_NETWORK_OK; 
        break;
    }
}

HalNetworkStatus hal_network_deinit(void)
{
    HalWifiStatus res1 = hal_wifi_deinit();
    switch (res1)
    {
    case HAL_WIFI_INVALID_STATE:
        return HAL_NETWORK_INVALID_STATE;
        break;
    case HAL_WIFI_ERROR:
        return HAL_NETWORK_ERROR;
        break;
    case HAL_WIFI_TIMEOUT:
        return HAL_NETWORK_TIMEOUT;
        break;
    default:
        HalWebSocketStatus res2 = hal_ws_deinit();
        switch (res2)
        {
        case HAL_WS_INVALID_STATE:
            return HAL_NETWORK_INVALID_STATE;
            break;
        case HAL_WS_ERROR:
            return HAL_NETWORK_ERROR;
            break;
        case HAL_WS_TIMEOUT:
            return HAL_NETWORK_TIMEOUT;
            break;
        default:
            return HAL_NETWORK_OK;
            break;
        }
    } 
}