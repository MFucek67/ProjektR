#pragma once
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "mmwave_core_interface.h"

//status operacije nad senzorom:
typedef enum {
    S_MMWAVE_OK = 0,
    S_MMWAVE_MEMORY_PROBLEM = 1,
    S_MMWAVE_ERR_INVALID_PARAM = 2,
    S_MMWAVE_ERR_TIMEOUT = 3
} mmwave_status_t;

//status pronalaska okvira:
typedef enum {
    MMWAVE_FRAME_OK = 0, //nađen je barem jedan cijeli okvir i stavljen u queue
    MMWAVE_NO_FRAMES = 1, //nije pronađen niti jedan valjan frame (bajtovi su svi odbačeni)
    MMWAVE_QUEUE_FULL = 2, //okvir je valjan, ali queue je pun, odbacuje se okvir
    MMWAVE_MEMORY_PROBLEM = 3, //nedovoljno memorije na heapu
    MMWAVE_UNFINISHED_FRAME = 4 //nije pronađen niti jedan cijeli okvir, ali postoji jedan u izgradnji
} mmwave_frame_status_t;

//definicije za headere i footere:
#define HEADER1 0x53
#define HEADER2 0x59
#define FOOTER1 0x54
#define FOOTER2 0x43

#define STARTING_PARSER_BUFFER_SIZE 20
#define MAX_PARSER_BUFFER_SIZE (65535 + 9)

mmWaveFrame* mmwave_build_frame(const uint8_t* payload, size_t payload_len, const uint8_t ctrl_w, const uint8_t cmd_w);
mmwave_frame_status_t mmwave_parse_data(const uint8_t* data, size_t data_len);
mmwave_status_t mmwave_init(void);
mmwave_status_t mmwave_stop(void);