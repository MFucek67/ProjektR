#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "my_hal/hal_mmwave.h"
#include "mmwave_interface/mmwave.h"

bool save_frame_test(mmWaveFrameData* frame) {
    printf("[HAL callback] frame saved, len=%zu\n", frame->data_len);
    return true;
}

static mmWave_core_callback cb_test;

void test_mmwave_core() {
    cb_test.mmwave_save_frame = save_frame_test;

    mmwave_core_bind_callbacks(&cb_test);

    uint8_t data[] = {0x80, 0x01, 0x55};
    printf("Parsing data...\n");
    cb_test.mmwave_save_frame(&(mmWaveFrameData){data, sizeof(data)});
}