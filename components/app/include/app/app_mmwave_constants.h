#pragma once
#include <stdint.h>

/*Popis CONTROL WORDS i COMMAND WORDS s obzirom na funkcije koje senzor izvršava
    CTRL = control word
    CMD = command word
    LEN = length
    DATA = data (ako je specifičan za tu funkciju)
*/

//Heartbeat:
#define HEARTBEAT_CTRL 0x01
#define HEARTBEAT_CMD 0x01
#define HEARTBEAT_LEN 1
#define HEARTBEAT_DATA 0x0F

//Module Reset
#define MODULE_RESET_CTRL 0x01
#define MODULE_RESET_CMD 0x02
#define MODULE_RESET_LEN 1
#define MODULE_RESET_DATA 0x0F

//Product Model Query
#define PR_MODEL_CTRL 0x02
#define PR_MODEL_CMD 0xA1
#define PR_MODEL_LEN 1
#define PR_MODEL_DATA 0x0F

//Product ID Query
#define PR_ID_CTRL 0x02
#define PR_ID_CMD 0xA2
#define PR_ID_LEN 1
#define PR_ID_DATA 0x0F

//Hardware Model Query
#define HW_MODEL_CTRL 0x02
#define HW_MODEL_CMD 0xA3
#define HW_MODEL_LEN 1
#define HW_MODEL_DATA 0x0F

//Firmware Version Query
#define FW_VERSION_CTRL 0x02
#define FW_VERSION_CMD 0xA4
#define FW_VERSION_LEN 1
#define FW_VERSION_DATA 0x0F

//Initialization Completed Info
#define INIT_COMPL_INFO_CTRL 0x05
#define INIT_COMPL_INFO_CMD 0x01
#define INIT_COMPL_INFO_LEN 1
#define INIT_COMPL_INFO_DATA 0x0F

//Scene Settings
#define SCENE_SETTINGS_CTRL 0x05
#define SCENE_SETTINGS_CMD 0x07
#define SCENE_SETTINGS_LEN 1

//Sensitivity Settings
#define SENSITIVITY_SETTINGS_CTRL 0x05
#define SENSITIVITY_SETTINGS_CMD 0x08
#define SENSITIVITY_SETTINGS_LEN 1

//Initialization Status Inquiry
#define INIT_STATUS_I_CTRL 0x05
#define INIT_STATUS_I_CMD 0x81
#define INIT_STATUS_I_LEN 1
#define INIT_STATUS_I_DATA 0x0F

//Scene Settings Inquiry
#define SCENE_SETTINGS_I_CTRL 0x05
#define SCENE_SETTINGS_I_CMD 0x87
#define SCENE_SETTINGS_I_LEN 1
#define SCENE_SETTINGS_I_DATA 0x0F

//Sensitivity Settings
#define SENSITIVITY_SETTINGS_I_CTRL 0x05
#define SENSITIVITY_SETTINGS_I_CMD 0x88
#define SENSITIVITY_SETTINGS_I_LEN 1
#define SENSITIVITY_SETTINGS_I_DATA 0x0F

//Active Reporting of Presence Information
#define PRESENCE_INFO_CTRL 0x80
#define PRESENCE_INFO_CMD 0x01
#define PRESENCE_INFO_LEN 1

//Active Reporting of Motion Information
#define MOTION_INFO_CTRL 0x80
#define MOTION_INFO_CMD 0x02
#define MOTION_INFO_LEN 1


//Active Reporting of Body Movement Parameter
#define BMP_INFO_CTRL 0x80
#define BMP_INFO_CMD 0x03
#define BMP_INFO_LEN 1

//Time for Entering no Person State Setting
#define TIME_FOR_NO_PERSON_SETTING_CTRL 0x80
#define TIME_FOR_NO_PERSON_SETTING_CMD 0x0A
#define TIME_FOR_NO_PERSON_SETTING_LEN 1

//Active Reporting of Proximity
#define PROXIMITY_INFO_CTRL 0x80
#define PROXIMITY_INFO_CMD 0x0B
#define PROXIMITY_INFO_LEN 1

//Presence Information Inquiry
#define PRESENCE_INFO_I_CTRL 0x80
#define PRESENCE_INFO_I_CMD 0x81
#define PRESENCE_INFO_I_LEN 1
#define PRESENCE_INFO_I_DATA 0x0F

//Motion Information Inquiry
#define MOTION_INFO_I_CTRL 0x80
#define MOTION_INFO_I_CMD 0x82
#define MOTION_INFO_I_LEN 1
#define MOTION_INFO_I_DATA 0x0F

//Body Movement Parameter Inquiry
#define BMP_INFO_I_CTRL 0x80
#define BMP_INFO_I_CMD 0x83
#define BMP_INFO_I_LEN 1
#define BMP_INFO_I_DATA 0x0F

//Time for Entering no Person State Inquiry
#define TIME_FOR_NO_PERSON_I_CTRL 0x80
#define TIME_FOR_NO_PERSON_I_CMD 0x8A
#define TIME_FOR_NO_PERSON_I_LEN 1
#define TIME_FOR_NO_PERSON_I_DATA 0x0F

//Proximity Inquiry
#define PROXIMITY_INFO_I_CTRL 0x80
#define PROXIMITY_INFO_I_CMD 0x8B
#define PROXIMITY_INFO_I_LEN 1
#define PROXIMITY_INFO_I_DATA 0x0F

//Start UART Upgrade
#define START_UART_UPGRADE_CTRL 0x03
#define START_UART_UPGRADE_CMD 0x01
#define START_UART_UPGRADE_LEN 1

//Upgrade Package Transmission
#define UPGRADE_PACKAGE_TRANSMISSION_CTRL 0x03
#define UPGRADE_PACKAGE_TRANSMISSION_CMD 0x02
#define UPGRADE_PACKAGE_TRANSMISSION_LEN 1

//Ending the UART Upgrade
#define ENDING_UART_UPGRADE_CTRL 0x03
#define ENDING_UART_UPGRADE_CMD 0x03
#define ENDING_UART_UPGRADE_LEN 1

//Scene Mode Data
typedef enum {
    SCENE_MODE_NO_SET = 0x00,
    LIVING_ROOM = 0x01,
    BEDROOM = 0x02,
    BATHROOM = 0x03,
    AREA_DETECTION = 0x04
} SceneMode;

//Sensitivity Level Data
typedef enum {
    SENSITIVITY_NOT_SET = 0x00,
    SENSITIVITY_1 = 0x01,
    SENSITIVITY_2 = 0x02,
    SENSITIVITY_3 = 0x03
} SensitivityLevel;

//Time for Entering No Person State
typedef enum {
    NONE = 0x00,
    TEN_SEC = 0x01,
    THIRTY_SEC = 0x02,
    ONE_MIN = 0x03,
    TWO_MIN = 0x04,
    FIVE_MIN = 0x05,
    TEN_MIN = 0x06,
    THIRTY_MIN = 0x07,
    SIXTY_MIN = 0x08
} TimeForNoPerson;

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//Underlying Open Function = UOF

typedef enum {
    TURN_OFF = 0x00,
    TURN_ON = 0x01
} OutputSwitch;

//UOF Information Output Switch
#define UOF_OUTPUT_SWITCH_CTRL 0x08
#define UOF_OUTPUT_SWITCH_CMD 0x00
#define UOF_OUTPUT_SWITCH_LEN 1

//UOF Information Output Switch Inquiry
#define UOF_OUTPUT_SWITCH_I_CTRL 0x08
#define UOF_OUTPUT_SWITCH_I_CMD 0x80
#define UOF_OUTPUT_SWITCH_I_LEN 1
#define UOF_OUTPUT_SWITCH_I_DATA 0x0F

//UOF Reporting of Sensor Information
#define UOF_REPORT_CTRL 0x08
#define UOF_REPORT_CMD 0x01
#define UOF_REPORT_LEN 5

//UOF Existence Energy Value Inquiry
#define UOF_EXISTENCE_ENERGY_I_CTRL 0x08
#define UOF_EXISTENCE_ENERGY_I_CMD 0x81
#define UOF_EXISTENCE_ENERGY_I_LEN 1
#define UOF_EXISTENCE_ENERGY_I_DATA 0x0F

//UOF Motion Energy Value Inquiry
#define UOF_MOTION_ENERGY_I_CTRL 0x08
#define UOF_MOTION_ENERGY_I_CMD 0x82
#define UOF_MOTION_ENERGY_I_LEN 1
#define UOF_MOTION_ENERGY_I_DATA 0x0F

//UOF Static Distance Inquiry
#define UOF_STATIC_DISTANCE_I_CTRL 0x08
#define UOF_STATIC_DISTANCE_I_CMD 0x83
#define UOF_STATIC_DISTANCE_I_LEN 1
#define UOF_STATIC_DISTANCE_I_DATA 0x0F

//UOF Motion Distance Inquiry
#define UOF_MOTION_DISTANCE_I_CTRL 0x08
#define UOF_MOTION_DISTANCE_I_CMD 0x84
#define UOF_MOTION_DISTANCE_I_LEN 1
#define UOF_MOTION_DISTANCE_I_DATA 0x0F

//UOF Motion Speed Inquiry
#define UOF_MOTION_SPEED_I_CTRL 0x05
#define UOF_MOTION_SPEED_I_CMD 0x85
#define UOF_MOTION_SPEED_I_LEN 1
#define UOF_MOTION_SPEED_I_DATA 0x0F

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//Custom Mode = CM

//CM Setting
#define CM_SETTING_CTRL 0x05
#define CM_SETTING_CMD 0x09
#define CM_SETTING_LEN 1

//End of CM Settings
#define CM_SETTING_END_CTRL 0x05
#define CM_SETTING_END_CMD 0x0A
#define CM_SETTING_END_LEN 1
#define CM_SETTING_END_DATA 0x0F

//CM Query
#define CM_Q_CTRL 0x05
#define CM_Q_CMD 0x89
#define CM_Q_LEN 1
#define CM_Q_DATA 0x0F

//CM Existence Judgment Threshold Settings
#define CM_EXISTENCE_JUDGMENT_THRESH_CTRL 0x08
#define CM_EXISTENCE_JUDGMENT_THRESH_CMD 0x08
#define CM_EXISTENCE_JUDGMENT_THRESH_LEN 1

//CM Motion Trigger Threshold Settings
#define CM_MOTION_TRIGGER_THRESH_CTRL 0x08
#define CM_MOTION_TRIGGER_THRESH_CMD 0x09
#define CM_MOTION_TRIGGER_THRESH_LEN 1

//CM Existence Perception Boundary Settings
#define CM_EXISTENCE_PERCEPTION_BOUND_CTRL 0x08
#define CM_EXISTENCE_PERCEPTION_BOUND_CMD 0x0A
#define CM_EXISTENCE_PERCEPTION_BOUND_LEN 1

//CM Motion Trigger Boundary Setting
#define CM_MOTION_TRIGGER_BOUND_CTRL 0x08
#define CM_MOTION_TRIGGER_BOUND_CMD 0x0B
#define CM_MOTION_TRIGGER_BOUND_LEN 1

//CM Motion Trigger Time Setting
#define CM_MOTION_TRIGGER_TIME_CTRL 0x08
#define CM_MOTION_TRIGGER_TIME_CMD 0x0C
#define CM_MOTION_TRIGGER_TIME_LEN 4

//CM Motion-to-Still Time Setting
#define CM_MOTION_TO_STILL_TIME_CTRL 0x08
#define CM_MOTION_TO_STILL_TIME_CMD 0x0D
#define CM_MOTION_TO_STILL_TIME_LEN 4

//CM Time for Entering no Person State Setting
#define CM_TIME_FOR_NO_PERSON_CTRL 0x08
#define CM_TIME_FOR_NO_PERSON_CMD 0x0E
#define CM_TIME_FOR_NO_PERSON_LEN 4

//CM-UOP Existence Judgment Threshold Inquiry
#define CM_UOF_EXISTENCE_JUDGMENT_THRESH_I_CTRL 0x08
#define CM_UOF_EXISTENCE_JUDGMENT_THRESH_I_CMD 0x88
#define CM_UOF_EXISTENCE_JUDGMENT_THRESH_I_LEN 1
#define CM_UOF_EXISTENCE_JUDGMENT_THRESH_I_DATA 0x0F

//CM-UOP Motion Trigger Threshold Inquiry
#define CM_UOF_MOTION_TRIGGER_THRESH_I_CTRL 0x08
#define CM_UOF_MOTION_TRIGGER_THRESH_I_CMD 0x89
#define CM_UOF_MOTION_TRIGGER_THRESH_I_LEN 1
#define CM_UOF_MOTION_TRIGGER_THRESH_I_DATA 0x0F

//CM-UOP Existence Perception Boundary Inquiry
#define CM_UOF_EXISTENCE_PERCEPTION_BOUND_I_CTRL 0x08
#define CM_UOF_EXISTENCE_PERCEPTION_BOUND_I_CMD 0x8A
#define CM_UOF_EXISTENCE_PERCEPTION_BOUND_I_LEN 1
#define CM_UOF_EXISTENCE_PERCEPTION_BOUND_I_DATA 0x0F

//CM-UOP Motion Trigger Boundary Inquiry
#define CM_UOF_MOTION_TRIGGER_BOUND_I_CTRL 0x08
#define CM_UOF_MOTION_TRIGGER_BOUND_I_CMD 0x8B
#define CM_UOF_MOTION_TRIGGER_BOUND_I_LEN 1
#define CM_UOF_MOTION_TRIGGER_BOUND_I_DATA 0x0F

//CM-UOP Motion Trigger Time Inquiry
#define CM_UOF_MOTION_TRIGGER_TIME_I_CTRL 0x08
#define CM_UOF_MOTION_TRIGGER_TIME_I_CMD 0x8C
#define CM_UOF_MOTION_TRIGGER_TIME_I_LEN 1
#define CM_UOF_MOTION_TRIGGER_TIME_I_DATA 0x0F

//CM-UOP Motion-to-Still Time Inquiry
#define CM_UOF_MOTION_TO_STILL_TIME_I_CTRL 0x08
#define CM_UOF_MOTION_TO_STILL_TIME_I_CMD 0x8D
#define CM_UOF_MOTION_TO_STILL_TIME_I_LEN 1
#define CM_UOF_MOTION_TO_STILL_TIME_I_DATA 0x0F

//CM-UOP Time for Entering no Person State Inquiry
#define CM_UOF_TIME_FOR_NO_PERSON_I_CTRL 0x08
#define CM_UOF_TIME_FOR_NO_PERSON_I_CMD 0x8E
#define CM_UOF_TIME_FOR_NO_PERSON_I_LEN 1
#define CM_UOF_TIME_FOR_NO_PERSON_I_DATA 0x0F

//Custom Mode Numbers
typedef enum {
    CUSTOM_MODE_1 = 0x01,
    CUSTOM_MODE_2 = 0x02,
    CUSTOM_MODE_3 = 0x03,
    CUSTOM_MODE_4 = 0x04,
} CustomMode;

//Presence values:
typedef enum {
    UNOCCUPIED = 0x00,
    OCCUPIED = 0x01
} PresenceInfo;

//Motion values:
typedef enum {
    NONE = 0x00,
    MOTIONLESS = 0x01,
    ACTIVE = 0x02
} MotionInfo;

//Proximity values:
typedef enum {
    NO_STATE = 0x00,
    NEAR = 0x01,
    FAR = 0x02
} ProximityInfo;

//Existence perception boundaries:
typedef enum {
    HALF_M = 0x01,
    ONE_M = 0x02,
    ONE_AND_HALF_M = 0x03,
    TWO_M = 0x04,
    TWO_AND_HALF_M = 0x05,
    THREE_M = 0x06,
    THREE_AND_HALF_M = 0x07,
    FOUR_M = 0x08,
    FOUR_AND_HALF_M = 0x09,
    FIVE_M = 0x0a
} ExistencePerceptionBound;

//Motion trigger boundaries:
typedef enum {
    HALF_M = 0x01,
    ONE_M = 0x02,
    ONE_AND_HALF_M = 0x03,
    TWO_M = 0x04,
    TWO_AND_HALF_M = 0x05,
    THREE_M = 0x06,
    THREE_AND_HALF_M = 0x07,
    FOUR_M = 0x08,
    FOUR_AND_HALF_M = 0x09,
    FIVE_M = 0x0a
} MotionTriggerBound;