#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/ringbuf.h"

#define TX_PIN GPIO_NUM_17
#define RX_PIN GPIO_NUM_16
#define RX_THRESH 10 //očekujemo duljinu okvira 10 bajta 

static const int RX_BUFF_SIZE = 128;
static QueueHandle_t event_queue;
static QueueHandle_t frames_queue;
TickType_t last_frame_time;

static void check_mmwave(void *args) 
{
    for(;;) {
        //ispitivanje Heartbeat senzora -> svakih 500 ms ako ne šalje nove okvire
        if(xTaskGetTickCount() - last_frame_time > (TickType_t) 500) {
            const uint8_t checksum = ((uint16_t) (0x53 + 0x59 + 0x01 + 0x01 + 0x00 + 0x01 + 0x0F) & 0xFF);
            const uint8_t msg[10] = {0x53, 0x59, 0x01, 0x01, 0x00, 0x01, 0x0F, checksum, 0x54, 0x43};
            uart_write_bytes(UART_NUM_2, msg, sizeof(uint8_t) * 10);
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

static void uart_parser(void *args)
{
    uart_event_t event;
    uint8_t rx_buf[128];
    uint8_t frame[RX_THRESH];
    int built_frame_len = 0;
    bool head = false;

    for(;;) {
        if(xQueueReceive(event_queue, &event, portMAX_DELAY)) {
            switch(event.type)
            {
                case UART_DATA:
                    //postavljamo broj tikova kod dolaska okvira - ne treba Heartbeat provjera
                    last_frame_time = xTaskGetTickCount();

                    //maksimalno 128 bajtova se čita iz RX buffera
                    //ako 20 tickova ne dođe event - pročitaj buffer - moguće da je dio okvira ostao unutra
                    const int len = uart_read_bytes(UART_NUM_2, rx_buf, sizeof(rx_buf), 20 / portTICK_PERIOD_MS);
                    for(int i = 0; i < len; i++) {
                        uint8_t b = rx_buf[i];
                        if(!head) {
                            //samo odbacujemo bajtove dok ne nađemo head
                            if(i + 1 < len && rx_buf[i] == 0x53 && rx_buf[i+1] == 0x59) {
                                //HEAD nađen
                                frame[0] = b;
                                frame[1] = rx_buf[i+1];
                                head = true;
                                i++; //preskoči drugi bajt heada, jer smo ga već zabilježili
                                built_frame_len = 2;
                            } else {
                                //probaj naći head dalje
                                continue;
                            }
                        } else {
                            //sad kada imamo HEAD - krećemo graditi okvir dalje -> prvo tražimo TAIL
                            frame[built_frame_len] = rx_buf[i];
                            built_frame_len++;
                            if(built_frame_len == RX_THRESH) {
                                //ako smo našli 10 bajtova i HEAD, moramo provjeriti TAIL
                                if(frame[8] == 0x54 && frame[9] == 0x43) {
                                    //TAIL dobar -> još ispitujemo checksum
                                    uint16_t sum = 0;
                                    for(int j = 0; j < 7; j++) {
                                        sum += frame[j];
                                    }
                                    if(frame[7] == ((uint8_t) (sum & 0xFF))) {
                                        //checksum je dobar
                                        if(xQueueSend(frames_queue, (void *)frame, 10 / portTICK_PERIOD_MS) != pdTRUE) {
                                            printf("Queue pun, frame izgubljen!\n");
                                        }
                                    } else {
                                        //checksum nije dobar -> opet traži okvir (ponovno HEAD)
                                        head = false;
                                        built_frame_len = 0;
                                        i -= 9; //ne odbacujemo svih 10 bajtova, već samo prva 2 (HEAD) i opet tražimo ispočetka
                                        continue;
                                    }
                                } else {
                                    //TAIL nije dobar -> opet tražimo HEAD
                                    head = false;
                                    built_frame_len = 0;
                                    i -= 9; //ne odbacujemo svih 10 bajtova, već samo prva 2 (HEAD) i opet tražimo ispočetka
                                    continue;
                                }
                            }
                        }
                    }
                    break;
                case UART_FIFO_OVF: //overflow RX buffera - ne čitamo dovoljno brzo
                    printf("UART FIFO overflow");
                    uart_flush(UART_NUM_2);
                    xQueueReset(event_queue);
                    built_frame_len = 0;
                    break;
                case UART_BUFFER_FULL: //overflow driver buffera
                    printf("Driver RX buffer full");
                    uart_flush(UART_NUM_2);
                    xQueueReset(event_queue);
                    built_frame_len = 0;
                    break;
                default:
                    break;
            }
                
        }
    }
}

static void frame_reader(void *arg) {
    uint8_t frame[RX_THRESH];

    for(;;) {
        if(xQueueReceive(frames_queue, &frame, portMAX_DELAY)) {
            //u varijablu (buffer) frame čitamo okvir za koji znamo da je dobar
            uint8_t control_word = frame[2];
            uint8_t command_word = frame[3];
            uint8_t data = frame[6];
            switch (control_word)
            {
            case 0x01:
                //Heartbeat report
                printf("mmWave Module alive!");
                break;
            case 0x80:
                //Očitanja sa senzora - report
                //Ovdje ide obrada očitanja
                /*
                .
                .
                .
                .
                .
                .*/
            default:
                break;
            }
        }
    }
}

void init_function(void) 
{
    const uart_config_t uart_config = { //postavljanje konfiguracije UART veze
        .baud_rate = 9600, //brzina 9600bps
        .data_bits = UART_DATA_8_BITS, //8 data bita
        .parity = UART_PARITY_DISABLE, //paritet isključen
        .stop_bits = UART_STOP_BITS_1, //1 stop bit
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, //isključen hardware flow control
        .source_clk = UART_SCLK_DEFAULT
    };
    uart_param_config(UART_NUM_2, &uart_config); //koristimo UART_2
    
    //sada postavljamo UART driver - kako će se ponašati ISR na primljene evente
    //maksimalno 20 eventova u event Queue
    uart_driver_install(UART_NUM_2, RX_BUFF_SIZE * 2, RX_BUFF_SIZE * 2, 20, &event_queue, 0);
    uart_set_pin(UART_NUM_2, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    //postavljamo granicu za ISR event UART_DATA na 10 bajta, tj. svaki mmWave okvir
    uart_set_rx_fifo_full_threshold(UART_NUM_2, RX_THRESH);

    //kreiramo queue za gotove okvire -> max 20 okvira, svaki veličine 10 bajtova
    frames_queue = xQueueCreate(20, 10 * sizeof(uint8_t));
    if(frames_queue == NULL) {
        printf("Ne mogu stvoriti frames_queue!\n");
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    init_function();

    xTaskCreate(uart_parser, "uart_parser_task", 5000, NULL, 5, NULL);
    xTaskCreate(frame_reader, "frame_reader_task", 5000, NULL, 5, NULL);
    xTaskCreate(check_mmwave, "checking_mmwave_task", 1000, NULL, 10, NULL);
}