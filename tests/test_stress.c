#include <stdio.h>
#include <string.h>
#include "tests/test_stress.h"
#include "tests/test_wifi_config.h"
#include "app/app_mmwave_hal_config.h"
#include "app/app_mmwave_constants.h"
#include "app/app_mmwave_decoder.h"
#include "app/app_mmwave_manager.h"
#include "app/app_mmwave.h"
#include "app/app_network.h"
#include "platform/platform.h"

#define EVENT_POLL_TIMEOUT_IN_MS 20

static task_handler st_task;
static volatile uint32_t report_c;
static volatile uint32_t response_c;
static volatile bool stop_flag = false;
static volatile bool task_ended = false;

static void stress_task(void* arg)
{
    for(;;) {
        if(stop_flag) {
            task_ended = true;
            platform_delete_task(NULL);
        }

        app_log_system_snapshot();
        sent_via_network_statistics(&report_c, &response_c);
        printf("Reports: %ld; Responses: %ld;\n", report_c, response_c);
        printf("\n");
        platform_delay_task(SYSTEM_STATISTICS_LOG_INTERVAL);
    }
}

/*static void analise_event_lite(void) {
    DecodedReport sensor_report;
    DecodedResponse sensor_response;
    if(mmwave_poll_report(&sensor_report, EVENT_POLL_TIMEOUT_IN_MS)) {
        report_c++;
    }
    if(mmwave_poll_response(&sensor_response, EVENT_POLL_TIMEOUT_IN_MS)) {
        response_c++;
    }
}*/

void stress_run_test(void)
{
    mmwave_init();

    app_network_config cfg;
    strncpy(cfg.ssid, TEST_NW_SSID, (sizeof(cfg.ssid) - 1));
    strncpy(cfg.pass, TEST_NW_PASSWORD, (sizeof(cfg.pass) - 1));
    strncpy(cfg.uri, TEST_SERVER_URI, (sizeof(cfg.uri) - 1));
    cfg.port = TEST_SERVER_PORT;

    network_init(cfg);

    mmwave_start();
    network_start();

    stop_flag = false;
    task_ended = false;

    uint32_t start = platform_getNumOfMs();

    TaskConfig_t st = {stress_task, "stress_task", 12000, NULL, 8};
    st_task = platform_create_task(&st);

    platform_delay_task(5000);

    app_inquiry_scene_settings_set(LIVING_ROOM);
    app_inquiry_time_for_no_person_set(TEN_SEC);
    app_inquiry_uof_output_switch_set(TURN_ON);

    while(platform_getNumOfMs() - start < TEST_DURATION) {
        //wait
        platform_delay_task(50);
    }
    stop_flag = true;
    while(task_ended != true) {
        platform_delay_task(10);
    }
    
    network_stop();
    mmwave_stop();

    network_uninit();
    mmwave_deinit();
}