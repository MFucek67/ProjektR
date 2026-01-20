#include "app/app_mmwave_hal_config.h"
#include "board_mmwave_uart_config.h"
#include "esp32_board.h"
#include "mmwave_interface/mmwave.h"
#include "mmwave_interface/mmwave_core_interface.h"

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

static mmWave_core_interface mmwave_int = {
    .mmwave_parse_data = mmwave_parse_data,
    .mmwave_init = mmwave_init,
    .mmwave_stop = mmwave_stop,
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