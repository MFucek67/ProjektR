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
#define UART_PORT UART_NUM_2
#define TASK_STACK_SIZE 7000
#define UART_RX_BUFFER_SIZE 256
#define UART_TX_BUFFER_SIZE 256

//definicije za headere i footere:
#define HEADER1 0x53
#define HEADER2 0x59
#define FOOTER1 0x54
#define FOOTER2 0x43

//definicije za Control Word:
#define CONTROL_SYSTEM_FUNCTIONS 0x01
#define CONTROL_PRODUCT_INFORMATION 0x02
#define CONTROL_WORK_STATUS 0x05
#define CONTROL_HUMAN_PRESENCE_FUNCTION 0x80
#define CONTROL_UART_UPGRADE 0x03
#define CONTROL_OPEN_FUNCTION 0x08

//definicije za Command Word:
#define COMMAND_HEARTBEAT 0x01
#define COMMAND_MODULE_RESET 0x02
#define COMMAND_PRODUCT_MODEL 0xA1
#define COMMAND_PRODUCT_ID 0xA2
#define COMMAND_HARDWARE_MODEL 0xA3
#define COMMAND_FIRMWARE_VERSION 0xA4
#define COMMAND_SCENE_SETTINGS 0x07
#define COMMAND_SENSITIVITY_SETTINGS 0x08
#define COMMAND_INITIALIZATION_STATUS_INQUIRY 0x81
#define COMMAND_SCENE_SETTINGS_INQUIRY 0x87
#define COMMAND_SENSITIVITY_SETTINGS_INQUIRY 0x88
#define COMMAND_REPORTING_OF_PRESENCE 0x01
#define COMMAND_REPORTING_OF_MOTION 0x02
#define COMMAND_REPORTING_OF_BODY_MOVEMENT_PARAMETER 0x03
#define COMMAND_REPORTING_OF_PROXIMITY 0x0B
#define COMMAND_TIME_FOR_ENTERING_NO_PERSON_STATE 0x0A
#define COMMAND_PRESENCE_INFORMATION_INQUIRY 0x81
#define COMMAND_MOTION_INFORMATION_INQUIRY 0x82
#define COMMAND_BODY_MOVEMENT_PARAMETER_INQUIRY 0x83
#define COMMAND_PROXIMITY_INQUIRY 0x8B
#define COMMAND_TIME_FOR_ENTERING_NO_PERSON_STATE_INQUIRY 0x8A
#define COMMAND_START_UART_UPGRADE 0x01
#define COMMAND_UPGRADE_PACKAGE_TRANSMISSON 0x02
#define COMMAND_ENDING_THE_UART_UPGRADE 0x03
#define COMMAND_UNDERLYING_OPEN_FUNCTION_SWITCH 0x00
#define COMMAND_UNDERLYING_OPEN_FUNCTION_SWITCH_INQUIRY 0x80
#define COMMAND_REPORTING_OF_SENSOR_INFORMATION 0x01
#define COMMAND_EXISTENCE_ENERGY_VALUE_INQUIRY 0x81
#define COMMAND_MOTION_ENERGY_VALUE_INQUIRY 0x82
#define COMMAND_STATIC_DISTANCE_INQUIRY 0x83
#define COMMAND_MOTION_DISTANCE_INQUIRY 0x84
#define COMMAND_MOTION_SPEED_INQUIRY 0x85

//pomoćne funkcije:
uint8_t calculate_checksum(uint8_t* frame, int frame_length) {
    uint16_t summ = 0;
    for(int i = 0; i < frame_length - 3; i++) {
        summ += frame[i];
    }
    return ((uint8_t)(summ & 0xFF));
}

esp_err_t send_data_through_tx(uint8_t control, uint8_t command, uint8_t h_length, uint8_t l_length, uint8_t* data, int data_length) {
    uint8_t msg[9 + data_length];
    msg[0] = HEADER1;
    msg[1] = HEADER2;
    msg[2] = control;
    msg[3] = command;
    msg[4] = h_length;
    msg[5] = l_length;
    int i = 0;
    for(; i < data_length; i++) {
        msg[i + 6] = data[i];
    }
    msg[i + 6] = calculate_checksum(&msg, 7 + data_length);
    msg[i + 1 + 6] = FOOTER1;
    msg[i + 2 + 6] = FOOTER2;

    return uart_write_bytes(UART_PORT, msg, sizeof(uint8_t) * (9 + data_length));
}

typedef struct motion_data {
    uint8_t existance_energy;
    uint8_t static_distance;
    uint8_t motion_energy;
    uint8_t motion_distance;
    uint8_t motion_speed;
} open_motion_data_t;

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
    uint8_t frame[20]; //pomoćni buffer -> za izgradnju valjanog okvira -> maksimalna duljina neka bude 20 bajtova
    int parsing_buff_len = 0; //broj bajtova u pomoćnom bufferu za pohranu bajtova
    int built_frame_len = 0; //broj bajtova u pomoćnom buffera za izgradnju okvira
    int starting_point = 0;
    bool head1 = false; //head bajt 1 pronađen (0x53)
    bool head2 = false; //head bajt 2 pronađen (0x59)
    int fixed_elements = 0;
    int payload_len = 0;

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
                            //sad kada imamo HEAD - krećemo graditi okvir dalje

                            frame[built_frame_len] = b;
                            built_frame_len++;

                            //prvo uzimamo ControlWord, CommandWord i LengthIdentification (ukupno 4 bajta):
                            if(fixed_elements < 4) {
                                fixed_elements++;
                            } else if(fixed_elements == 4) {
                                //kada smo ih uzeli, čitamo duljinu payloada
                                payload_len = ((uint16_t)(frame[4] << 8) | (uint16_t)frame[5]);
                                fixed_elements++;
                            } else {
                                //tu čitamo ostale bajtove (Payload, Checksum i Tail):
                                if(built_frame_len == (2 + 4 + payload_len + 1 + 2)) {
                                    //ako smo ovdje, pročitali smo cijeli okvir - provjera taila i checksuma

                                    if(frame[2 + 4 + payload_len + 1] == 0x54 && frame[2 + 4 + payload_len + 2] == 0x43) {
                                        //TAIL dobar -> još ispitujemo checksum
                                        uint16_t sum = 0;
                                        for(int j = 0; j < (2 + 4 + payload_len - 1); j++) {
                                            sum += frame[j];
                                        }
                                        if(frame[2 + 4 + payload_len - 1] == ((uint8_t) (sum & 0xFF))) {
                                            //checksum je dobar
                                            if(xQueueSend(frames_queue, frame, pdMS_TO_TICKS(10)) != pdTRUE) {
                                                //timeout = 10ms -> čekaj toliko ako je queue pun i probaj opet, inače vrati pdFALSE
                                                printf("Queue pun, frame izgubljen!\n");
                                            }
                                            //nakon poslanog okvira resetiramo stanje
                                            head1 = false;
                                            head2 = false;
                                            built_frame_len = 0;
                                            fixed_elements = 0;

                                            if(parsing_buff_len > (2 + 4 + payload_len + 1 + 2)) {
                                                //Ako ima još bajtova iza kraja okvira
                                                memmove(parsing_buff, (parsing_buff + (2 + 4 + payload_len + 1 + 2)), 
                                                    (parsing_buff_len - (2 + 4 + payload_len + 1 + 2)));
                                                parsing_buff_len -= (2 + 4 + payload_len + 1 + 2);
                                                starting_point = 0;
                                                i = starting_point; //iteriraj od početka novog niza 
                                            } else if(parsing_buff_len == (2 + 4 + payload_len + 1 + 2)) {
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
                                            fixed_elements = 0;
                                            //ne odbacujemo svih N bajtova, već samo prva 2 (HEAD) i opet tražimo ispočetka
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
                                        fixed_elements = 0;
                                        //ne odbacujemo svih N bajtova, već samo prva 2 (HEAD) i opet tražimo ispočetka
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
                    fixed_elements = 0;
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
                    fixed_elements = 0;
                    break;
                default:
                    break;
            }
        }
    }
}

static void frame_reader(void *arg) {
    uint8_t frame[20];
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