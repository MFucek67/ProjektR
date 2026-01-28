#include <stdio.h>
#include "app/app_mmwave_decoder.h"
#include "app/app_types.h"

// Callback funkcije
void sendReport(DecodedReport* r) {
    printf("Report callback: has_bmp_info=%d, bmp=%d\n",
           r->has_bmp_info, r->bmp_info);
}

void sendResponse(DecodedResponse* r) {
    printf("Response callback: type=%d, data[0]=%d\n",
           r->type, (r->data) ? r->data[0] : -1);
}

void test_app_decoder() {
    AppDecoderContext ctx = {sendReport, sendResponse};
    app_mmwave_decoder_init(&ctx);

    uint8_t report_frame[] = {0x80, BMP_INFO_CMD, 0x42};
    uint8_t response_frame[] = {HEARTBEAT_CTRL, HEARTBEAT_CMD, 0x01};

    printf("Processing report...\n");
    app_mmwave_decoder_process_frame(report_frame, sizeof(report_frame));

    printf("Processing response...\n");
    app_mmwave_decoder_process_frame(response_frame, sizeof(response_frame));

    app_mmwave_decoder_deinit();
}