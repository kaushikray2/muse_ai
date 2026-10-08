/**
 * \file service_telemetry.c
 * \brief Packs GNSS data into telemetry packets and schedules LoRa transmission.
 * \details Builds fixed-size packets and tracks the configured telemetry interval.
 * \author Kaushik Ray
 * \date Last Modified: 30 Sep 2026
 * \copyright (C) 2026 Siliconbrane Inc. All rights reserved.
 * \note Proprietary source. Unauthorized use or distribution is prohibited.
 */

#include "service_telemetry.h"
#include "service_headers.h"
#include "appl_lora.h"
#include "appl_flood_mesh.h"

/* ========================================================================
   PRIVATE DEFINES
   ======================================================================== */

/** 30-second interval in milliseconds */
#define TELEMETRY_INTERVAL_MS  10000u

/** Tick interval in milliseconds (called every 5ms from 5ms scheduler slot 2) */
#define TICK_INTERVAL_MS       5u

/** Number of ticks to reach 60 seconds = 60000ms / 5ms = 12000 */
#define TELEMETRY_TICKS        (TELEMETRY_INTERVAL_MS / TICK_INTERVAL_MS)

/** Telemetry packet size in bytes */
#define TELEMETRY_PACKET_SIZE  20u


/* ========================================================================
   PRIVATE VARIABLES
   ======================================================================== */

/** Current message ID counter (auto-incremented on each transmission) */
static uint16_t telemetry_message_id = 0u;

/** Tick counter for 60-second interval timer */
static uint32_t telemetry_tick_counter = 0u;

/* ========================================================================
   EXTERN VARIABLES
   ======================================================================== */

/* ========================================================================
   PRIVATE FUNCTION PROTOTYPES
   ========================================================================  */
static void telemetry_pack_and_transmit(void);

/* ========================================================================
   PUBLIC FUNCTION IMPLEMENTATIONS
   ======================================================================== */

void service_telemetry_init(void)
{
    /* Initialize message ID counter to 0 (will be incremented before first transmission) */
    telemetry_message_id = 0u;

    /* Reset tick counter */
    telemetry_tick_counter = TELEMETRY_INTERVAL_MS;
}

void service_telemetry_tick(void)
{   
    /* Increment tick counter */
    telemetry_tick_counter++;

    /* Check if 60-second interval has elapsed */
    if (telemetry_tick_counter >= TELEMETRY_TICKS)
    {
        telemetry_tick_counter = 0u;

        /* Only transmit if GPS has valid fix */
        //if (service_gps_is_valid())
        {
            telemetry_pack_and_transmit();
        }
    }
}

uint16_t service_telemetry_get_message_id(void)
{
    return telemetry_message_id;
}

/* ========================================================================
   PRIVATE FUNCTION IMPLEMENTATIONS
   ======================================================================== */

/**
 * \brief Pack GPS data into 20-byte telemetry packet using bitwise operators
 *
 * \details Constructs packet using bitwise shifts and OR operations.
 *          No pointer dereferencing, no struct member assignments.
 *          GPS coordinates already in microdegrees from GPS driver.
 *
 * \param packet Pointer to 20-byte buffer for the packet
 */
static void telemetry_pack_packet(uint8_t packet[20])
{
    //uint8_t *id_ptr = (uint8_t *)0x000000B0;
    bsp_unique_id_t const * p_uid = R_BSP_UniqueIdGet();
    const gps_data_t *gps = service_gps_get_data();
    
    /* Byte 0-3: node_id */
    packet[0] = (uint8_t)((p_uid->unique_id_words[0] >> 24) & 0xFFU);
    packet[1] = (uint8_t)((p_uid->unique_id_words[0] >> 16) & 0xFFU);
    packet[2] = (uint8_t)((p_uid->unique_id_words[0] >> 8) & 0xFFU);
    packet[3] = (uint8_t)(p_uid->unique_id_words[0] & 0xFFU);
    
    /* Bytes 4-5: message_id (big-endian) */
    packet[4] = (uint8_t)((telemetry_message_id >> 8) & 0xFF);
    packet[5] = (uint8_t)((telemetry_message_id >> 0) & 0xFF);

    /* Byte 7-9: time [0]=Hr, [1]=Min, [2]=Sec.*/
    packet[7] = gps->hour;
    packet[8] = gps->minute;
    packet[9] = gps->second;

    /* Bytes 10-13: latitude in microdegrees (big-endian) */
    int32_t lat = gps->latitude_uDeg;
    packet[10] = (uint8_t)((lat >> 24) & 0xFF);
    packet[11] = (uint8_t)((lat >> 16) & 0xFF);
    packet[12] = (uint8_t)((lat >>  8) & 0xFF);
    packet[13] = (uint8_t)((lat >>  0) & 0xFF);
    
    /* Bytes 14-17: longitude in microdegrees (big-endian) */
    int32_t lon = gps->longitude_uDeg;
    packet[14] = (uint8_t)((lon >> 24) & 0xFF);
    packet[15] = (uint8_t)((lon >> 16) & 0xFF);
    packet[16] = (uint8_t)((lon >>  8) & 0xFF);
    packet[17] = (uint8_t)((lon >> 0) & 0xFF);

    /* Bytes 18: battery voltage in % */
    uint16_t battery = 3800u;
    packet[18] = (uint8_t)((battery * 100) / 4200); /* Scale to percentage (assuming 4.2V max) */
    
    appl_flood_mesh_prepare_origin(packet);
}

/**
 * \brief Pack and transmit telemetry packet
 *
 * \details Constructs a 20-byte telemetry packet with current GPS data
 *          and transmits it via LoRa radio. Increments message ID counter.
 *          No intermediate struct or pointer usage.
 */
 //uint8_t packet[TELEMETRY_PACKET_SIZE]; //for debug only
static void telemetry_pack_and_transmit(void)
{
    /* Local packet buffer on stack (no dynamic allocation) */
    uint8_t packet[TELEMETRY_PACKET_SIZE];
    
    /* Pack the packet using bitwise operators */
    telemetry_pack_packet(packet);
    
    /* Increment message ID for next transmission */
    telemetry_message_id++;
    
    /* Transmit the packet via LoRa */
    lora_tx_packet_it(packet);
}
