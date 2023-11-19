#include "decoder.h"
#include "encoder.h"
#include "usbpd.h"
#include <stdio.h>
#include <stdlib.h> 


bool decode_bcm_bit(BCM_Decoder_t *decoder, bool sample, bool *bit) {
    decoder->last_samples = (decoder->last_samples << 1) | sample;
    if(sample == decoder->last_sample) {
        decoder->last_transition++;
        if(decoder->last_transition == 3) {
            // emit '0'
            *bit = 0;
            return true;
        }
        else if(decoder->last_transition > 4) {
            // TIMEOUT
        }
    }
    else {
        if(decoder->last_transition <= 2 && !decoder->second_transition) {
            // second transition, emit '1'
            decoder->last_sample = sample;
            decoder->last_transition = 1;
            decoder->second_transition = true;
            *bit = 1;
            return true;
        }
        else {
            // first transition, common for both '0' and '1', don't emit yet
            decoder->second_transition = false;
            decoder->last_sample = sample;
            decoder->last_transition = 1;
        }
    }
    return false;
}

void decode_bcm(uint8_t *bcm, uint32_t bcm_length, uint8_t **bitstream, uint32_t *bitstream_length_bits) {
    int pos = 0;
    int num_preamble_bits = 0;
    BCM_Decoder_t decoder = decoder_init();

    int bitstream_length_bytes = (bcm_length)/8 + ((bcm_length)%8 != 0);
    *bitstream = malloc(bitstream_length_bytes);

    uint32_t bit_counter = 0;
    for(int i = 0; i < bcm_length; i++) {
        bool bit;
        if(decode_bcm_bit(&decoder, getbit(bcm, i), &bit)) {
            printf("got %d\n", bit);
            setbit(*bitstream, bit_counter, bit);
            bit_counter++;
        }
    }
    *bitstream_length_bits = bit_counter;

    // while(num_preamble_bits < BITSTREAM_PREAMBLE_LENGTH && pos < bcm_length) {
    //     bool bit;
    //     if(decode_bcm_bit(&decoder, getbit(bcm, pos), &bit)) {
    //         bool expected_bit = (num_preamble_bits % 2 != 0);
    //         if(bit != expected_bit) {
    //           printf("error in preamble decoding, pos: %d, expected: %d, got: "
    //                  "%d\n",
    //                  pos, expected_bit, bit);
    //         }
    //         num_preamble_bits++;
    //     }
    //     pos++;
    // }
    // printf("number of decoded preable bits: %d, position at end: %d\n", num_preamble_bits, pos);
}

BCM_Decoder_t decoder_init() {
    BCM_Decoder_t decoder = {
        .last_sample = 1,
        .last_transition = -1,
        .second_transition = false,
        .last_samples = 0xff,
    };
    return decoder;
}