#include <stdio.h>
#include <string.h>
#include "app/app_mmwave_constants.h"
#include "app/app_mmwave.h"
#include "app/app_network.h"
#include "platform/platform.h"
#include "tests/test_wifi_config.h"
#include "tests/dataset_collector.h"

static task_handler c_task;
static volatile uint32_t report_c;
static volatile uint32_t response_c;
static volatile bool stop_flag = false;
static volatile bool task_ended = false;
static volatile bool recording_active = false;
static volatile uint32_t time_interval = 0;
static volatile bool run = false;
static volatile bool first_heartbeat_received = false;

static void collector_task(void *arg) 
{
    for(;;) {
        if(stop_flag) {
            task_ended = true;
            printf("[COLLECTOR TASK] zavrsio s radom\n");
            platform_delete_task(NULL);
            break;
        }

        app_log_system_snapshot();
        sent_via_network_statistics(&report_c, &response_c);
        printf("Reports: %ld; Responses: %ld;\n", report_c, response_c);
        printf("\n");
        platform_delay_task(SYSTEM_STATISTICS_LOG_INTERVAL);
    }
}

void run_dataset_collector(void)
{
    app_network_config cfg;
    strncpy(cfg.ssid, TEST_NW_SSID, (sizeof(cfg.ssid) - 1));
    strncpy(cfg.pass, TEST_NW_PASSWORD, (sizeof(cfg.pass) - 1));
    strncpy(cfg.uri, TEST_SERVER_URI, (sizeof(cfg.uri) - 1));
    cfg.port = TEST_SERVER_PORT;

    network_init(cfg);

    network_start();
    time_interval = platform_getNumOfMs();

    stop_flag = false;
    task_ended = false;

    TaskConfig_t st = {collector_task, "stress_task", 12000, NULL, 8};
    c_task = platform_create_task(&st);

    platform_delay_task(1000);
    run = true;

    uint8_t packet[MAX_PACKET_LEN];
    size_t packet_length = 0;

    while(run) {
        if(network_poll_rx(&packet, MAX_PACKET_LEN, &packet_length, 200)) {
            CommandPacket* cmd_packet = (CommandPacket*) packet;
            if(cmd_packet->protocol_num != PROTOCOL_NUM) {
                continue; //nepoznate pakete ignoriramo
            }
            switch (cmd_packet->command_type)
            {
            case SYSTEM_HEARTBEAT:
                if(!first_heartbeat_received) {
                    first_heartbeat_received = true;
                }
                time_interval = platform_getNumOfMs();
                break;
            case SYSTEM_SHUTDOWN:
                if(recording_active) {
                    recording_active = false;
                    mmwave_stop();
                    mmwave_deinit();
                }
                stop_flag = true;
                while(task_ended != true) {
                    platform_delay_task(10);
                }
                run = false;
                break;
            case START_RECORDING:
                if(!recording_active) {
                    recording_active = true;
                    mmwave_init();
                    mmwave_start();
                    platform_delay_task(1000);
                    
                    app_inquiry_scene_settings_set(LIVING_ROOM);
                    app_inquiry_sensitivity_settings_set(SENSITIVITY_3);
                    app_inquiry_time_for_no_person_set(TEN_SEC);
                    app_inquiry_uof_output_switch_set(TURN_ON);
                }
                break;
            case STOP_RECORDING:
                if(recording_active) {
                    recording_active = false;
                    mmwave_stop();
                    mmwave_deinit();
                }
                break;
            default:
                break;
            }
        }
        if(first_heartbeat_received && (platform_getNumOfMs() - time_interval) > SHUTDOWN_TIMEOUT) { //ako određeno vrijeme ne dođe HEARTBEAT - napravi SYSTEM SHUTDOWN
            if(recording_active) {
                recording_active = false;
                mmwave_stop();
                mmwave_deinit();
                stop_flag = true;
                while(task_ended != true) {
                    platform_delay_task(10);
                }
            }
            run = false;
            break;
        }
    }
    network_stop();
    network_uninit();
}