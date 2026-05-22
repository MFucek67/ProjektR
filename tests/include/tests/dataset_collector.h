#pragma once
#include "stdio.h"

#define SYSTEM_STATISTICS_LOG_INTERVAL (30 * 1000) //svakih koliko logiramo stanje - 30 sekundi
#define MAX_PAYLOAD_LEN 128
#define MAX_PACKET_LEN sizeof(CommandPacket)

#define PROTOCOL_NUM 0xFF

#define SHUTDOWN_TIMEOUT 30000

typedef enum {
    START_RECORDING = 0x01,
    STOP_RECORDING = 0x02,
    SYSTEM_SHUTDOWN = 0x03,
    SYSTEM_HEARTBEAT = 0x04
} CommandType;

typedef struct __attribute__((packed)) {
    uint8_t protocol_num;
    uint8_t command_type;
    uint16_t payload_len;
    uint8_t payload[MAX_PAYLOAD_LEN];
} CommandPacket;

void run_dataset_collector(void);