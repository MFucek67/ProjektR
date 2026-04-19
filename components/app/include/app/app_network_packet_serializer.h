/**
 * @file app_network_packet_serializer.h
 * @author Marko Fuček
 * @brief Modul koji služi za serijalizaciju report i response objekata.
 * 
 * Ovaj modul pruža strukture koje čine serijaliziran report i response objekt. Takav je pogodan za slanje preko
 * mreže i daljnju obradu.
 * 
 * Sve strukture sadrže __attribute__((packed)), što znači da nema kompajlerski dodanog paddinga između elemenata.
 * 
 * Struktura payloada serijaliziranog reporta sastoji se od 34 bajta ona je predstavljena na način:
 *  -prvih 6 bajtova -> samo onaj koji tip reporta dolazi će biti 0x01, ostali 0x00
 *  -bajt 7 -> informacija o uspješnosti inicijalizacije senzora
 *  -bajt 8 -> informacija o presence
 *  -bajt 9 -> informacija o motion
 *  -bajtovi 10-13 -> informacija o BMP indeksu
 *  -bajt 14 -> informacija o proximity
 *  -bajtovi 15-34 -> informacije iz UOF reporta (4 bajta existence_energy, 4 bajta static_distance, 4 bajta motion_energy, 4 bajta motion_distance i 4 bajta motion_speed)
 * 
 * Struktura payloada serijaliziranog responsea sastoji se od (3 + MAX_RESPONSE_DATA_LEN) bajta, te je predstavljena na način:
 *  -bajt 1 -> tip responsea
 *  -bajtovi 2-3 -> duljina podataka
 *  -bajtovi 4-(3 + MAX_RESPONSE_DATA_LEN) -> podatci
 * 
 * Dodatno, kako bi se olakšalo slanje paketa mrežom, serijalizirani report/response se spaja s (univerzalnim) headerom i tada
 * je u obliku pogodnom za slanje. Header se sastoji od 7 bajta:
 *  -bajt 1 -> verzija paketa
 *  -bajt 2 -> tip paketa
 *  -bajtovi 3-6 -> timestamp
 *  -bajt 7 -> veličina payloada
 * 
 * Payload se spaja s headerom i to nam daje paket koji je pogodan za slanje mrežom.
 * 
 * @note Paketi su najjednostavnije verzije i nemaju mehanizme za zaštitu od grešaka.
 * 
 * @version 0.1
 * @date 2026-04-11
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "app/app_mmwave_constants.h"
#include "app/app_types.h"

/**
 * @brief Verzija paketa.
 * 
 */
#define PACKET_VERSION 0x01

/**
 * @enum PacketType_t
 * @brief Vrsta paketa.
 * 
 */
typedef enum {
    PACKET_REPORT,
    PACKET_RESPONSE
} PacketType_t;

/**
 * @struct PacketHeader_t
 * @brief Header paketa.
 * 
 */
typedef struct __attribute__((packed)) {
    uint8_t version;
    uint8_t type;
    uint32_t timestamp;
    uint8_t payload_len;
} PacketHeader_t;

/**
 * @struct ReportPacketPayload_t
 * @brief Payload report paketa.
 * 
 */
typedef struct __attribute__((packed)) {
    uint8_t has_init_completed_info;
    uint8_t has_presence_info;
    uint8_t has_motion_info;
    uint8_t has_bmp_info;
    uint8_t has_proximity_info;
    uint8_t has_uof_report;

    uint8_t init_completed_info;
    uint8_t presence_info; //PresenceInfo enum
    uint8_t motion_info; //MotionInfo enum
    int32_t bmp_info;
    uint8_t proximity_info; //ProximityInfo enum
    //uof_report:
    int32_t existence_energy;
    float static_distance;
    int32_t motion_energy;
    float motion_distance;
    float motion_speed;
} ReportPacketPayload_t;

/**
 * @struct ResponsePacketPayload_t
 * @brief Payload response paketa.
 * 
 */
typedef struct __attribute__((packed)) {
    uint8_t response_type;
    uint16_t data_len;
    uint8_t data[MAX_RESPONSE_DATA_LEN];
} ResponsePacketPayload_t;

/**
 * @struct ReportPacket_t
 * @brief Struktura čitavog report paketa.
 * 
 */
typedef struct __attribute__((packed)) {
    PacketHeader_t header;
    ReportPacketPayload_t report_payload;
} ReportPacket_t;

/**
 * @struct ResponsePacket_t
 * @brief Struktura čitavog response paketa.
 * 
 */
typedef struct __attribute__((packed)) {
    PacketHeader_t header;
    ResponsePacketPayload_t response_payload;
} ResponsePacket_t;

/**
 * @brief Serijalizira report.
 * 
 * @param report Pokazivač na dekodirani report objekt
 * @param packet Pokazivač na packet u kojemu će biti serijalizirani objekt 
 * @param packet_len Duljina buffera za spremanje paketa
 * @return Veličina izrađenog paketa ili -1 kod neuspjeha
 */
int app_serialize_report(DecodedReport* report, uint8_t* packet, size_t packet_len);

/**
 * @brief Serijalizira response.
 * 
 * @param response Pokazivač na dekodirani response objekt
 * @param packet Pokazivač na packet u kojemu će biti serijalizirani objekt
 * @param packet_len Duljina buffera za spremanje paketa
 * @return Veličina izrađenog paketa ili -1 kod neuspjeha
 */
int app_serialize_response(DecodedResponse* response, uint8_t* packet, size_t packet_len);