#ifndef _PD_CRC_H
#define _PD_CRC_H

#include <stdint.h>

void pd_crc_init();
void pd_crc_update(uint8_t data);
uint32_t pd_crc_finalize();

#endif