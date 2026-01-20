#include <stdio.h>
#include "HAL/hal_mmwave.h"
#include "platform/platform_events.h"
#include "platform/platform_uart.h"
#include "platform/platform_task.h"
#include "platform/platform_mutex.h"

#define MAX_SINGLE_ALLOC 2048 //2KB
#define MAX_TOTAL_ALLOC 32768 //32KB

//HAL sloj će biti orkestrator taskova, inicijalizacije UARTA, eventova itd., mmwave_core samo parsira i radi frame-ove
//HAL ne zna ništa o protokolu i frame-ovima
//HAL je vlasnik sve memorije, glavnih struktura, a isto tako i parsera -> samo HAL može zvati mmwave_core funkcije

static MutexHandle_t mutex;
static size_t currently_allocated_mem = 0; //broji zauzetu mem ukupno
static bool flag1 = false; //dok završi rx task
static bool flag2 = false; //dok završi tx task
static HalEventHandle_t event_queue = NULL; //queue s platform events
static HalMmwaveState current_state = HAL_MMWAVE_UNINIT; //stanje u kojem je machine
static task_handler rx_task = NULL; //pokazivač na task koji čita eventove i dobiva parsirane frame-ove
static task_handler tx_task = NULL; //pokazivač na tasko koji piše u TX
static BoardUartId current_board_id = BOARD_UART_UNINIT; //board id
static mmWave_core_interface* mmwave_core_API = NULL; //calbackovi na mmwave API -> pozivanje mmwave_core funkcija preko tog
static mmWave_core_callback mmwave_core_callback; //callbackovi na HAL naredbe -> zvat će ih mmWave_core
static QueueHandle_t frame_queue = NULL;
static QueueHandle_t tx_queue = NULL;

//1. Funkcija za čitanje platform evenata iz platform_event_queue i onda čitanje podataka iz RX te slanje parseru:
static void hal_receive_task(void* arg)
{
    PlatformEvent_t buff;
    for(;;) {
        if(current_state != HAL_MMWAVE_RUNNING) {
            platform_delay_task(20);
            continue;
        }
        if(platform_event_wait(event_queue, &buff, 20)) {
            //pošalji na parsiranje
            mmwave_core_API->mmwave_parse_data(buff.data, buff.len);
            //kada se izparsira bit će u frame_queue - koristi application layer
        }
        if(hal_dispatcher_ended_flag && platform_get_num_of_queue_elements(event_queue) == 0) {
            flag1 = true;
            break;
        }
    }
}

//2. Funkcija za slanje na TX:
static void hal_send_task(void* arg)
{
    QueueElement_t buff;
    for(;;) {
        if(current_state != HAL_MMWAVE_RUNNING) {
            platform_delay_task(20);
            continue;
        }

        if(platform_queue_get(tx_queue, &buff, 20) == QUEUE_OK) {
            platform_uart_write(current_board_id, buff.data, &buff.len);
            platform_free(buff.data);
        } else {
            platform_delay_task(20);
            continue;
        }
        if(hal_dispatcher_ended_flag &&  platform_get_num_of_queue_elements(tx_queue) == 0) {
            flag2 = true;
            break;
        }
    }
}

//Ove funckije su vanjske - prenose se aplikacijskom sloju:
HalMmwaveStatus hal_mmwave_init(hal_mmwave_config* configuration, mmWave_core_interface* core_api)
{
    UARTStatus us;

    if(current_state != HAL_MMWAVE_UNINIT) {
        return HAL_MMWAVE_INVALID_STATE;
    }
    if(configuration == NULL || core_api == NULL) {
        return HAL_ERROR;
    }

    mutex = platform_create_mutex();

    current_board_id = configuration->id;
    mmwave_core_API = core_api;

    {
        //dajemo strukturi callbackova pokazivače na HAL funkcije
        mmwave_core_callback.mmwave_save_frame = _saveFrame;
        mmwave_core_callback.alloc_mem = hal_malloc;
        mmwave_core_callback.free_mem = hal_free;

        //dajemo mmwave_core sloju strukturu s konkretnim pokazivačima na HAL funkcije
        mmwave_core_bind_callbacks(&mmwave_core_callback);
    }

    //restartamo interne buffere core sloja (parsera):
    mmwave_core_API->mmwave_init();

    platform_uart_config_t uart_platform_conf = {
        .baudrate = configuration->baudrate,
        .data_bits = configuration->data_bits,
        .parity = configuration->parity,
        .stop_bits = configuration->stop_bits,
        .rx_buff_size = configuration->rx_buff_size,
        .tx_buff_size = configuration->tx_buff_size
    };

    //U ovom trenutku ISR kreće slati evente - potrebno očistiti event_queue i UART RX buffer kod hal_mmwave_start() -> interno u platform layeru
    //To je nužno jer se eventi stvaraju i ISR ih puni čim inicijaliziramo driver
    us = platform_uart_init(configuration->id, &uart_platform_conf);
    if(us != UART_OK) {
        return HAL_ERROR;
    }

    us = platform_uart_set_rx_threshold(configuration->id, configuration->rx_thresh);
    if(us != UART_OK) {
        return HAL_ERROR;
    }

    event_queue = platform_uart_get_event_queue();

    frame_queue = platform_queue_create(MAX_FRAMES_IN_QUEUE, sizeof(FrameData_t));
    tx_queue = platform_queue_create(MAX_FRAMES_IN_QUEUE, sizeof(FrameData_t));

    current_state = HAL_MMWAVE_INIT;
    return HAL_MMWAVE_OK;
}

HalMmwaveStatus hal_mmwave_start(void)
{
    UARTStatus us;
    if(current_state != HAL_MMWAVE_INIT && current_state != HAL_MMWAVE_STOPPED) {
        return HAL_MMWAVE_INVALID_STATE;
    }
    //Obnova flagova da ništa nije završilo od taskova:
    flag1 = false;
    flag2 = false;
    hal_dispatcher_ended_flag = false;
    //Dozvoljavanje RX uart prekida:
    us = platform_ISR_enable(current_board_id);
    if(us != UART_OK) {
        return HAL_ERROR;
    }
    //Pokretanje converter taska u platform layeru:
    us = platform_uart_event_converter_start(current_board_id);
    if(us != UART_OK) {
        return HAL_ERROR;
    }
    //Pokretanje taska za prepoznavanje eventova, slanje na TX i primanje reportova:
    TaskConfig_t rx1 = {hal_receive_task, "rx_task", 4000, NULL, 5};
    rx_task = platform_create_task(&rx1);
    //Pokretanje taska za slanje frame-ova u TX:
    TaskConfig_t tx1 = {hal_send_task, "tx_task", 4000, NULL, 5};
    tx_task = platform_create_task(&tx1);

    current_state = HAL_MMWAVE_RUNNING;
    return HAL_MMWAVE_OK;
}

HalMmwaveStatus hal_mmwave_stop(void)
{
    UARTStatus us;
    if(current_state != HAL_MMWAVE_RUNNING) {
        return HAL_MMWAVE_INVALID_STATE;
    }
    //Prvo gasimo ISR - mora prestati slanje uart_eventova:
    us = platform_ISR_disable(current_board_id);
    if(us != UART_OK) {
        return HAL_ERROR;
    }
    //Svi taskovi moraju prestati da ih možemo obrisati:
    while(!flag1 || !flag2 || !hal_dispatcher_ended_flag) {
        platform_delay_task(10);
    }
    platform_uart_event_converter_stop();
    platform_delete_task(rx_task);
    rx_task = NULL;
    platform_delete_task(tx_task);
    tx_task = NULL;
    mmwave_core_API->mmwave_stop();

    current_state = HAL_MMWAVE_STOPPED;
    return HAL_MMWAVE_OK;
}

HalMmwaveStatus hal_mmwave_deinit(void)
{
    UARTStatus us;
    if(current_state != HAL_MMWAVE_STOPPED && current_state != HAL_MMWAVE_INIT) {
        return HAL_MMWAVE_INVALID_STATE;
    }

    us = platform_uart_deinit(current_board_id);
    if(us != UART_OK) {
        return HAL_ERROR;
    }
    platform_queue_delete(event_queue);
    event_queue = NULL;
    platform_queue_delete(frame_queue);
    frame_queue = NULL;
    platform_queue_delete(tx_queue);
    tx_queue = NULL;

    platform_delete_mutex(mutex);
    mutex = NULL;

    current_board_id = BOARD_UART_UNINIT;
    mmwave_core_API = NULL;

    {
        mmwave_core_callback.mmwave_save_frame = NULL;
        mmwave_core_callback.alloc_mem = NULL;
        mmwave_core_callback.free_mem = NULL;

        mmwave_core_bind_callbacks(NULL);
    }

    current_state = HAL_MMWAVE_UNINIT;
    return HAL_MMWAVE_OK;
}

HalMmwaveStatus hal_mmwave_send_frame(const uint8_t* data, size_t data_len, const uint8_t ctrl_w, const uint8_t cmd_w)
{
    //Funkcionalnost stavljanja tx frame-a u TX queue -> može se zvati iz vana
    if(current_state == HAL_MMWAVE_UNINIT || current_state == HAL_MMWAVE_STOPPED) {
        return HAL_MMWAVE_INVALID_STATE;
    }
    mmWaveFrame* out_frame = mmwave_core_API->mmwave_build_frame(data, data_len, ctrl_w, cmd_w);
    if(!out_frame) {
        return HAL_ERROR;
    }
    if(out_frame->frame_len > 0) {
        QueueOperationStatus qos;
        QueueElement_t queue_frame = {out_frame->frame, out_frame->frame_len};
        qos = platform_queue_send(tx_queue, &queue_frame, 20);
        if(qos != QUEUE_OK) {
            platform_free(out_frame->frame);
            platform_free(out_frame);
            return HAL_ERROR;
        }
        return HAL_MMWAVE_OK;
    } else {
        return HAL_ERROR;
    }
}

//Funkcija koju mmwave_core preko callbacka zove za slanje SAMO semantički KORISNIH PODATAKA frame-ova u queue:
//TRUE = frame poslan, FALSE = neuspješno slanje
static bool _saveFrame(mmWaveFrameData* frame_data)
{
    QueueOperationStatus qos;

    if(frame_data != NULL && frame_queue != NULL) {
        FrameData_t new_frame_data = {
            .data = frame_data->data,
            .len = frame_data->data_len
        };
        qos = platform_queue_send(frame_queue, &new_frame_data, 10);
        if(qos != QUEUE_OK) {
            platform_free(frame_data->data);
            return false;
        }
        return true;
    }
}

//Funkcija koju mmwave_core preko callbacka zove za alociranje heap memorije:
static uint8_t* hal_malloc(size_t byte_size)
{
    if(byte_size > MAX_SINGLE_ALLOC) {
        return NULL;
    }
    if(platform_lock_mutex(mutex, 20) == MUTEX_OP_UNSUCCESSFUL) {
        return NULL;
    }
    if((currently_allocated_mem + byte_size) > MAX_TOTAL_ALLOC) {
        return NULL;
    }
    uint8_t* memory = NULL;
    if(platform_malloc(&memory, byte_size) == MEM_OK) {
        currently_allocated_mem += byte_size;
    }
    platform_unlock_mutex(mutex);
    return memory;
}

//funkcija za free -> application sloj će kod čitanja frame-a zvati preko callbacka
//šalje se na callback i mmwave_core sloju
static void hal_free(uint8_t* mem, size_t size_of_mem)
{
    if(mem == NULL) return;

    platform_free(mem);


    if(platform_lock_mutex(mutex, 20) == MUTEX_OP_SUCCESSFUL) {
        if(currently_allocated_mem - size_of_mem < 0) {
            currently_allocated_mem = 0;
        } else {
            currently_allocated_mem -= size_of_mem;
        }
        platform_unlock_mutex(mutex);
    }
    return;
}

//funkcija za dobivanje frame-a (eventa) iz frame queue (HAL vlasnik, pa app ne smije dobiti pokazivač na queue)
//app poziva, HAL vraća strukturu
HalMmwaveStatus hal_mmwave_get_frame_from_queue(QueueElement_t* buffer, uint32_t timeout_in_ms)
{
    QueueOperationStatus status;
    status = platform_queue_get(frame_queue, buffer, timeout_in_ms);
    if(status != QUEUE_OK) {
        return HAL_ERROR;
    }
    return HAL_MMWAVE_OK;
}

//funkcija za free frame_data iz app sloja
void hal_mmwave_release_frame_memory(mmWaveFrameData* frame_data)
{
    return hal_free(frame_data->data, frame_data->data_len);
}