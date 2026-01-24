/**
 * @file esp32_uart.c
 * @author Marko Fuček
 * @brief ESP32 implementacija platform_uart API-ja.
 * 
 * Ovaj modul implementira platform_uart API koristeći ESP-IDF UART driver,
 * ISR za rad s eventima u RX bufferu i FreeRTOS mehanizme.
 * 
 * Modul uključuje:
 * Inicijalizaciju i deinicijalizaciju UART drivera.
 * Čitanje bajtova preko RX i pisanje bajtova preko TX.
 * Konverziju UART ISR generiranih eventova u platform generic evente.
 * Dispatcher funkciju za čekanje UART ISR generiranje eventova, njihovo prevođenje u platform i proslijeđivanje u queue.
 * Sigurno gašenje taska i onemogućivanje RX te TX pinova.
 * 
 * @note Ova implementacija specifična je za ESP32 Wroom implementaciju.
 * @warning Funkcije nisu thread-safe.
 * 
 * @version 0.1
 * @date 2026-01-21
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/queue.h"
#include "platform_uart.h"
#include "platform_events.h"
#include "board.h"
#include "esp32_board.h"

static bool rx_closed = false;
static QueueHandle_t uart_event_queue = NULL;
static QueueHandle_t platform_event_queue = NULL;
static TaskHandle_t dispatcher_task = NULL;

bool hal_dispatcher_ended_flag = false;
bool dispatcher_stop = false;

/**
 * @brief Mapira logički UART id na konkretne ESP32 UART pinove i UART broj.
 * 
 * Funkcija prevodi izmišljeni BoardUartId u konkretni UART_NUM i RX i TX pinove
 * na ESP32 pločici.
 * 
 * @param id Logički UART id
 * @return Struktura s ESP32 UART brojem te TX i RX pinovima
 */
static esp32_uart_struct find_uart(const BoardUartId id)
{
    esp32_uart_struct uart;
    switch (id)
    {
    case BOARD_UART_CONSOLE:
        uart.uart_num = ESP32_BOARD_UART_CONSOLE_NUM;
        uart.tx_gpio_port_num = ESP32_BOARD_UART_CONSOLE_TX;
        uart.rx_gpio_port_num = ESP32_BOARD_UART_CONSOLE_RX;
        break;
    case BOARD_UART_PROTOCOL:
        uart.uart_num = ESP32_BOARD_UART_PROTOCOL_NUM;
        uart.tx_gpio_port_num = ESP32_BOARD_UART_PROTOCOL_TX;
        uart.rx_gpio_port_num = ESP32_BOARD_UART_PROTOCOL_RX;
        break;
    default:
        uart.uart_num = ESP32_BOARD_UART_PROTOCOL_NUM;
        uart.tx_gpio_port_num = ESP32_BOARD_UART_PROTOCOL_TX;
        uart.rx_gpio_port_num = ESP32_BOARD_UART_PROTOCOL_RX;
        break;
    }
    return uart;
}

/**
 * @brief Pomoćna funkcija koja pretvara ESP-IDF ISR UART event u generički platform event.
 * 
 * Funkcija prevodi konkretni ESP-IDF event koji u queue postavlja ISR.
 * Ovisno o tipu preko pokazivača na PlatformEvent_t strukturu postavlja odgovarajuće
 * podatke o pripadnom platform eventu.
 * 
 * @param uart_event Pokazivač na strukturu uart eventa koji generira ISR, a pročitan je iz event queue
 * @param platform_event Pokazivač na strukturu platform eventa gdje se sprema prevedeni platform event
 * @return true ako su dani validni pokazivači na strukture 
 * @return false ako je bilo koji od danih pokazivača na strukture NULL
 */
static bool uart_event_to_platform_event(uart_event_t* uart_event, PlatformEvent_t* platform_event)
{
    if(!uart_event || !platform_event) {
        return false;
    }

    switch (uart_event->type)
    {
    case UART_DATA:
        platform_event->type = PLATFORM_EVENT_RX_DATA;
        platform_event->data = NULL;
        platform_event->len = uart_event->size;
        break;
    case UART_FIFO_OVF:
        platform_event->type = PLATFORM_EVENT_FIFO_OVF;
        platform_event->data = NULL;
        platform_event->len = uart_event->size;
        break;
    case UART_BUFFER_FULL:
        platform_event->type = PLATFORM_EVENT_BUFFER_FULL;
        platform_event->data = NULL;
        platform_event->len = uart_event->size;
        break;
    default:
        platform_event->type = PLATFORM_EVENT_ERR;
        platform_event->data = NULL;
        platform_event->len = uart_event->size;
        break;
    }
    return true;
}

/**
 * @brief FreeRTOS task funkcija za konverziju UART ISR eventova u platform eventove.
 * 
 * Task čita UART eventove koje je ISR postavio, prevodi ih
 * u PlatformEvent_t platform eventove i prosljeđuje u platform event queue.
 * 
 * Task se sam gasi i oslobađa svoje resurse kada je RX pin closed i queue s UART
 * eventovima od ISR-a prazan.
 * 
 * @param arg Ne koristi se
 * 
 * @warning Funkcija se izvršava u task kontekstu.
 */
static void dispatcher_function(void* arg)
{
    uart_event_t uart_ev;
    for(;;) {
        if((uxQueueMessagesWaiting(uart_event_queue) == 0 && rx_closed) || dispatcher_stop) {
            dispatcher_task = NULL;
            hal_dispatcher_ended_flag = true;
            vTaskDelete(NULL);
        }

        if(uart_event_queue == NULL || platform_event_queue == NULL) {
            vTaskDelay(20);
            continue;
        }
        if(xQueueReceive(uart_event_queue, &uart_ev, pdMS_TO_TICKS(20)) == pdTRUE) {
            PlatformEvent_t platform_ev;
            if(uart_event_to_platform_event(&uart_ev, &platform_ev)) {
                xQueueSend(platform_event_queue, &platform_ev, pdMS_TO_TICKS(50));
            }
        }
    }
}

UARTStatus platform_uart_set_rx_threshold(const BoardUartId id, uint32_t bytes) 
{
    esp32_uart_struct uart_numbers = find_uart(id);
    if(uart_set_rx_full_threshold(uart_numbers.uart_num, bytes) == ESP_OK) {
        return UART_OK;
    } else {
        return UART_ERROR;
    }
}

/**
 * @note Funkcija stvara interni platform event queue i instalira
 * ESP32 UART driver. 
 */
UARTStatus platform_uart_init(const BoardUartId id, const platform_uart_config_t* uart_config)
{   
    esp32_uart_struct uart_numbers = find_uart(id);
    if(uart_config == NULL) {
        return UART_TIMEOUT;
    }

    platform_event_queue = platform_create_event_queue(20);
    if(platform_event_queue == NULL) {
        return UART_TIMEOUT;
    }

    const uart_config_t config = {
        .baud_rate = uart_config->baudrate,
        .data_bits = uart_config->data_bits,
        .parity = uart_config->parity,
        .stop_bits = uart_config->stop_bits,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    if(uart_param_config(uart_numbers.uart_num, &config) != ESP_OK) {
        platform_uart_deinit(id);
        return UART_ERROR;
    }

    if(uart_driver_install(uart_numbers.uart_num, uart_config->rx_buff_size * 2, uart_config->tx_buff_size * 2,
            20, &uart_event_queue, 0) != ESP_OK) {
        platform_uart_deinit(id);
        return UART_ERROR;
    }

    if(uart_set_pin(uart_numbers.uart_num, uart_numbers.tx_gpio_port_num, uart_numbers.rx_gpio_port_num, 
            UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
        platform_uart_deinit(id);
        return UART_ERROR;
    }

    return UART_OK;
}

UARTStatus platform_uart_flush(const BoardUartId id)
{
    esp32_uart_struct uart_numbers = find_uart(id);
    if(uart_flush(uart_numbers.uart_num) != ESP_OK) {
        return UART_ERROR;
    }
    return UART_OK;
}

uint32_t platform_uart_write(const BoardUartId id, uint8_t* data, int len)
{
    esp32_uart_struct uart_numbers = find_uart(id);
    return uart_write_bytes(uart_numbers.uart_num, data, len);
}

uint32_t platform_uart_read(const BoardUartId id, uint8_t* buffer, int max_len, uint32_t ticks_to_wait)
{
    esp32_uart_struct uart_numbers = find_uart(id);
    return uart_read_bytes(uart_numbers.uart_num, buffer, max_len, ticks_to_wait);
}

/**
 * @note Funkcija stvara dispatcher task. Prije njegovog stvaranja nužno je očistiti RX buffer i
 * uart_events ISR queue kako ne bi počeo čitati eventove koji su se u međuvremenu mogli dogoditi
 * (fresh start). U slučaju poziva kod postojanja dispatcher taska, vraća error.
 */
UARTStatus platform_uart_event_converter_start(const BoardUartId id)
{
    if(dispatcher_task != NULL) {
        return UART_ERROR;
    }
    if(uart_event_queue == NULL || platform_event_queue == NULL) {
        return UART_ERROR;
    }

    esp32_uart_struct uart_numbers = find_uart(id);
    uart_flush(uart_numbers.uart_num);
    xQueueReset(uart_event_queue);
    dispatcher_stop = false;
    hal_dispatcher_ended_flag = false;

    if(xTaskCreate(dispatcher_function, "dispatcher", 4000, NULL, 4, &dispatcher_task) == pdPASS) {
        return UART_OK;
    } else {
        return UART_ERROR;
    }
}

/**
 * @note Postavlja kontrolni flag koji javlja dispatcher tasku da odmah prestane s radom.
 * Koristi se kod poziva izvana, errora ili shutdowna/restarta sustava.
 * 
 */
void platform_uart_event_converter_stop(void)
{
    if(dispatcher_task != NULL) {
        dispatcher_stop = true;
    }
}

UARTStatus platform_ISR_disable(const BoardUartId id) 
{
    esp32_uart_struct uart_numbers = find_uart(id);
    if(uart_disable_rx_intr(uart_numbers.uart_num) != ESP_OK) {
        return UART_ERROR;
    }
    rx_closed = true;
    return UART_OK;
}

UARTStatus platform_ISR_enable(const BoardUartId id) 
{
    esp32_uart_struct uart_numbers = find_uart(id);
    if(uart_enable_rx_intr(uart_numbers.uart_num) != ESP_OK) {
        return UART_ERROR;
    }
    rx_closed = false;
    return UART_OK;
}

UARTStatus platform_uart_deinit(const BoardUartId id)
{
    if(dispatcher_task != NULL) {
        return UART_ERROR;
    }

    esp32_uart_struct uart_numbers = find_uart(id);

    if(platform_event_queue != NULL) {
        vQueueDelete(platform_event_queue);
        platform_event_queue = NULL;
    }
    
    if(uart_driver_delete(uart_numbers.uart_num) != ESP_OK) {
        return UART_ERROR;
    } else {
        uart_event_queue = NULL;
        return UART_OK;
    }
}

void* platform_uart_get_event_queue(void)
{
    return platform_event_queue;
}