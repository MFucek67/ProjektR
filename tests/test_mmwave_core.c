/**
 * @file test_mmwave_core.c
 * @author Marko Fuček
 * @brief Mock test mmWave core sloja.
 * 
 * Ovaj modul predstavlja mock test mmWave core sloja.
 * 
 * Modul koristi mockane (izmišljene i pojednostavljenje) HAL funkcije za svoj rad.
 * 
 * Test se sastoji od:
 * - Bindanja (povezivanja) HAL callback funkcija
 * - Inicijalizacije mmWave core sloja
 * - Slanja i parsiranja ispravnog frame-a
 * - Pokušaja parsiranja frame-a s neispravnim checksumom
 * - Slanja i parsiranja više frame-ova u istom skupu ulaznih podataka
 * - Pokušaja parsiranja neispravnog frame-a
 * - Izgradnje frame-a
 * - Zaustavljanja rada mmWave core sloja
 * 
 * @note Test se bavi isključivo testiranjem mmWave core sloja i ne obuhvaća ostale slojeve.
 * @version 0.1
 * @date 2026-01-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "mmwave_interface/mmwave_core_types.h"
#include "mmwave_interface/mmwave_core_interface.h"
#include "mmwave_interface/mmwave.h"

//Napravit ćemo umjetne HAL callbackove, jer nam HAL nije u cilju testiranja, samo želimo što jednostavniju funkcionalnost:
//Umjetna (mock) funkcija za alokaciju memorije:
static uint8_t* test_alloc_mem(size_t size) {
    return malloc(size);
}

//Umjetna (mock) funkcija za oslobađanje alocirane memorije:
static void test_free_mem(uint8_t* mem, size_t size) {
    free(mem);
}

static int frames_saved = 0;
static uint8_t last_ctrl_w = 0;
static uint8_t last_cmd_w = 0;
static uint8_t last_payload[64];
static size_t last_payload_len = 0;

//Umjetna (mock) funkcija za onu koja bi u pravom sustavu spremala frame u queue.
//za test je dovoljno da samo spremi u globalne varijable i "zabilježi" novi frame
static bool test_save_frame(mmWaveFrameSemanticData* frame_data) {
    if(frame_data->len < 2) {
        if(frame_data->data) {
            free(frame_data->data);
        }
        return false;
    }
    
    frames_saved++;
    last_ctrl_w = frame_data->data[0];
    last_cmd_w = frame_data->data[1];
    last_payload_len = frame_data->len - 2;
    
    if(last_payload_len > 0) {
        //samo kopiramo podatke u buffer, nećemo raditi s heapom
        memcpy(last_payload, &frame_data->data[2], last_payload_len);
    }
    
    printf("[CORE test] Frame spremljen: ctrl=0x%02X cmd=0x%02X payload_len=%zu\n",
        last_ctrl_w, last_cmd_w, last_payload_len);
    
    if(frame_data->data) {
        free(frame_data->data);
    }
    return true;
}

//Funkcija koja resetira sve globalne varijable
void reset_test_state(void) {
    frames_saved = 0;
    last_ctrl_w = 0;
    last_cmd_w = 0;
    last_payload_len = 0;
    memset(last_payload, 0, sizeof(last_payload));
}

void mmwave_core_run_test(void) {
    printf("------------MMWAVE CORE TEST START------------\n");

    mmWave_core_callback callbacks = {
        .alloc_mem = test_alloc_mem,
        .free_mem = test_free_mem,
        .mmwave_save_frame = test_save_frame
    };
    
    //Prije inicijalizacije dat ćemo (mock) funkcije u callback strukturu
    mmwave_core_bind_callbacks(&callbacks);
    
    //Inicijalizacija:
    if(mmwave_core_init() == S_MMWAVE_OK) {
        printf("[CORE test] init successful\n");
    } else {
        printf("[CORE test] init unsuccessful\n");
        return;
    }

    //[1]. Prvo ispitujemo slanje i parsiranje ispravnog frame-a:
    reset_test_state();
    
    // Stvaramo valjan frame: HEARTBEAT response
    // Header: 0x53 0x59
    // Ctrl: 0x01
    //Cmd: 0x01
    // Len: 0x00 0x01
    // Payload: 0x0F
    // Checksum: 0x53 + 0x59 + 0x01 + 0x01 + 0x00 + 0x01 + 0x0F = 0xBE
    // Footer: 0x54 0x43
    uint8_t valid_frame[] = {0x53, 0x59, 0x01, 0x01, 0x00, 0x01, 0x0F, 0xBE, 0x54, 0x43};
    
    mmwave_frame_status_t status = mmwave_parse_data(valid_frame, sizeof(valid_frame));
    
    if(status == MMWAVE_FRAME_OK) {
        printf("[CORE test] Frame parsiran uspjesno\n");
    } else {
        printf("[CORE test] ERROR Frame parse neuspjesan\n");
    }
    
    if(frames_saved == 1) {
        printf("[CORE test] 1 frame spremljen\n");
    } else {
        printf("[CORE test] ERROR Ocekivan 1 frame, a imamo %d\n", frames_saved);
    }
    
    if(last_ctrl_w == 0x01 && last_cmd_w == 0x01) {
        printf("[CORE test] Ispravni ctrl/cmd words\n");
    } else {
        printf("[CORE test] ERROR Neispravni ctrl/cmd\n");
    }
    
    if(last_payload_len == 1 && last_payload[0] == 0x0F) {
        printf("[CORE test] Ispravan payload\n");
    } else {
        printf("[CORE test] Neispravan payload\n");
    }

    //[2]. Sada ispitujemo hoće li parser otkriti neispravan checksum:
    reset_test_state();
    
    // Frame s lošim checksumom
    uint8_t bad_frame[] = {0x53, 0x59, 0x01, 0x01, 0x00, 0x01, 0x0F,0xFF, 0x54, 0x43};
    
    mmwave_parse_data(bad_frame, sizeof(bad_frame));
    
    if(frames_saved == 0) {
        printf("[CORE test] Neispravan checksum detektiran - frame odbacen\n");
    } else {
        printf("[CORE test] ERROR Frame s neispravnim checksum prihvacen!\n");
    }

    //[3]. Sada ispitujemo kako parser reagira na višestruki prijam frame-ova:
    reset_test_state();

    // Dva frame-a zajedno
    uint8_t two_frames[] = {
        0x53, 0x59, 0x01, 0x01, 0x00, 0x01, 0x0F, 0xBE, 0x54, 0x43, // Frame 1: HEARTBEAT
        0x53, 0x59, 0x01, 0x02, 0x00, 0x01, 0x01, 0xB1, 0x54, 0x43 // Frame 2: MODULE_RESET
    };
    
    mmwave_parse_data(two_frames, sizeof(two_frames));
    
    if(frames_saved == 2) {
        printf("[CORE test] Oba frame-a su uspješno parsirana: frames_saved=%d\n", frames_saved);
    } else {
        printf("[CORE test] ERROR Očekivano je 2 parsirana frame-a, a imamo: %d\n", frames_saved);
    }

    //[4]. Sada ispitujemo kako parser reagira kada mu dođe "smeće":
    reset_test_state();

    //Smeće frame
    uint8_t garbage[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    
    status = mmwave_parse_data(garbage, sizeof(garbage));
    
    if(status == MMWAVE_NO_FRAMES && frames_saved == 0) {
        printf("[CORE test] Smece frame odbacen\n");
    } else {
        printf("[CORE test] ERROR Smece frame uspjesno prihvacen i parsiran - frames_saved=%d\n", frames_saved);
    }

    //[5]. Sada nam još preostaje ispitati izgradnju frame-a:
    uint8_t payload[] = {0x0F};
    mmWaveFrameForTX tx_frame;
    
    bool success = mmwave_build_frame(&tx_frame, payload, 1, 0x01, 0x01);
    
    if(success) {
        printf("[CORE test] Frame uspjesno izgraden\n");
        test_free_mem(tx_frame.data, tx_frame.len);
    } else {
        printf("[CORE test] ERROR Frame neuspjesno izgraden\n");
    }

    //Zaustavljamo rad parsera:
    if(mmwave_core_stop() == S_MMWAVE_OK) {
        printf("[CORE test] stop successful\n");
    } else {
        printf("[CORE test] stop unsuccessful\n");
        return;
    }

    printf("------------MMWAVE CORE TEST STOP------------\n");
}