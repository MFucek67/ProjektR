/**
 * @file app_mmwave_decoder.h
 * @author Marko Fuček
 * @brief Dekoder mmWave podataka iz okvira na aplikacijskom sloju.
 * 
 * Ovaj modul definira strukture i API za dekodiranje payloada i određivanje vrste response-a ili
 * reporta. Modul koristi već parsirane i protokolski odvojene semantički korisne podatke koje tumači
 * i pomoću kojih gradi strukture iz kojih podatci postaju korisni nekoj aplikaciji koja koristi
 * kranji API.
 * 
 * Decoder pripada aplikacijskom sloju, ali je potpuno neovisan o platformi i HAL-u.
 * 
 * @note Ulaz u modul su bajtovi, a izlaz semantički potpuno protumačeni podatci u strukturama
 * koje predstavljaju response-ove i report-ove a šalju se prema aplikaciji putem callbacka.
 *  
 * @version 0.1
 * @date 2026-01-23
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "app_mmwave_constants.h"
#include "app_mmwave_manager.h"

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
 */
typedef struct {
    AppInquiryType type; /**< Tip odgovora */
    void* data; /**< Pokazivač na podatke odgovora */
    size_t data_l; /**< Duljina podataka odgovora */
} DecodedResponse;

/**
 * @struct AppDecorerContext
 * @brief Kontekst dekodera.
 * 
 * Sadrži callbackove koje application manager implementira, a preko njih
 * decoder šalje dekodirane responsove i reportove.
 * 
 */
typedef struct
{
    /**
     * @brief Callback za slanje dekodiranog reporta.
     * @param report Pokazivač na strukturu dekodiranog reporta
     */
    void (*sendReportCallback)(DecodedReport* report);

    /**
     * @brief Callback za slanje dekodiranog responsea.
     * @param response Pokazivač na strukturu dekodiranog responsea
     */
    void (*sendResponseCallback)(DecodedResponse* response);
} AppDecoderContext;

/**
 * @brief Inicijalizira mmWave dekoder.
 * 
 * Sprema kontekst s callbackovima koji se koriste za isporuku
 * dekodiranih podataka aplikaciji.
 * 
 * @param ctx Pokazivač na decoder context
 */
void app_mmwave_decoder_init(AppDecoderContext* ctx);

/**
 * @brief Deinicijalizira mmWave dekoder.
 * 
 * Briše interni kontekst i onemogućava daljnje dekodiranje podataka.
 * 
 */
void app_mmwave_decoder_deinit();

/**
 * @brief Obrada podataka iz jednog frame-a.
 * 
 * Funkcija dekodira primljene podatke na temelju ctrl_w (data[0]) i cmd_w (data[1]),
 * te sprema određeni payload u strukture ovisno o tome je li riječ o reportu ili
 * response-u i sukladno tome poziva određeni callback za spremanje strukture.
 * 
 * @param data Semantički korisni podatci jednog frame-a
 * @param data_len Duljina semantički korisnih podataka jednog frame-a
 */
void app_mmwave_decoder_process_frame(uint8_t* data, size_t data_len);