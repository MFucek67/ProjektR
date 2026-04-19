/**
 * @file wifi_client.h
 * @author Marko Fuček
 * @brief Platform API za upravljanje WiFi konekcijom i driverom za WiFi.
 * 
 * Ovaj modul pruža podršku za upravljanje povezivanjem uređaja na WiFi, te upravljanje WiFi
 * konekcijom i konačno odspajanjem i oslobađanjem resursa.
 * 
 * @note Važan dio ovog sustava je registracija callbacka na hal sloj - pokazivač na funkciju
 * na hal sloju koja upravlja aplikacijskim protokolom. Pošto WiFi nudi samo mrežni sloj podatkovne poveznice,
 * potreban je dio sustava koji će urediti i implementirati ostale slojeve internetskog modela.
 * 
 * @version 0.1
 * @date 2026-04-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "stdio.h"
#include "my_hal/hal_wifi_interface.h"

/**
 * @enum WiFiState
 * @brief Stanje WiFi konekcije (drivera).
 * 
 */
typedef enum {
    WIFI_STATE_UNINIT, /**< WiFi nije inicijaliziran */
    WIFI_STATE_INIT, /**< WiFi inicijaliziran */
    WIFI_STATE_CONFIGURED, /**< Mreža je konfigurirana */
    WIFI_STATE_STARTED, /**< WiFi driver pokrenut i konekcija na mrežnoj poveznici obavljena */
    WIFI_STATE_CONNECTING, /**< Dohvaćanje IP-adrese */
    WIFI_STATE_CONNECTED, /**< Klijent je dobio valjanu WiFi adresu i klijent je spojen u mrežu */
    WIFI_STATE_DISCONNECTED, /**< Klijent je izgubio WiFi adresu - WiFi zaustavljen */
    WIFI_STATE_ERROR /**< Pogreška u WiFi sustavu */
} WiFiState;

/**
 * @enum WiFiStatus
 * @brief Status operacije nad WiFi driverom.
 * 
 */
typedef enum {
    WIFI_OK, /**< Operacija nad WiFi driverom uspješna */
    WIFI_WRONG_STATE, /**< WiFi driver je u pogrešnom stanju */
    WIFI_ERROR /**< Operacija nad WiFi driverom neuspješna */
} WiFiStatus;

/**
 * @struct wifi_config
 * @brief Konfiguracija mreže koju zadaje korisnik.
 * 
 */
typedef struct {
    char ssid[32]; /**< ssid mreže */
    char pass[64]; /**< zaporka mreže */
} wifi_config;

/**
 * @brief Inicijalizacija WiFi drivera.
 * 
 * Obavlja stvaranje NVS-a za postavke drivera, postavljanje TCP/IP stacka i stvaranje event loopa za
 * slanje WiFi eventova, te registraciju handlera za eventove.
 * 
 * @return Status operacije nad WiFi driverom
 */
WiFiStatus wifi_client_init(void);

/**
 * @brief Konfiguracija mreže prema zadanim postavkama.
 * 
 * @param config Konfiguracija mreže koju zadaje korisnik
 * @return Status operacije nad WiFi driverom
 */
WiFiStatus wifi_client_configure_network(wifi_config *config);

/**
 * @brief Pokretanje WiFi drivera i aktivacija WiFi hardware.
 * 
 * Nakon ovog koraka WiFi hardware je aktivan, ali još nemamo IP adresu - čekamo DHCP dodjelu.
 * 
 * @note Kada dobijemo IP adresu, doći će event IP_EVENT_STA_GOT_IP.
 * 
 * @return Status operacije nad WiFi driverom
 */
WiFiStatus wifi_client_start_and_connect(void);

/**
 * @brief Prekidanje WiFi konekcije i invalidiranje IP adrese.
 * 
 * @return Status operacije nad WiFi driverom
 */
WiFiStatus wifi_client_disconnect(void);

/**
 * @brief Oslobađanje resursa i deinicijalizacija WiFi drivera.
 * 
 * @return Status operacije nad WiFi driverom
 */
WiFiStatus wifi_client_uninit(void);

/**
 * @brief Vraća state u kojem se nalazi WiFi konekcija.
 * 
 * @return Stanje WiFi konekcije
 */
WiFiState get_wifi_client_state(void);

/**
 * @brief Registrira callback na HAL sloj - preko njega javlja HAL-u što se dogodilo s WiFi konekcijom.
 * 
 * Registiramo pokazivač na HAL funkciju koja ovisno o WiFi eventu dalje obavlja potrebne radnje na višim
 * slojevima.
 * 
 * @param hal_callback Pokazivač na HAL funkciju koja reagira na WiFi evente
 * @return Status operacije nad WiFi driverom
 */
WiFiStatus wifi_register_hal_callback(on_wifi_event hal_callback);