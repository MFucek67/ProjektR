/**
 * @file hal_mmwave.h
 * @author Marko Fuček
 * @brief HAL sloj za upravljanje mmWave senzorom.
 * 
 * Ovaj modul predstavlja Hardware Abstraction Layer (HAL) za komunikaciju s mmWave modulom
 * preko UART sučelja.
 * 
 * Sastoji se od:
 * Funkcija za inicijalizaciju i deinicijalizaciju platform resursa.
 * Funkcija za upravljanje lifecycleom mmWave modula.
 * Funkcija za slanje evenata na mmWave core i prihvaćanje evenata s app sloja.
 * Vlasnik je memorije dolaznih frame-ova te on vrši alokaciju i oslobađanje memorije.
 * 
 * Modul je dizajniran kao state machine te stoga većina funkcija provjerava valjanost
 * trenutnog stanja prije izvršavanja funkcije.
 * 
 * @note Modul ne sadrži hardware specific funkcije - one su zadane u platform sloju
 * i HAL sloj se njima samo koristi.
 * 
 * @version 0.1
 * @date 2026-01-21
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "stdio.h"
#include "stdint.h"
#include "mmwave_interface/mmwave_core_interface.h"
#include "mmwave_interface/mmwave_core_types.h"
#include "hal_mmwave_types.h"


/**
 * @brief Inicijalizira mmWave HAL modul.
 * 
 * Funkcija vrši povezivanje i slanje pokazivača na svoje (HAL) funkcije s mmWave_core slojem,
 * inicijalizaciju platform UART-a, čišćenje buffera i event queue-a te stvaranje internih queue-ova
 * za frames i tx task.
 * 
 * @param configuration Pokazivač na konfiguracijsku strukturu
 * @param core_api Pokazivač na mmWave core interface
 * @return HAL_MMWAVE_OK ako je inicijalizacija uspješno obavljena
 * @return HAL_MMWAVE_ERROR ako je inicijalizacija u nekom koraku zakazala
 * @return HAL_MMWAVE_INVALID_STATE ako je modul u stanju iz kojeg se ne smije ponovno inicijalizirati
 * ili je inicijalizacija već prije izvršena
 */
HalMmwaveStatus hal_mmwave_init(hal_mmwave_config* configuration, mmWave_core_interface* core_api);

/**
 * @brief Pokreće HAL mmWave modul.
 * 
 * Funkcija resetira interne zastavice za taskove, pokreće ISR za UART, pokreće platform converter task,
 * te pokreće sender i receiver taskove - efektivno započinje primanje, obradu i slanje frame-ova.
 * 
 * @return HAL_MMWAVE_OK ako je modul uspješno pokrenut
 * @return HAL_MMWAVE_ERROR ako je modul neuspješno pokrenut
 * @return HAL_MMWAVE_INVALID_STATE ako je modul u stanju iz kojeg se ne smije pokrenuti ili već pokrenut
 */
HalMmwaveStatus hal_mmwave_start(void);

/**
 * @brief Zaustavlja rad HAL mmWave modula.
 * 
 * Funkcija zaustavlja rad ISR-a, čeka na obradu preostalih frame-ova u sustavu te nakon toga
 * gasi sender i receiver taskove, oslobađa njihovu memoriju te postavlja modul u STOPPED stanje.
 * 
 * @return HAL_MMWAVE_OK ako je modul uspješno zaustavljen
 * @return HAL_MMWAVE_ERROR ako je modul neuspješno zaustavljen
 * @return HAL_MMWAVE_INVALID_STATE ako je modul u stanju iz kojeg se ne smije zaustaviti ili već zaustavljen
 */
HalMmwaveStatus hal_mmwave_stop(void);

/**
 * @brief Deinicijalizira mmWave HAL modul.
 * 
 * Funkcija oslobađa sve alocirane resurse (interni queue-ovi i UART driver) te postavlja interne
 * varijable u NULL stanje.
 * 
 * @return HAL_MMWAVE_OK ako je deinicijalizacija uspješno obavljena
 * @return HAL_MMWAVE_ERROR ako je deinicijalizacija u nekom koraku zakazala
 * @return HAL_MMWAVE_INVALID_STATE ako je modul u stanju iz kojeg se ne smije deinicijalizirati ili
 * je već deinicijaliziran
 */
HalMmwaveStatus hal_mmwave_deinit(void);

/**
 * @brief Šalje korisničke podatke preko TX pina.
 * 
 * Funkcija prima podatke za frame s aplikacijskog sloja te koristi mogućnosti mmWave_core modula
 * čiji API poziva za wrappanje podataka, pripadajućeg control i command worda od čega stvara frame
 * i šalje ga preko UART-a.
 * 
 * @param data Pokazivač na payload
 * @param data_len Duljina payload-a
 * @param ctrl_w Control word
 * @param cmd_w Command word
 * @return HAL_MMWAVE_OK ako je slanje uspješno
 * @return HAL_MMWAVE_ERROR ako je slanje neuspješno
 * @return HAL_MMWAVE_INVALID_STATE ako je modul u stanju iz kojeg se ne smije izvršiti slanje
 */
HalMmwaveStatus hal_mmwave_send_frame(const uint8_t* data, size_t data_len, const uint8_t ctrl_w, const uint8_t cmd_w);

/**
 * @brief Dohvaća primljeni mmWave frame iz internog queue.
 * 
 * Funkcija preuzima frame iz internog queue (blokira do dolaska frame-a ili isteka timeouta).
 * 
 * HAL je vlasnik queue-ova i memorije, pa aplikacijski sloj ne smije dobiti izravno pokazivač na queue, već
 * HAL vrši funkciju preuzimanja frame-a iz queue-a i predaje ga aplikacijskom sloju.
 * 
 * @param buffer Pokazivač na buffer u koji se sprema primljeni frame
 * @param timeout_in_ms Vrijeme čekanja u ms
 * @return HAL_MMWAVE_OK ako je dohvaćanje uspješno
 * @return HAL_MMWAVE_ERROR ako je dohvaćanje neuspješno
 * @return HAL_MMWAVE_INVALID_STATE ako je modul u stanju iz kojeg se ne smije izvršiti dohvaćanje
 */
HalMmwaveStatus hal_mmwave_get_frame_from_queue(FrameData_t* buffer, uint32_t timeout_in_ms);

/**
 * @brief Oslobađa memoriju zauzetu mmWave frame-om.
 * 
 * Funkcija se poziva iz Aplication sloja nakon što on završi s obradom frame-a.
 * 
 * @param frame_data Pokazivač na strukturu frame-a
 */
void hal_mmwave_release_frame_memory(FrameData_t* frame_data);

/**
 * @brief Prazni HAL-ov frame queue.
 * 
 * Funkcija na poziv u potpunosti prazni HAL-ov queue u kojem čuva parsirane frame-ove.
 * 
 */
void hal_mmwave_flush_frames(void);