/**
 * @file app_mmwave_constants.h
 * @author Marko Fuček
 * @brief Modul koji definira konstante korištene u aplikacijskom sloju za mmWave senzor.
 * 
 * Konstante uključuju definicije CONTROL WORDS (CRTL), COMMAND WORDS (CMD),
 * dužine podataka (LEN) i specifične podatke (DATA) za različite upite (inquiries)
 * koje aplikacijski sloj može poslati mmWave modulu.
 * 
 * @note UOF = Underlying Open Function
 * @note CM = Custom Mode
 * 
 * @version 0.1
 * @date 2026-01-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include <stdint.h>

/**
 * @brief Heartbeat Inquiry parametri.
 * 
 */
#define HEARTBEAT_CTRL 0x01
#define HEARTBEAT_CMD 0x01
#define HEARTBEAT_LEN 1
#define HEARTBEAT_DATA 0x0F

/**
 * @brief Module Reset parametri.
 * 
 */
#define MODULE_RESET_CTRL 0x01
#define MODULE_RESET_CMD 0x02
#define MODULE_RESET_LEN 1
#define MODULE_RESET_DATA 0x0F

/**
 * @brief Product Model Query parametri.
 * 
 */
#define PR_MODEL_CTRL 0x02
#define PR_MODEL_CMD 0xA1
#define PR_MODEL_LEN 1
#define PR_MODEL_DATA 0x0F

/**
 * @brief Product ID Query parametri.
 * 
 */
#define PR_ID_CTRL 0x02
#define PR_ID_CMD 0xA2
#define PR_ID_LEN 1
#define PR_ID_DATA 0x0F

/**
 * @brief Hardware Model Query parametri.
 * 
 */
#define HW_MODEL_CTRL 0x02
#define HW_MODEL_CMD 0xA3
#define HW_MODEL_LEN 1
#define HW_MODEL_DATA 0x0F

/**
 * @brief Firmware Version Query parametri.
 * 
 */
#define FW_VERSION_CTRL 0x02
#define FW_VERSION_CMD 0xA4
#define FW_VERSION_LEN 1
#define FW_VERSION_DATA 0x0F

/**
 * @brief Initialization Completed Info parametri.
 * 
 */
#define INIT_COMPL_INFO_CTRL 0x05
#define INIT_COMPL_INFO_CMD 0x01
#define INIT_COMPL_INFO_LEN 1
#define INIT_COMPL_INFO_DATA 0x0F

/**
 * @brief Scene Settings parametri.
 * 
 */
#define SCENE_SETTINGS_CTRL 0x05
#define SCENE_SETTINGS_CMD 0x07
#define SCENE_SETTINGS_LEN 1

/**
 * @brief Sensitivity Settings parametri.
 * 
 */
#define SENSITIVITY_SETTINGS_CTRL 0x05
#define SENSITIVITY_SETTINGS_CMD 0x08
#define SENSITIVITY_SETTINGS_LEN 1

/**
 * @brief Initialization Status Inquiry parametri.
 * 
 */
#define INIT_STATUS_I_CTRL 0x05
#define INIT_STATUS_I_CMD 0x81
#define INIT_STATUS_I_LEN 1
#define INIT_STATUS_I_DATA 0x0F

/**
 * @brief Scene Settings Inquiry parametri.
 * 
 */
#define SCENE_SETTINGS_I_CTRL 0x05
#define SCENE_SETTINGS_I_CMD 0x87
#define SCENE_SETTINGS_I_LEN 1
#define SCENE_SETTINGS_I_DATA 0x0F

/**
 * @brief Sensitivity Settings Inquiry parametri.
 * 
 */
#define SENSITIVITY_SETTINGS_I_CTRL 0x05
#define SENSITIVITY_SETTINGS_I_CMD 0x88
#define SENSITIVITY_SETTINGS_I_LEN 1
#define SENSITIVITY_SETTINGS_I_DATA 0x0F

/**
 * @brief Active Reporting of Presence Information parametri.
 * 
 */
#define PRESENCE_INFO_CTRL 0x80
#define PRESENCE_INFO_CMD 0x01
#define PRESENCE_INFO_LEN 1

/**
 * @brief Active Reporting of Motion Information parametri.
 * 
 */
#define MOTION_INFO_CTRL 0x80
#define MOTION_INFO_CMD 0x02
#define MOTION_INFO_LEN 1

/**
 * @brief Active Reporting of Body Movement Parameter (BMP) parametri.
 * 
 */
#define BMP_INFO_CTRL 0x80
#define BMP_INFO_CMD 0x03
#define BMP_INFO_LEN 1

/**
 * @brief Time for Entering No Person State Setting parametri.
 * 
 */
#define TIME_FOR_NO_PERSON_SETTING_CTRL 0x80
#define TIME_FOR_NO_PERSON_SETTING_CMD 0x0A
#define TIME_FOR_NO_PERSON_SETTING_LEN 1

/**
 * @brief Active Reporting of Proximity Information parametri.
 * 
 */
#define PROXIMITY_INFO_CTRL 0x80
#define PROXIMITY_INFO_CMD 0x0B
#define PROXIMITY_INFO_LEN 1

/**
 * @brief Presence Information Inquiry parametri.
 * 
 */
#define PRESENCE_INFO_I_CTRL 0x80
#define PRESENCE_INFO_I_CMD 0x81
#define PRESENCE_INFO_I_LEN 1
#define PRESENCE_INFO_I_DATA 0x0F

/**
 * @brief Motion Information Inquiry parametri.
 * 
 */
#define MOTION_INFO_I_CTRL 0x80
#define MOTION_INFO_I_CMD 0x82
#define MOTION_INFO_I_LEN 1
#define MOTION_INFO_I_DATA 0x0F

/**
 * @brief Body Movement Parameter (BMP) Inquiry parametri.
 * 
 */
#define BMP_INFO_I_CTRL 0x80
#define BMP_INFO_I_CMD 0x83
#define BMP_INFO_I_LEN 1
#define BMP_INFO_I_DATA 0x0F

/**
 * @brief Time for Entering No Person State Inquiry parametri.
 * 
 */
#define TIME_FOR_NO_PERSON_I_CTRL 0x80
#define TIME_FOR_NO_PERSON_I_CMD 0x8A
#define TIME_FOR_NO_PERSON_I_LEN 1
#define TIME_FOR_NO_PERSON_I_DATA 0x0F

/**
 * @brief Proximity Information Inquiry parametri.
 * 
 */
#define PROXIMITY_INFO_I_CTRL 0x80
#define PROXIMITY_INFO_I_CMD 0x8B
#define PROXIMITY_INFO_I_LEN 1
#define PROXIMITY_INFO_I_DATA 0x0F

/**
 * @brief Start UART Upgrade parametri.
 * 
 */
#define START_UART_UPGRADE_CTRL 0x03
#define START_UART_UPGRADE_CMD 0x01
#define START_UART_UPGRADE_LEN 1

/**
 * @brief Upgrade Package Transmission parametri.
 * 
 */
#define UPGRADE_PACKAGE_TRANSMISSION_CTRL 0x03
#define UPGRADE_PACKAGE_TRANSMISSION_CMD 0x02
#define UPGRADE_PACKAGE_TRANSMISSION_LEN 1

/**
 * @brief Ending the UART Upgrade parametri.
 * 
 */
#define ENDING_UART_UPGRADE_CTRL 0x03
#define ENDING_UART_UPGRADE_CMD 0x03
#define ENDING_UART_UPGRADE_LEN 1

/**
 * @enum SceneMode
 * @brief Scene Mode Data
 * 
 */
typedef enum {
    SCENE_MODE_NO_SET = 0x00, /**< No scene mode set */
    LIVING_ROOM = 0x01, /**< Living room scene mode */
    BEDROOM = 0x02, /**< Bedroom scene mode */
    BATHROOM = 0x03, /**< Bathroom scene mode */
    AREA_DETECTION = 0x04 /**< Area detection scene mode */
} SceneMode;

/**
 * @enum SensitivityLevel
 * @brief Sensitivity Level Data
 * 
 */
typedef enum {
    SENSITIVITY_NOT_SET = 0x00, /**< Sensitivity level not set */
    SENSITIVITY_1 = 0x01, /**< Sensitivity level 1 */
    SENSITIVITY_2 = 0x02, /**< Sensitivity level 2 */
    SENSITIVITY_3 = 0x03 /**< Sensitivity level 3 */
} SensitivityLevel;

/**
 * @enum TimeForNoPerson
 * @brief Time For No Person Data
 * 
 */
typedef enum {
    TIME_NONE = 0x00, /**< No time for no person */
    TEN_SEC = 0x01, /**< 10 sec */
    THIRTY_SEC = 0x02, /**< 30 sec */
    ONE_MIN = 0x03, /**< 1 min */
    TWO_MIN = 0x04, /**< 2 min */
    FIVE_MIN = 0x05, /**< 5 min */
    TEN_MIN = 0x06, /**< 10 min */
    THIRTY_MIN = 0x07, /**< 30 min */
    SIXTY_MIN = 0x08 /**< 60 min */
} TimeForNoPerson;

/**
 * @enum OutputSwitch
 * @brief UOF Information Output Switch Data
 * 
 */
typedef enum {
    TURN_OFF = 0x00, /**< UOF Information Output Switch OFF */
    TURN_ON = 0x01 /**< UOF Information Output Switch ON */
} OutputSwitch;

/**
 * @brief UOF Information Output Switch parametri.
 * 
 */
#define UOF_OUTPUT_SWITCH_CTRL 0x08
#define UOF_OUTPUT_SWITCH_CMD 0x00
#define UOF_OUTPUT_SWITCH_LEN 1

/**
 * @brief UOF Information Output Switch Inquiry parametri.
 * 
 */
#define UOF_OUTPUT_SWITCH_I_CTRL 0x08
#define UOF_OUTPUT_SWITCH_I_CMD 0x80
#define UOF_OUTPUT_SWITCH_I_LEN 1
#define UOF_OUTPUT_SWITCH_I_DATA 0x0F

/**
 * @brief UOF Report parametri.
 * 
 */
#define UOF_REPORT_CTRL 0x08
#define UOF_REPORT_CMD 0x01
#define UOF_REPORT_LEN 5

/**
 * @brief UOF Existence Energy Value Inquiry parametri.
 * 
 */
#define UOF_EXISTENCE_ENERGY_I_CTRL 0x08
#define UOF_EXISTENCE_ENERGY_I_CMD 0x81
#define UOF_EXISTENCE_ENERGY_I_LEN 1
#define UOF_EXISTENCE_ENERGY_I_DATA 0x0F

/**
 * @brief UOF Motion Energy Value Inquiry parametri.
 * 
 */
#define UOF_MOTION_ENERGY_I_CTRL 0x08
#define UOF_MOTION_ENERGY_I_CMD 0x82
#define UOF_MOTION_ENERGY_I_LEN 1
#define UOF_MOTION_ENERGY_I_DATA 0x0F

/**
 * @brief UOF Static Distance Inquiry parametri.
 * 
 */
#define UOF_STATIC_DISTANCE_I_CTRL 0x08
#define UOF_STATIC_DISTANCE_I_CMD 0x83
#define UOF_STATIC_DISTANCE_I_LEN 1
#define UOF_STATIC_DISTANCE_I_DATA 0x0F

/**
 * @brief UOF Motion Distance Inquiry parametri.
 * 
 */
#define UOF_MOTION_DISTANCE_I_CTRL 0x08
#define UOF_MOTION_DISTANCE_I_CMD 0x84
#define UOF_MOTION_DISTANCE_I_LEN 1
#define UOF_MOTION_DISTANCE_I_DATA 0x0F

/**
 * @brief UOF Motion Speed Inquiry parametri.
 * 
 */
#define UOF_MOTION_SPEED_I_CTRL 0x05
#define UOF_MOTION_SPEED_I_CMD 0x85
#define UOF_MOTION_SPEED_I_LEN 1
#define UOF_MOTION_SPEED_I_DATA 0x0F

/**
 * @brief CM Setting parametri.
 * 
 */
#define CM_SETTING_CTRL 0x05
#define CM_SETTING_CMD 0x09
#define CM_SETTING_LEN 1

/**
 * @brief End of CM Setting parametri.
 * 
 */
#define CM_SETTING_END_CTRL 0x05
#define CM_SETTING_END_CMD 0x0A
#define CM_SETTING_END_LEN 1
#define CM_SETTING_END_DATA 0x0F

/**
 * @brief CM Query parametri.
 * 
 */
#define CM_Q_CTRL 0x05
#define CM_Q_CMD 0x89
#define CM_Q_LEN 1
#define CM_Q_DATA 0x0F

/**
 * @brief Existence Judgment Threshold Settings parametri.
 * 
 */
#define CM_EXISTENCE_JUDGMENT_THRESH_CTRL 0x08
#define CM_EXISTENCE_JUDGMENT_THRESH_CMD 0x08
#define CM_EXISTENCE_JUDGMENT_THRESH_LEN 1

/**
 * @brief CM Motion Trigger Threshold Settings parametri.
 * 
 */
#define CM_MOTION_TRIGGER_THRESH_CTRL 0x08
#define CM_MOTION_TRIGGER_THRESH_CMD 0x09
#define CM_MOTION_TRIGGER_THRESH_LEN 1

/**
 * @brief CM Existence Perception Boundary Setting parametri.
 * 
 */
#define CM_EXISTENCE_PERCEPTION_BOUND_CTRL 0x08
#define CM_EXISTENCE_PERCEPTION_BOUND_CMD 0x0A
#define CM_EXISTENCE_PERCEPTION_BOUND_LEN 1

/**
 * @brief CM Motion Trigger Boundary Setting parametri.
 * 
 */
#define CM_MOTION_TRIGGER_BOUND_CTRL 0x08
#define CM_MOTION_TRIGGER_BOUND_CMD 0x0B
#define CM_MOTION_TRIGGER_BOUND_LEN 1

/**
 * @brief CM Motion Trigger Time Setting parametri.
 * 
 */
#define CM_MOTION_TRIGGER_TIME_CTRL 0x08
#define CM_MOTION_TRIGGER_TIME_CMD 0x0C
#define CM_MOTION_TRIGGER_TIME_LEN 4

/**
 * @brief CM Motion-to-Still Time Setting parametri.
 * 
 */
#define CM_MOTION_TO_STILL_TIME_CTRL 0x08
#define CM_MOTION_TO_STILL_TIME_CMD 0x0D
#define CM_MOTION_TO_STILL_TIME_LEN 4

/**
 * @brief CM Time for Entering No Person State Setting parametri.
 * 
 */
#define CM_TIME_FOR_NO_PERSON_CTRL 0x08
#define CM_TIME_FOR_NO_PERSON_CMD 0x0E
#define CM_TIME_FOR_NO_PERSON_LEN 4

/**
 * @brief CM-UOP Existence Judgment Threshold Inquiry parametri.
 * 
 */
#define CM_UOF_EXISTENCE_JUDGMENT_THRESH_I_CTRL 0x08
#define CM_UOF_EXISTENCE_JUDGMENT_THRESH_I_CMD 0x88
#define CM_UOF_EXISTENCE_JUDGMENT_THRESH_I_LEN 1
#define CM_UOF_EXISTENCE_JUDGMENT_THRESH_I_DATA 0x0F

/**
 * @brief CM-UOP Motion Trigger Threshold Inquiry parametri.
 * 
 */
#define CM_UOF_MOTION_TRIGGER_THRESH_I_CTRL 0x08
#define CM_UOF_MOTION_TRIGGER_THRESH_I_CMD 0x89
#define CM_UOF_MOTION_TRIGGER_THRESH_I_LEN 1
#define CM_UOF_MOTION_TRIGGER_THRESH_I_DATA 0x0F

/**
 * @brief CM-UOP Existence Perception Boundary Inquiry parametri.
 * 
 */
#define CM_UOF_EXISTENCE_PERCEPTION_BOUND_I_CTRL 0x08
#define CM_UOF_EXISTENCE_PERCEPTION_BOUND_I_CMD 0x8A
#define CM_UOF_EXISTENCE_PERCEPTION_BOUND_I_LEN 1
#define CM_UOF_EXISTENCE_PERCEPTION_BOUND_I_DATA 0x0F

/**
 * @brief CM-UOP Motion Trigger Boundary Inquiry parametri.
 * 
 */
#define CM_UOF_MOTION_TRIGGER_BOUND_I_CTRL 0x08
#define CM_UOF_MOTION_TRIGGER_BOUND_I_CMD 0x8B
#define CM_UOF_MOTION_TRIGGER_BOUND_I_LEN 1
#define CM_UOF_MOTION_TRIGGER_BOUND_I_DATA 0x0F

/**
 * @brief CM-UOP Motion Trigger Time Inquiry parametri.
 * 
 */
#define CM_UOF_MOTION_TRIGGER_TIME_I_CTRL 0x08
#define CM_UOF_MOTION_TRIGGER_TIME_I_CMD 0x8C
#define CM_UOF_MOTION_TRIGGER_TIME_I_LEN 1
#define CM_UOF_MOTION_TRIGGER_TIME_I_DATA 0x0F

/**
 * @brief CM-UOP Motion-to-Still Time Inquiry parametri.
 * 
 */
#define CM_UOF_MOTION_TO_STILL_TIME_I_CTRL 0x08
#define CM_UOF_MOTION_TO_STILL_TIME_I_CMD 0x8D
#define CM_UOF_MOTION_TO_STILL_TIME_I_LEN 1
#define CM_UOF_MOTION_TO_STILL_TIME_I_DATA 0x0F

/**
 * @brief CM-UOP Time for Entering No Person State Inquiry parametri.
 * 
 */
#define CM_UOF_TIME_FOR_NO_PERSON_I_CTRL 0x08
#define CM_UOF_TIME_FOR_NO_PERSON_I_CMD 0x8E
#define CM_UOF_TIME_FOR_NO_PERSON_I_LEN 1
#define CM_UOF_TIME_FOR_NO_PERSON_I_DATA 0x0F

/**
 * @enum CustomMode
 * @brief Custom Mode Numbers
 * 
 */
typedef enum {
    CUSTOM_MODE_1 = 0x01, /**< Custom Mode 1 */
    CUSTOM_MODE_2 = 0x02, /**< Custom Mode 2 */
    CUSTOM_MODE_3 = 0x03, /**< Custom Mode 3 */
    CUSTOM_MODE_4 = 0x04, /**< Custom Mode 4 */
} CustomMode;

/**
 * @enum PresenceInfo
 * @brief Presence Info Values
 * 
 */
typedef enum {
    UNOCCUPIED = 0x00, /**< Unoccupied */
    OCCUPIED = 0x01 /**< Occupied */
} PresenceInfo;

/**
 * @enum MotionInfo
 * @brief Motion Info Values
 * 
 */
typedef enum {
    MOTION_NONE = 0x00, /**< None */
    MOTIONLESS = 0x01, /**< Motionless */
    ACTIVE = 0x02 /**< Active */
} MotionInfo;

/**
 * @enum ProximityInfo
 * @brief Proximity Info Values
 * 
 */
typedef enum {
    NO_STATE = 0x00, /**< No State */
    NEAR = 0x01, /**< Near */
    FAR = 0x02 /**< Far */
} ProximityInfo;

/**
 * @enum ExistencePerceptionBound
 * @brief Existence perception boundaries:
 * 
 */
typedef enum {
    EP_HALF_M = 0x01, /**< 0.5 m */
    EP_ONE_M = 0x02, /**< 1 m */
    EP_ONE_AND_HALF_M = 0x03, /**< 1.5 m */
    EP_TWO_M = 0x04, /**< 2 m */
    EP_TWO_AND_HALF_M = 0x05, /**< 2.5 m */
    EP_THREE_M = 0x06, /**< 3 m */
    EP_THREE_AND_HALF_M = 0x07, /**< 3.5 m */
    EP_FOUR_M = 0x08, /**< 4 m */
    EP_FOUR_AND_HALF_M = 0x09, /**< 4.5 m */
    EP_FIVE_M = 0x0a /**< 5 m */
} ExistencePerceptionBound;

/**
 * @enum MotionTriggerBound
 * @brief Motion trigger boundaries:
 * 
 */
typedef enum {
    MT_HALF_M = 0x01, /**< 0.5 m */
    MT_ONE_M = 0x02, /**< 1 m */
    MT_ONE_AND_HALF_M = 0x03, /**< 1.5 m */
    MT_TWO_M = 0x04, /**< 2 m */
    MT_TWO_AND_HALF_M = 0x05, /**< 2.5 m */
    MT_THREE_M = 0x06, /**< 3 m */
    MT_THREE_AND_HALF_M = 0x07, /**< 3.5 m */
    MT_FOUR_M = 0x08, /**< 4 m */
    MT_FOUR_AND_HALF_M = 0x09, /**< 4.5 m */
    MT_FIVE_M = 0x0a /**< 5 m */
} MotionTriggerBound;