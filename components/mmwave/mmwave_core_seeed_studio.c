/**
 * @file mmwave_core_seeed_studio.c
 * @author Marko Fuček
 * @brief Implementacija core sloja za parsiranje i izgradnju mmWave frameova.
 * 
 * Ovaj modul implementira mehanizme za parsiranje dolaznih bajtova i izgradnju mmWave frame-ova
 * koji se koriste za upite na senzor. Modul je potpuno platform-independent i njegov pristup hardware-u
 * je isključivo preko HAL sloja. Stoga se za funkcionalnosti spremanja u queue te alokacije i oslobađanja
 * memorije koristi HAL-ovim callbackovima.
 * 
 * @note Modul nije thread-safe.
 * @note Vlasništvo nad parsiranim frame-ovima se pozivom callbacka prenosi HAL sloju.
 * 
 * @version 0.1
 * @date 2026-01-22
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include <string.h>
#include "mmwave_interface/mmwave.h"
#include "mmwave_interface/mmwave_core_interface.h"

static mmWave_core_callback* hal_functions;

static int big_frames_count = 0; //brojit ćemo "velike" okvire -> često dolaze za redom

static uint8_t* building_buffer = NULL; //pomoćni buffer -> za izgradnju valjanog okvira
static int parsing_buff_current_size = 0;
static int built_frame_len = 0; //broj bajtova u pomoćnom bufferu koji su izgrađeni (u okviru koji trenutno gradimo)
static bool head1 = false; //head bajt 1 pronađen (0x53)
static bool head2 = false; //head bajt 2 pronađen (0x59)
static int payload_len = 0; //duljina payloada

static uint8_t* data_saving_buff = NULL; //interni buffer za spremanje podataka koje treba parsirati
static size_t data_saving_buff_size = 0; //pamtimo koliko max mjesta imamo u tom bufferu


void mmwave_core_bind_callbacks(const mmWave_core_callback* cb)
{
    hal_functions = cb;
}

/**
 * @brief Pomoćna funkcija za proširenje internog buffera za izgradnju frame-a.
 * 
 * Pošto veličina payloada ne mora biti ista, ne možemo predvidjeti veličinu buffera za parsiranje.
 * Funkcija se poziva kada treba alocirati veći buffer za izgradnju frame-a.
 * 
 * Alocira novi buffer zadane veličine, kopira već izgrađeni dio frame-a i
 * oslobađa memoriju starog buffera.
 * 
 * @param size Nova veličina buffera u bajtovima
 * @return Pokazivač na novi buffer ili NULL
 * 
 * @note Maksimalna veličina buffera ograničena je s MAX_PARSER_BUFFER_SIZE, no ona može biti
 * nedostižna zbog maksimalne veličine alokacije zadane u HAL sloju.
 */
static uint8_t* extend_building_space(size_t size)
{
    if(size <= MAX_PARSER_BUFFER_SIZE) {
        uint8_t* new_space = hal_functions->alloc_mem(size);
        if(!new_space) {
            return NULL;
        }
        memcpy(new_space, building_buffer, built_frame_len);
        hal_functions->free_mem(building_buffer, parsing_buff_current_size); //oslobađamo mem starog buffera
        parsing_buff_current_size = size;
        return new_space;
    } else {
        return NULL;
    }
}

/**
 * @brief Pomoćna funkcija koja resetira interno stanje parsera.
 * 
 * Briše sve varijable koje pamte trenutno stanje izgradnje okvira i postavlja ih na početne vrijedosti,
 * čisti sve interne buffere i oslobađa memoriju. Ovisno o učestalosti velikih okvira (većih od početno
 * zadane vrijednosti) veličinu buffera nekad ne resetira (kod "burstova" velikih okvira to štedi fragmentaciju
 * memorije i procesorsko vrijeme).
 * 
 */
static void restart_parser(void)
{
    printf("[CORE] restart_parser (big_frames=%d)\n", big_frames_count); //KASNIJE MAKNUTI
    head1 = false;
    head2 = false;
    built_frame_len = 0;
    payload_len = 0;
    
    //imamo prag od 3 velika, ako ih dođe 3 ili više, ostavljamo veličinu na najvećoj prošloj
    //čim dođe prvi manji od trenutne veličine resetira se na početnu veličinu
    if(big_frames_count < 3) {
        if(building_buffer) {
            hal_functions->free_mem(building_buffer, parsing_buff_current_size);
        }
        building_buffer = hal_functions->alloc_mem(STARTING_PARSER_BUFFER_SIZE);
        if(!building_buffer) {
            parsing_buff_current_size = 0;
            return;
        }
        parsing_buff_current_size = STARTING_PARSER_BUFFER_SIZE;
    }
}

mmwave_status_t mmwave_core_init(void)
{
    if(!hal_functions) {
        return S_MMWAVE_ERR_TIMEOUT;
    }
    restart_parser();
    if(!building_buffer) {
        //ako nije uspjela dodjela memorije za početni building buffer vrati error stanje i status
        return S_MMWAVE_MEMORY_PROBLEM;
    }
    return S_MMWAVE_OK;
}

mmwave_status_t mmwave_core_stop(void)
{
    if(!hal_functions) {
        return S_MMWAVE_ERR_TIMEOUT;
    }
    
    hal_functions->free_mem(building_buffer, parsing_buff_current_size);
    building_buffer = NULL;
    parsing_buff_current_size = 0;
    head1 = false;
    head2 = false;
    built_frame_len = 0;
    payload_len = 0;
    hal_functions->free_mem(data_saving_buff, data_saving_buff_size);
    data_saving_buff = NULL;
    data_saving_buff_size = 0;
    hal_functions = NULL;
    big_frames_count = 0;
    return S_MMWAVE_OK;
}

/**
 * @brief Interna funkcija za parsiranje ulaznih podataka.
 * 
 * Analizira buffer primljenih bajtova i pokušava pronaći sastavne dijelove mmWave frame-a.
 * Ako uspješno pronađe okvir, uzima samo semantički korisne podatke iz njega (ctrl_w, cmd_w,
 * duljinu payloada i payload) te ih prosljeđuje HAL sloju preko callbacka.
 * 
 * @param parsing_buff Buffer sa sirovim bajtovima
 * @param len Duljina poslanih bajtova
 * @return Status parsiranja ulaznih podataka
 */
static mmwave_frame_status_t process_data(uint8_t* parsing_buff, size_t len)
{
    mmwave_frame_status_t status_of_operation;
    int finished_frames = 0;

    status_of_operation = MMWAVE_NO_FRAMES;
    for(int i = 0; i < len; i++) {
        uint8_t b = parsing_buff[i];
        printf("[CORE] byte[%d]=0x%02X h1=%d h2=%d built=%d\n",
                i, b, head1, head2, built_frame_len); //KASNIJE MAKNUTI OVO

        if(!head1) {
            //nismo još pronašli 0x53
            if(parsing_buff[i] == HEADER1) {
                printf("[CORE] HEADER1 found at index %d\n", i); //KASNIJE MAKNUTI
                //HEAD1 nađen -> okvir započet
                if(finished_frames == 0) { //ako nismo našli niti jedan frame do kraja, ali imamo dio jednog
                    status_of_operation = MMWAVE_UNFINISHED_FRAME;
                }

                //dodajemo bajt u buffer za izgradnju
                *(building_buffer) = b;
                head1 = true;
                built_frame_len += 1;
            } else {
                //probaj naći head1 dalje -> samo pomići pokazivač dok ne nađeš HEADER1
                continue;
            }
        } else if(head1 && !head2) {
            //imamo 0x53, ali nemamo 0x59
            if(parsing_buff[i] == HEADER2) {
                printf("[CORE] HEADER2 found\n"); //KASNIJE MAKNUTI
                //HEAD2 nađen
                *(building_buffer + 1) = b;
                head2 = true;
                built_frame_len += 1;
            } else {
                //HEAD2 nije nađen
                //moramo izbaciti "smeće bajt" (zapravo HEAD1) i ponovno pokrenuti traženje HEAD1
                restart_parser();
                i -= 1; //dogodit će se i++ pa ćemo krenuti tražiti HEADER1 od trenutnog bajta -> to i želimo
                continue;
            }
        } else if(head1 && head2) {
            //sad kada imamo HEAD - krećemo graditi okvir dalje

            *(building_buffer + built_frame_len) = b;
            built_frame_len++;

            //prvo uzimamo ControlWord, CommandWord i LengthIdentification (ukupno 6 bajta u building_buffer-u):
            if(built_frame_len < 6) {
                continue;
            } else if(built_frame_len == 6) {
                //kada smo ih uzeli, čitamo duljinu payloada
                payload_len = ((uint16_t)(*(building_buffer + 4) << 8) | (uint16_t)*(building_buffer + 5));
                printf("[CORE] payload_len = %d\n", payload_len); //KASNIJE MAKNUTI
                //povećavamo buffer ako trebamo više od 20 bajtova za okvir (tj. >11 bajtova za payload)
                if((payload_len + 9) >= parsing_buff_current_size) {
                    big_frames_count++;
                } else {
                    big_frames_count = 0;
                }
                if(payload_len > (parsing_buff_current_size - 9)) {
                    printf("[CORE] need bigger buffer: %d bytes\n", payload_len + 9); //KASNIJE MAKNUTI
                    building_buffer = extend_building_space(payload_len + 9);
                    if(!building_buffer) {
                        //preveliki payload ili nemamo dovoljno memorije - dobili smo NULL
                        //odbacujemo okvir i krećemo tražiti drugi (opet HEADER1)
                        restart_parser();
                        status_of_operation = MMWAVE_MEMORY_PROBLEM;
                        continue;
                        //restart parsera (efektivno odbaciujemo ovaj frame) i pokušavamo parsirati drugi frame (ako ih još ima)
                        /*bilo bi glupo ovdje napraviti return jer onda odbacujemo i frameove koje bi potencijalno mogli parsirati,
                        a mogu se nalaziti iza ovoga koji odbacujemo*/
                    }
                }
            } else {
                //tu čitamo ostale bajtove (Payload, Checksum i Tail):
                if(built_frame_len == (2 + 4 + payload_len + 1 + 2)) {
                    printf("[CORE] full frame received, checking tail & checksum\n"); //KASNIJE MAKNUTI
                //ako smo ovdje, pročitali smo cijeli okvir - provjera taila i checksuma
                    if(*(building_buffer + 2 + 4 + payload_len + 1) == FOOTER1 
                        && *(building_buffer + 2 + 4 + payload_len + 2) == FOOTER2) {
                        //TAIL dobar -> još ispitujemo checksum
                        uint16_t sum = 0;
                        for(int j = 0; j < (2 + 4 + payload_len); j++) {
                            sum += *(building_buffer + j);
                        }
                        if(*(building_buffer + 2 + 4 + payload_len) == ((uint8_t) (sum & 0xFF))) {
                            //checksum je dobar
                            //alociramo memoriju za payload + ctrl_w + cmd_w
                            uint8_t* frame_data = hal_functions->alloc_mem(2 + payload_len);
                            if(!frame_data) {
                                //nismo instancirali heap objekt za frame data -> preskoči okvir i pokušaj napraviti drugi
                                status_of_operation = MMWAVE_MEMORY_PROBLEM;
                                restart_parser();
                                continue;
                            } else {
                                //spremamo podatke iz frame u queue -> free memorije od frame-a će vršiti application sloj
                                frame_data[0] = building_buffer[2];
                                frame_data[1] = building_buffer[3];
                                memcpy(&frame_data[2], &building_buffer[6], payload_len);
                                mmWaveFrameSemanticData frame_data_obj = {
                                    .data = frame_data,
                                    .len = payload_len + 2
                                };

                                if(hal_functions->mmwave_save_frame(&frame_data_obj)) {
                                    printf("[CORE] frame saved to HAL queue (len=%d)\n",
                                        frame_data_obj.len); //KASNIJE MAKNUTI
                                    status_of_operation = MMWAVE_FRAME_OK;
                                    finished_frames++;
                                } else {
                                    //queue je pun -> izgubili smo frame
                                    hal_functions->free_mem(frame_data, (2 + payload_len)); //čistimo frame data s heapa
                                    status_of_operation = MMWAVE_QUEUE_FULL;
                                }
                            }
                            //bilo uspješno slanje okvira ili neuspješno - MORA SE restartati stanje
                            //dakle ili će se frame preskočiti (neuspješno slanje) ili će se poslati i preskočiti
                            restart_parser();

                            //nastavljamo parsing novog okvira (ili dijela)
                        } else {
                            printf("[CORE] checksum FAIL calc=0x%02X frame=0x%02X\n",
                                (uint8_t)(sum & 0xFF),
                                *(building_buffer + 2 + 4 + payload_len)); //KASNIJE MAKNUTI
                            //checksum nije dobar -> opet traži okvir (ponovno HEAD)
                            restart_parser();

                            //ne odbacujemo svih N bajtova, već samo prva 2 (HEAD) i opet tražimo ispočetka
                            i -= (9 + payload_len - 2);
                        }
                    } else {
                        //TAIL nije dobar -> opet tražimo HEAD
                        restart_parser();

                        //ne odbacujemo svih N bajtova, već samo prva 2 (HEAD) i opet tražimo ispočetka
                        i -= (9 + payload_len - 2);
                    }
                }
            }
        }
    }
    if(finished_frames == 0 && head1 == false) {
        status_of_operation = MMWAVE_NO_FRAMES;
    }
    return status_of_operation;
}

/**
 * @note Ulazni podatci se kopiraju u interni buffer, kako bi se izbjegla direktna manipulacija
 * HAL bufferom.
 */
mmwave_frame_status_t mmwave_parse_data(const uint8_t* data, size_t data_len)
{
    printf("[CORE] parse_data: len=%zu\n", data_len); //ZA MAKNUTI KASNIJE
    if(!hal_functions) {
        return S_MMWAVE_ERR_TIMEOUT;
    }
    if(data_len > data_saving_buff_size) {
        uint8_t* new_buff = hal_functions->alloc_mem(data_len);
        if(new_buff == NULL) {
            return MMWAVE_MEMORY_PROBLEM;
        }
        hal_functions->free_mem(data_saving_buff, data_saving_buff_size);
        data_saving_buff = new_buff;
        data_saving_buff_size = data_len;
    }
    //i ako nije bilo dovoljno mjesta i ako je bilo, sad ima, pa kopiramo nove podatke u interni buffer
    memcpy(data_saving_buff, data, data_len);
    return process_data(data_saving_buff, data_len);
}

bool mmwave_build_frame(mmWaveFrameForTX* out,
    const uint8_t* payload, size_t payload_len, const uint8_t ctrl_w, const uint8_t cmd_w)
{
    if(!out || !hal_functions) {
        return false;
    }

    uint8_t* frame_data = hal_functions->alloc_mem(payload_len + 9);
    if(!frame_data) {
        return false;
    }
    frame_data[0] = HEADER1;
    frame_data[1] = HEADER2;
    frame_data[2] = ctrl_w;
    frame_data[3] = cmd_w;

    uint16_t payload16_len = (uint16_t) (payload_len & 0xFFFF);
    frame_data[4] = (payload16_len >> 8) & 0xFF; //viših 8 bitova duljine (viši bajt za duljinu)
    frame_data[5] = payload_len & 0xFF; //nižih 8 bitova duljine (niži bajt za duljinu)

    for(int i = 6; i < (payload_len + 6); i++) {
        frame_data[i] = *(payload + (i - 6));
    }
    //računanje zaštitne sume:
    uint16_t sum = HEADER1 + HEADER2 + ctrl_w + cmd_w + ((payload16_len >> 8) & 0xFF) + (payload_len & 0xFF);
    for(int j = 0; j < payload_len; j++) {
        sum += *(payload + j);
    }
    frame_data[6 + payload_len] = ((uint8_t) (sum & 0xFF));

    frame_data[7 + payload_len] = FOOTER1;
    frame_data[8 + payload_len] = FOOTER2;
    out->data = frame_data;
    out->len = payload_len + 9;
    return true;
}