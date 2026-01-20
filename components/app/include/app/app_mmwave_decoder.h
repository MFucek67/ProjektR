#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "app_mmwave_constants.h"
#include "app_mmwave_manager.h"

typedef struct
{
    int existence_energy;
    float static_distance;
    int motion_energy;
    float motion_distance;
    float motion_speed;
} uof_report;


typedef struct {
    bool has_init_completed_info;
    bool has_presence_info;
    bool has_motion_info;
    bool has_bmp_info;
    bool has_proximity_info;
    bool has_uof_report;

    bool init_completed_info;
    PresenceInfo presence_info;
    MotionInfo motion_info;
    int bmp_info;
    ProximityInfo proximity_info;
    uof_report uof_rep;
} DecodedReport;

typedef struct {
    AppInquiryType type;
    void* data;
    size_t data_l;
} DecodedResponse;

typedef struct
{
    void (*sendReportCallback)(DecodedReport* report);
    void (*sendResponseCallback)(DecodedResponse* response);
} AppDecoderContext;

void app_mmwave_decoder_init(AppDecoderContext* ctx);
void app_mmwave_decoder_deinit();
void app_mmwave_decoder_process_frame(uint8_t* data, size_t data_len);