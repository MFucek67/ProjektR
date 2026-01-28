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
#include "app/app_mmwave_decoder.h"
#include "app/app_mmwave_constants.h"
#include "app/app_mmwave.h"
#include "app/app_types.h"

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
 * @brief Deinicijalizira sustav.
 * 
 * Funkcija briše HAL konfiguraciju i mmWave core interface te kontekst s callbackovima koje
 * koristi aplication decoder. Resetira interne varijable i postavlja sustav u neinicijalizirano stanje.
 * 
 * @return Status operacije nad modulom
 */
AppSensorStatus app_deinit_sys(void);

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
void mmwave_register_event_callback(MMwaveResponseCallback res_cb, MMwaveReportCallback rep_cb);

/**
 * @brief Dohvaća response event iz queue-a.
 * 
 * Kopira response u poslanu strukturu i oslobađa memoriju koju je on zauzimao.
 * 
 * @param out_event Pokazivač na strukturu aplikacijskog response
 * @param timeout_ms Vrijeme čekanja u ms
 * @return true ako je vraćen response
 * @return false ako nije vraćen response
 */
bool app_get_response(DecodedResponse* out_event, uint32_t timeout_ms);

/**
 * @brief Dohvaća report event iz queue-a.
 * 
 * Kopira report u poslanu strukturu i oslobađa memoriju koju je on zauzimao.
 * 
 * @param out_event Pokazivač na strukturu aplikacijskog reporta
 * @param timeout_ms Vrijeme čekanja u ms
 * @return true ako je vraćen report
 * @return false ako nije vraćen report
 */
bool app_get_report(DecodedReport* out_report, uint32_t timeout_ms);

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