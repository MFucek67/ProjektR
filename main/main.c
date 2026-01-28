/**
 * @file main.c
 * @author Marko Fuček
 * @brief Minimalni funkcionalni test mmWave API-ja.
 * 
 * Ovaj main služi isključivo za test i prikaz osnovnih funkcionalnosti mmWave
 * aplikacijskog API-ja i ne predstavlja punu aplikaciju.
 * 
 * @version 0.1
 * @date 2026-01-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include <string.h>
#include "app/app_mmwave.h"
#include "platform/platform_task.h"
#include "tests/test_hal.h"

#define EVENT_POLL_TIMEOUT_IN_MS 20

/*void fun1(DecodedReport* report) 
{
    if(!report) return;
    printf("Callback funkcija primila report.\n");
}

void fun2(DecodedResponse* response) 
{
    if(!response) return;
    printf("Callback funkcija primila response.\n");
}

void analise_event(int* reports_to_wait, int* responses_to_wait)
{
    DecodedReport sensor_report;
    if(mmwave_poll_report(&sensor_report, EVENT_POLL_TIMEOUT_IN_MS)) {
        printf("Pollan je : report.\n");
        printf("[DEBUG] Report flags - init: %d, presence: %d, motion: %d, bmp: %d, proximity: %d, uof: %d\n",
                sensor_report.has_init_completed_info,
                sensor_report.has_presence_info,
                sensor_report.has_motion_info,
                sensor_report.has_bmp_info,
                sensor_report.has_proximity_info,
                sensor_report.has_uof_report); //MAKNUTI KASNIJE
        (*reports_to_wait)--;

        if(sensor_report.has_init_completed_info) {
            if(sensor_report.init_completed_info) { //ako je init completed info
                printf("Init completed!\n");
            } else {
                printf("Init not completed!\n");
            }
        } else if(sensor_report.has_bmp_info) { //ako je BMP info
            printf("BMP report: %d\n", sensor_report.bmp_info);
        } else if(sensor_report.has_motion_info) { //ako je motion info
            switch (sensor_report.motion_info)
            {
            case MOTION_NONE:
                printf("Motion: NONE\n");
                break;
            case MOTIONLESS:
                printf("Motion: MOTIONLESS\n");
                break;
            case ACTIVE:
                printf("Motion: ACTIVE\n");
                break;
            default:
                break;
            }
        } else if(sensor_report.has_presence_info) { //ako je presence info
            switch (sensor_report.presence_info)
            {
            case UNOCCUPIED:
                printf("Presence: UNOCCUPIED\n");
                break;
            case OCCUPIED:
                printf("Presence: OCCUPIED\n");
                break;
            default:
                break;
            }
        } else if(sensor_report.has_proximity_info) { //ako je proximity info
            switch (sensor_report.proximity_info)
            {
            case NO_STATE:
                printf("Proximity: NO_STATE\n");
                break;
            case NEAR:
                printf("Proximity: NEAR\n");
                break;
            case FAR:
                printf("Proximity: FAR\n");
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
            printf("UOF report: Existence energy: %d, Static distance: %f, Motion energy: %d, Motion distance: %f, Motion speed: %f\n", existence_e, static_d, motion_e, motion_d, motion_s);
        } else {
            printf("Report bez prepoznatog payloada.\n");
        }
    }
    DecodedResponse sensor_response;
    if(mmwave_poll_response(&sensor_response, EVENT_POLL_TIMEOUT_IN_MS)) {
        printf("Pollan je : response.\n");

        if(sensor_response.data_l == 0) {
            printf("[WARNING] Response nema podataka, preskacem.\n");
            return;
        }
        (*responses_to_wait)--;

        switch (sensor_response.type)
        {
        case HEARTBEAT:
            printf("Heartbeat - module alive.\n");
            break;
        case MODULE_RESET:
            printf("Module reset successful.\n");
            break;
        case PRODUCT_MODEL:
            printf("Product model: %d.\n", sensor_response.data[0]);
            break;
        case PRODUCT_ID:
            printf("Product ID: %d.\n", sensor_response.data[0]);
            break;
        case INIT_STATUS:
            printf("Initialization completed.\n");
            break;
        case HARDWARE_MODEL:
            printf("Hardware model: %d.\n", sensor_response.data[0]);
            break;
        case FIRMWARE_VERSION:
            printf("Firmware version: %d.\n", sensor_response.data[0]);
            break;
        case SCENE_SETTINGS:
            printf("Scene settings set.\n");
            break;
        case SCENE_SETTINGS_I:
            printf("Scene settings are: %d.\n", sensor_response.data[0]);
            break;
        case SENSITIVITY:
            printf("Sensitivity settings set.\n");
            break;
        case SENSITIVITY_I:
            printf("Sensitivity settings are: %d.\n", sensor_response.data[0]);
            break;
        case PRESENCE:
            printf("Presence is: %d.\n", sensor_response.data[0]);
            break;
        case MOTION:
            printf("Motion is: %d.\n", sensor_response.data[0]);
            break;
        case BMP:
            printf("BMP is: %d.\n", sensor_response.data[0]);
            break;
        case TIME_FOR_NO_PERSON:
            printf("Time for no person set.\n");
            break;
        case TIME_FOR_NO_PERSON_I:
            printf("Time for no person is: %d.\n", sensor_response.data[0]);
            break;
        case PROXIMITY:
            printf("Proximity is: %d.\n", sensor_response.data[0]);
            break;
        case OUTPUT_SWITCH:
            printf("Output switch set.\n");
            break;
        case OUTPUT_SWITCH_I:
            printf("Output switch is: %d.\n", sensor_response.data[0]);
            break;
        case EXISTENCE_ENERGY:
            printf("Existence energy is: %d.\n", sensor_response.data[0]);
            break;
        case MOTION_ENERGY:
            printf("Motion energy is: %d.\n", sensor_response.data[0]);
            break;
        case STATIC_DISTANCE:
            printf("Static distance is: %d.\n", sensor_response.data[0]);
            break;
        case MOTION_DISTANCE:
            printf("Motion distance is: %d.\n", sensor_response.data[0]);
            break;
        case MOTION_SPEED:
            printf("Motion speed is: %d.\n", sensor_response.data[0]);
            break;
        case CUSTOM_MODE:
            printf("Custom mode settings open.\n");
            break;
        case CUSTOM_MODE_END:
            printf("Custom mode settings closed.\n");
            break;
        case CUSTOM_MODE_I:
            printf("Current custom mode is: %d.\n", sensor_response.data[0]);
            break;
        case EXISTENCE_JUDGMENT_THRESH:
            printf("Existence judgment thresh set.\n");
            break;
        case MOTION_TRIGGER_THRESH:
            printf("Motion trigger thresh set.\n");
            break;
        case EXISTENCE_PERCEPTION_BOUND:
            printf("Existence perception boundary set.\n");
            break;
        case MOTION_TRIGGER_BOUND:
            printf("Motion trigger boundary set.\n");
            break;
        case MOTION_TRIGGER_TIME:
            printf("Motion trigger time set.\n");
            break;
        case MOTION_TO_STILL_TIME:
            printf("Motion to still time set.\n");
            break;
        case CM_TIME_FOR_NO_PERSON:
            printf("Custom mode time for entering no person set.\n");
            break;
        case EXISTENCE_JUDGMENT_THRESH_I:
            printf("Existence judgment thresh is currently: %d.\n", sensor_response.data[0]);
            break;
        case MOTION_TRIGGER_THRESH_I:
            printf("Motion trigger thresh is currently: %d.\n", sensor_response.data[0]);
            break;
        case EXISTENCE_PERCEPTION_BOUND_I:
            printf("Existence perception boundary is currently: %d.\n", sensor_response.data[0]);
            break;
        case MOTION_TRIGGER_BOUND_I:
            printf("Motion trigger boundary is currently: %d.\n", sensor_response.data[0]);
            break;
        case MOTION_TRIGGER_TIME_I:
            printf("Motion trigger time is currently: %d.\n", sensor_response.data[0]);
            break;
        case MOTION_TO_STILL_TIME_I:
            printf("Motion to still time is currently: %d.\n", sensor_response.data[0]);
            break;
        case CM_TIME_FOR_NO_PERSON_I:
            printf("Custom mode time for entering no person is currently: %d.\n", sensor_response.data[0]);
            break;
        default:
            break;
        }
    }
}*/

void app_main(void)
{
    /*printf("-------- mmWave API test --------\n\n");

    AppSensorStatus status;
    int inquiries_sent = 0;
    int reports_to_wait_standard = 5;
    int reports_to_wait_uof = 5;

    if((status = mmwave_init()) != APP_SENSOR_OK) {
        printf("Neuspjesna inicijalizacija sustava!\n");
        return;
    }
    printf("Inicijalizacija sustava uspjesna!\n");
    if((status = mmwave_start()) != APP_SENSOR_OK) {
        printf("Neuspjesno pokretanje sustava!\n");
        return;
    }
    printf("Pokretanje sustava uspjesno!\n");

    printf("Registracija funkcije koju sustav zove kod javljanja eventa.\n");
    registrate_onEvent_function(fun2, fun1);

    printf("Postavljanje scene i sensitivity (scene = Living room) (sensitivity = 3).\n");

    app_inquiry_scene_settings_set(LIVING_ROOM);
    app_inquiry_sensitivity_settings_set(SENSITIVITY_3);
    inquiries_sent++;
    inquiries_sent++;

    printf("Prvo isprobavamo rad u Standard mode.\n");
    app_inquiry_uof_output_switch_set(TURN_OFF);
    inquiries_sent++;

    printf("Postavljamo jos i time for entering no person state na 10s.\n");
    app_inquiry_time_for_no_person_set(TEN_SEC);
    inquiries_sent++;

    printf("Saljemo inquiry za presence.\n");
    app_inquiry_presence();
    inquiries_sent++;

    printf("Saljemo inquiry za proximity.\n");
    app_inquiry_proximity();
    inquiries_sent++;

    printf("Saljemo HEARTBEAT.\n");
    app_inquiry_heartbeat();
    inquiries_sent++;

    while(reports_to_wait_standard > 0 || inquiries_sent > 0) {
        //ovdje ćemo pollati iz queue dok god čekamo 5 reportova i odgovore na naše inquiries
        analise_event(&reports_to_wait_standard, &inquiries_sent);
    }

    printf("Sada cemo se prebaciti u Underlying Open Function.\n");
    app_inquiry_uof_output_switch_set(TURN_ON);
    inquiries_sent++;

    printf("Saljemo inquiry za existence energy.\n");
    app_inquiry_existence_energy();
    inquiries_sent++;

    printf("Saljemo inquiry za static distance.\n");
    app_inquiry_static_distance();
    inquiries_sent++;

    while(reports_to_wait_uof > 0 || inquiries_sent > 0) {
        //ovdje ćemo pollati iz queue dok god čekamo 5 reportova i odgovore na naše inquiries
        analise_event(&reports_to_wait_uof, &inquiries_sent);
    }

    if((status = mmwave_stop()) != APP_SENSOR_OK) {
        printf("Neuspjesno zaustavljanje sustava!\n");
        return;
    }
    printf("Zaustavljanje sustava uspjesno!\n");

    printf("-------- ZAVRSETAK testa --------\n\n");*/

    platform_delay_task(1000);
    hal_mmwave_run_test();

}