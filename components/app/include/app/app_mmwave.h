#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "app_mmwave_decoder.h"
#include "app_mmwave_constants.h"
#include "app_mmwave_manager.h"

//krovne funkcije za polling i obradu evenata kod dobivenog reporta ili odgovora
//funkcije za start i end

//callback na funkciju koja šalje signal da je u queue dodan novi Report/Response
typedef void (*MMwaveEventCallback)(const MmwaveEvent* ev);

AppSensorStatus mmwave_init(void);
AppSensorStatus mmwave_start(void);
AppSensorStatus mmwave_stop(void);

//polling iz queue s Responses i Reports
bool mmwave_poll_event(MmwaveEvent* out, uint32_t timeout_ms);

//Inquiry/send funkcije:
AppSensorStatus app_inquiry_heartbeat(void);
AppSensorStatus app_inquiry_module_reset(void);
AppSensorStatus app_inquiry_product_model(void);
AppSensorStatus app_inquiry_product_id(void);
AppSensorStatus app_inquiry_hardware_model(void);
AppSensorStatus app_inquiry_firmware_version(void);
AppSensorStatus app_inquiry_scene_settings_set(SceneMode scene);
AppSensorStatus app_inquiry_scene_settings_get(void);
AppSensorStatus app_inquiry_sensitivity_settings_set(SensitivityLevel sensitivity);
AppSensorStatus app_inquiry_sensitivity_settings_get(void);
AppSensorStatus app_inquiry_presence(void);
AppSensorStatus app_inquiry_motion(void);
AppSensorStatus app_inquiry_bmp(void);
AppSensorStatus app_inquiry_time_for_no_person_set(TimeForNoPerson time);
AppSensorStatus app_inquiry_time_for_no_person_get(void);
AppSensorStatus app_inquiry_proximity(void);
AppSensorStatus app_inquiry_uof_output_switch_set(OutputSwitch option);
AppSensorStatus app_inquiry_uof_output_switch_get(void);
AppSensorStatus app_inquiry_existence_energy(void);
AppSensorStatus app_inquiry_motion_energy(void);
AppSensorStatus app_inquiry_static_distance(void);
AppSensorStatus app_inquiry_motion_distance(void);
AppSensorStatus app_inquiry_motion_speed(void);
AppSensorStatus app_inquiry_cm_set(CustomMode mode_num);
AppSensorStatus app_inquiry_cm_get(void);
AppSensorStatus app_inquiry_cm_end(void);
AppSensorStatus app_inquiry_cm_existence_judgement_thresh_set(int existence_judgement_thresh);
AppSensorStatus app_inquiry_cm_existence_judgement_thresh_get(void);
AppSensorStatus app_inquiry_cm_motion_trigger_thresh_set(int motion_trigger_thresh);
AppSensorStatus app_inquiry_cm_motion_trigger_thresh_get(void);
AppSensorStatus app_inquiry_cm_existence_perception_bound_set(ExistencePerceptionBound bound);
AppSensorStatus app_inquiry_cm_existence_perception_bound_get(void);
AppSensorStatus app_inquiry_cm_motion_trigger_bound_set(MotionTriggerBound bound);
AppSensorStatus app_inquiry_cm_motion_trigger_bound_get(void);
AppSensorStatus app_inquiry_cm_motion_trigger_time_set(int time_in_ms); //0-1000ms
AppSensorStatus app_inquiry_cm_motion_trigger_time_get(void);
AppSensorStatus app_inquiry_cm_motion_to_still_time_set(int time_in_ms); //1000-60000ms (1-60sec)
AppSensorStatus app_inquiry_cm_motion_to_still_time_get(void);
AppSensorStatus app_inquiry_cm_time_for_no_person_set(int time_in_ms); //0-3600000ms (0-3600sec)
AppSensorStatus app_inquiry_cm_time_for_no_person_get(void);