#ifndef APP_APPL_FLOOD_MESH_H_
#define APP_APPL_FLOOD_MESH_H_

#include <stdbool.h>
#include <stdint.h>

#define APPL_LORA_PACKET_SIZE 20U
#define APPL_LORA_MESH_MAX_HOPS 5U

#ifndef APPL_LORA_MESH_RSSI_CUTOFF_DBM
#define APPL_LORA_MESH_RSSI_CUTOFF_DBM (-100)
#endif

void appl_flood_mesh_init(void);
void appl_flood_mesh_prepare_origin(uint8_t packet[APPL_LORA_PACKET_SIZE]);
bool appl_flood_mesh_process_received(uint8_t packet[APPL_LORA_PACKET_SIZE], int8_t rssi_dbm);
void appl_flood_mesh_remember_transmitted(const uint8_t packet[APPL_LORA_PACKET_SIZE]);

#endif /* APP_APPL_FLOOD_MESH_H_ */