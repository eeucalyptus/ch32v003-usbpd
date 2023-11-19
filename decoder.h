#ifndef _DECODER_H_
#define _DECODER_H_

#include "usbpd.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PACKET_DECODER_STATE_PREAMBLE,
    PACKET_DECODER_STATE_SOP,
    PACKET_DECODER_STATE_HEADER,
    PACKET_DECODER_STATE_PAYLOAD,
    PACKET_DECODER_STATE_DONE,
    PACKET_DECODER_STATE_ERROR,
} Packet_Decoder_State_t;

typedef struct {
    Packet_Decoder_State_t state;
    uint32_t bit_counter;
    uint16_t bit_buffer;
    uint32_t state_counter;
    uint8_t header[2];
    // Payload contains CRC in the last 4 bytes
    uint8_t payload[MAX_PAYLOAD_LENGTH];
    uint32_t payload_length;
} Packet_Decoder_t;

// decodes a packet from a bit stream. Returns true if a packet was decoded
bool decode_packet_bit(Packet_Decoder_t *decoder, bool bit);

typedef struct {
    uint32_t last_transition;
    bool last_sample;
    bool second_transition;
    uint8_t last_samples;
} BMC_Decoder_t;

// decodes a BMC bit from a sample stream. Returns true if a bit was decoded
bool decode_bmc_sample(BMC_Decoder_t *decoder, bool sample, bool *bit);

// decodes a BMC stream to a bit stream
void decode_bmc(uint8_t *bmc, uint32_t bmc_length, uint8_t **bitstream, uint32_t *bitstream_length_bits);

BMC_Decoder_t bmc_decoder_init();
Packet_Decoder_t packet_decoder_init();

#endif // _DECODER_H_