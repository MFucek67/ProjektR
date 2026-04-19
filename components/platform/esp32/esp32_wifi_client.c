/**
 * @file esp32_wifi_client.c
 * @author Marko Fuček
 * @brief ESP32 implementacija API-ja za upravljanje WiFi driverom i konekcijom.
 * 
 * Ovaj modul implementira ESP-IDF driver za upravljanje WiFi konekcijom na platform sloju.
 * 
 * Implementacija modula obuhvaća funkcije zadane u headeru wifi_client.h i dodatno funkciju
 * koja je handler WiFi eventova.
 * 
 * @note Nisu obrađeni svi WiFi eventi.
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
#include "platform/platform_task.h"
#include "platform/wifi_client.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"

static WiFiState current_state = WIFI_STATE_UNINIT;
static esp_netif_t* netif_handler = NULL;
static volatile bool manual_disconnect = false;
static on_wifi_event hal_cb = NULL;

/**
 * @brief Handler koji WiFi driver poziva kada event loop dobije neki WiFi event.
 * 
 * Ovisno o primljenom eventu (i grupi eventova) funkcija mapira event i postavlja stanje WiFi drivera
 * ili poziva HAL callback da odradi radnju sukladno trenutnom eventu.
 * 
 * @param arg 
 * @param event_base 
 * @param event_id 
 * @param event_data 
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT) {
        switch(event_id) {
            case WIFI_EVENT_STA_START:
                //WiFi TCP/IP stack i driver pokrenuti -> spremno za connectanje
                current_state = WIFI_STATE_STARTED;
                if(esp_wifi_connect() != ESP_OK) {
                    printf("Error while trying to connect!\n");
                    current_state = WIFI_STATE_ERROR;
                }
                if(hal_cb != NULL) {
                    hal_cb(current_state);
                }
                break;
            case WIFI_EVENT_STA_CONNECTED:
                //konekcija na mrežnoj poveznici obavljena, čeka se na IP adresu
                current_state = WIFI_STATE_CONNECTING;
                if(hal_cb != NULL) {
                    hal_cb(current_state);
                }
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                if(manual_disconnect) {
                    printf("Manual disconnect - disconnecting device...\n");
                    manual_disconnect = false;
                    current_state = WIFI_STATE_DISCONNECTED;
                } else {
                    printf("Unexpected disconnect - reconnecting...\n");
                    if(esp_wifi_connect() != ESP_OK) {
                        current_state = WIFI_STATE_ERROR;
                    }
                }
                if(hal_cb != NULL) {
                    hal_cb(current_state);
                }
                break;
            case WIFI_EVENT_STA_STOP:
                //WiFi TCP/IP stack i driver zaustavljeni -> potreban novi start
                current_state = WIFI_STATE_CONFIGURED;
                if(hal_cb != NULL) {
                    hal_cb(current_state);
                }
        }
    } else if(event_base == IP_EVENT) {
        if(event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t *ip_e = (ip_event_got_ip_t*) event_data;
            printf("Client got IP address (" IPSTR ")!\n", IP2STR(&ip_e->ip_info.ip));
            current_state = WIFI_STATE_CONNECTED;
        } else if(event_id == IP_EVENT_STA_LOST_IP) {
            printf("Client lost IP!");
            current_state = WIFI_STATE_CONNECTING;
        }
        if(hal_cb != NULL) {
            hal_cb(current_state);
        }
    }
}

WiFiStatus wifi_client_init(void)
{
    if(current_state != WIFI_STATE_UNINIT) {
        return WIFI_WRONG_STATE;
    }

    if(nvs_flash_init() != ESP_OK) { //inicijalizacija NVS flasha
        return WIFI_ERROR;
    }
    if(esp_netif_init() != ESP_OK) { //inicijalizacija TCP/IP stacka
        return WIFI_ERROR;
    }
    if(esp_event_loop_create_default() != ESP_OK) { //stvaranje event loopa za wifi evente
        return WIFI_ERROR;
    }

    netif_handler = esp_netif_create_default_wifi_sta();
    if(netif_handler == NULL) {
        return WIFI_ERROR;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); //Koristimo osnovnu konfiguraciju
    if(esp_wifi_init(&cfg) != ESP_OK) { //Pokretanje wifi subsystema i kreiranje internih resursa
        return WIFI_ERROR;
    }

    if(esp_event_handler_register(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, wifi_event_handler, NULL) != ESP_OK) { //registracija handlera za wifi evente
        return WIFI_ERROR;
    }

    current_state = WIFI_STATE_INIT;
    return WIFI_OK;
}

WiFiStatus wifi_client_configure_network(wifi_config *config)
{
    if(current_state != WIFI_STATE_INIT) {
        return WIFI_WRONG_STATE;
    }

    if(esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK) {
        return WIFI_ERROR;
    }

    wifi_config_t esp_conf = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK
        }
    };
    strncpy((char*) esp_conf.sta.ssid, config->ssid, (sizeof(esp_conf.sta.ssid) - 1));
    strncpy((char*) esp_conf.sta.password, config->pass, (sizeof(esp_conf.sta.password) - 1));

    if(esp_wifi_set_config(WIFI_IF_STA, &esp_conf) != ESP_OK) {
        return WIFI_ERROR;
    }

    current_state = WIFI_STATE_CONFIGURED;
    return WIFI_OK;
}

WiFiStatus wifi_client_start_and_connect(void)
{
    if(current_state != WIFI_STATE_CONFIGURED) {
        return WIFI_WRONG_STATE;
    }

    if(esp_wifi_start() != ESP_OK) {
        return WIFI_ERROR;
    }
    return WIFI_OK;
}

WiFiStatus wifi_client_disconnect(void)
{
    manual_disconnect = true;
    if(esp_wifi_disconnect() != ESP_OK) {
        manual_disconnect = false;
        return WIFI_ERROR;
    }
    return WIFI_OK;
}

WiFiStatus wifi_client_uninit(void)
{
    if(current_state != WIFI_STATE_INIT && current_state != WIFI_STATE_DISCONNECTED && current_state != WIFI_STATE_ERROR) {
        return WIFI_WRONG_STATE;
    }
    if(esp_wifi_deinit() != ESP_OK) {
        return WIFI_ERROR;
    }
    esp_netif_destroy(netif_handler);
    if(esp_event_handler_unregister(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, wifi_event_handler) != ESP_OK) {
        return WIFI_ERROR;
    }
    if(esp_event_loop_delete_default() != ESP_OK) {
        return WIFI_ERROR;
    }
    current_state = WIFI_STATE_UNINIT;
    return WIFI_OK;
}

WiFiState get_wifi_client_state(void)
{
    return current_state;
}

WiFiStatus wifi_register_hal_callback(on_wifi_event hal_callback)
{
    if(hal_callback != NULL) {
        hal_cb = hal_callback;
        return WIFI_OK;
    } else {
        return WIFI_ERROR;
    }
}