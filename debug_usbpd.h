#ifndef _DEBUG_USBPD_H_
#define _DEBUG_USBPD_H_

#include "usbpd.h"

#include <stdbool.h>
#include <stdint.h>

// prints a bit stream
void print_bitstream(uint8_t *bitstream, int bitstream_length_bits);

// return true if the bitstreams are equal
bool compare_bitstreams(uint8_t *bitstream1, uint8_t *bitstream2, int bitstream_length_bits);

#endif // _DEBUG_USBPD_H_