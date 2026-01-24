/**
 * @file board_mmwave_uart_config.h
 * @author Marko Fuček
 * @brief Zadane postavke UART drivera i komunikacije.
 * 
 * Ova datoteka definira osnovne parametre za konfiguraciju UART komunikacije
 * koje se koriste kod inicijalizacije UART drivera.
 * 
 * @version 0.1
 * @date 2026-01-22
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "driver/uart.h"

/**
 * @brief Brzina prijenosa (baud rate).
 * 
 */
#define BAUDRATE 115200

/**
 * @brief Broj podatkovinih bitova.
 * 
 */
#define DATA_BITS UART_DATA_8_BITS

/**
 * @brief Postavka pariteta.
 * 
 */
#define PARITY UART_PARITY_DISABLE

/**
 * @brief Broj stop bitova.
 * 
 */
#define STOP_BITS UART_STOP_BITS_1

/**
 * @brief Veličina RX buffera u bajtovima.
 * 
 */
#define RX_BUFF_SIZE 1024

/**
 * @brief Veličina TX buffera u bajtovima.
 * 
 */
#define TX_BUFF_SIZE 1024

/**
 * @brief Prag za generiranje UART DATA eventa.
 * 
 */
#define RX_THRESH 10

/**
 * @brief Veličina queue-a za UART eventove.
 * 
 */
#define EVENT_QUEUE_LEN 20