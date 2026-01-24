/**
 * @file app_mmwave.h
 * @author Marko Fuček
 * @brief Krovni API aplikacijskog sloja za mmWave senzor.
 * 
 * Ovaj modul pruža API aplikacijskog sloja za mmWave senzor (ujedno i vanjski API).
 * Omogućava inicijalizaciju, pokretanje i zaustavljanje sustava,
 * polling eventova (reportova/responsova) te slanje upita (inquiry) na mmWave modul.
 * 
 * @version 0.1
 * @date 2026-01-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "app_mmwave_decoder.h"
#include "app_mmwave_constants.h"
#include "app_mmwave_manager.h"

/**
 * @brief Inicijalizira mmWave senzor i sustav koji njime upravlja.
 * 
 * Wrapper funkcija koja inicijalizira aplikacijski sloj, HAL sloj i pripadajuće komponente.
 * 
 * @return Status operacije 
 */
AppSensorStatus mmwave_init(void);

/**
 * @brief Pokreće mmWave senzor i pripadajući sustav koji njime upravlja.
 * 
 * Wrapper funkcija koja pokreće aplikacijski sloj, HAL sloj i pripadajuće komponente.
 * 
 * @return Status operacije
 */
AppSensorStatus mmwave_start(void);

/**
 * @brief Zaustavlja rad mmWave senzora i pripadajućeg sustava koji njime upravlja.
 * 
 * Wrapper funkcija koja zaustavlja aplikacijski sloj, HAL sloj i pripadajuće komponente.
 * 
 * @return Status operacije 
 */
AppSensorStatus mmwave_stop(void);

/**
 * @brief Omogućava dohvaćanje (polling) eventa (reporta/responsa) iz mmWave senzora.
 * 
 * Wrapper funkcija koja dohvaća event iz aplikacijskog sloja.
 * 
 * @param out Pokazivač na strukturu u koju se sprema dohvaćeni event
 * @param timeout_ms Vrijeme čekanja u ms
 * @return true ako je uspješno dohvaćen event
 * @return false ako nije uspješno dohvaćen event
 */
bool mmwave_poll_event(MmwaveEvent* out, uint32_t timeout_ms);

/**
 * @brief Šalje upit (inquiry) za heartbeat na mmWave modul.
 * 
 * @return Status operacije
 */
AppSensorStatus app_inquiry_heartbeat(void);

/**
 * @brief Šalje upit (inquiry) za reset modula na mmWave modul.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_module_reset(void);

/**
 * @brief Šalje upit (inquiry) za dohvat modela čipa na mmWave modul.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_product_model(void);

/**
 * @brief Šalje upit (inquiry) za dohvat id-a čipa na mmWave modul.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_product_id(void);

/**
 * @brief Šalje upit (inquiry) za dohvat hardverskog modela na mmWave modul.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_hardware_model(void);

/**
 * @brief Šalje upit (inquiry) za dohvat verzije firmware-a na mmWave modul.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_firmware_version(void);

/**
 * @brief Šalje upit (inquiry) za postavljanje scene na mmWave modul.
 * 
 * @param scene 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_scene_settings_set(SceneMode scene);

/**
 * @brief Šalje upit (inquiry) za dohvat scene na mmWave modul.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_scene_settings_get(void);

/**
 * @brief Šalje upit (inquiry) za postavljanje sensitivityja na mmWave modul.
 * 
 * @param sensitivity 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_sensitivity_settings_set(SensitivityLevel sensitivity);

/**
 * @brief Šalje upit (inquiry) za dohvat sensitivity-a na mmWave modul.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_sensitivity_settings_get(void);

/**
 * @brief Šalje upit (inquiry) za dohvat presence-a osobe.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_presence(void);

/**
 * @brief Šalje upit (inquiry) za dohvat motion-a.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_motion(void);

/**
 * @brief Šalje upit (inquiry) za dohvat BMP-a.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_bmp(void);

/**
 * @brief Šalje upit (inquiry) za postavljanje time-for-no-person.
 * 
 * @param time Vrijeme nakon kojeg se detekcija osobe smatra no-person
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_time_for_no_person_set(TimeForNoPerson time);

/**
 * @brief Šalje upit (inquiry) za dohvat time-for-no-person.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_time_for_no_person_get(void);

/**
 * @brief Šalje upit (inquiry) za dohvat proximity-a.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_proximity(void);

/**
 * @brief Šalje upit (inquiry) za postavljanje UOF output switch-a.
 * 
 * @param option Postavka output switch-a (Turn off/Turn on)
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_uof_output_switch_set(OutputSwitch option);

/**
 * @brief Šalje upit (inquiry) za dohvat UOF output switch-a.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_uof_output_switch_get(void);

/**
 * @brief Šalje upit (inquiry) za dohvat existence energy-a.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_existence_energy(void);

/**
 * @brief Šalje upit (inquiry) za dohvat motion energy-a.
 * 
 * @return Status operacije
 */
AppSensorStatus app_inquiry_motion_energy(void);

/**
 * @brief Šalje upit (inquiry) za dohvat static distance-a.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_static_distance(void);

/**
 * @brief Šalje upit (inquiry) za dohvat motion distance-a.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_motion_distance(void);

/**
 * @brief Šalje upit (inquiry) za dohvat motion speed-a.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_motion_speed(void);

/**
 * @brief Šalje upit (inquiry) za postavljanje određenog custom mode-a.
 * 
 * @param mode_num Broj custom mode-a koji se postavlja
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_cm_set(CustomMode mode_num);

/**
 * @brief Šalje upit (inquiry) za dohvat custom mode-a.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_cm_get(void);

/**
 * @brief Šalje upit (inquiry) za završetak postavljanja custom mode-a.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_cm_end(void);

/**
 * @brief Šalje upit (inquiry) za postavljanje threshold-a za existence.
 * 
 * @param existence_judgement_thresh Threshold za existence (0-250)
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_cm_existence_judgement_thresh_set(uint8_t existence_judgement_thresh);

/**
 * @brief Šalje upit (inquiry) za dohvat threshold-a za existence.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_cm_existence_judgement_thresh_get(void);

/**
 * @brief Šalje upit (inquiry) za postavljanje threshold-a za motion trigger.
 * 
 * @param motion_trigger_thresh Threshold za motion trigger (0-250)
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_cm_motion_trigger_thresh_set(uint8_t motion_trigger_thresh);

/**
 * @brief Šalje upit (inquiry) za dohvat threshold-a za motion trigger.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_cm_motion_trigger_thresh_get(void);

/**
 * @brief Šalje upit (inquiry) za postavljanje existence perception bound-a.
 * 
 * @param bound Existence perception bound vrijednost
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_cm_existence_perception_bound_set(ExistencePerceptionBound bound);

/**
 * @brief Šalje upit (inquiry) za dohvat existence perception bound-a.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_cm_existence_perception_bound_get(void);

/**
 * @brief Šalje upit (inquiry) za postavljanje motion trigger bound-a.
 * 
 * @param bound Motion trigger bound vrijednost
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_cm_motion_trigger_bound_set(MotionTriggerBound bound);

/**
 * @brief Šalje upit (inquiry) za dohvat motion trigger bound-a.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_cm_motion_trigger_bound_get(void);

/**
 * @brief Šalje upit (inquiry) za postavljanje motion trigger time-a.
 * 
 * @param time_in_ms Vrijeme u milisekundama (0-1000)
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_cm_motion_trigger_time_set(uint32_t time_in_ms);

/**
 * @brief Šalje upit (inquiry) za dohvat motion trigger time-a.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_cm_motion_trigger_time_get(void);

/**
 * @brief Šalje upit (inquiry) za postavljanje motion to still time-a.
 * 
 * @param time_in_ms Vrijeme u milisekundama (1000-60000)
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_cm_motion_to_still_time_set(uint32_t time_in_ms);

/**
 * @brief Šalje upit (inquiry) za dohvat motion to still time-a.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_cm_motion_to_still_time_get(void);

/**
 * @brief Šalje upit (inquiry) za postavljanje time for no person-a.
 * 
 * @param time_in_ms Vrijeme u milisekundama (0-3600000)
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_cm_time_for_no_person_set(uint32_t time_in_ms);

/**
 * @brief šalje upit (inquiry) za dohvat time for no person-a.
 * 
 * @return Status operacije 
 */
AppSensorStatus app_inquiry_cm_time_for_no_person_get(void);