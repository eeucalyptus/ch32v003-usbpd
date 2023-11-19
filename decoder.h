#ifndef _DECODER_H_
#define _DECODER_H_

#include "usbpd.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BMC_DECODER_STATE_IDLE = 0,
    BMC_DECODER_STATE_PREAMBLE,
    BMC_DECODER_STATE_DATA
} BMC_Decoder_State_t;

typedef struct {
    BMC_Decoder_State_t state;
    uint32_t last_transition;
    bool last_sample;
    uint32_t preamble_count;
    bool second_transition;
    uint8_t last_samples;
} BMC_Decoder_t;

// // decodes a BMC stream, returns true if a transmission start was detected
// bool decode_bmc_start(BMC_Decoder_t *decoder, uint8_t bit);

// // decodes a BMC stream, returns true if a preamble was decoded
// bool decode_bmc_preamble(BMC_Decoder_t *decoder, uint8_t bit);

// // decodes a BMC stream, returns true if a K-code was decoded
// bool decode_bmc_kcode(BMC_Decoder_t *decoder, uint8_t bit, K_Codes_t *kcode);

// bool kcodes_to_byte(K_Codes_t *kcodes, uint8_t *byte);

void decode_bmc(uint8_t *bmc, uint32_t bmc_length, uint8_t **bitstream, uint32_t *bitstream_length_bits);

BMC_Decoder_t decoder_init();

#endif // _DECODER_H_