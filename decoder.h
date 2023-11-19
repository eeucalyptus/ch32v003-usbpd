#ifndef _DECODER_H_
#define _DECODER_H_

#include "usbpd.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BCM_DECODER_STATE_IDLE = 0,
    BCM_DECODER_STATE_PREAMBLE,
    BCM_DECODER_STATE_DATA
} BCM_Decoder_State_t;

typedef struct {
    BCM_Decoder_State_t state;
    uint32_t last_transition;
    bool last_sample;
    uint32_t preamble_count;
    bool second_transition;
    uint8_t last_samples;
} BCM_Decoder_t;

// // decodes a BCM stream, returns true if a transmission start was detected
// bool decode_bcm_start(BCM_Decoder_t *decoder, uint8_t bit);

// // decodes a BCM stream, returns true if a preamble was decoded
// bool decode_bcm_preamble(BCM_Decoder_t *decoder, uint8_t bit);

// // decodes a BCM stream, returns true if a K-code was decoded
// bool decode_bcm_kcode(BCM_Decoder_t *decoder, uint8_t bit, K_Codes_t *kcode);

// bool kcodes_to_byte(K_Codes_t *kcodes, uint8_t *byte);

void decode_bcm(uint8_t *bcm, uint32_t bcm_length, uint8_t **bitstream, uint32_t *bitstream_length_bits);

BCM_Decoder_t decoder_init();

#endif // _DECODER_H_