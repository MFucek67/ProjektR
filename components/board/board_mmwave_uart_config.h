#pragma once
#include "driver/uart.h"

#define BAUDRATE 115200
#define DATA_BITS UART_DATA_8_BITS
#define PARITY UART_PARITY_DISABLE
#define STOP_BITS UART_STOP_BITS_1
#define RX_BUFF_SIZE 1024
#define TX_BUFF_SIZE 1024
#define RX_THRESH 10
#define EVENT_QUEUE_LEN 20