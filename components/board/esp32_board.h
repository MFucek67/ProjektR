#pragma once
#include "driver/uart.h"
#include "driver/gpio.h"
#include "board.h"

#define ESP32_BOARD_UART_CONSOLE_NUM UART_NUM_1
#define ESP32_BOARD_UART_CONSOLE_TX GPIO_NUM_10
#define ESP32_BOARD_UART_CONSOLE_RX GPIO_NUM_9

#define ESP32_BOARD_UART_PROTOCOL_NUM UART_NUM_2
#define ESP32_BOARD_UART_PROTOCOL_TX GPIO_NUM_17
#define ESP32_BOARD_UART_PROTOCOL_RX GPIO_NUM_16

typedef struct {
    uart_port_t uart_num;
    gpio_num_t tx_gpio_port_num;
    gpio_num_t rx_gpio_port_num;
} esp32_uart_struct;
