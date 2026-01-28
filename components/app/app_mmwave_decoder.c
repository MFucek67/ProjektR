/**
 * @file app_mmwave_decoder.c
 * @author Marko Fuček
 * @brief Implementacija aplikacijskog dekodera mmWave poruka.
 * 
 * Ovaj modul implementira aplikacijski dekoder mmWave poruka (semantički korisnih podataka).
 * Prima već prevedene i izdvojene podatke, te ih tumači i prevodi u aplikacijske strukture
 * reportove i responsove.
 * 
 * Modul je potpuno neovisan o platformi i ne poznaje detalje komunikacijskog protokola.
 * Komunikacija s ostatkom aplikacijskg sloja obavlja se isključivo preko callbackova.
 * 
 * @version 0.1
 * @date 2026-01-23
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include <string.h>
#include "app/app_mmwave_constants.h"
#include "app/app_mmwave_decoder.h"

static AppDecoderContext* context = NULL;
static bool initialized = false;

/**
 * @brief Pomoćna funkcija koja resetira sadržaj dekodiranog reporta.
 * 
 * Postavlja sve zastavice reporta na false.
 * Koristi se prije obrade nove poruke kako bi se osiguralo čisto početno
 * stanje report strukture.
 * 
 * @param dr Pokazivač na strukturu reporta
 */
static void decoded_report_reset(DecodedReport* dr)
{
    dr->has_init_completed_info = false;
    dr->has_presence_info = false;
    dr->has_motion_info = false;
    dr->has_bmp_info = false;
    dr->has_proximity_info = false;
    dr->has_uof_report = false;
}

/**
 * @brief Pomoćna funkcija koja resetira sadržaj dekodiranog responsa.
 * 
 * Briše tip responsa i pripadajuće podatke.
 * Koristi se prije obrade nove poruke kako bi se osiguralo čisto početno
 * stanje response strukture.
 * 
 * @param dr Pokazivač na strukturu response-a
 */
static void decoded_response_reset(DecodedResponse* dr)
{
    dr->type = NO_TYPE;
    dr->data_l = 0;
    memset(dr->data, 0, sizeof(dr->data));
}

/**
 * @brief Sigurno kopira response podatke s provjerom veličine.
 * 
 * Provjerava je li payload_len <= MAX_RESPONSE_DATA_LEN prije kopiranja.
 * Ako je provjera uspješna, kopira podatke i šalje response callback-u.
 * 
 * @param response Pokazivač na response strukturu (tip mora biti već postavljen)
 * @param payload Pokazivač na payload podatke
 * @param payload_len Duljina payload-a
 */
static void send_response_safe(DecodedResponse* response, const uint8_t* payload, int payload_len)
{
    printf("[decoder send_response_safe] payload_len=%d, MAX=%d\n", payload_len, MAX_RESPONSE_DATA_LEN);
    
    if(payload_len <= MAX_RESPONSE_DATA_LEN && payload_len >= 0) {
        response->data_l = payload_len;
        if(payload_len > 0) {
            memcpy(response->data, payload, payload_len);
        }
        printf("[decoder send_response_safe] Pozivam callback sa type=%d, len=%d\n", response->type, response->data_l);
        context->sendResponseCallback(response);
        printf("[decoder send_response_safe] Callback se vratio\n");
    } else {
        printf("[decoder] ERROR: payload_len=%d exceeds MAX_RESPONSE_DATA_LEN=%d\n", payload_len, MAX_RESPONSE_DATA_LEN);
    }
}

void app_mmwave_decoder_init(AppDecoderContext* ctx)
{
    context = ctx;
    initialized = true;
}

void app_mmwave_decoder_deinit(void)
{
    context = NULL;
    initialized = false;
}

/**
 * @note (data[0] = ctrl_w, data[1] = cmd_w, data[2, ...] = payload)
 * 
 * @note Ako dekoder nije inicijaliziran ili su ulazni parametri neispravni,
 * funkcija se prekida bez obrade.
 */
void app_mmwave_decoder_process_frame(uint8_t* data, size_t data_len)
{
    if(!initialized) {
        printf("[decoder] ERROR: not initialized\n"); //KASNIJE MAKNUTI
        return;
    }
    if(!context) {
        printf("[decoder] ERROR: context=NULL\n"); //KASNIJE MAKNUTI
        return;
    }
    if(!context->sendReportCallback) {
        printf("[decoder] ERROR: sendReportCallback not set\n"); //KASNIJE MAKNUTI
        return;
    }
    if(!context->sendResponseCallback) {
        printf("[decoder] ERROR: sendResponseCallback not set\n"); //KASNIJE MAKNUTI
        return;
    }
    if(!data) {
        printf("[decoder] ERROR: data=NULL\n"); //KASNIJE MAKNUTI
        return;
    }
    if(data_len < 2) {
        printf("[decoder] ERROR: frame too short (%zu)\n", data_len); //KASNIJE MAKNUTI
        return;
    }

    printf("[decoder] frame len=%zu ctrl=0x%02X cmd=0x%02X\n",
       data_len, data[0], data[1]); //KASNIJE MAKNUTI

    DecodedReport report; //callback mora kopirati podatak 
    DecodedResponse response; //callback mora kopirati podatak
    decoded_report_reset(&report);
    decoded_response_reset(&response);
    uint8_t ctrl_w = data[0];
    uint8_t cmd_w = data[1];
    int payload_len = data_len - 2;

    //Reports:
    if(ctrl_w == INIT_COMPL_INFO_CTRL && cmd_w == INIT_COMPL_INFO_CMD) {
        report.has_init_completed_info = true;
        report.init_completed_info = true;
        context->sendReportCallback(&report);
    } else if(ctrl_w == 0x80) {
        switch (cmd_w)
        {
        case PRESENCE_INFO_CMD:
            if(payload_len == PRESENCE_INFO_LEN) {
                if(data[2] == 0x00) {
                    report.has_presence_info = true;
                    report.presence_info = UNOCCUPIED;
                    context->sendReportCallback(&report);
                } else if(data[2] == 0x01) {
                    report.has_presence_info = true;
                    report.presence_info = OCCUPIED;
                    context->sendReportCallback(&report);
                }
            }
            break;
        case MOTION_INFO_CMD:
            if(payload_len == MOTION_INFO_LEN) {
                if(data[2] == 0x00) {
                    report.has_motion_info = true;
                    report.motion_info = MOTION_NONE;
                    context->sendReportCallback(&report);
                } else if(data[2] == 0x01) {
                    report.has_motion_info = true;
                    report.motion_info = MOTIONLESS;
                    context->sendReportCallback(&report);
                } else if(data[2] == 0x02) {
                    report.has_motion_info = true;
                    report.motion_info = ACTIVE;
                    context->sendReportCallback(&report);
                }
            }
            break;
        case BMP_INFO_CMD:
            if(payload_len == BMP_INFO_LEN) {
                report.has_bmp_info = true;
                report.bmp_info = data[2];
                context->sendReportCallback(&report);
            }
            break;
        case PROXIMITY_INFO_CMD:
            if(payload_len == PROXIMITY_INFO_LEN) {
                if(data[2] == 0x00) {
                    report.has_proximity_info = true;
                    report.proximity_info = NO_STATE;
                    context->sendReportCallback(&report);
                } else if(data[2] == 0x01) {
                    report.has_proximity_info = true;
                    report.proximity_info = NEAR;
                    context->sendReportCallback(&report);
                } else if(data[2] == 0x02) {
                    report.has_proximity_info = true;
                    report.proximity_info = FAR;
                    context->sendReportCallback(&report);
                }
            }
            break;
        default:
            break;
        }
    } else if(ctrl_w == UOF_REPORT_CTRL) {
        report.has_uof_report = true;
        report.uof_rep.existence_energy = data[2];
        report.uof_rep.motion_energy = data[4];
        switch (data[3])
        {
        case 0x01:
            report.uof_rep.static_distance = 0.5;
            break;
        case 0x02:
            report.uof_rep.static_distance = 1;
            break;
        case 0x03:
            report.uof_rep.static_distance = 1.5;
            break;
        case 0x04:
            report.uof_rep.static_distance = 2;
            break;
        case 0x05:
            report.uof_rep.static_distance = 2.5;
            break;
        case 0x06:
            report.uof_rep.static_distance = 3;
            break;
        default:
            break;
        }
        switch (data[5])
        {
        case 0x01:
            report.uof_rep.motion_distance = 0.5;
            break;
        case 0x02:
            report.uof_rep.motion_distance = 1;
            break;
        case 0x03:
            report.uof_rep.motion_distance = 1.5;
            break;
        case 0x04:
            report.uof_rep.motion_distance = 2;
            break;
        case 0x05:
            report.uof_rep.motion_distance = 2.5;
            break;
        case 0x06:
            report.uof_rep.motion_distance = 3;
            break;
        case 0x07:
            report.uof_rep.motion_distance = 3.5;
            break;
        case 0x08:
            report.uof_rep.motion_distance = 4;
            break;
        default:
            break;
        }
        switch (data[6])
        {
        case 0x01:
            report.uof_rep.motion_speed = -4.5;
            break;
        case 0x02:
            report.uof_rep.motion_speed = -4;
            break;
        case 0x03:
            report.uof_rep.motion_speed = -3.5;
            break;
        case 0x04:
            report.uof_rep.motion_speed = -3;
            break;
        case 0x05:
            report.uof_rep.motion_speed = -2.5;
            break;
        case 0x06:
            report.uof_rep.motion_speed = -2;
            break;
        case 0x07:
            report.uof_rep.motion_speed = -1.5;
            break;
        case 0x08:
            report.uof_rep.motion_speed = -1;
            break;
        case 0x09:
            report.uof_rep.motion_speed = -0.5;
            break;
        case 0x0a:
            report.uof_rep.motion_speed = 0;
            break;
        case 0x0b:
            report.uof_rep.motion_speed = 0.5;
            break;
        case 0x0c:
            report.uof_rep.motion_speed = 1;
            break;
        case 0x0d:
            report.uof_rep.motion_speed = 1.5;
            break;
        case 0x0e:
            report.uof_rep.motion_speed = 2;
            break;
        case 0x0f:
            report.uof_rep.motion_speed = 2.5;
            break;
        case 0x10:
            report.uof_rep.motion_speed = 3;
            break;
        case 0x11:
            report.uof_rep.motion_speed = 3.5;
            break;
        case 0x12:
            report.uof_rep.motion_speed = 4;
            break;
        case 0x13:
            report.uof_rep.motion_speed = 4.5;
            break;
        case 0x14:
            report.uof_rep.motion_speed = 5;
            break;
        default:
            break;
        }
        context->sendReportCallback(&report);
    }

    //Responses
    //Implementirat ću samo najbitnije responsove (za funkcije koje se navjviše koriste) - ostali ili kasnije ili nisu potrebni
    if(ctrl_w == HEARTBEAT_CTRL && cmd_w == HEARTBEAT_CMD && payload_len == HEARTBEAT_LEN) {
        response.type = HEARTBEAT;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == MODULE_RESET_CTRL && cmd_w == MODULE_RESET_CMD && payload_len == MODULE_RESET_LEN) {
        response.type = MODULE_RESET;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == PR_MODEL_CTRL && cmd_w == PR_MODEL_CMD && payload_len == PR_MODEL_LEN) {
        response.type = PRODUCT_MODEL;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == PR_ID_CTRL && cmd_w == PR_ID_CMD && payload_len == PR_ID_LEN) {
        response.type = PRODUCT_ID;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == HW_MODEL_CTRL && cmd_w == HW_MODEL_CMD && payload_len == HW_MODEL_LEN) {
        response.type = HARDWARE_MODEL;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == FW_VERSION_CTRL && cmd_w == FW_VERSION_CMD && payload_len == FW_VERSION_LEN) {
        response.type = FIRMWARE_VERSION;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == SCENE_SETTINGS_CTRL && cmd_w == SCENE_SETTINGS_CMD && payload_len == SCENE_SETTINGS_LEN) {
        response.type = SCENE_SETTINGS;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == SCENE_SETTINGS_I_CTRL && cmd_w == SCENE_SETTINGS_I_CMD && payload_len == SCENE_SETTINGS_I_LEN) {
        response.type = SCENE_SETTINGS_I;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == SENSITIVITY_SETTINGS_I_CTRL && cmd_w == SENSITIVITY_SETTINGS_I_CMD && payload_len == SENSITIVITY_SETTINGS_I_LEN) {
        response.type = SENSITIVITY_I;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == SENSITIVITY_SETTINGS_CTRL && cmd_w == SENSITIVITY_SETTINGS_CMD && payload_len == SENSITIVITY_SETTINGS_LEN) {
        response.type = SENSITIVITY;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == PRESENCE_INFO_I_CTRL && cmd_w == PRESENCE_INFO_I_CMD && payload_len == PRESENCE_INFO_I_LEN) {
        response.type = PRESENCE;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == MOTION_INFO_I_CTRL && cmd_w == MOTION_INFO_I_CMD && payload_len == MOTION_INFO_I_LEN) {
        response.type = MOTION;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == BMP_INFO_I_CTRL && cmd_w == BMP_INFO_I_CMD && payload_len == BMP_INFO_I_LEN) {
        response.type = BMP;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == TIME_FOR_NO_PERSON_I_CTRL && cmd_w == TIME_FOR_NO_PERSON_I_CMD && payload_len == TIME_FOR_NO_PERSON_I_LEN) {
        response.type = TIME_FOR_NO_PERSON_I;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == TIME_FOR_NO_PERSON_SETTING_CTRL && cmd_w == TIME_FOR_NO_PERSON_SETTING_CMD && payload_len == TIME_FOR_NO_PERSON_SETTING_LEN) {
        response.type = TIME_FOR_NO_PERSON;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == PROXIMITY_INFO_I_CTRL && cmd_w == PROXIMITY_INFO_I_CMD && payload_len == PROXIMITY_INFO_I_LEN) {
        response.type = PROXIMITY;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == UOF_OUTPUT_SWITCH_CTRL && cmd_w == UOF_OUTPUT_SWITCH_CMD && payload_len == UOF_OUTPUT_SWITCH_LEN) {
        response.type = OUTPUT_SWITCH;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == UOF_OUTPUT_SWITCH_I_CTRL && cmd_w == UOF_OUTPUT_SWITCH_I_CMD && payload_len == UOF_OUTPUT_SWITCH_I_LEN) {
        response.type = OUTPUT_SWITCH_I;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == UOF_EXISTENCE_ENERGY_I_CTRL && cmd_w == UOF_EXISTENCE_ENERGY_I_CMD && payload_len == UOF_EXISTENCE_ENERGY_I_LEN) {
        response.type = EXISTENCE_ENERGY;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == UOF_MOTION_ENERGY_I_CTRL && cmd_w == UOF_MOTION_ENERGY_I_CMD && payload_len == UOF_MOTION_ENERGY_I_LEN) {
        response.type = MOTION_ENERGY;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == UOF_STATIC_DISTANCE_I_CTRL && cmd_w == UOF_STATIC_DISTANCE_I_CMD && payload_len == UOF_STATIC_DISTANCE_I_LEN) {
        response.type = STATIC_DISTANCE;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == UOF_MOTION_DISTANCE_I_CTRL && cmd_w == UOF_MOTION_DISTANCE_I_CMD && payload_len == UOF_MOTION_DISTANCE_I_LEN) {
        response.type = MOTION_DISTANCE;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == UOF_MOTION_SPEED_I_CTRL && cmd_w == UOF_MOTION_SPEED_I_CMD && payload_len == UOF_MOTION_SPEED_I_LEN) {
        response.type = MOTION_SPEED;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == CM_SETTING_CTRL && cmd_w == CM_SETTING_CMD && payload_len == CM_SETTING_LEN) {
        response.type = CUSTOM_MODE;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == CM_SETTING_END_CTRL && cmd_w == CM_SETTING_END_CMD && payload_len == CM_SETTING_END_LEN) {
        response.type = CUSTOM_MODE_END;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == CM_Q_CTRL && cmd_w == CM_Q_CMD && payload_len == CM_Q_LEN) {
        response.type = CUSTOM_MODE_I;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == CM_UOF_EXISTENCE_JUDGMENT_THRESH_I_CTRL && cmd_w == CM_UOF_EXISTENCE_JUDGMENT_THRESH_I_CMD && payload_len == CM_UOF_EXISTENCE_JUDGMENT_THRESH_I_LEN) {
        response.type = EXISTENCE_JUDGMENT_THRESH_I;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == CM_UOF_MOTION_TRIGGER_THRESH_I_CTRL && cmd_w == CM_UOF_MOTION_TRIGGER_THRESH_I_CMD && payload_len == CM_UOF_MOTION_TRIGGER_THRESH_I_LEN) {
        response.type = MOTION_TRIGGER_THRESH_I;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == CM_UOF_EXISTENCE_PERCEPTION_BOUND_I_CTRL && cmd_w == CM_UOF_EXISTENCE_PERCEPTION_BOUND_I_CMD && payload_len == CM_UOF_EXISTENCE_PERCEPTION_BOUND_I_LEN) {
        response.type = EXISTENCE_PERCEPTION_BOUND_I;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == CM_UOF_MOTION_TRIGGER_BOUND_I_CTRL && cmd_w == CM_UOF_MOTION_TRIGGER_BOUND_I_CMD && payload_len == CM_UOF_MOTION_TRIGGER_BOUND_I_LEN) {
        response.type = MOTION_TRIGGER_BOUND_I;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == CM_UOF_MOTION_TRIGGER_TIME_I_CTRL && cmd_w == CM_UOF_MOTION_TRIGGER_TIME_I_CMD && payload_len == CM_UOF_MOTION_TRIGGER_TIME_I_LEN) {
        response.type = MOTION_TRIGGER_TIME_I;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == CM_UOF_MOTION_TO_STILL_TIME_I_CTRL && cmd_w == CM_UOF_MOTION_TO_STILL_TIME_I_CMD && payload_len == CM_UOF_MOTION_TO_STILL_TIME_I_LEN) {
        response.type = MOTION_TO_STILL_TIME_I;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == CM_UOF_TIME_FOR_NO_PERSON_I_CTRL && cmd_w == CM_UOF_TIME_FOR_NO_PERSON_I_CMD && payload_len == CM_UOF_TIME_FOR_NO_PERSON_I_LEN) {
        response.type = CM_TIME_FOR_NO_PERSON_I;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == CM_EXISTENCE_JUDGMENT_THRESH_CTRL && cmd_w == CM_EXISTENCE_JUDGMENT_THRESH_CMD && payload_len == CM_EXISTENCE_JUDGMENT_THRESH_LEN) {
        response.type = EXISTENCE_JUDGMENT_THRESH;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == CM_MOTION_TRIGGER_THRESH_CTRL && cmd_w == CM_MOTION_TRIGGER_THRESH_CMD && payload_len == CM_MOTION_TRIGGER_THRESH_LEN) {
        response.type = MOTION_TRIGGER_THRESH;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == CM_EXISTENCE_PERCEPTION_BOUND_CTRL && cmd_w == CM_EXISTENCE_PERCEPTION_BOUND_CMD && payload_len == CM_EXISTENCE_PERCEPTION_BOUND_LEN) {
        response.type = EXISTENCE_PERCEPTION_BOUND;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == CM_MOTION_TRIGGER_BOUND_CTRL && cmd_w == CM_MOTION_TRIGGER_BOUND_CMD && payload_len == CM_MOTION_TRIGGER_BOUND_LEN) {
        response.type = MOTION_TRIGGER_BOUND;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == CM_MOTION_TRIGGER_TIME_CTRL && cmd_w == CM_MOTION_TRIGGER_TIME_CMD && payload_len == CM_MOTION_TRIGGER_TIME_LEN) {
        response.type = MOTION_TRIGGER_TIME;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == CM_MOTION_TO_STILL_TIME_CTRL && cmd_w == CM_MOTION_TO_STILL_TIME_CMD && payload_len == CM_MOTION_TO_STILL_TIME_LEN) {
        response.type = MOTION_TO_STILL_TIME;
        send_response_safe(&response, &data[2], payload_len);
    }
    if(ctrl_w == CM_TIME_FOR_NO_PERSON_CTRL && cmd_w == CM_TIME_FOR_NO_PERSON_CMD && payload_len == CM_TIME_FOR_NO_PERSON_LEN) {
        response.type = CM_TIME_FOR_NO_PERSON;
        send_response_safe(&response, &data[2], payload_len);
    }
}