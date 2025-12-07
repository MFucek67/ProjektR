#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/ringbuf.h"
#include "esp_mac.h"

#define TX_PIN GPIO_NUM_17
#define RX_PIN GPIO_NUM_16
#define RX_THRESH 10 //očekujemo duljinu okvira 10 bajta 

static const int RX_BUFF_SIZE = 128;
static QueueHandle_t event_queue;
static QueueHandle_t frames_queue;
TickType_t last_frame_time = 0;
TickType_t last_heartbeat = 0;

static void check_mmwave(void *args)
{
    printf("Check mmWave Task started!\n");

    for(;;) {
        //ispitivanje Heartbeat senzora -> svakih 500 ms ako ne šalje nove okvire
        if((xTaskGetTickCount() - last_frame_time > pdMS_TO_TICKS(500)) && (xTaskGetTickCount() - last_heartbeat > pdMS_TO_TICKS(5000))) {
            const uint8_t checksum = ((uint16_t) (0x53 + 0x59 + 0x01 + 0x01 + 0x00 + 0x01 + 0x0F) & 0xFF);
            const uint8_t msg[10] = {0x53, 0x59, 0x01, 0x01, 0x00, 0x01, 0x0F, checksum, 0x54, 0x43};
            uart_write_bytes(UART_NUM_2, msg, sizeof(uint8_t) * 10);
            last_heartbeat = xTaskGetTickCount();
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void uart_parser(void *args)
{
    uart_event_t event;
    uint8_t parsing_buff[128]; //pomoćni buffer -> za pohranu dobivenih bajtova
    uint8_t frame[RX_THRESH]; //pomoćni buffer -> za izgradnju valjanog okvira
    int parsing_buff_len = 0; //broj bajtova u pomoćnom bufferu za pohranu bajtova
    int built_frame_len = 0; //broj bajtova u pomoćnom buffera za izgradnju okvira
    int starting_point = 0;
    bool head1 = false; //head bajt 1 pronađen (0x53)
    bool head2 = false; //head bajt 2 pronađen (0x59)

    printf("Uart parser Task started!\n");

    for(;;) {
        if(xQueueReceive(event_queue, &event, portMAX_DELAY)) {
            switch(event.type)
            {
                case UART_DATA:
                    //postavljamo broj tikova kod dolaska okvira - ne treba Heartbeat provjera
                    last_frame_time = xTaskGetTickCount();
                    last_heartbeat = xTaskGetTickCount();

                    //maksimalno 128 bajtova se čita iz RX buffera
                    int space = sizeof(parsing_buff) - parsing_buff_len;
                    if(space > 0) {
                        int len = uart_read_bytes(UART_NUM_2, (parsing_buff + parsing_buff_len), space, pdMS_TO_TICKS(20));
                        parsing_buff_len += len;
                        //Provjera:
                        printf("RX (%d): ", len);
                        for (int k = 0; k < len; k++) {
                            printf("%02X ", parsing_buff[parsing_buff_len - len + k]);
                        }
                        printf("\n");
                    }

                    for(int i = starting_point; i < parsing_buff_len; i++) {
                        uint8_t b = parsing_buff[i];
                        if(!head1) {
                            //nismo još pronašli 0x53
                            if(parsing_buff[i] == 0x53) {
                                //HEAD1 nađen
                                //prvo brišemo "smeće bajtove"
                                memmove(parsing_buff, (parsing_buff + starting_point), (parsing_buff_len - starting_point));
                                parsing_buff_len -= starting_point;
                                starting_point = 0;
                                i = starting_point;

                                //zatim dodajemo bajt u buffer za izgradnju
                                frame[0] = b;
                                head1 = true;
                                built_frame_len += 1;
                            } else {
                                //probaj naći head1 dalje
                                starting_point++;
                                continue;
                            }
                        } else if(head1 && !head2) {
                            //imamo 0x53, ali nemamo 0x59
                            if(parsing_buff[i] == 0x59) {
                                //HEAD2 nađen
                                frame[1] = b;
                                head2 = true;
                                built_frame_len += 1;
                            } else {
                                //HEAD2 nije nađen
                                //moramo izbaciti "smeće bajt" i ponovno pokrenuti traženje HEAD1
                                memmove(parsing_buff, (parsing_buff + 1), (parsing_buff_len - 1));
                                i--;
                                parsing_buff_len--;
                                head1 = false;
                                built_frame_len = 0;
                            }
                        } else if(head1 && head2) {
                            //sad kada imamo HEAD - krećemo graditi okvir dalje -> prvo tražimo TAIL
                            if(built_frame_len < RX_THRESH) { //gradimo dok ne nađemo 10 bajtova
                                frame[built_frame_len] = b;
                                built_frame_len++;
                            }
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
                                        if(xQueueSend(frames_queue, frame, pdMS_TO_TICKS(10)) != pdTRUE) {
                                            //timeout = 10ms -> čekaj toliko ako je queue pun i probaj opet, inače vrati pdFALSE
                                            printf("Queue pun, frame izgubljen!\n");
                                        }
                                        //nakon poslanog okvira resetiramo stanje
                                        head1 = false;
                                        head2 = false;
                                        built_frame_len = 0;
                                        if(parsing_buff_len > 10) {
                                            //Ako ima još bajtova iza kraja okvira
                                            memmove(parsing_buff, (parsing_buff + 10), (parsing_buff_len - 10));
                                            parsing_buff_len -= 10;
                                            starting_point = 0;
                                            i = starting_point; //iteriraj od početka novog niza 
                                        } else if(parsing_buff_len == 10) {
                                            //Ima još točno 1 valjan okvir i onda nema bajtova
                                            parsing_buff_len = 0;
                                            starting_point = 0;
                                        } else {
                                            //Ako nema više bajtova iza okvira, samo završi
                                        }
                                    } else {
                                        //checksum nije dobar -> opet traži okvir (ponovno HEAD)
                                        head1 = false;
                                        head2 = false;
                                        built_frame_len = 0;
                                        //ne odbacujemo svih 10 bajtova, već samo prva 2 (HEAD) i opet tražimo ispočetka
                                        if(parsing_buff_len > 2) {
                                            memmove(parsing_buff, (parsing_buff + 2), (parsing_buff_len - 2));
                                        }
                                        parsing_buff_len -= 2;
                                        starting_point = 0;
                                        i = starting_point;
                                    }
                                } else {
                                    //TAIL nije dobar -> opet tražimo HEAD
                                    head1 = false;
                                    head2 = false;
                                    built_frame_len = 0;
                                    //ne odbacujemo svih 10 bajtova, već samo prva 2 (HEAD) i opet tražimo ispočetka
                                    if(parsing_buff_len > 2) {
                                        memmove(parsing_buff, (parsing_buff + 2), (parsing_buff_len - 2));
                                    }
                                    parsing_buff_len -= 2;
                                    starting_point = 0;
                                    i = starting_point;
                                }
                            }
                        }
                    }
                    break;
                case UART_FIFO_OVF: //overflow RX buffera - ne čitamo dovoljno brzo
                    printf("UART FIFO overflow!\n");
                    uart_flush(UART_NUM_2);
                    xQueueReset(event_queue);
                    built_frame_len = 0;
                    head1 = false;
                    head2 = false;
                    parsing_buff_len = 0;
                    starting_point = 0;
                    break;
                case UART_BUFFER_FULL: //overflow driver buffera
                    printf("Driver RX buffer full!\n");
                    uart_flush(UART_NUM_2);
                    xQueueReset(event_queue);
                    built_frame_len = 0;
                    head1 = false;
                    head2 = false;
                    parsing_buff_len = 0;
                    starting_point = 0;
                    break;
                default:
                    break;
            } 
        }
    }
}

static void frame_reader(void *arg) {
    uint8_t frame[RX_THRESH];
    bool presence_state = false;
    uint8_t motion_state = 0x00;

    printf("Frame reader task started!\n");

    for(;;) {
        if(xQueueReceive(frames_queue, frame, portMAX_DELAY)) {
            //u varijablu (buffer) frame čitamo okvir za koji znamo da je dobar
            uint8_t control_word = frame[2];
            uint8_t command_word = frame[3];
            uint8_t data = frame[6];
            switch (control_word)
            {
            case 0x01:
                //Heartbeat report
                printf("mmWave Module alive!\n");
                break;
            case 0x80:
                //Očitanja sa senzora - report ljudske prisutnosti i aktivnosti
                //Ovdje ide obrada očitanja
                switch (command_word)
                {
                case 0x01:
                    //prisutnost
                    if(data == 0x00) {
                        if(presence_state == true) {
                            printf("Person left the room!\n");
                            presence_state = false;
                        }
                    } else {
                        if(presence_state == false) {
                            printf("Person entered the room!\n");
                            presence_state = true;
                        }
                    }
                    break;
                case 0x02:
                    //kretanje
                    if(data == 0x00) {
                        if(motion_state == 0x01) {
                            printf("Person which was motionless in the room left!\n");                            
                        } else if(motion_state == 0x02) {
                            printf("Person which was moving in the room left!\n");
                        }
                        motion_state = 0x00;
                    } else if(data == 0x01) {
                        if(motion_state == 0x00) {
                            printf("Somebody entered the room and now is motionless!\n");
                        } else if(motion_state == 0x02) {
                            printf("Somebody who was moving is now motionless!\n");
                        }
                        motion_state = 0x01;
                    } else {
                        if(motion_state == 0x00) {
                            printf("Somebody who was not here entered the room and is moving!\n");
                        } else if(motion_state == 0x01) {
                            printf("Somebody who was motionless is now moving!\n");
                        }
                        motion_state = 0x02;
                    }
                    break;
                case 0x03:
                    //BMP - za sada ništa
                    break;
                case 0x0B:
                    //očitanje blizine
                    if(data == 0x00) {
                        //ništa
                    } else if(data == 0x01) {
                        printf("Person approaching sensor!\n");
                    } else {
                        printf("Person moving away from sensor!\n");
                    }
                    break;
                default:
                    break;
                }
            default:
                break;
            }
        }
    }
}

bool init_function(void) 
{
    const uart_config_t uart_config = { //postavljanje konfiguracije UART veze
        .baud_rate = 115200, //brzina 115200bps
        .data_bits = UART_DATA_8_BITS, //8 data bita
        .parity = UART_PARITY_DISABLE, //paritet isključen
        .stop_bits = UART_STOP_BITS_1, //1 stop bit
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, //isključen hardware flow control
        .source_clk = UART_SCLK_DEFAULT
    };
    if(uart_param_config(UART_NUM_2, &uart_config) != ESP_OK) { //koristimo UART_2
        printf("Neuspješna konfiguracija UART parametara!\n");
        return false;
    }
    
    //sada postavljamo UART driver - kako će se ponašati ISR na primljene evente
    //maksimalno 20 eventova u event Queue
    if(uart_driver_install(UART_NUM_2, RX_BUFF_SIZE * 2, RX_BUFF_SIZE * 2, 20, &event_queue, 0) != ESP_OK) {
        printf("Neuspješna instalacija UART drivera!\n");
        return false;
    }
    assert(event_queue != NULL); //assert ruši program ako je uvjet neistinit
    if(uart_set_pin(UART_NUM_2, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
        printf("Neuspješno postavljanje UART pinova!\n");
        return false;
    }

    //postavljamo granicu za ISR event UART_DATA na 10 bajta, tj. svaki mmWave okvir
    if(uart_set_rx_full_threshold(UART_NUM_2, RX_THRESH) != ESP_OK) {
        printf("Neuspješno postavljanje ISR event UART_DATA granice!\n");
        return false;
    }

    //kreiramo queue za gotove okvire -> max 20 okvira, svaki veličine 10 bajtova
    frames_queue = xQueueCreate(20, 10 * sizeof(uint8_t));
    if(frames_queue == NULL) {
        printf("Neuspješno stvaranje frames_queue!\n");
        return false;
    }
    printf("Driver initialized!\n");
    return true;
}

void app_main(void)
{
    bool safe_start = init_function();
    last_frame_time = xTaskGetTickCount(); //inicijalno postavi broj tickova kod stvaranja taska
    last_heartbeat = xTaskGetTickCount(); //inicijalno postavi broj tickova kod stvaranja taska

    if(safe_start) { //uspješna inicijalizacije drivera -> stvori taskove
        //Postavljanje određenih postavki:
        uint8_t checksum = ((uint16_t) (0x53 + 0x59 + 0x08 + 0x00 + 0x00 + 0x01 + 0x00) & 0xFF);
        const uint8_t msg0[10] = {0x53, 0x59, 0x08, 0x00, 0x00, 0x01, 0x00, checksum, 0x54, 0x43};
        uart_write_bytes(UART_NUM_2, msg0, sizeof(uint8_t) * 10);
        printf("Poslani bajtovi za postavljanje Standard Mode!\n");

        checksum = ((uint16_t) (0x53 + 0x59 + 0x80 + 0x0A + 0x00 + 0x01 + 0x00) & 0xFF);
        const uint8_t msg1[10] = {0x53, 0x59, 0x80, 0x0A, 0x00, 0x01, 0x00, checksum, 0x54, 0x43};
        uart_write_bytes(UART_NUM_2, msg1, sizeof(uint8_t) * 10);
        printf("Poslani bajtovi za postavljanje Time for entering no person state setting!\n");

        xTaskCreate(uart_parser, "uart_parser_task", 7000, NULL, 5, NULL);
        xTaskCreate(frame_reader, "frame_reader_task", 7000, NULL, 5, NULL);
        xTaskCreate(check_mmwave, "checking_mmwave_task", 4000, NULL, 10, NULL);
    } else {
        printf("HALT!");
    }
}