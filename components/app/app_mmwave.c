#include <stdint.h>
#include <stdbool.h>
#include "app/app_mmwave.h"

static uint8_t data;

//iz STANDARD MODE se ne smiju pozivati UOF upiti
//iz UOF se ne bi smjeli pozivati STANDARD MODE upiti (zato su zabranjeni)
//MODE se mijenja promjenom OUTPUT_SWITCHA
//CUSTOM MODE se smije mijenjati i mijenjati parametri UOF-a tek kad se nalazimo u UOF
//COSTOM MODE i služi isključivo za promjenu parametara UOF-a

AppSensorStatus mmwave_init(void)
{
    return app_init_sys();
}

AppSensorStatus mmwave_start(void)
{
    return app_start_sys();
}

AppSensorStatus mmwave_stop(void)
{
    return app_stop_sys();
}

AppSensorStatus app_inquiry_heartbeat(void)
{
    data = HEARTBEAT_DATA;
    return app_send_inquiry(&data, HEARTBEAT_LEN, HEARTBEAT_CTRL, HEARTBEAT_CMD );
}

AppSensorStatus app_inquiry_module_reset(void)
{
    data = MODULE_RESET_DATA;
    return app_send_inquiry(&data, MODULE_RESET_LEN, MODULE_RESET_CTRL, MODULE_RESET_CMD);
}

AppSensorStatus app_inquiry_product_model(void)
{
    data = PR_MODEL_DATA;
    return app_send_inquiry(&data, PR_MODEL_LEN, PR_MODEL_CTRL, PR_MODEL_CMD);
}

AppSensorStatus app_inquiry_product_id(void)
{
    data = PR_ID_DATA;
    return app_send_inquiry(&data, PR_ID_LEN, PR_ID_CTRL, PR_ID_CMD);
}

AppSensorStatus app_inquiry_hardware_model(void)
{
    data = HW_MODEL_DATA;
    return app_send_inquiry(&data, HW_MODEL_LEN, HW_MODEL_CTRL, HW_MODEL_CMD);
}

AppSensorStatus app_inquiry_firmware_version(void)
{
    data = FW_VERSION_DATA;
    return app_send_inquiry(&data, FW_VERSION_LEN, FW_VERSION_CTRL, FW_VERSION_CMD);
}

AppSensorStatus app_inquiry_scene_settings_set(SceneMode scene)
{
    if(scene != 0x00 && scene != 0x01 && scene != 0x02 && scene != 0x03 && scene != 0x04) {
        return APP_SENSOR_BAD_ARGUMENT;
    }
    return app_send_inquiry(&scene, SCENE_SETTINGS_LEN, SCENE_SETTINGS_CTRL, SCENE_SETTINGS_CMD);
}

AppSensorStatus app_inquiry_scene_settings_get(void)
{
    data = SCENE_SETTINGS_I_DATA;
    return app_send_inquiry(&data, SCENE_SETTINGS_I_LEN, SCENE_SETTINGS_CTRL, SCENE_SETTINGS_I_CMD);
}

AppSensorStatus app_inquiry_sensitivity_settings_set(SensitivityLevel sensitivity)
{
    if(sensitivity != 0x01 && sensitivity != 0x02 && sensitivity != 0x03) {
        return APP_SENSOR_BAD_ARGUMENT;
    }
    return app_send_inquiry(&sensitivity, SENSITIVITY_SETTINGS_LEN, SENSITIVITY_SETTINGS_CTRL, SENSITIVITY_SETTINGS_CMD);
}

AppSensorStatus app_inquiry_sensitivity_settings_get(void)
{
    data = SENSITIVITY_SETTINGS_I_DATA;
    return app_send_inquiry(&data, SENSITIVITY_SETTINGS_I_LEN, SENSITIVITY_SETTINGS_I_CTRL, SENSITIVITY_SETTINGS_I_CMD);
}

AppSensorStatus app_inquiry_presence(void)
{
    if(app_get_mode() == SENSOR_MODE_UNDERLYING_OPEN) {
        return APP_SENSOR_BAD_MODE;
    }
    data = PRESENCE_INFO_I_DATA;
    return app_send_inquiry(&data, PRESENCE_INFO_I_LEN, PRESENCE_INFO_I_CTRL, PRESENCE_INFO_I_CMD);
}

AppSensorStatus app_inquiry_motion(void)
{
    if(app_get_mode() == SENSOR_MODE_UNDERLYING_OPEN) {
        return APP_SENSOR_BAD_MODE;
    }
    data = MOTION_INFO_I_DATA;
    return app_send_inquiry(&data, MOTION_INFO_I_LEN, MOTION_INFO_I_CTRL, MOTION_INFO_I_CMD);
}

AppSensorStatus app_inquiry_bmp(void)
{
    if(app_get_mode() == SENSOR_MODE_UNDERLYING_OPEN) {
        return APP_SENSOR_BAD_MODE;
    }
    data = BMP_INFO_I_DATA;
    return app_send_inquiry(&data, BMP_INFO_I_LEN, BMP_INFO_I_CTRL, BMP_INFO_I_CMD);
}

AppSensorStatus app_inquiry_time_for_no_person_set(TimeForNoPerson time)
{
    if(app_get_mode() == SENSOR_MODE_UNDERLYING_OPEN) {
        return APP_SENSOR_BAD_MODE;
    }
    if(time != 0x00 && time != 0x01 && time != 0x02 && time != 0x03 && 
        time != 0x04 && time != 0x05 && time != 0x06 && time != 0x07 && time != 0x08) {
            return APP_SENSOR_BAD_ARGUMENT;
        }
    return app_send_inquiry(&time, TIME_FOR_NO_PERSON_SETTING_LEN, CM_TIME_FOR_NO_PERSON_CTRL, TIME_FOR_NO_PERSON_SETTING_CMD);
}

AppSensorStatus app_inquiry_time_for_no_person_get(void)
{
    if(app_get_mode() == SENSOR_MODE_UNDERLYING_OPEN) {
        return APP_SENSOR_BAD_MODE;
    }
    data = TIME_FOR_NO_PERSON_I_DATA;
    return app_send_inquiry(&data, TIME_FOR_NO_PERSON_I_LEN, TIME_FOR_NO_PERSON_I_CTRL, TIME_FOR_NO_PERSON_I_CMD);
}

AppSensorStatus app_inquiry_proximity(void)
{
    if(app_get_mode() == SENSOR_MODE_UNDERLYING_OPEN) {
        return APP_SENSOR_BAD_MODE;
    }
    data = PROXIMITY_INFO_I_DATA;
    return app_send_inquiry(&data, PROXIMITY_INFO_I_LEN, PROXIMITY_INFO_I_CTRL, PROXIMITY_INFO_I_CMD);
}

AppSensorStatus app_inquiry_uof_output_switch_set(OutputSwitch option)
{
    if(option != 0x00 && option != 0x01) {
        return APP_SENSOR_BAD_ARGUMENT;
    }
    if(option == 0x00) {
        app_set_mode(SENSOR_MODE_STANDARD);
    } else if(option == 0x01) {
        app_set_mode(SENSOR_MODE_UNDERLYING_OPEN);
    }
    return app_send_inquiry(&option, UOF_OUTPUT_SWITCH_LEN, UOF_OUTPUT_SWITCH_CTRL, UOF_OUTPUT_SWITCH_CMD);
}

AppSensorStatus app_inquiry_uof_output_switch_get(void)
{
    data = UOF_OUTPUT_SWITCH_I_DATA;
    return app_send_inquiry(&data, UOF_OUTPUT_SWITCH_I_LEN, UOF_OUTPUT_SWITCH_I_CTRL, UOF_OUTPUT_SWITCH_I_CMD);
}

AppSensorStatus app_inquiry_existence_energy(void)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    data = UOF_EXISTENCE_ENERGY_I_DATA;
    return app_send_inquiry(&data, UOF_EXISTENCE_ENERGY_I_LEN, UOF_EXISTENCE_ENERGY_I_CTRL, UOF_EXISTENCE_ENERGY_I_CMD);
}

AppSensorStatus app_inquiry_motion_energy(void)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    data = UOF_MOTION_ENERGY_I_DATA;
    return app_send_inquiry(&data, UOF_MOTION_ENERGY_I_LEN, UOF_MOTION_ENERGY_I_CTRL, UOF_MOTION_ENERGY_I_CMD);
}

AppSensorStatus app_inquiry_static_distance(void)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    data = UOF_STATIC_DISTANCE_I_CTRL;
    return app_send_inquiry(&data, UOF_STATIC_DISTANCE_I_LEN, UOF_STATIC_DISTANCE_I_CTRL, UOF_STATIC_DISTANCE_I_CMD);
}

AppSensorStatus app_inquiry_motion_distance(void)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    data = UOF_MOTION_DISTANCE_I_DATA;
    return app_send_inquiry(&data, UOF_MOTION_DISTANCE_I_LEN, UOF_MOTION_DISTANCE_I_CTRL, UOF_MOTION_DISTANCE_I_CMD);
}

AppSensorStatus app_inquiry_motion_speed(void)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    data = UOF_MOTION_SPEED_I_DATA;
    return app_send_inquiry(&data, UOF_MOTION_SPEED_I_LEN, UOF_MOTION_SPEED_I_CTRL, UOF_MOTION_SPEED_I_CMD);
}

AppSensorStatus app_inquiry_cm_set(CustomMode mode_num)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    if(mode_num != 0x01 && mode_num != 0x02 && mode_num != 0x03 && mode_num != 0x04) {
        return APP_SENSOR_BAD_ARGUMENT;
    }
    return app_send_inquiry(&mode_num, CM_SETTING_LEN, CM_SETTING_CTRL, CM_SETTING_CMD);
}

AppSensorStatus app_inquiry_cm_get(void)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    data = CM_Q_DATA;
    return app_send_inquiry(&data, CM_Q_LEN, CM_Q_CTRL, CM_Q_CMD);
}

AppSensorStatus app_inquiry_cm_end(void)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    data = CM_SETTING_END_DATA;
    return app_send_inquiry(&data, CM_SETTING_END_LEN, CM_SETTING_END_CTRL, CM_SETTING_END_CMD);
}

AppSensorStatus app_inquiry_cm_existence_judgement_thresh_set(int existence_judgement_thresh)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    if(existence_judgement_thresh < 0 || existence_judgement_thresh > 250) {
        return APP_SENSOR_BAD_ARGUMENT;
    }
    return app_send_inquiry(&existence_judgement_thresh, CM_EXISTENCE_JUDGMENT_THRESH_LEN, CM_EXISTENCE_JUDGMENT_THRESH_CTRL, CM_EXISTENCE_JUDGMENT_THRESH_CMD);

}
AppSensorStatus app_inquiry_cm_existence_judgement_thresh_get(void)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    data = CM_UOF_EXISTENCE_JUDGMENT_THRESH_I_DATA;
    return app_send_inquiry(&data, CM_UOF_EXISTENCE_JUDGMENT_THRESH_I_LEN, CM_UOF_EXISTENCE_JUDGMENT_THRESH_I_CTRL, CM_UOF_EXISTENCE_JUDGMENT_THRESH_I_CMD);
}
AppSensorStatus app_inquiry_cm_motion_trigger_thresh_set(int motion_trigger_thresh)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    if(motion_trigger_thresh < 0 ||motion_trigger_thresh > 250) {
        return APP_SENSOR_BAD_ARGUMENT;
    }
    return app_send_inquiry(&motion_trigger_thresh, CM_MOTION_TRIGGER_THRESH_LEN, CM_MOTION_TRIGGER_THRESH_CTRL, CM_MOTION_TRIGGER_THRESH_CMD);
}
AppSensorStatus app_inquiry_cm_motion_trigger_thresh_get(void)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    data = CM_UOF_MOTION_TRIGGER_THRESH_I_DATA;
    return app_send_inquiry(&data, CM_UOF_MOTION_TRIGGER_THRESH_I_LEN, CM_UOF_MOTION_TRIGGER_THRESH_I_CTRL, CM_UOF_MOTION_TRIGGER_THRESH_I_CMD);
}
AppSensorStatus app_inquiry_cm_existence_perception_bound_set(ExistencePerceptionBound bound)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    if(bound != 0x01 && bound != 0x02 && bound != 0x03 && bound != 0x04 && bound != 0x05 &&
         bound != 0x06 && bound != 0x07 && bound != 0x08 && bound != 0x09 && bound != 0x0a) {
        return APP_SENSOR_BAD_ARGUMENT;
    }
    return app_send_inquiry(&bound, CM_EXISTENCE_PERCEPTION_BOUND_LEN, CM_EXISTENCE_PERCEPTION_BOUND_CTRL, CM_EXISTENCE_PERCEPTION_BOUND_CMD);
}
AppSensorStatus app_inquiry_cm_existence_perception_bound_get(void)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    data = CM_UOF_EXISTENCE_PERCEPTION_BOUND_I_DATA;
    return app_send_inquiry(&data, CM_UOF_EXISTENCE_PERCEPTION_BOUND_I_LEN, CM_UOF_EXISTENCE_PERCEPTION_BOUND_I_CTRL, CM_UOF_EXISTENCE_PERCEPTION_BOUND_I_CMD);
}
AppSensorStatus app_inquiry_cm_motion_trigger_bound_set(MotionTriggerBound bound)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    if(bound != 0x01 && bound != 0x02 && bound != 0x03 && bound != 0x04 && bound != 0x05 &&
         bound != 0x06 && bound != 0x07 && bound != 0x08 && bound != 0x09 && bound != 0x0a) {
        return APP_SENSOR_BAD_ARGUMENT;
    }
    return app_send_inquiry(&bound, CM_MOTION_TRIGGER_BOUND_LEN, CM_MOTION_TRIGGER_BOUND_CTRL, CM_MOTION_TRIGGER_BOUND_CMD);
}
AppSensorStatus app_inquiry_cm_motion_trigger_bound_get(void)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    data = CM_UOF_MOTION_TRIGGER_BOUND_I_DATA;
    return app_send_inquiry(&data, CM_UOF_MOTION_TRIGGER_BOUND_I_LEN, CM_UOF_MOTION_TRIGGER_BOUND_I_CTRL, CM_UOF_MOTION_TRIGGER_BOUND_I_CMD);
}
AppSensorStatus app_inquiry_cm_motion_trigger_time_set(int time_in_ms)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    if(time_in_ms < 0 || time_in_ms > 1000) {
        return APP_SENSOR_BAD_ARGUMENT;
    }
    return app_send_inquiry(&time_in_ms, CM_MOTION_TRIGGER_TIME_LEN, CM_MOTION_TRIGGER_TIME_CTRL, CM_MOTION_TRIGGER_TIME_CMD);
}
AppSensorStatus app_inquiry_cm_motion_trigger_time_get(void)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    data = CM_UOF_MOTION_TRIGGER_TIME_I_DATA;
    return app_send_inquiry(&data, CM_UOF_MOTION_TRIGGER_TIME_I_LEN, CM_UOF_MOTION_TRIGGER_TIME_I_CTRL, CM_UOF_MOTION_TRIGGER_TIME_I_CMD);
}
AppSensorStatus app_inquiry_cm_motion_to_still_time_set(int time_in_ms)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    if(time_in_ms < 1000 ||time_in_ms > 60000) {
        return APP_SENSOR_BAD_ARGUMENT;
    }
    return app_send_inquiry(&time_in_ms, CM_MOTION_TO_STILL_TIME_LEN, CM_MOTION_TO_STILL_TIME_CTRL, CM_MOTION_TO_STILL_TIME_CMD);
}
AppSensorStatus app_inquiry_cm_motion_to_still_time_get(void)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    data = CM_UOF_MOTION_TO_STILL_TIME_I_DATA;
    return app_send_inquiry(&data, CM_UOF_MOTION_TO_STILL_TIME_I_LEN, CM_UOF_MOTION_TO_STILL_TIME_I_CTRL, CM_UOF_MOTION_TO_STILL_TIME_I_CMD);
}
AppSensorStatus app_inquiry_cm_time_for_no_person_set(int time_in_ms)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    if(time_in_ms < 0 ||time_in_ms > 3600000) {
        return APP_SENSOR_BAD_ARGUMENT;
    }
    return app_send_inquiry(&time_in_ms, CM_TIME_FOR_NO_PERSON_LEN, CM_TIME_FOR_NO_PERSON_CTRL, CM_TIME_FOR_NO_PERSON_CMD);
}
AppSensorStatus app_inquiry_cm_time_for_no_person_get(void)
{
    if(app_get_mode() == SENSOR_MODE_STANDARD) {
        return APP_SENSOR_BAD_MODE;
    }
    data = CM_UOF_TIME_FOR_NO_PERSON_I_DATA;
    return app_send_inquiry(&data, CM_UOF_TIME_FOR_NO_PERSON_I_LEN, CM_UOF_TIME_FOR_NO_PERSON_I_CTRL, CM_UOF_TIME_FOR_NO_PERSON_I_CMD);
}