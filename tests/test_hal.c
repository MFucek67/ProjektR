/**
 * @file test_hal.c
 * @author Marko Fuček
 * @brief Mock test HAL sloja.
 * 
 * Ovaj modul predstavlja mock test HAL sloja.
 * 
 * Test se sastoji od:
 * - Inicijalizacije HAL sloja
 * - Pokušaja dohvata frame-a iz neispravnog stanja
 * - Pokretanja HAL sloja
 * - Ispitivanja slanja zahtjeva na senzor (inquiry) i primanja response-a (odgovora) od senzora
 * - Ispitivanja dohvaćanja report-a (izvještaja) sa senzora
 * - Ispitivanja pokušaja deinicijalizacije HAL sloja prije zaustavljanja HAL sloja
 * - Zaustavljanja HAL sloja
 * - Deinicijalizacije HAL sloja
 * 
 * @note Test se bavi isključivo testiranjem HAL sloja i ne obuhvaća ostale slojeve (osim core sloja interno).
 * 
 * @version 0.1
 * @date 2026-01-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include "tests/test_hal.h"
#include "my_hal/hal_mmwave.h"
#include "esp32_board.h"
#include "board_mmwave_uart_config.h"
#include "mmwave_interface/mmwave.h"
#include "mmwave_interface/mmwave_core_types.h"
#include "mmwave_interface/mmwave_core_interface.h"
#include "platform/platform.h"

static mmWave_core_interface mmwave_int = {
    .mmwave_parse_data = mmwave_parse_data,
    .mmwave_core_init = mmwave_core_init,
    .mmwave_core_stop = mmwave_core_stop,
    .mmwave_build_frame = mmwave_build_frame
}; 

static hal_mmwave_config hal_cfg = {
    .id = ESP32_BOARD_UART_PROTOCOL_NUM,
    .baudrate = BAUDRATE,
    .data_bits = DATA_BITS,
    .parity = PARITY,
    .stop_bits = STOP_BITS,
    .rx_buff_size = RX_BUFF_SIZE,
    .tx_buff_size = TX_BUFF_SIZE,
    .rx_thresh = RX_THRESH,
    .event_queue_len = EVENT_QUEUE_LEN
};

void hal_mmwave_run_test(void)
{
    printf("------------HAL TEST START------------\n");

    //Inicijalizacija
    if(hal_mmwave_init(&hal_cfg, &mmwave_int) != HAL_MMWAVE_OK) {
        printf("[HAL test] Init error\n");
        return;
    } else {
        printf("[HAL test] Init successful\n");
    }

    //Pokušamo dohvatiti frame prije nego što je išta parsirano (očekivan HAL ERROR):
    FrameData_t buffer;
    if(hal_mmwave_get_frame_from_queue(&buffer, 20) != HAL_MMWAVE_OK) {
        printf("[HAL test] Ocekivano ponasanje - jos nema frame-ova\n");
    } else {
        printf("[HAL test] Neocekivano ponasanje - ne bi smjelo vec imati frame-ova\n");
        return;
    }

    //Ovdje startamo HAL taskove:
    if(hal_mmwave_start() != HAL_MMWAVE_OK) {
        printf("[HAL test] Start error\n");
        return;
    } else {
        printf("[HAL test] Start successful\n");
    }

    //[1]. Prvo ispitujemo slanje zahtjeva i odgovor senzora:
    //Slanje upita (inquiry):
    uint8_t payload[] = {0x0F};
    if(hal_mmwave_send_frame(payload, 1, 0x01, 0x01) != HAL_MMWAVE_OK) {
        printf("[HAL test] Slanje HEARTBEAT upita neuspjesno\n");
    } else {
        printf("[HAL test] Upit za HEARTBEAT uspjesno poslan\n");

        //Ako smo uspjesno poslali, sada ćemo čekati SAMO ODGOVOR NA NAŠ UPIT, ostalo odbacujemo
        uint32_t start = platform_getNumOfMs();
        bool got_response = false;

        while(platform_getNumOfMs() - start < 3000) { //čekamo 3s na odgovor, nakon toga se može smatrati neuspjehom
            if(hal_mmwave_get_frame_from_queue(&buffer, 20) == HAL_MMWAVE_OK) {
                if(buffer.len >= 3) {
                    if(buffer.data[0] == 0x01 && buffer.data[1] == 0x01) {
                        if(buffer.data[2] == 0x0F) {
                            printf("[HAL test] HEARTBEAT odgovor uspjesno zaprimljen\n");
                            got_response = true;
                            hal_mmwave_release_frame_memory(&buffer);
                            break;
                        } else {
                            printf("[HAL test] Zagubljen je payload\n");
                        }
                    } else {
                        printf("[HAL test] Zaprimljeni su podatci, ali ne od HEARTBEAT zahtjeva\n");
                    }
                } else {
                    printf("[HAL test] Minimalna duljina semantickih podataka prekrsena.\n");
                }
                hal_mmwave_release_frame_memory(&buffer);
            }
        }
        if(!got_response) {
            printf("[HAL test] Response za HEARTBEAT nije dosao\n");
        }
    }

    //[2]. Sada ispitujemo primanje reportova:
    //Za potrebe ovog testa dovoljno je da senzor bude u Standard mode i da je No-Person-State postavljen na 10 sekundi
    uint8_t payload1[] = {0x00};
    if(hal_mmwave_send_frame(payload1, 1, 0x08, 0x00) != HAL_MMWAVE_OK) {
        printf("[HAL test] Neuspjesno postavljanje Standard Mode - test se prekida\n");
        return;
    }
    platform_delay_task(100);
    uint8_t payload2[] = {0x01};
    if(hal_mmwave_send_frame(payload2, 1, 0x80, 0x0A) != HAL_MMWAVE_OK) {
        printf("[HAL test] Neuspjesno postavljanje No-Person-State - test se prekida\n");
        return;
    }
    platform_delay_task(200); //čekamo da se pošalje No-Person-State i da parser obradi, kako njega ne bi pročitali kao report

    //Čistimo queue, da ne čitamo stare reportove i odgovore
    printf("[HAL test] Cistimo queue s frame-ovima da imamo frest start za reportove\n");
    hal_mmwave_flush_frames();

    //Ako su postavke uspješne i UART prazan, možemo čekati reportove:
    uint32_t start = platform_getNumOfMs();
    bool got_report = false;

    while(platform_getNumOfMs() - start < 10000) { //čekamo max 10 sekundi da dobijemo neki report, ako prođe može se smatrati neuspjehom
        if(hal_mmwave_get_frame_from_queue(&buffer, 20) == HAL_MMWAVE_OK) {
            if(buffer.len >= 3) {
                got_report = true;
                printf("[HAL test] Dobili smo report: ");
                for(int i = 0; i < buffer.len; i++) {
                    printf("0x%02x, ", buffer.data[i]);
                }
                printf("\n");
                hal_mmwave_release_frame_memory(&buffer);
                break;
            } else {
                printf("[HAL test] Minimalna duljina semantickih podataka prekrsena\n");
            }
            hal_mmwave_release_frame_memory(&buffer);
        }
    }
    if(!got_report) {
        printf("[HAL test] Nismo dobili niti jedan report\n");
    }

    //Pokušavamo deinit HAL-a prije zaustavljanja (očekivano INVALID_STATE):
    if(hal_mmwave_deinit() == HAL_MMWAVE_INVALID_STATE) {
        printf("[HAL test] Uspjeh - ocekivano ponasanje - ne smije se deinit prije stop\n");
    } else {
        printf("[HAL test] ERROR - deinit prosao\n");
        return;
    }

    //Zaustavljamo rad HAL-a:
    if(hal_mmwave_stop() != HAL_MMWAVE_OK) {
        printf("[HAL test] Stop error\n");
        return;
    } else {
        printf("[HAL test] Stop successful\n");
    }

    //Deinicijaliziramo HAL:
    if(hal_mmwave_deinit() != HAL_MMWAVE_OK) {
        printf("[HAL test] Deinit error\n");
        return;
    } else {
        printf("[HAL test] Deinit successful\n");
    }

    printf("------------HAL TEST STOP------------\n");
}