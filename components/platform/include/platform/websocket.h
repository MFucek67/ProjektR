/**
 * @file websocket.h
 * @author Marko Fuček
 * @brief Platform API za upravljanje Web Socketima.
 * 
 * Ovaj modul pruža Web Socket client sučelje. Konkretno, pruža opcije
 * inicijalizacije, pokretanja, zaustavljanja, deinicijalizacije i slanja
 * podataka putem Web Socketa.
 * 
 * @note Važan dio ovog API-ja je registracija callbacka na hal sloj - pokazivača
 * na funkcije koje se pozivaju u slučaju uspješne uspostave web socket konekcije,
 * pucanja web socket konekcije, te primanja podataka preko web socketa i error handlera.
 * 
 * @version 0.1
 * @date 2026-04-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "stdio.h"
#include "esp_websocket_client.h"
#include "my_hal/hal_ws_interface.h"

/**
 * @enum webSocketState
 * @brief Stanje web socket konekcije (drivera).
 * 
 */
typedef enum {
    WS_UNINIT, /**< Web Socket nije inicijaliziran */
    WS_INIT, /**< Web Socket inicijaliziran */
    WS_STARTED, /**< Web Socket pokrenut (konekcija ostvarena) */
    WS_CLOSED, /**< Web Socket ugašen (konekcija prekinuta) */
    WS_ERROR_STATE /**< Error web socketa */
} webSocketState;

/**
 * @enum webSocketStatus
 * @brief Status operacije nad Web Socketom.
 * 
 */
typedef enum {
    WS_OK, /**< Operacija nad Web Socketom uspješna */
    WS_WRONG_STATE, /**< Web Socket sustav se nalazi u pogrešnom stanju */
    WS_ERROR /**< Operacija nad Web Socketom neuspješna */
} webSocketStatus;

/**
 * @struct ws_config
 * @brief Konfiguracija Web Socketa koju zadaje korisnik.
 * 
 */
typedef struct {
    char uri[64]; /**< URI na koji se spajamo putem Web Socketa */
    int port; /**< Port servera na koji se spajamo putem Web Socketa */
} ws_config;

/**
 * @brief Apstrakcija pokazivača na Web Socket klijent objekt.
 * 
 */
typedef esp_websocket_client_handle_t websocket_handler;

/**
 * @brief Inicijalizira Web Socket objekt i postavlja parametre Web Socket klijenta.
 * 
 * @param config Konfiguracija web socket klijent objekta
 * @return Status operacije nad Web Socketom
 */
webSocketStatus ws_client_init(ws_config *config);

/**
 * @brief Pokreće Web Socket konekciju između klijenta i URI-ja danog u konfiguraciji.
 * 
 * @return Status operacije nad Web Socketom
 */
webSocketStatus ws_client_start(void);

/**
 * @brief Šalje podatke s klijenta na URI dan u konfiguraciji preko Web Socketa.
 * 
 * @param data Pokazivač na bajtove podataka
 * @param len Duljina poslanih podataka u bajtovima
 * @param timeout_in_ms Vrijeme koje se čeka na slanje
 * @return Status operacije nad Web Socketom 
 */
webSocketStatus ws_send(uint8_t* data, int len, uint32_t timeout_in_ms);

/**
 * @brief Gasi Web Socket konekciju između klijenta i URI-ja danog u konfiguraciji.
 * 
 * @return Status operacije nad Web Socketom
 */
webSocketStatus ws_client_stop(void);

/**
 * @brief Deinicijalizira i briše objekt na Web Socket klijenta.
 * 
 * @return Status operacije nad Web Socketom
 */
webSocketStatus ws_client_uninit(void);

/**
 * @brief Vraća state u kojem se nalazi Web Socket konekcija.
 * 
 * @return Stanje konekcije (drivera)
 */
webSocketState get_ws_client_state(void);

/**
 * @brief Registrira callbackove na HAL sloj - preko njih javlja HAL-u što se dogodilo s Web Socketom.
 * 
 * Konkretno, registira se tri callbacka preko kojih se HAL sloju dojavljuju zbivanja s Web Socketom:
 * 1. hal_cb_on_data -> poziva se kada se preko Web Socketa prime podaci
 * 2. hal_cb_on_err -> poziva se kada se dogodi error u slanju/primanju podataka preko Web Socketa
 * 3. hal_cb_on_start_stop -> poziva se kod otvaranja/zatvaranja Web Socket konekcije, bilo namjernog bilo nenamjernog
 * 
 * @param hal_cb_on_data Pokazivač na funkciju HAL-sloja za data event
 * @param hal_cb_on_err Pokazivač na funkciju HAL-sloja za error event
 * @param hal_cb_on_start_stop Pokazivač na funkciju HAL-sloja za start/stop event 
 * @return Status operacije nad Web Socketom
 */
webSocketStatus ws_register_hal_callback(on_ws_data hal_cb_on_data, on_ws_error hal_cb_on_err, on_ws_start_stop hal_cb_on_start_stop);