/**
 * @file app_mmwave_hal_config.c
 * @author Marko Fuček
 * @brief Implementacija konfiguracije HAL sloja i mmWave core interface-a.
 * 
 * Modul dohvaća parametre za konfiguraciju HAL-a i sučelje s callbackovima na funkcije mmWave
 * core interface-a. Pritom on ne implementira niti HAL niti core, već služi kao centralno mjesto
 * aplikacijske konfiguracije.
 * 
 * @version 0.1
 * @date 2026-01-23
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "app/app_mmwave_hal_config.h"
#include "board_mmwave_uart_config.h"
#include "esp32_board.h"
#include "mmwave_interface/mmwave.h"
#include "mmwave_interface/mmwave_core_interface.h"

/**
 * @brief Aplikacijska konfiguracija za HAL.
 * 
 * Vrijednosti su definirane na razini pločice, a aplikacijski sloj ih samo
 * povezuje i prosljeđuje HAL-u.
 * 
 */
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

/**
 * @brief Aplikacijsko sučelje (interface) prema mmWave core.
 * 
 * Struktura sadrži pokazivače na funkcije koje aplikacijski sloj ne koristi, 
 * već samo prosljeđuje HAL sloju.
 * 
 */
static mmWave_core_interface mmwave_int = {
    .mmwave_parse_data = mmwave_parse_data,
    .mmwave_core_init = mmwave_core_init,
    .mmwave_core_stop = mmwave_core_stop,
    .mmwave_build_frame = mmwave_build_frame
};  

const hal_mmwave_config* app_mmwave_get_hal_config(void)
{
    return &hal_cfg;
}

const mmWave_core_interface* app_mmwave_get_core_interface(void)
{
    return &mmwave_int;
}