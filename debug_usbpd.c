#include "debug_usbpd.h"

#include <stdio.h>

void print_bitstream(uint8_t *bitstream, int bitstream_length_bits)
{
    for(int i = 0; i < bitstream_length_bits; i++) {
        printf("%d,", getbit(bitstream, i));
    }
    printf("\n");
}


bool compare_bitstreams(uint8_t *bitstream1, uint8_t *bitstream2, int bitstream_length_bits) {
    for(int  i = 0; i < bitstream_length_bits; i++) {
        if(getbit(bitstream1, i) != getbit(bitstream2, i)) {
            return false;
        }
    }
    return true;
}