/**
 * @file esp32_board.h
 * @author Marko Fuček
 * @brief Konfiguracijska datoteka s UART sučeljima za ESP32 Wroom pločicu.
 * 
 * Ova datoteka definira mapiranje UART periferije na GPIO pinove ESP32 Wroom ploče.
 * 
 * Postoje dva logička UART sučelja (zadana u board.h):
 * -BOARD_UART_CONSOLE
 * -BOARD_UART_PROTOCOL
 * 
 * Logičkim UART sučeljima se kroz pomoćne funkcije u platform sloju dodjeljuju konkretne
 * vrijednosti temeljem mapiranja.
 * 
 * @version 0.1
 * @date 2026-01-22
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "driver/uart.h"
#include "driver/gpio.h"
#include "board.h"

/**
 * @brief Fizičko UART sučelje korišteno za konzolu i debbug.
 * 
 */
#define ESP32_BOARD_UART_CONSOLE_NUM UART_NUM_1

/**
 * @brief TX GPIO pin za konzolni UART.
 * 
 */
#define ESP32_BOARD_UART_CONSOLE_TX GPIO_NUM_10

/**
 * @brief RX GPIO pin za konzolni UART.
 * 
 */
#define ESP32_BOARD_UART_CONSOLE_RX GPIO_NUM_9

/**
 * @brief Fizičko UART sučelje korišteno za protokolnu komunikaciju.
 * 
 */
#define ESP32_BOARD_UART_PROTOCOL_NUM UART_NUM_2

/**
 * @brief TX GPIO pin za konzolni UART.
 * 
 */
#define ESP32_BOARD_UART_PROTOCOL_TX GPIO_NUM_17

/**
 * @brief RX GPIO pin za konzolni UART.
 * 
 */
#define ESP32_BOARD_UART_PROTOCOL_RX GPIO_NUM_16

/**
 * @struct esp32_uart_struct
 * @brief Struktura koja opisuje konfiguraciju jednog ESP32 UART sučelja.
 * 
 */
typedef struct {
    uart_port_t uart_num;
    gpio_num_t tx_gpio_port_num;
    gpio_num_t rx_gpio_port_num;
} esp32_uart_struct;
