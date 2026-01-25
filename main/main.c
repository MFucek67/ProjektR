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

void fun1(MmwaveEvent* event) 
{
    if(!event) {
        printf("Pogreska sustava.\n");
    }
    if(event->type == MMWAVE_EVENT_REPORT) {
        printf("Dosao je novi report, pollaj ga.\n");
    } else if(event->type == MMWAVE_EVENT_RESPONSE) {
        printf("Dosao je response na inquiry, pollaj ga.\n");
    } else {
        printf("Greska u sustavu oko event callbacka.\n");
    }
}

void analise_event(int* reports_to_wait, int* responses_to_wait)
{
    MmwaveEvent sensor_event;
    if(mmwave_poll_event(&sensor_event, 20)) {
            MmwaveEventType ev_type = sensor_event.type;
            DecodedReport rep;
            DecodedResponse res;
            printf("Pollan je : %s", (ev_type == MMWAVE_EVENT_REPORT) ? "report\n" : "response\n");

            if(ev_type == MMWAVE_EVENT_REPORT) { //slučaj reporta
                rep = sensor_event.report;
                *(reports_to_wait)--;

                if(rep.has_init_completed_info) {
                    if(rep.init_completed_info) { //ako je init completed info
                        printf("Init completed!\n");
                    } else {
                        printf("Init not completed!\n");
                    }
                } else if(rep.has_bmp_info) { //ako je BMP info
                    printf("BMP report: %d\n", rep.bmp_info);
                } else if(rep.has_motion_info) { //ako je motion info
                    switch (rep.motion_info)
                    {
                    case NONE:
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
                } else if(rep.has_presence_info) { //ako je presence info
                    switch (rep.presence_info)
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
                } else if(rep.has_proximity_info) { //ako je proximity info
                    switch (rep.proximity_info)
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
                } else if(rep.has_uof_report) { //ako je UOF report
                    int existence_e = rep.uof_rep.existence_energy;
                    float static_d = rep.uof_rep.static_distance;
                    int motion_e = rep.uof_rep.motion_energy;
                    float motion_d = rep.uof_rep.motion_distance;
                    float motion_s = rep.uof_rep.motion_speed;
                    printf("UOF report: Existence energy: %d, Static distance: %f, Motion energy: %d, Motion distance: %f, Motion speed: %f\n", existence_e, static_d, motion_e, motion_d, motion_s);
                }
            } else if(ev_type == MMWAVE_EVENT_RESPONSE) { //slučaj responsea
                res = sensor_event.response;
                *(responses_to_wait)--;

                switch (res.type)
                {
                case HEARTBEAT:
                    printf("Heartbeat - module alive.\n");
                    break;
                case MODULE_RESET:
                    printf("Module reset successful.\n");
                    break;
                case PRODUCT_MODEL:
                    printf("Product model: %d.\n", res.data);
                    break;
                case PRODUCT_ID:
                    printf("Product ID: %d.\n", res.data);
                    break;
                case INIT_STATUS:
                    printf("Initialization completed.\n");
                    break;
                case HARDWARE_MODEL:
                    printf("Hardware model: %d.\n", res.data);
                    break;
                case FIRMWARE_VERSION:
                    printf("Firmware version: %d.\n", res.data);
                    break;
                case SCENE_SETTINGS:
                    printf("Scene settings set.\n");
                    break;
                case SCENE_SETTINGS_I:
                    printf("Scene settings are: %d.\n", res.data);
                    break;
                case SENSITIVITY:
                    printf("Sensitivity settings set.\n");
                    break;
                case SENSITIVITY_I:
                    printf("Sensitivity settings are: %d.\n", res.data);
                    break;
                case PRESENCE:
                    printf("Presence is: %d.\n", res.data);
                    break;
                case MOTION:
                    printf("Motion is: %d.\n", res.data);
                    break;
                case BMP:
                    printf("BMP is: %d.\n", res.data);
                    break;
                case TIME_FOR_NO_PERSON:
                    printf("Time for no person set.\n");
                    break;
                case TIME_FOR_NO_PERSON_I:
                    printf("Time for no person is: %d.\n", res.data);
                    break;
                case PROXIMITY:
                    printf("Proximity is: %d.\n", res.data);
                    break;
                case OUTPUT_SWITCH:
                    printf("Output switch set.\n");
                    break;
                case OUTPUT_SWITCH_I:
                    printf("Output switch is: %d.\n", res.data);
                    break;
                case EXISTENCE_ENERGY:
                    printf("Existence energy is: %d.\n", res.data);
                    break;
                case MOTION_ENERGY:
                    printf("Motion energy is: %d.\n", res.data);
                    break;
                case STATIC_DISTANCE:
                    printf("Static distance is: %d.\n", res.data);
                    break;
                case MOTION_DISTANCE:
                    printf("Motion distance is: %d.\n", res.data);
                    break;
                case MOTION_SPEED:
                    printf("Motion speed is: %d.\n", res.data);
                    break;
                case CUSTOM_MODE:
                    printf("Custom mode settings open.\n");
                    break;
                case CUSTOM_MODE_END:
                    printf("Custom mode settings closed.\n");
                    break;
                case CUSTOM_MODE_I:
                    printf("Current custom mode is: %d.\n", res.data);
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
                    printf("Existence judgment thresh is currently: %d.\n", res.data);
                    break;
                case MOTION_TRIGGER_THRESH_I:
                    printf("Motion trigger thresh is currently: %d.\n", res.data);
                    break;
                case EXISTENCE_PERCEPTION_BOUND_I:
                    printf("Existence perception boundary is currently: %d.\n", res.data);
                    break;
                case MOTION_TRIGGER_BOUND_I:
                    printf("Motion trigger boundary is currently: %d.\n", res.data);
                    break;
                case MOTION_TRIGGER_TIME_I:
                    printf("Motion trigger time is currently: %d.\n", res.data);
                    break;
                case MOTION_TO_STILL_TIME_I:
                    printf("Motion to still time is currently: %d.\n", res.data);
                    break;
                case CM_TIME_FOR_NO_PERSON_I:
                    printf("Custom mode time for entering no person is currently: %d.\n", res.data);
                    break;
                default:
                    break;
                }
            }
        }
}

void app_main(void)
{
    printf("-------- mmWave API test --------\n\n");

    AppSensorStatus status;
    int inquiries_sent = 0;
    int reports_to_wait_standard = 5;
    int reports_to_wait_uof = 5;

    if((status = mmwave_init()) != APP_SENSOR_OK) {
        printf("Neuspjesna inicijalizacija sustava!\n");
        return 0;
    }
    printf("Inicijalizacija sustava uspjesna!\n");
    if((status = mmwave_start()) != APP_SENSOR_OK) {
        printf("Neuspjesno pokretanje sustava!\n");
        return 0;
    }
    printf("Pokretanje sustava uspjesno!\n");

    printf("Registracija funkcije koju sustav zove kod javljanja eventa.\n");
    registrate_onEvent_function(fun1);

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
        return 0;
    }
    printf("Zaustavljanje sustava uspjesno!\n");

    printf("-------- ZAVRSETAK testa --------\n\n");

}