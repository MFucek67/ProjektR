/**
 * @file esp32_websocket.c
 * @author Marko Fuček
 * @brief ESP32 implementacija API-ja za upravljanje Web Socketom.
 * 
 * Ovaj modul implementira ESP-IDF driver za upravljanje Web Socketom na platform sloju.
 * 
 * Modul obuhvaća implementacije funkcija zadanih u headeru websocket.h i dodatno handler za Web Socket
 * evente.
 * 
 * @note Nisu obrađeni svi Web Socket eventi (vidi TO-DO).
 * @warning Implementacija koristi espressif/esp_websocket_client eksterni modul.
 * 
 * @version 0.1
 * @date 2026-04-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "stdio.h"
#include "stdint.h"
#include "freertos/FreeRTOS.h"
#include "platform/websocket.h"
#include "esp_websocket_client.h"
#include "my_hal/hal_ws_interface.h"

static websocket_handler websocket_h = NULL;
static webSocketState current_state = WS_UNINIT;
static on_ws_data hal_on_data_cb = NULL;
static on_ws_error hal_on_error_cb = NULL;
static on_ws_start_stop hal_on_start_stop_cb = NULL;

/**
 * @brief Handler koji Web Socket driver poziva kada se dogodi neki event.
 * 
 * Funkcija ovisno o eventu mapira događaj i obavlja potrebne radnje - bilo da postavlja stanje Web Socket
 * sustava, bilo da poziva HAL callbackove.
 * 
 * @param arg Pokazivač na argumente koji se prosljeđuju iz drivera.
 * @param base Skupina eventa.
 * @param event_id ID eventa.
 * @param event_data Podatci koji pobliže opisuju event.
 */
static void ws_handler(void* arg, esp_event_base_t base, int32_t event_id, void* event_data)
{
    switch (event_id)
    {
    case WEBSOCKET_EVENT_CONNECTED:
        current_state = WS_STARTED;
        if(hal_on_start_stop_cb) {
            hal_on_start_stop_cb(true);
        }
        break;
    case WEBSOCKET_EVENT_ERROR:
        //TO-DO kod dvosmjerne komunikacije!!!!
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        current_state = WS_CLOSED;
        if(hal_on_start_stop_cb) {
            hal_on_start_stop_cb(false);
        }
        break;
    case WEBSOCKET_EVENT_DATA:
        esp_websocket_event_data_t* data_ = (esp_websocket_event_data_t*) event_data;
        //napraviti kopiju i poslati u HAL (dinamički - HAL free-a???)
        //TO-DO kod dvosmjerne komunikacije!!!!
        break;
    case WEBSOCKET_EVENT_CLOSED:
        current_state = WS_CLOSED;
        if(hal_on_start_stop_cb) {
            hal_on_start_stop_cb(false);
        }
        break;
    default:
        break;
    }
}

webSocketStatus ws_client_init(ws_config *config)
{
    if(current_state != WS_UNINIT) {
        return WS_WRONG_STATE;
    }

    esp_websocket_client_config_t ws_config = {
        .uri = config->uri,
        .port = config->port
    };
    websocket_h = esp_websocket_client_init(&ws_config);

    if(websocket_h) {
        if(esp_websocket_register_events(websocket_h, WEBSOCKET_EVENT_ANY, ws_handler, NULL) != ESP_OK) {
            return WS_ERROR;
        }
        current_state = WS_INIT;
        return WS_OK;
    }
    return WS_ERROR;
}

webSocketStatus ws_client_start(void)
{
    if(current_state != WS_CLOSED && current_state != WS_INIT) {
        return WS_WRONG_STATE;
    }

    if(websocket_h) {
        if(esp_websocket_client_start(websocket_h) != ESP_OK) {
            return WS_ERROR;
        }
        return WS_OK;
    }
    return WS_ERROR;
}

webSocketStatus ws_send(uint8_t* data, int len, uint32_t timeout_in_ms)
{
    if(current_state != WS_STARTED) {
        return WS_WRONG_STATE;
    }
    if(websocket_h) {
        int res = esp_websocket_client_send_bin(websocket_h, (const char*) data, len, pdMS_TO_TICKS(timeout_in_ms));
        if(res >= 0) {
            return WS_OK;
        } else {
            return WS_ERROR;
        }
    }
    return WS_ERROR;
}

webSocketStatus ws_client_stop(void)
{
    if(current_state != WS_STARTED) {
        return WS_WRONG_STATE;
    }

    if(websocket_h) {
        if(esp_websocket_client_close(websocket_h, pdMS_TO_TICKS(1000)) != ESP_OK) {
            return WS_ERROR;
        }
        return WS_OK;
    }
    return WS_ERROR;
}

webSocketStatus ws_client_uninit(void)
{
    if(current_state != WS_INIT && current_state != WS_CLOSED) {
        return WS_WRONG_STATE;
    }

    if(websocket_h) {
        if(esp_websocket_client_destroy(websocket_h) != ESP_OK) {
            return WS_ERROR;
        }
        websocket_h = NULL;
        current_state = WS_UNINIT;
        return WS_OK;
    }
    return WS_ERROR;
}

webSocketState get_ws_client_state(void)
{
    return current_state;
}

webSocketStatus ws_register_hal_callback(on_ws_data hal_cb_on_data, on_ws_error hal_cb_on_err, on_ws_start_stop hal_cb_on_start_stop)
{
    if(hal_cb_on_data && hal_cb_on_err && hal_cb_on_start_stop) {
        hal_on_data_cb = hal_cb_on_data;
        hal_on_error_cb = hal_cb_on_err;
        hal_on_start_stop_cb = hal_cb_on_start_stop;
        return WS_OK;
    }
    return WS_ERROR;
}