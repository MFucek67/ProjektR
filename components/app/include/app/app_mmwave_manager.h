/**
 * @file app_mmwave_manager.h
 * @author Marko Fuček
 * @brief Upravljačka logika application sloja.
 * 
 * Ovaj modul pruža svu upravljačku logiku aplikacijskog sloja. Sadrži tipove za stanja i statuse sustava,
 * logiku za inicijalizaciju, pokretanje i zaustavljanje sustava, drži task za obradu i tumačenje eventova 
 * koji drže podatke, funkcije koje se pozivaju kada je protumačen response ili report te općenitu funkciju
 * za slanje upita na mmWave modul.
 * 
 * Modul ne sadrži implementaciju komunikacije niti dekodiranja podataka, već je zadužen da upravlja radom
 * ostalih aplikacijskih komponenti.
 * 
 * @version 0.1
 * @date 2026-01-23
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "app_mmwave_decoder.h"
#include "app_mmwave_constants.h"
#include "app_mmwave.h"

/**
 * @brief Veličina queue-a za evente aplikacijskog sloja.
 * 
 */
#define APP_EVENT_QUEUE_LEN 20

/**
 * @enum AppSensorStatus
 * @brief Statusi rada mmWave modula na aplikacijskom sloju.
 * 
 */
typedef enum {
    APP_SENSOR_OK, /**< Operacija uspješna */
    APP_SENSOR_ERROR, /**< Operacija neuspješna */
    APP_SENSOR_INVALID_STATE, /**< Modul se nalazi u pogrešnom stanju */
    APP_SENSOR_BUSY, /**< Sustav zauzet */
    APP_SENSOR_BAD_ARGUMENT, /**< Neispravan argument funkcije */
    APP_SENSOR_BAD_MODE /**< Neispravan mode */
} AppSensorStatus;

/**
 * @enum AppSensorState
 * @brief Stanje u kojem se nalazi mmWave sustav na aplikacijskom sloju.
 * 
 */
typedef enum {
    APP_SENSOR_UNINIT, /**< Sustav neinicijaliziran */
    APP_SENSOR_INIT, /**< Sustav inicijaliziran */
    APP_SENSOR_RUNNING, /**< Sustav radi */
    APP_SENSOR_STOPPED /**< Sustav je stao s radom */
} AppSensorState;

/**
 * @enum SensorOperationMode
 * @brief Načini rada (mode) sustava.
 * 
 */
typedef enum {
    SENSOR_MODE_STANDARD, /**< Standardni način rada */
    SENSOR_MODE_UNDERLYING_OPEN /**< Napredni način rada */
} SensorOperationMode;

/**
 * @enum AppInquiryType
 * @brief Tipovi aplikacijskih upita na sustav.
 * 
 * Vrijednosti se koriste za identifikaciju response-a koje aplikacija očekuje
 * od mmWave modula.
 * 
 */
typedef enum {
    NO_TYPE,
    HEARTBEAT,
    MODULE_RESET,
    PRODUCT_MODEL,
    PRODUCT_ID,
    INIT_STATUS,
    HARDWARE_MODEL,
    FIRMWARE_VERSION,
    SCENE_SETTINGS,
    SCENE_SETTINGS_I,
    SENSITIVITY,
    SENSITIVITY_I,
    PRESENCE,
    MOTION,
    BMP,
    TIME_FOR_NO_PERSON,
    TIME_FOR_NO_PERSON_I,
    PROXIMITY,
    OUTPUT_SWITCH,
    OUTPUT_SWITCH_I,
    EXISTENCE_ENERGY,
    MOTION_ENERGY,
    STATIC_DISTANCE,
    MOTION_DISTANCE,
    MOTION_SPEED,
    CUSTOM_MODE,
    CUSTOM_MODE_END,
    CUSTOM_MODE_I,
    EXISTENCE_JUDGMENT_THRESH,
    MOTION_TRIGGER_THRESH,
    EXISTENCE_PERCEPTION_BOUND,
    MOTION_TRIGGER_BOUND,
    MOTION_TRIGGER_TIME,
    MOTION_TO_STILL_TIME,
    CM_TIME_FOR_NO_PERSON
} AppInquiryType;

/**
 * @enum MmwaveEventType
 * @brief Vrsta eventa.
 * 
 * Event queue na aplikacijskom sloju može primiti ili response ili report tip. Ovaj enum
 * definira ta dva tipa. 
 * 
 */
typedef enum {
    MMWAVE_EVENT_REPORT, /**< Report tip eventa */
    MMWAVE_EVENT_RESPONSE /**< Response tip eventa */
} MmwaveEventType;

/**
 * @struct MmwaveEvent
 * @brief Event na aplication sloju.
 * 
 * Ova struktura predstavlja event koji aplikacijski sloj šalje kao rezultat response-a ili reporta.
 * 
 * @note U jednom eventu može biti samo jedna vrsta eventa - dakle, ili report ili response.
 * 
 */
typedef struct {
    MmwaveEventType type; /**< Vrsta eventa */
    union
    {
        DecodedReport report; /**< Dekodirani report */
        DecodedResponse response; /**< Dekodirani response */
    };
} MmwaveEvent;

/**
 * @typedef MMwaveEventCallback
 * @brief Callback funkcija za notifikacije vanjskog programa o novim eventima.
 * 
 * @param event Pokazivač na MmwaveEvent strukturu
 */
typedef void (*MMwaveEventCallback)(MmwaveEvent* event);

/**
 * @brief Pokreće sustav.
 * 
 * Funkcija pokreće HAL i stvara task koji upravlja decoderom. Postavlja interno stanje
 * sustava u radno.
 * 
 * @return Status operacije nad modulom
 */
AppSensorStatus app_start_sys(void);

/**
 * @brief Inicijalizira modul i priprema sustav za rad.
 * 
 * Funkcija dohvaća HAL konfiguraciju i mmWave core interface te kontekst s callbackovima koje
 * koristi aplication decoder. Resetira interne varijable i postavlja sustav u inicijalizirano stanje.
 * 
 * @return Status operacije nad modulom
 */
AppSensorStatus app_init_sys(void);

/**
 * @brief Zaustavlja rad sustava.
 * 
 * Funkcija zaustavlja rad HAL-a, čeka na završetak rada dekodera (graceful stop) te deinicijalizira
 * aplication decoder. Postavlja sustav u neinicijalizirano stanje.
 * 
 * @return Status operacije nad modulom
 */
AppSensorStatus app_stop_sys(void);

/**
 * @brief Vraća trenutni način rada (mode) sustava.
 * 
 * @return Trenutni način rada (mode) sustava
 */
SensorOperationMode app_get_mode(void);

/**
 * @brief Postavlja način rada (mode) sustava.
 * 
 * @param mode Način rada sustava koji se postavlja
 * @return Status operacije nad modulom
 */
AppSensorStatus app_set_mode(SensorOperationMode mode);

/**
 * @brief Registracija callbackova vanjskog programa.
 * 
 * Prosljeđuje pokazivač vanjske funkcije koji aplicaton manager poziva u slučaju dodavanja
 * reporta ili response-a u queue.
 * 
 * @param cb Callback vanjske funkcije
 */
void mmwave_register_event_callback(MMwaveEventCallback cb);

/**
 * @brief Dohvaća response/report event iz queue-a.
 * 
 * Kopira event u poslanu strukturu i oslobađa memoriju koju je event zauzimao.
 * 
 * @param out_event Pokazivač na strukturu aplikacijskog eventa (report/response)
 * @param timeout_ms Vrijeme čekanja u ms
 * @return true ako je vraćen event
 * @return false ako nije vraćen event
 */
bool app_get_event(MmwaveEvent* out_event, uint32_t timeout_ms);

/**
 * @brief Funkcija koja se poziva kod response eventa.
 * 
 * Funkcija se poziva preko callbacka iz aplication decodera kod dekodiranja response eventa.
 * Funkcija na poziv stavlja event u interni queue u manageru, te, ako je zadan, poziva callback
 * vanjskog programa. 
 * 
 * @param response Pokazivač na response event strukturu
 */
void onResponse(DecodedResponse* response);

/**
 * @brief Funkcija koja se poziva kod report eventa.
 * 
 * Funkcija se poziva preko callbacka iz aplication decodera kod dekodiranja report eventa.
 * Funkcija na poziv stavlja event u interni queue u manageru, te, ako je zadan, poziva callback
 * vanjskog programa.
 * 
 * @param report Pokazivač na report event strukturu
 */
void onReport(DecodedReport* report);

/**
 * @brief Šalje aplikacisjki inquiry (upit) na mmWave modul.
 * 
 * Poziva HAL funkciju za slanje frame-a, a HAL na sebe preuzima daljnji posao izgradnje frame-a
 * i slanja frame-a.
 * 
 * @param data Pokazivač na podatke koji se šalju
 * @param data_len Duljina podataka koji se šalju
 * @param ctrl_w Control word
 * @param cmd_w Command word
 * @return AppSensorStatus 
 */
AppSensorStatus app_send_inquiry(const uint8_t* data, size_t data_len, const uint8_t ctrl_w, const uint8_t cmd_w);