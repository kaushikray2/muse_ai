#include "appl_flood_mesh.h"

#include <string.h>

#define FLOOD_MESH_IDENTITY_SIZE 6U
#define FLOOD_MESH_CACHE_SIZE 16U
#define FLOOD_MESH_TTL_OFFSET 6U
#define FLOOD_MESH_CHECKSUM_OFFSET (APPL_LORA_PACKET_SIZE - 1U)

typedef struct
{
    uint8_t identity[FLOOD_MESH_IDENTITY_SIZE];
    bool valid;
} flood_mesh_cache_entry_t;

static flood_mesh_cache_entry_t packet_cache[FLOOD_MESH_CACHE_SIZE];
static uint8_t packet_cache_next;

static uint8_t flood_mesh_checksum(const uint8_t packet[APPL_LORA_PACKET_SIZE])
{
    uint8_t checksum = 0U;

    for (uint8_t index = 0U; index < FLOOD_MESH_CHECKSUM_OFFSET; index++)
    {
        checksum ^= packet[index];
    }

    return checksum;
}

static bool flood_mesh_checksum_valid(const uint8_t packet[APPL_LORA_PACKET_SIZE])
{
    return (flood_mesh_checksum(packet) == packet[FLOOD_MESH_CHECKSUM_OFFSET]);
}

static bool flood_mesh_packet_seen(const uint8_t packet[APPL_LORA_PACKET_SIZE])
{
    for (uint8_t index = 0U; index < FLOOD_MESH_CACHE_SIZE; index++)
    {
        if (packet_cache[index].valid &&
            (0 == memcmp(packet_cache[index].identity, packet, FLOOD_MESH_IDENTITY_SIZE)))
        {
            return true;
        }
    }

    return false;
}

static void flood_mesh_remember_packet(const uint8_t packet[APPL_LORA_PACKET_SIZE])
{
    if (flood_mesh_packet_seen(packet))
    {
        return;
    }

    memcpy(packet_cache[packet_cache_next].identity, packet, FLOOD_MESH_IDENTITY_SIZE);
    packet_cache[packet_cache_next].valid = true;
    packet_cache_next = (uint8_t)((packet_cache_next + 1U) % FLOOD_MESH_CACHE_SIZE);
}

void appl_flood_mesh_init(void)
{
    memset(packet_cache, 0, sizeof(packet_cache));
    packet_cache_next = 0U;
}

void appl_flood_mesh_prepare_origin(uint8_t packet[APPL_LORA_PACKET_SIZE])
{
    if (NULL == packet)
    {
        return;
    }

    packet[FLOOD_MESH_TTL_OFFSET] = APPL_LORA_MESH_MAX_HOPS;
    packet[FLOOD_MESH_CHECKSUM_OFFSET] = flood_mesh_checksum(packet);
}

bool appl_flood_mesh_process_received(uint8_t packet[APPL_LORA_PACKET_SIZE], int8_t rssi_dbm)
{
    if ((NULL == packet) || (rssi_dbm < APPL_LORA_MESH_RSSI_CUTOFF_DBM) ||
        !flood_mesh_checksum_valid(packet) ||
        (packet[FLOOD_MESH_TTL_OFFSET] > APPL_LORA_MESH_MAX_HOPS) ||
        flood_mesh_packet_seen(packet))
    {
        return false;
    }

    flood_mesh_remember_packet(packet);

    if (0U == packet[FLOOD_MESH_TTL_OFFSET])
    {
        return false;
    }

    packet[FLOOD_MESH_TTL_OFFSET]--;
    packet[FLOOD_MESH_CHECKSUM_OFFSET] = flood_mesh_checksum(packet);
    return true;
}

void appl_flood_mesh_remember_transmitted(const uint8_t packet[APPL_LORA_PACKET_SIZE])
{
    if ((NULL != packet) && flood_mesh_checksum_valid(packet))
    {
        flood_mesh_remember_packet(packet);
    }
}