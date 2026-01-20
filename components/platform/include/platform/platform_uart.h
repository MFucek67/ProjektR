#pragma once
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "../../../board/board.h"

extern bool hal_dispatcher_ended_flag;

typedef enum {
    UART_OK = 0,
    UART_TIMEOUT = 1,
    UART_ERROR = 2
} UARTStatus;

typedef struct 
{
    uint32_t baudrate;
    uint8_t data_bits;
    uint8_t parity;
    uint8_t stop_bits;
    size_t rx_buff_size;
    size_t tx_buff_size;
} platform_uart_config_t;

UARTStatus platform_uart_set_rx_threshold(const BoardUartId id, uint32_t bytes);
UARTStatus platform_uart_init(const BoardUartId id, const platform_uart_config_t* uart_config);
UARTStatus platform_uart_flush(const BoardUartId id);
uint32_t platform_uart_write(const BoardUartId id, uint8_t* data, int len);
uint32_t platform_uart_read(const BoardUartId id, uint8_t* buffer, int max_len, uint32_t ticks_to_wait);
UARTStatus platform_uart_event_converter_start(const BoardUartId id);
void platform_uart_event_converter_stop(void);
UARTStatus platform_uart_deinit(const BoardUartId id);
void* platform_uart_get_event_queue(void);
UARTStatus platform_ISR_disable(const BoardUartId id);
UARTStatus platform_ISR_enable(const BoardUartId id);

//BoardUartId je samo izmišljeni (generalizirani) id koji predstavlja jedan od (trenutno) 2 UART-a na logičkoj razini
//BOARD_UART_CONSOLE je za ispis, debugganje itd.
//BOARD_UART_PROTOCOL je za konkretan prijenos bajtova podataka (okvira)
//svaki od ova dva se mogu pridjeliti nekom od točno određenih uart-a za neku pločicu (zadanih pod <boardName>_board.h)