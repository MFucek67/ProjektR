/**
 * @file hal_mmwave_uart.c
 * @author Marko Fuček
 * @brief Implementacija HAL sloja za mmWave modul.
 * 
 * Ovaj modul implementira Hardware Abstraction Layer (HAL) za mmWave driver.
 * HAL djeluje kao orkestrator između:
 * -platform sloja (UART, taskovi, eventi, memorija)
 * -mmWave core sloja (parser, frame protocol logika)
 * -application sloja (tumačenje podataka iz frame-a)
 * 
 * Odgovornosti HAL sloja:
 * Upravljanje lifecycle-om mmWave sustava.
 * Stvaranje i gašenje taskova.
 * Upravljanje UART-om i ISR eventima.
 * Upravljanje memorijom korištenom za frame-ove.
 * Sinkronizacija i zaštita heap memorije.
 * Jedini je sloj koji smije pozivati mmWave core API.
 * Ne poznaje protokol, strukturu frame-a, niti vrstu senzora.
 * 
 * @note Modul je implementiran kao state machine. 
 * 
 * @version 0.1
 * @date 2026-01-22
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include "my_hal/hal_mmwave.h"
#include "platform/platform_events.h"
#include "platform/platform_uart.h"
#include "platform/platform_task.h"
#include "platform/platform_mutex.h"
#include "platform/platform_queue.h"

/**
 * @brief Maksimalna količina memorije koja se smije alocirati odjednom u mmWave core sloju.
 * 
 * @note 2KB
 */
#define MAX_SINGLE_ALLOC 2048

/**
 * @brief Ukupna količina memorije koja se maksimalno smije alocirati iz mmWave core sloja.
 * 
 * @note 32KB
 */
#define MAX_TOTAL_ALLOC 32768

static MutexHandle_t mutex; /**< Mutex za zaštitu heap memorije */
static size_t currently_allocated_mem = 0; /**< Brojač ukupno zauzete memorije na heapu */
static volatile bool flag1 = false; /**< Signal završetka RX taska */
static volatile bool flag2 = false; /**< Signal završetka TX taska */
static HalEventHandle_t event_queue = NULL; /**< Queue s platform UART eventima */
static HalMmwaveState current_state = HAL_MMWAVE_UNINIT; /**< Trenutno stanje HAL state machine-a */
static task_handler rx_task = NULL; /**< Pokazivač na task koji čita eventove i dobiva parsirane frame-ove */
static task_handler tx_task = NULL; /**< Pokazivač na task koji piše u TX */
static BoardUartId current_board_id = BOARD_UART_UNINIT; /**< Trenutni UART id */
static mmWave_core_interface* mmwave_core_API = NULL; /**< mmWave core API -> pozivanje mmwave_core funkcija preko tih callbackova */
static mmWave_core_callback mmwave_core_callback; /**< Callbackovi na HAL naredbe -> zvat će ih mmWave_core */
static PlatformQueueHandle frame_queue = NULL; /**< Queue za primljene frame-ove */
static PlatformQueueHandle tx_queue = NULL; /**< Queue koji se koristi za TX frame-ove */

/**
 * @brief Implementacija callback funkcije za spremanje semantički korisnih podataka iz parsiranog frame-a.
 * 
 * Funkciju preko callbacka poziva mmWave core sloj kada prepozna semantički ispravan frame.
 * 
 * HAL sloj preuzima vlasništvo nad podatcima iz frame-a i sprema ih u interni frame_queue.
 * 
 * @param frame_data Pokazivač na strukturu s podatcima iz parsiranog frame-a
 * @return true ako su podatci uspješno poslani u queue
 * @return false ako su podatci neuspješno poslani u queue
 */
static bool _saveFrame(mmWaveFrameSemanticData* frame_data)
{
    QueueOperationStatus qos;
    printf("[HAL] saveFrame len=%zu\n", frame_data->len); //KASNIJE MAKNUTI

    if(frame_data != NULL && frame_queue != NULL) {
        FrameData_t new_frame_data = {
            .data = frame_data->data,
            .len = frame_data->len
        };
        qos = platform_queue_send(frame_queue, &new_frame_data, 10);
        if(qos != QUEUE_OK) {
            platform_free(frame_data->data);
            printf("[HAL] saveFrame FAILED (queue full)\n"); //KASNIJE MAKNUTI
            return false;
        }
        printf("[HAL] saveFrame queued\n"); //KASNIJE MAKNUTI
        return true;
    }
    return false;
}

/**
 * @brief Implementacija callback funkcije za alokaciju heap memorije.
 * 
 * HAL ograničava maksimalnu veličinu pojedinačno alocirane memorije, te ukupno zauzetu heap memoriju.
 * 
 * Funkcija je thread-safe.
 * 
 * @param byte_size Broj bajtova za alokaciju
 * @return Pokazivač na zauzetu memoriju ili NULL
 */
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
    void* memory = NULL;
    if(platform_malloc(&memory, byte_size) == MEM_OK) {
        currently_allocated_mem += byte_size;
    }
    platform_unlock_mutex(mutex);
    return (uint8_t*)memory;
}

/**
 * @brief Implementacija callback funkcije za oslobađanje heap memorije.
 * 
 * Funkciju preko callbacka pozivaju drugi slojevi koji koriste heap objekte
 * koje je HAL zauzeo (ili oni preko HAL callbacka).
 * 
 * HAL vodi evidenciju o ukupno zauzetoj i oslobođenoj memoriji.
 * 
 * @param mem Pokazivač na memoriju koja se oslobađa
 * @param size_of_mem Veličina oslobođene memorije u bajtovima
 */
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

/**
 * @brief Task za obradu UART RX event-ova.
 * 
 * Task dohvaća event-ove iz platform UART event queue (koji su prošli kroz konverziju na platform sloju),
 * a zatim poziva mmWave core API za parsiranje frame-a.
 * 
 * Parsirani frame-ovi se preko HAL callbacka iz mmWave core sloja spremaju u frame queue.
 * 
 * Task će se sam ugasiti i osloboditi zauzete resurse kada dispatcher task (definiran u platform sloju)
 * završi i isprazne se svi do tada dodani eventi iz event queue.
 * 
 * @param arg Ne koristi se
 */
static void hal_receive_task(void* arg)
{
    PlatformEvent_t buff;
    for(;;) {
        if(current_state != HAL_MMWAVE_RUNNING) {
            platform_delay_task(20);
            continue;
        }
        if(platform_event_wait(event_queue, &buff, 200) == PLATFORM_EVENT_OK) {
            printf("[HAL RX] uart event type=%d\n", buff.type); //KASNIJE MAKNUTI
            //sad imamo event i ovisno o eventu radimo operaciju:
            static uint8_t rx_tmp_buff[512];
            if(buff.type == PLATFORM_EVENT_RX_DATA && buff.len > 0) {
                int read_len = platform_uart_read(current_board_id, rx_tmp_buff, buff.len, 20);
                //pošalji na parsiranje
                printf("[HAL RX] -> core parse\n"); //KASNIJE MAKNUTI
                if(read_len > 0) {
                    mmwave_core_API->mmwave_parse_data(rx_tmp_buff, read_len);
                    //kada se izparsira bit će u frame_queue - koristi application layer
                }
            } else {
                continue;
            }
        } else {
            platform_delay_task(20);
        }
        if(hal_dispatcher_ended_flag && (platform_get_num_of_queue_elements(event_queue) == 0)) {
            printf("[HAL RX] zavrsio s radom (flag1 = true)\n");
            flag1 = true;
            rx_task = NULL;
            platform_delete_task(NULL); //task postavlja flag1 (da je završio) i briše sam sebe
        }
    }
}

/**
 * @brief Task za slanje frame-ova preko TX UART pina.
 * 
 * Task čeka frame-ove u tx_queue, dohvaća ih i šalje preko UART TX pina, te zatim oslobađa memoriju
 * koja je bila alocirana za frame.
 * 
 * Task će se sam ugasiti i osloboditi zauzete resurse kada dispatcher task (definiran u platform sloju)
 * završi i isprazne se svi do tada dodani frame-ovi iz tx_queue.
 * 
 * @param arg Ne koristi se
 */
static void hal_send_task(void* arg)
{
    QueueElement_t buff;
    for(;;) {
        if(current_state != HAL_MMWAVE_RUNNING) {
            platform_delay_task(20);
            continue;
        }

        if(platform_queue_get(tx_queue, &buff, 20) == QUEUE_OK) {
            printf("[HAL TX TASK] buff.data=%p len=%d\n", buff.data, buff.len); //KASNIJE MAKNUTI
            platform_uart_write(current_board_id, buff.data, buff.len);
            platform_free(buff.data);
        } else {
            platform_delay_task(20);
        }
        if(hal_dispatcher_ended_flag && (platform_get_num_of_queue_elements(tx_queue) == 0)) {
            printf("[HAL TX TASK] zavrsio s radom (flag2 = true)\n");
            flag2 = true;
            tx_task = NULL;
            platform_delete_task(NULL);
        }
    }
}

HalMmwaveStatus hal_mmwave_init(hal_mmwave_config* configuration, mmWave_core_interface* core_api)
{
    printf("[HAL] init called, state=%d\n", current_state); //KASNIJE MAKNUTI
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
        printf("[HAL] core callbacks bound\n"); //KASNIJE MAKNUTI
    }

    //restartamo interne buffere core sloja (parsera):
    mmwave_core_API->mmwave_core_init();
    printf("[HAL] core init done\n"); //KASNIJE MAKNUTI

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
    printf("[HAL] init OK\n"); //KASNIJE MAKNUTI
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
    TaskConfig_t rx1 = {hal_receive_task, "rx_task", 10000, NULL, 5};
    rx_task = platform_create_task(&rx1);
    //Pokretanje taska za slanje frame-ova u TX:
    TaskConfig_t tx1 = {hal_send_task, "tx_task", 10000, NULL, 5};
    tx_task = platform_create_task(&tx1);

    current_state = HAL_MMWAVE_RUNNING;
    return HAL_MMWAVE_OK;
}

HalMmwaveStatus hal_mmwave_stop(void)
{
    UARTStatus us;
    if(current_state != HAL_MMWAVE_RUNNING) {
        return HAL_MMWAVE_INVALID_STATE;
        printf("[HAL STOP] neuspjesan stop - krivo stanje.\n");
    }
    //Prvo gasimo ISR - mora prestati slanje uart_eventova:
    us = platform_ISR_disable(current_board_id);
    if(us != UART_OK) {
        return HAL_ERROR;
    }
    //Svi taskovi moraju prestati da ih možemo obrisati:
    while(!flag1 || !flag2 || !hal_dispatcher_ended_flag) {
        platform_delay_task(1);
    }
    //Na kraju STOP core-a
    mmwave_core_API->mmwave_core_stop();

    current_state = HAL_MMWAVE_STOPPED;
    return HAL_MMWAVE_OK;
}

HalMmwaveStatus hal_mmwave_deinit(void)
{
    UARTStatus us;
    if(current_state != HAL_MMWAVE_STOPPED && current_state != HAL_MMWAVE_INIT) {
        return HAL_MMWAVE_INVALID_STATE;
    }

    while(rx_task != NULL || tx_task != NULL) {
        platform_delay_task(50);
    }

    us = platform_uart_deinit(current_board_id);
    if(us != UART_OK) {
        printf("[HAL DEINIT] neuspjesno deinicijaliziran UART\n");
        return HAL_ERROR;
    }
    printf("[HAL DEINIT] uspjesno deinicijaliziran UART\n");

    FrameData_t tmp;
    while (platform_queue_get(frame_queue, &tmp, 0) == QUEUE_OK) {
        hal_free(tmp.data, tmp.len);
    }
    while (platform_queue_get(tx_queue, &tmp, 0) == QUEUE_OK) {
        hal_free(tmp.data, tmp.len);
    }

    if(frame_queue) {
        platform_queue_delete(frame_queue);
        printf("[HAL DEINIT] obrisan frame queue\n");
        frame_queue = NULL;
    }
    if(tx_queue) {
        platform_queue_delete(tx_queue);
        printf("[HAL DEINIT] obrisan TX queue\n");
        tx_queue = NULL;
    }
    if(mutex) {
        platform_delete_mutex(mutex);
        mutex = NULL;
    }
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
    printf("[HAL TX] send_frame ctrl=0x%02X cmd=0x%02X len=%zu\n",
       ctrl_w, cmd_w, data_len); //KASNIJE KAMNUTI
    //Funkcionalnost stavljanja tx frame-a u TX queue -> može se zvati iz vana
    if(current_state == HAL_MMWAVE_UNINIT || current_state == HAL_MMWAVE_STOPPED) {
        return HAL_MMWAVE_INVALID_STATE;
    }
    mmWaveFrameForTX out_frame;
    bool success = mmwave_core_API->mmwave_build_frame(&out_frame, data, data_len, ctrl_w, cmd_w);
    printf("[HAL TX] frame built len=%zu\n", out_frame.len); //KASNIJE MAKNUTI
    if(!success) {
        return HAL_ERROR;
    }
    if(out_frame.len > 0) {
        QueueOperationStatus qos;
        FrameData_t queue_frame = {out_frame.data, out_frame.len};
        qos = platform_queue_send(tx_queue, &queue_frame, 20);
        if(qos != QUEUE_OK) {
            platform_free(out_frame.data);
            printf("[HAL TX] FAILED to queue TX frame\n"); //KASNIJE MAKNUTI
            return HAL_ERROR;
        }
        printf("[HAL TX] frame queued\n"); //KASNIJE MAKNUTI
        return HAL_MMWAVE_OK;
    } else {
        return HAL_ERROR;
    }
}

HalMmwaveStatus hal_mmwave_get_frame_from_queue(FrameData_t* buffer, uint32_t timeout_in_ms)
{
    if(current_state == HAL_MMWAVE_UNINIT || current_state == HAL_MMWAVE_STOPPED) {
        return HAL_MMWAVE_INVALID_STATE;
    }
    QueueOperationStatus status;
    status = platform_queue_get(frame_queue, buffer, timeout_in_ms);
    if(status != QUEUE_OK) {
        return HAL_ERROR;
    }
    printf("[HAL] app got frame len=%zu\n", buffer->len); //KASNIJE MAKNUTI
    return HAL_MMWAVE_OK;
}

void hal_mmwave_release_frame_memory(FrameData_t* frame_data)
{
    return hal_free(frame_data->data, frame_data->len);
}

void hal_mmwave_flush_frames(void)
{
    FrameData_t tmp;
    while (hal_mmwave_get_frame_from_queue(&tmp, 0) == HAL_MMWAVE_OK) {
        hal_mmwave_release_frame_memory(&tmp);
    }
    return;
}