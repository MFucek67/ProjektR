/**
 * @file platform_uart.h
 * @author Marko Fuček
 * @brief Platform API za upravljanje UART prijenosom bajtova.
 * 
 * Ovaj modul pruža apstrakciju UART prijenosa uključujući
 * inicijalizaciju, pisanje/čitanje s UART-a, flush,
 * ISR kontrolu i event converter.
 * 
 * @note BoardUartId je samo zamišljeni (logički) UART port.
 *          - BOARD_UART_CONSOLE: debbug/ispis
 *          - BOARD_UART_PROTOCOL: prijenos podataka (okvira)
 * 
 * Svaki od ova dva logička porta se mogu pridodijeliti nekom od točno određenih,
 * fizičkih UART portova na pločici (zadani u mapi board pod <board_name>_board.h)
 * @version 0.1
 * @date 2026-01-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "board.h"

extern bool volatile hal_dispatcher_ended_flag;

/**
 * @enum UARTStatus
 * @brief Statusi UART operacija.
 * 
 */
typedef enum {
    UART_OK, /**< UART operacija uspješna */
    UART_TIMEOUT, /**< Istek vremena */
    UART_ERROR /**< UART operacija neuspješna */
} UARTStatus;

/**
 * @struct platform_uart_config_t
 * @brief Konfiguracija UART drivera
 * 
 */
typedef struct 
{
    uint32_t baudrate; /**< UART baudrate */
    uint8_t data_bits; /**< Broj data bitova */
    uint8_t parity; /**< Paritet */
    uint8_t stop_bits; /**< Broj stop bitova */
    size_t rx_buff_size; /**< Veličina UART RX buffera */
    size_t tx_buff_size; /**< Veličina UART TX buffera */
} platform_uart_config_t;

/**
 * @brief Postavlja RX threshold na koji ISR šalje UART_DATA event.
 * 
 * @param id Logički UART id
 * @param bytes Granica za slanje UART_DATA
 * @return Status UART operacije
 */
UARTStatus platform_uart_set_rx_threshold(const BoardUartId id, uint32_t bytes);

/**
 * @brief Vrši inicijalizaciju UART drivera korištenjem zadane konfiguracije.
 * 
 * @param id Logički UART id
 * @param uart_config UART driver konfiguracija
 * @return Status UART operacije
 */
UARTStatus platform_uart_init(const BoardUartId id, const platform_uart_config_t* uart_config);

/**
 * @brief Čisti RX/TX buffere UART-a.
 * 
 * @param id Logički UART id
 * @return Status UART operacije
 */
UARTStatus platform_uart_flush(const BoardUartId id);

/**
 * @brief Piše podatke na UART.
 * 
 * @param id Logički UART id
 * @param data Pokazivač na podatke koji se šalju
 * @param len Duljina podataka koji se šalju
 * @return Broj stvarno poslanih podataka preko UART-a
 */
uint32_t platform_uart_write(const BoardUartId id, uint8_t* data, size_t len);

/**
 * @brief Dohvaća podatke s UART-a.
 * 
 * @param id Logički UART id
 * @param buffer Pokazivač na buffer u koji se spremaju dohvaćeni podatci
 * @param max_len Maksimalna veličina dohvaćenih podataka
 * @param ticks_to_wait Timeout u tickovima
 * @return Broj dohvaćenih podataka
 */
uint32_t platform_uart_read(const BoardUartId id, uint8_t* buffer, int max_len, uint32_t ticks_to_wait);

/**
 * @brief Pokreće task koji upravlja konverzijom ISR eventova u platform evente.
 * 
 * @param id Logički UART id
 * @return Status UART operacije
 */
UARTStatus platform_uart_event_converter_start(const BoardUartId id);

/**
 * @brief Zaustavlja task koji vrši konverziju ISR evenata u platform evente i oslobađa zauzetu memoriju.
 * 
 */
void platform_uart_event_converter_stop(void);

/**
 * @brief Vrši deinicijalizaciju UART drivera.
 * 
 * @param id Logički UART id
 * @return Status UART operacije
 */
UARTStatus platform_uart_deinit(const BoardUartId id);

/**
 * @brief Dohvaća pokazivač na event queue koji dolaze s UART-a.
 * 
 * @return Pokazivač na UART event queue
 */
void* platform_uart_get_event_queue(void);

/**
 * @brief Onemogućuje UART RX interruptove.
 * 
 * @param id Logički UART id
 * @return Status UART operacije
 */
UARTStatus platform_ISR_disable(const BoardUartId id);

/**
 * @brief Omogućuje UART RX interruptove.
 * 
 * @param id Logički UART id
 * @return Status UART operacije 
 */
UARTStatus platform_ISR_enable(const BoardUartId id);