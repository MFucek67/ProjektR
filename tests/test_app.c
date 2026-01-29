/**
 * @file test_app.c
 * @author Marko Fuček
 * @brief Ovaj modul predstavlja test vanjskog API-ja application sloja.
 * 
 * Test predstavlja vanjsku aplikaciju koja poziva API aplikacijskog sloja drivera.
 * 
 * Test se sastoji od:
 * - Inicijalizacije sustava
 * - Pokretanja sustava
 * - Registriranja funkcija koje se pozivaju kao callback na dolazak report-ova i response-ova
 * - Slanja više inquirija (upita) na senzor i čekanja odgovora na sve njih u Standard Mode
 * - Čekanja dolaska više reportova sa senzora u Standard Mode
 * - Prebacivanja u Underlying Open Function Mode način rada senzora
 * - Slanja više inquirija (upita) na senzor i čekanja odgovora na sve njih u Underlying Open Function Mode
 * - Čekanja dolaska više reportova sa senzora u Underlying Open Function Mode
 * - Zaustavljanja rada sustava
 * - Deinicijalizacije sustava
 * 
 * @note Test se bavi testiranjem application sloja, ali obuhvaća i ostale slojeve interno.
 * @version 0.1
 * @date 2026-01-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include "tests/test_app.h"
#include "app/app_mmwave_hal_config.h"
#include "app/app_mmwave_constants.h"
#include "app/app_mmwave_decoder.h"
#include "app/app_mmwave_manager.h"
#include "app/app_mmwave.h"
#include "platform/platform.h"

#define EVENT_POLL_TIMEOUT_IN_MS 20

void fun1(DecodedReport report)
{
    printf("[APP CALLBACK] Callback funkcija primila report\n");
}

void fun2(DecodedResponse response)
{
    printf("[APP CALLBACK] Callback funkcija primila response\n");
}

void analise_event(int* reports_to_wait, int* responses_to_wait, bool poll_reports)
{
    DecodedReport sensor_report;
    if(poll_reports) {
        if(mmwave_poll_report(&sensor_report, EVENT_POLL_TIMEOUT_IN_MS)) {
            printf("[APP test] Pollan je report\n");
            (*reports_to_wait)--;

            if(sensor_report.has_init_completed_info) {
                if(sensor_report.init_completed_info) { //ako je init completed info
                    printf("[APP test] Init completed\n");
                } else {
                    printf("[APP test] Init not completed\n");
                }
            } else if(sensor_report.has_bmp_info) { //ako je BMP info
                printf("[APP test] BMP report: %d\n", sensor_report.bmp_info);
            } else if(sensor_report.has_motion_info) { //ako je motion info
                switch (sensor_report.motion_info)
                {
                case MOTION_NONE:
                    printf("[APP test] Motion: NONE\n");
                    break;
                case MOTIONLESS:
                    printf("[APP test] Motion: MOTIONLESS\n");
                    break;
                case ACTIVE:
                    printf("[APP test] Motion: ACTIVE\n");
                    break;
                default:
                    break;
                }
            } else if(sensor_report.has_presence_info) { //ako je presence info
                switch (sensor_report.presence_info)
                {
                case UNOCCUPIED:
                    printf("[APP test] Presence: UNOCCUPIED\n");
                    break;
                case OCCUPIED:
                    printf("[APP test] Presence: OCCUPIED\n");
                    break;
                default:
                    break;
                }
            } else if(sensor_report.has_proximity_info) { //ako je proximity info
                switch (sensor_report.proximity_info)
                {
                case NO_STATE:
                    printf("[APP test] Proximity: NO_STATE\n");
                    break;
                case NEAR:
                    printf("[APP test] Proximity: NEAR\n");
                    break;
                case FAR:
                    printf("[APP test] Proximity: FAR\n");
                    break;
                default:
                    break;
                }
            } else if(sensor_report.has_uof_report) { //ako je UOF report
                int existence_e = sensor_report.uof_rep.existence_energy;
                float static_d = sensor_report.uof_rep.static_distance;
                int motion_e = sensor_report.uof_rep.motion_energy;
                float motion_d = sensor_report.uof_rep.motion_distance;
                float motion_s = sensor_report.uof_rep.motion_speed;
                printf("[APP test] UOF report: Existence energy: %d, Static distance: %f, Motion energy: %d, Motion distance: %f, Motion speed: %f\n",
                    existence_e, static_d, motion_e, motion_d, motion_s);
            } else {
                printf("[APP test] Report bez prepoznatog payloada\n");
            }
        }
    }

    DecodedResponse sensor_response;
    if(mmwave_poll_response(&sensor_response, EVENT_POLL_TIMEOUT_IN_MS)) {
        printf("[APP test] Pollan je response\n");

        if(sensor_response.data_l == 0) {
            printf("[WARNING] Response nema podataka, preskacem\n");
            return;
        }
        (*responses_to_wait)--;

        switch (sensor_response.type)
        {
        case HEARTBEAT:
            printf("[APP test] Heartbeat - module ziv\n");
            break;
        case MODULE_RESET:
            printf("[APP test] Module reset uspjesan\n");
            break;
        case PRODUCT_MODEL:
            printf("[APP test] Product model: %d\n", sensor_response.data[0]);
            break;
        case PRODUCT_ID:
            printf("[APP test] Product ID: %d\n", sensor_response.data[0]);
            break;
        case INIT_STATUS:
            printf("[APP test] Inicijalizacija uspjesna\n");
            break;
        case HARDWARE_MODEL:
            printf("[APP test] Hardware model: %d\n", sensor_response.data[0]);
            break;
        case FIRMWARE_VERSION:
            printf("[APP test] Firmware version: %d\n", sensor_response.data[0]);
            break;
        case SCENE_SETTINGS:
            printf("[APP test] Scene settings postavljen\n");
            break;
        case SCENE_SETTINGS_I:
            printf("[APP test] Scene settings su: %d\n", sensor_response.data[0]);
            break;
        case SENSITIVITY:
            printf("[APP test] Sensitivity settings postavljen\n");
            break;
        case SENSITIVITY_I:
            printf("[APP test] Sensitivity settings su: %d\n", sensor_response.data[0]);
            break;
        case PRESENCE:
            printf("[APP test] Presence je: %d\n", sensor_response.data[0]);
            break;
        case MOTION:
            printf("[APP test] Motion je: %d\n", sensor_response.data[0]);
            break;
        case BMP:
            printf("[APP test] BMP je: %d\n", sensor_response.data[0]);
            break;
        case TIME_FOR_NO_PERSON:
            printf("[APP test] Time for no person postavljen\n");
            break;
        case TIME_FOR_NO_PERSON_I:
            printf("[APP test] Time for no person je: %d\n", sensor_response.data[0]);
            break;
        case PROXIMITY:
            printf("[APP test] Proximity je: %d\n", sensor_response.data[0]);
            break;
        case OUTPUT_SWITCH:
            printf("[APP test] Output switch postavka promijenjena\n");
            break;
        case OUTPUT_SWITCH_I:
            printf("[APP test] Output switch postavka je: %d.\n", sensor_response.data[0]);
            break;
        case EXISTENCE_ENERGY:
            printf("[APP test] Existence energy je: %d\n", sensor_response.data[0]);
            break;
        case MOTION_ENERGY:
            printf("[APP test] Motion energy je: %d\n", sensor_response.data[0]);
            break;
        case STATIC_DISTANCE:
            printf("[APP test] Static distance je: %d\n", sensor_response.data[0]);
            break;
        case MOTION_DISTANCE:
            printf("[APP test] Motion distance je: %d\n", sensor_response.data[0]);
            break;
        case MOTION_SPEED:
            printf("[APP test] Motion speed je: %d\n", sensor_response.data[0]);
            break;
        case CUSTOM_MODE:
            printf("[APP test] Custom mode settings open\n");
            break;
        case CUSTOM_MODE_END:
            printf("[APP test] Custom mode settings closed\n");
            break;
        case CUSTOM_MODE_I:
            printf("[APP test] Trenutni custom mode je: %d\n", sensor_response.data[0]);
            break;
        case EXISTENCE_JUDGMENT_THRESH:
            printf("[APP test] Existence judgment thresh postavljena\n");
            break;
        case MOTION_TRIGGER_THRESH:
            printf("[APP test] Motion trigger thresh postavljena\n");
            break;
        case EXISTENCE_PERCEPTION_BOUND:
            printf("[APP test] Existence perception boundary postavljena\n");
            break;
        case MOTION_TRIGGER_BOUND:
            printf("[APP test] Motion trigger boundary postavljen\n");
            break;
        case MOTION_TRIGGER_TIME:
            printf("[APP test] Motion trigger time postavljen\n");
            break;
        case MOTION_TO_STILL_TIME:
            printf("[APP test] Motion to still time postavljen\n");
            break;
        case CM_TIME_FOR_NO_PERSON:
            printf("[APP test] Custom mode time for entering no person postavljen\n");
            break;
        case EXISTENCE_JUDGMENT_THRESH_I:
            printf("[APP test] Existence judgment thresh je trenutno: %d\n", sensor_response.data[0]);
            break;
        case MOTION_TRIGGER_THRESH_I:
            printf("[APP test] Motion trigger thresh je trenutno: %d\n", sensor_response.data[0]);
            break;
        case EXISTENCE_PERCEPTION_BOUND_I:
            printf("[APP test] Existence perception boundary je trenutno: %d\n", sensor_response.data[0]);
            break;
        case MOTION_TRIGGER_BOUND_I:
            printf("[APP test] Motion trigger boundary je trenutno: %d\n", sensor_response.data[0]);
            break;
        case MOTION_TRIGGER_TIME_I:
            printf("[APP test] Motion trigger time je trenutno: %d\n", sensor_response.data[0]);
            break;
        case MOTION_TO_STILL_TIME_I:
            printf("[APP test] Motion to still time je trenutno: %d\n", sensor_response.data[0]);
            break;
        case CM_TIME_FOR_NO_PERSON_I:
            printf("[APP test] Custom mode time for entering no person je trenutno: %d\n", sensor_response.data[0]);
            break;
        default:
            break;
        }
    }
}

void app_mmwave_run_test(void)
{
    printf("------------APP TEST START------------\n");

    //Kontrolne varijable
    AppSensorStatus status;
    int inquiries_sent = 0;
    int reports_to_wait_standard = 3;
    int reports_to_wait_uof = 3;

    //Inicijalizacija sustava:
    if((status = mmwave_init()) != APP_SENSOR_OK) {
        printf("[APP test] Neuspjesna inicijalizacija sustava\n");
        return;
    }
    printf("[APP test] Uspjesna inicijalizacija sustava\n");

    //Pokretanje sustava:
    if((status = mmwave_start()) != APP_SENSOR_OK) {
        printf("[APP test] Neuspjesno pokretanje sustava\n");
        return;
    }
    printf("[APP test] Uspjesno pokretanje sustava\n");

    printf("[APP test] Registracija funkcija koje sustav zove kod javljanja eventa\n");
    if(registrate_onEvent_function(fun2, fun1) != APP_SENSOR_OK) {
        printf("[APP test] Neuspjesna registracija funkcija\n");
        return;
    }
    printf("[APP test] Uspjesna registracija funkcija\n");

    printf("[APP test] Postavljanje scene i sensitivity (scene = Living room) (sensitivity = 3)\n");

    app_inquiry_scene_settings_set(LIVING_ROOM);
    inquiries_sent++;
    platform_delay_task(100);
    app_inquiry_sensitivity_settings_set(SENSITIVITY_3);
    inquiries_sent++;
    platform_delay_task(100);

    printf("[APP test] Prvo isprobavamo rad u Standard mode\n");
    app_inquiry_uof_output_switch_set(TURN_OFF);
    inquiries_sent++;
    platform_delay_task(100);

    printf("[APP test] Postavljamo jos i time for entering no person state na 10s\n");
    app_inquiry_time_for_no_person_set(TEN_SEC);
    inquiries_sent++;
    platform_delay_task(100);

    printf("[APP test] Saljemo inquiry za presence\n");
    app_inquiry_presence();
    inquiries_sent++;
    platform_delay_task(100);

    printf("[APP test] Saljemo inquiry za proximity\n");
    app_inquiry_proximity();
    inquiries_sent++;
    platform_delay_task(100);

    printf("[APP test] Saljemo inquiry za HEARTBEAT\n");
    app_inquiry_heartbeat();
    inquiries_sent++;
    platform_delay_task(100);

    int loop_count = 0; //sigurnosni mehanizam da ne zaglavimo u livelock-u
    while(inquiries_sent > 0) {
        loop_count++;
        //ovdje ćemo pollati iz queue dok god čekamo odgovore na naše inquiries
        analise_event(&reports_to_wait_standard, &inquiries_sent, false);

        if(loop_count > 1000) {
            printf("[ERROR] Petlja se vrti predugo! inquiries_sent=%d\n", inquiries_sent);
            break;
        }
    }
    while(reports_to_wait_standard > 0) {
        //ovdje ćemo pollati iz queue dok god čekamo 3 reporta
        analise_event(&reports_to_wait_standard, &inquiries_sent, true);
    }
    
    printf("Sada cemo se prebaciti u Underlying Open Function\n");
    app_inquiry_uof_output_switch_set(TURN_ON);
    inquiries_sent++;
    platform_delay_task(100);

    printf("Saljemo inquiry za existence energy\n");
    app_inquiry_existence_energy();
    inquiries_sent++;
    platform_delay_task(100);

    printf("Saljemo inquiry za static distance\n");
    app_inquiry_static_distance();
    inquiries_sent++;
    platform_delay_task(100);

    loop_count = 0;
    while(inquiries_sent > 0) {
        loop_count++;
        //ovdje ćemo pollati iz queue dok god čekamo odgovore na naše inquiries
        analise_event(&reports_to_wait_uof, &inquiries_sent, false);

        if(loop_count > 1000) {
            printf("[ERROR] Petlja se vrti predugo! inquiries_sent=%d\n", inquiries_sent);
            break;
        }
    }
    while(reports_to_wait_uof > 0) {
        //ovdje ćemo pollati iz queue dok god čekamo 3 reporta
        analise_event(&reports_to_wait_uof, &inquiries_sent, true);
    }

    //Zaustavljanje rada sustava:
    if((status = mmwave_stop()) != APP_SENSOR_OK) {
        printf("Neuspjesno zaustavljanje sustava\n");
        return;
    }
    printf("Zaustavljanje sustava uspjesno\n");

    //Deinicijalizacija:
    if((status = mmwave_deinit()) != APP_SENSOR_OK) {
        printf("Neuspjesna deinicijalizacija sustava\n");
        return;
    }
    printf("Deinicijalizacija sustava uspjesna\n");

    platform_delay_task(1000);

    printf("------------APP TEST STOP------------\n");
}