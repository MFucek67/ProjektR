#pragma once
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "mmwave.h"

typedef struct {
    uint8_t* frame;
    size_t frame_len;
} mmWaveFrame; //struktura za cijeli frame (kod TX taska)

typedef struct
{
    //data -> data[0] = ctrl_w, data[1] = cmd_w, ostalo je payload
    //data_len = payload_len + 2
    uint8_t* data;
    size_t data_len;
} mmWaveFrameData; //struktura samo semantički korisnih podataka


//callbackovi koji se definiraju u HAL-u:
typedef bool (*mmWave_saveFrame)(const mmWaveFrameData* frame_data); //callback preko kojeg mmwave šalje frame u frame_queue preko HAL-a
typedef uint8_t* (*mmWave_alloc_memory)(size_t byte_size); //za heap alokaciju memorije za frame
typedef void (*mmWave_free_alloc_memory)(uint8_t* mem, size_t mem_size); //callback za free memorije preko HAL-a

//OD OVIH funkcija su callbackovi samo definirani tu, jer tu i pripadaju (to je public API core-a), ali preko strukture se šalju HAL-u na uporabu
//biti će pridruženi funkcijama u mmwave_core.c kad se te funkcije definiraju i implementiraju
//Dakle, ove tri zločke su samo obične API funkcije definirane ovako, samo kao pokazivači
/*Kad ih implementiram, napravit ću dodatnu funkciju koja će napraviti strukturu s pokazivačima na njih i te
pokazivače spojiti s konkretnim adresama implementiranih funkcija.

Ovako je bolje jer HAL koristi mmwave_core samo kao uslugu -> evo ti ovo, obavi ovo! Ne zna ništa kako je to implementirano.*/
typedef mmwave_frame_status_t (*mmWave_parse_data)(const uint8_t* data, size_t data_len); //za parsiranje
//za izgradnju frame-a:
typedef mmWaveFrame* (*mmWave_build_frame)(const uint8_t* payload, size_t payload_len, const uint8_t ctrl_w, const uint8_t cmd_w);
typedef mmwave_status_t (*mmWave_init)(void); //za restart parsera kod ponovnog startanja
typedef mmwave_status_t (*mmWave_stop)(void);

//za prosljeđivanje strukture u kojoj su pokazivači na funkcije povezani s pravim HAL funkcijama iz HAL sloja u mmwave_core sloj
void mmwave_core_bind_callbacks(const mmWave_core_callback* cb);

typedef struct
{
    mmWave_parse_data mmwave_parse_data;
    mmWave_build_frame mmwave_build_frame;
    mmWave_init mmwave_init;
    mmWave_stop mmwave_stop;
} mmWave_core_interface;

typedef struct
{
    mmWave_saveFrame mmwave_save_frame;
    mmWave_alloc_memory alloc_mem;
    mmWave_free_alloc_memory free_mem;
} mmWave_core_callback;
