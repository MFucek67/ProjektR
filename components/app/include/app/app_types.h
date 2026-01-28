#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Veličina queue-a za evente aplikacijskog sloja.
 * 
 */
#define APP_EVENT_QUEUE_LEN 20

/**
 * @brief Maksimalna dopuštena duljina response payloada (u bajtovima).
 * 
 */
#define MAX_RESPONSE_DATA_LEN 64

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
    CM_TIME_FOR_NO_PERSON,
    EXISTENCE_JUDGMENT_THRESH_I,
    MOTION_TRIGGER_THRESH_I,
    EXISTENCE_PERCEPTION_BOUND_I,
    MOTION_TRIGGER_BOUND_I,
    MOTION_TRIGGER_TIME_I,
    MOTION_TO_STILL_TIME_I,
    CM_TIME_FOR_NO_PERSON_I
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
 * @struct uof_report
 * @brief Underlying Open Function (UOF) report.
 * 
 */
typedef struct
{
    int existence_energy; /**< Existence energy */
    float static_distance; /**< Static distance */
    int motion_energy; /**< Motion energy */
    float motion_distance; /**< Motion distance */
    float motion_speed; /**< Motion speed */
} uof_report;

/**
 * @struct DecodedReport
 * @brief Dekodirani report senzora.
 * 
 * Samo onaj flag koji predstavlja vrstu podatka koju report nosi je postavljen na true,
 * svi ostali su na false.
 * 
 * @note Sukladno postavljenom flagu, očekuje se da aplikacija čita tu vrstu podatka.
 * 
 */
typedef struct {
    bool has_init_completed_info; /**< Flag o init completed info */
    bool has_presence_info; /**< Flag o presence info */
    bool has_motion_info; /**< Flag o motion info */
    bool has_bmp_info; /**< Flag o BodyMotionParameter (BMP) info */
    bool has_proximity_info; /**< Flag o proximity info */
    bool has_uof_report; /**< Flag o UOF report */

    bool init_completed_info; /**< Init completed info podatak */
    PresenceInfo presence_info; /**< Presence info podatak */
    MotionInfo motion_info; /**< Motion info podatak */
    int bmp_info; /**< BodyMotionParameter (BMP) podatak */
    ProximityInfo proximity_info; /**< Proximity info podatak */
    uof_report uof_rep; /**< UOF report podatak */
} DecodedReport;

/**
 * @struct DecodedResponse
 * @brief Dekodirani odgovor na prethodni upit.
 * 
 * Koristi se za odgovore senzora na konfiguracijske i informacijske upite.
 * 
 * @warning Svaki response s payloadom većim od MAX_RESPONSE_DATA_LEN bit će odbačen.
 * 
 */
typedef struct {
    AppInquiryType type; /**< Tip odgovora */
    uint8_t data[MAX_RESPONSE_DATA_LEN]; /**< Polje s podatke odgovora */
    size_t data_l; /**< Duljina podataka odgovora */
} DecodedResponse;

/**
 * @typedef MMwaveReportCallback
 * @brief Callback funkcija za notifikacije vanjskog programa o novim report-ovima.
 * 
 * @param event Pokazivač na mmWave report strukturu
 */
typedef void (*MMwaveReportCallback)(DecodedReport* report);

/**
 * @typedef MMwaveResponseCallback
 * @brief Callback funkcija za notifikacije vanjskog programa o novim response-ovima.
 * 
 * @param event Pokazivač na mmWave response strukturu
 */
typedef void (*MMwaveResponseCallback)(DecodedResponse* response);