#pragma once
#include "stdint.h"

typedef int32_t BoardUartId;

#define BOARD_UART_UNINIT ((BoardUartId)-1)
#define BOARD_UART_CONSOLE ((BoardUartId)0)
#define BOARD_UART_PROTOCOL ((BoardUartId)1)