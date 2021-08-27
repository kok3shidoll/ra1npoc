#ifndef CHECKRA1N_COMMON_H
#define CHECKRA1N_COMMON_H

int payload_stage2(io_client_t client, uint16_t cpid, checkra1n_payload_t payload);
int pongo(io_client_t client, uint16_t cpid, checkra1n_payload_t payload);
int connect_to_stage2(io_client_t client, uint16_t cpid, checkra1n_payload_t payload);

#endif
