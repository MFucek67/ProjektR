/**
 * @file hal_network_interface.h
 * @author Marko Fuček
 * @brief HAL sučelje koje nudi sve apstraktne tipove i enumeracije koje definiraju network sloj na HAL dijelu.
 * 
 * Network sloj HAL-a kombinira sloj podatkovne poveznice (WiFi) i aplikacijski sloj (Web Socket), stoga su funkcije
 * network sloja pretežno samo wrapperi koji pozivaju funkcije bilo sloja podatkovne poveznice, bilo oba sloja.
 * @version 0.1
 * @date 2026-04-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "stdio.h"
#include "stdint.h"
#include "my_hal/hal_ws.h"
#include "my_hal/hal_wifi.h"

/**
 * @enum NetworkEvent
 * @brief Event tipovi na network sloju.
 * 
 */
typedef enum {
    NETWORK_EVENT_CONNECTED, /**< Network sloj spojen */
    NETWORK_EVENT_DISCONNECTED, /**< Network sloj odspojen */
    NETWORK_EVENT_CONNECTING, /**< Network sloj u procesu spajanja */
    NETWORK_EVENT_ERROR /**< Pogreška na network sloju */
} NetworkEvent;

/**
 * @enum HalNetworkStatus
 * @brief Status operacija na network sloju.
 * 
 */
typedef enum {
    HAL_NETWORK_OK, /**< Operacija nad funkcijom network sloja uspješna */
    HAL_NETWORK_ERROR, /**< Operacija nad funkcijom network sloja neuspješna */
    HAL_NETWORK_TIMEOUT, /**< Istek vremena kod izvršavanja operacije nad funkcijom network sloja */
    HAL_NETWORK_INVALID_STATE /**< Neki od podsustava network sloja u krivom stanju */
} HalNetworkStatus;

/**
 * @struct hal_network_config
 * @brief Struktura konfiguracije HAL network sloja.
 * 
 * Kombinira strukture podsustava network sloja (sloj podatkovne poveznice, transportni sloj, aplikacijski sloj).
 * 
 */
typedef struct {
    char ssid[32]; /**< SSID mreže */
    char pass[64]; /**< Lozinka mreže */
    char uri[64]; /**< URI servera */
    int port; /**< Port na kojem server sluša */
} hal_network_config;

/**
 * @typedef on_network_event
 * @brief Pokazivač na funkciju app sloja koju HAL sloj poziva kada se desi neki network event.
 * 
 */
typedef void (*on_network_event)(NetworkEvent nw_event);

typedef void (*on_network_data)(const uint8_t* data, size_t len);

/**
 * @brief Poziva se kako bi HAL dobio pokazivač na application funkciju koju poziva kod
 * network eventova.
 * 
 * @param app_cb Pokazivač na application funkciju za obradu network eventova
 */
void hal_bind_network_callback(on_network_event app_cb, on_network_data app_nw_cb);

/**
 * @brief Šalje podatke na platform sloj koji ih šalje preko mreže.
 * 
 * @param packet Pokazivač na podatke (u bajtovima)
 * @param packet_len Duljina podataka u bajtovima
 */
void hal_network_send(uint8_t* packet, size_t packet_len);

/**
 * @brief Inicijalizira network sustav na HAL sloju.
 * 
 * @param configuration 
 * @return Status operacije nad HAL modulom
 */
HalNetworkStatus hal_network_init(hal_network_config* configuration);

/**
 * @brief Pokreće network sustav na HAL sloju.
 * 
 * @return Status operacije nad HAL modulom
 */
HalNetworkStatus hal_network_start(void);

/**
 * @brief Zaustavlja network sustav na HAL sloju.
 * 
 * @return Status operacije nad HAL modulom
 */
HalNetworkStatus hal_network_stop(void);

/**
 * @brief Deinicijalizira network sustav na HAL sloju.
 * 
 * @return Status operacije nad HAL modulom
 */
HalNetworkStatus hal_network_deinit(void);