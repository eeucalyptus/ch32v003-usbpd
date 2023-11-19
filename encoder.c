#include "encoder.h"
#include "usbpd.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

// Number of prepended '1's in BCM stream
#define BCM_PREFIX_LENGTH (0)
// Number of appended '1's in BCM stream
#define BCM_POSTFIX_LENGTH (0)

// sets the 5 bits of a K code in the buffer and returns the number of bits set
int setkcode(uint8_t *buffer, int pos, K_Codes_t code) {
    for(int i = 0; i < 5; i++) {
        setbit(buffer, pos+4-i, ((code >> i) & 1));
    }
    return KCODE_BITS;
}

// sets the 10 kcode-bits of a byte in the buffer and returns the number of bits set
int setbyte(uint8_t *buffer, int pos, uint8_t byte) {
    setkcode(buffer, pos, K_CODE_FOR_VALUE[byte & 0b1111]);
    setkcode(buffer, pos+5, K_CODE_FOR_VALUE[byte >> 4]);
    return KCODE_BYTE_BITS;
}

void convert_to_bitstream(USB_PD_Message_t *msg, uint8_t **bitstream, uint32_t *bitstream_length) {
    // calculate length of bitstream
    int bitstream_length_bits = BITSTREAM_MESSAGE_LENGTH(msg->payload_length);
    int bitstream_length_bytes = bitstream_length_bits/8 + (bitstream_length_bits%8 != 0);
    *bitstream_length = bitstream_length_bits;
    // allocate memory for bitstream
    *bitstream = malloc(bitstream_length_bytes);
    
    int bitcount = 0;
    // preamble
    for(int i = 0; i < BITSTREAM_PREAMBLE_LENGTH; i++) {
        setbit(*bitstream, i, (i%2 != 0));
    }
    bitcount += BITSTREAM_PREAMBLE_LENGTH;
    
    // start of packet
    bitcount += setkcode(*bitstream, bitcount, K_CODE_SYNC1);
    bitcount += setkcode(*bitstream, bitcount, K_CODE_SYNC1);
    bitcount += setkcode(*bitstream, bitcount, K_CODE_SYNC1);
    bitcount += setkcode(*bitstream, bitcount, K_CODE_SYNC2);
    
    // header
    bitcount += setbyte(*bitstream, bitcount, msg->header[0]);
    bitcount += setbyte(*bitstream, bitcount, msg->header[1]);

    // payload
    for(int i = 0; i < msg->payload_length; i++) {
        bitcount += setbyte(*bitstream, bitcount, msg->payload[i]);
    }

    // crc
    bitcount += setbyte(*bitstream, bitcount, msg->crc & 0xFF);
    bitcount += setbyte(*bitstream, bitcount, (msg->crc >> 8) & 0xFF);
    bitcount += setbyte(*bitstream, bitcount, (msg->crc >> 16) & 0xFF);
    bitcount += setbyte(*bitstream, bitcount, (msg->crc >> 24) & 0xFF);

    // end of packet
    bitcount += setkcode(*bitstream, bitcount, K_CODE_EOP);
}

void convert_to_bcm(uint8_t *bitstream, uint32_t bitstream_length,
                    uint8_t **bcm, uint32_t *bcm_length) {
    // calculate length of bcm
    *bcm_length =
        bitstream_length*2 + BCM_PREFIX_LENGTH + BCM_POSTFIX_LENGTH;
    int bcm_length_bytes = *bcm_length / 8 + (*bcm_length % 8 != 0);
    // allocate memory for bcm
    *bcm = malloc(bcm_length_bytes);

    // write prefix
    for (int i = 0; i < BCM_PREFIX_LENGTH; i++) {
        setbit(*bcm, i, 1);
    }
    bool last_state = 1;
    // write bitstream as differential manchester code (BCM)
    // Every bit starts with a transition. A 1 has a second transition, a 0 
    // doesn't have a second transition
    for(int i = 0; i < bitstream_length; i++) {
        bool bit = getbit(bitstream, i);
        if(bit) {
            setbit(*bcm, BCM_PREFIX_LENGTH + i*2, !last_state);
            setbit(*bcm, BCM_PREFIX_LENGTH + i*2 + 1, last_state);
            last_state = last_state;
        } else {
            setbit(*bcm, BCM_PREFIX_LENGTH + i*2, !last_state);
            setbit(*bcm, BCM_PREFIX_LENGTH + i*2 + 1, !last_state);
            last_state = !last_state;
        }
    }
    // write postfix
    for(int i = 0; i < BCM_POSTFIX_LENGTH; i++) {
        setbit(*bcm, (*bcm_length)-i-1, 1);
    }
}

void resample_bcm(uint8_t *bcm, uint32_t bcm_length, float resampling_factor, 
                  uint8_t **rbcm, uint32_t *rbcm_length) {
    // calculate length of rbcm
    float rbcm_length_float = bcm_length * resampling_factor;
    *rbcm_length = ceil(rbcm_length_float);
    int rbcm_length_bytes = *rbcm_length / 8 + (*rbcm_length % 8 != 0);
    // allocate memory for rbcm
    *rbcm = malloc(rbcm_length_bytes);

    // resample, floor sample position
    for(uint32_t i = 0; i < *rbcm_length; i++) {
        uint32_t sample_pos = floor(i/resampling_factor);
        setbit(*rbcm, i, getbit(bcm, sample_pos));
    }
}