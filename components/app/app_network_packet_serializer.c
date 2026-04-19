/**
 * @file app_network_packet_serializer.c
 * @author Marko Fuček
 * @brief Implementacija packet serializer API-ja.
 * 
 * Implementirane funkcija za serijalizaciju koje dobivaju dekodirani report/response objekt i vrše provjeru paketa,
 * usporedbu veličine buffera s veličinom paketa, te grade paket - prvo zaglavlje, zatim payload i spremaju ga u buffer.
 * 
 * @version 0.1
 * @date 2026-04-11
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "app/app_mmwave_constants.h"
#include "app/app_types.h"
#include "app/app_network_packet_serializer.h"
#include "platform/platform_time.h"

int app_serialize_report(DecodedReport* report, uint8_t* packet, size_t packet_len)
{
    if(report == NULL || packet == NULL) {
        return -1;
    }
    size_t total_size = sizeof(PacketHeader_t) + sizeof(ReportPacketPayload_t);
    if(packet_len < total_size) {
        return -1;
    }

    PacketHeader_t header = {
        .version = PACKET_VERSION,
        .type = PACKET_REPORT,
        .timestamp = platform_getNumOfMs(),
        .payload_len = sizeof(ReportPacketPayload_t)
    };

    ReportPacketPayload_t report_payload = {
        .has_init_completed_info = report->has_init_completed_info,
        .has_presence_info = report->has_presence_info,
        .has_motion_info = report->has_motion_info,
        .has_bmp_info = report->has_bmp_info,
        .has_proximity_info = report->has_proximity_info,
        .has_uof_report = report->has_uof_report,
        .init_completed_info = report->init_completed_info,
        .presence_info = (uint8_t) report->presence_info,
        .motion_info = (uint8_t) report->motion_info,
        .bmp_info = (int32_t) report->bmp_info,
        .proximity_info = (uint8_t) report->proximity_info,
        .existence_energy = (int32_t) report->uof_rep.existence_energy,
        .static_distance = (float) report->uof_rep.static_distance,
        .motion_energy = (int32_t) report->uof_rep.motion_energy,
        .motion_distance = (float) report->uof_rep.motion_distance,
        .motion_speed = (float) report->uof_rep.motion_speed
    };

    ReportPacket_t report_packet = {
        .header = header,
        .report_payload = report_payload
    };

    memcpy(packet, (uint8_t*) &report_packet, total_size);
    return total_size;
}

int app_serialize_response(DecodedResponse* response, uint8_t* packet, size_t packet_len)
{
    if(response == NULL || packet == NULL) {
        return -1;
    }
    size_t total_size = sizeof(PacketHeader_t) + sizeof(ResponsePacketPayload_t);
    if(packet_len < total_size) {
        return -1;
    }

    PacketHeader_t header = {
        .version = PACKET_VERSION,
        .type = PACKET_RESPONSE,
        .timestamp = platform_getNumOfMs(),
        .payload_len = sizeof(ResponsePacketPayload_t)
    };

    ResponsePacketPayload_t response_payload = {
        .response_type = (uint8_t) response->type,
        .data_len = (uint16_t) response->data_l
    };
    memcpy(response_payload.data, response->data, response->data_l);

    ResponsePacket_t response_packet = {
        .header = header,
        .response_payload = response_payload
    };

    memcpy(packet, (uint8_t*) &response_packet, total_size);
    return total_size;
}