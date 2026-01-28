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
#include "app/app_mmwave_constants.h"
#include "app/app_types.h"

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