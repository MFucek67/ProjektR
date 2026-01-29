/**
 * @file mmwave_core_interface.h
 * @author Marko Fuček
 * @brief Javni API i sučelje mmWave core sloja
 * 
 * U ovom file-u definirano je callback sučelje prema HAL sloju i callback sučelje funkcijskog API-ja
 * za mmWave core sloj.
 * 
 * mmWave core sloj ne upravlja taskovima, UART-om niti memorijom, ne poznaje
 * detalje HAL-a i implementacije platforme, već samo koristi HAL preko callbackova.
 * 
 * HAL sloj je zadužen za kontrolu nad memorijom. HAL sloj implementira funkcije koje
 * mmWave core poziva preko callbacka, te on jedini smije pozivati mmWave core API.
 * 
 * @note Ovim header file-om definira se granica između HAL i mmWave core sloja.
 * 
 * @version 0.1
 * @date 2026-01-22
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "mmwave_interface/mmwave_core_types.h"


/**
 * @brief Callback za spremanje semantički korisnih podataka iz parsiranog frame-a.
 * 
 * Funkcija koju mmWave core preko pokazivača poziva kad pronađe ispravan frame.
 * Funkciju implementira HAL sloj.
 * 
 * HAL sloj nakon poziva ove funkcije preuzima vlasništvo nad memorijom frame-a.
 * 
 * @param frame_data Pokazivač na strukturu sa semantički korisnim podacima iz parsiranog frame-a
 * @return true ako su podatci uspješno spremljeni
 * @return false ako podatci nisu uspješno spremljeni
 * 
 */
typedef bool (*mmWave_saveFrame)(const mmWaveFrameSemanticData* frame_data);

/**
 * @brief Callback za alokaciju memorije.
 * 
 * mmWave core koristi ovaj callback za alokaciju memorije bez da zna stvarni mehanizam heap-a
 * i način dodjele.
 * Funkciju implementira HAL sloj.
 * 
 * @param byte_size Broj bajtova za alokaciju
 * @return Pokazivač na alociranu memoriju ili NULL u slučaju neuspjeha
 * 
 */
typedef uint8_t* (*mmWave_alloc_memory)(size_t byte_size);

/**
 * @brief Callback za oslobađanje memorije.
 * 
 * mmWave core poziva ovu funkviju preko pokazivača kada treba osloboditi alociranu memoriju.
 * Funkciju implementira HAL sloj.
 * 
 * @param mem Pokazivač na memoriju koja se treba osloboditi
 * @param mem_size Veličina memorije koja se oslobađa u bajtovima
 * 
 */
typedef void (*mmWave_free_alloc_memory)(uint8_t* mem, size_t mem_size);

/**
 * @brief Public API callback mmWave core sloja za parsiranje ulaznih podataka.
 * 
 * Funkcija analizira ulazne bajtove i inerno gradi frame.
 * Kada se frame prepozna, iz nje se poziva mmWave_saveFrame callback.
 * 
 * Funkciju implementira mmWave core sloj.
 * 
 * @param data Pokazivač na ulazne podatke
 * @param data_len Duljina ulaznih podataka u bajtovima
 * @return Status parsiranja
 * 
 */
typedef mmwave_frame_status_t (*mmWave_parse_data)(const uint8_t* data, size_t data_len);

/**
 * @brief Public API callback mmWave core sloja za izgradnju frame-a koji se šalje na TX.
 * 
 * Funkcija iz ulaznih podataka stvara konkretan frame, dodajući headere, footere i
 * checksum (poznati samo mmWave core sloju).
 * 
 * Funkciju implementira mmWave core sloj.
 * 
 * @param payload Pokazivač na payload podatke
 * @param payload_len Duljina payload podataka u bajtovima
 * @param ctrl_w Control word
 * @param cmd_w Command word
 * @return Pokazivač na strukturu kreiranog frame-a ili NULL
 * 
 */
typedef bool (*mmWave_build_frame)(mmWaveFrameForTX* out, const uint8_t* payload, size_t payload_len,
    const uint8_t ctrl_w, const uint8_t cmd_w);

/**
 * @brief Public API callback mmWave core sloja za inicijalizaciju core sloja prije startanja.
 * 
 * Funkcija restarta parser i postavlja na početnu vrijednost interne varijable koje parser koristi.
 * 
 * Funkciju implementira mmWave core sloj. 
 * 
 * @return Status operacije nad core parserom
 * 
 */
typedef mmwave_status_t (*mmWave_init)(void);

/**
 * @brief Public API callback mmWave core sloja za zaustavljanje rada core sloja.
 * 
 * Funkcija trenutno zaustavlja rad parsera. Čisti memoriju koja je bila zauzeta u trenutku prekida
 * i vraća sve interne varijable na neinicijalizirano stanje.
 * 
 * Funkciju implementira mmWave core sloj.
 * 
 * @return Status operacije nad core parserom
 * 
 */
typedef mmwave_status_t (*mmWave_stop)(void);

/**
 * @struct mmWave_core_callback
 * @brief Strukutra callbackova koje implementira HAL sloj.
 * 
 * Ovu strukutru HAL sloj prosljeđuje mmWave core sloju, te on "binda" tj. povezuje
 * svoje implementacije na callbackove, kako bi core sloj mogao pozivati njegove funkcije
 * bez znanja njihovih implementacija.
 * 
 */
typedef struct
{
    mmWave_saveFrame mmwave_save_frame;
    mmWave_alloc_memory alloc_mem;
    mmWave_free_alloc_memory free_mem;
} mmWave_core_callback;

/**
 * @struct mmWave_core_interface
 * @brief Struktura koja sadrži API funkcije mmWave core sloja u obliku callbackova.
 * 
 * HAL sloj koristi ovu strukturu za pozivanje funkcija mmWave core sloja.
 * 
 */
typedef struct
{
    mmWave_parse_data mmwave_parse_data;
    mmWave_build_frame mmwave_build_frame;
    mmWave_init mmwave_core_init;
    mmWave_stop mmwave_core_stop;
} mmWave_core_interface;

/**
 * @brief Povezuje HAL funkcije s callbackovima koje mmWave core poziva.
 * 
 * Poziva HAL sloj kod inicijalizacije kako bi mmWave core sloju pružio prave pokazivače na funkcije.
 * 
 * @param cb Struktura s callbackovima za HAL sloj
 */
void mmwave_core_bind_callbacks(const mmWave_core_callback* cb);