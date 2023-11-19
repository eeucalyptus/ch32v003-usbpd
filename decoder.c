#include "decoder.h"
#include "debug_usbpd.h"
#include "encoder.h"
#include "usbpd.h"
#include <stdio.h>
#include <stdlib.h> 


bool decode_packet_bit(Packet_Decoder_t *decoder, bool bit) {
    static int global_bitcounter = 0;
    global_bitcounter++;
    switch (decoder->state) {
        case PACKET_DECODER_STATE_PREAMBLE:
            if(bit == (decoder->bit_counter%2 != 0)) {
                decoder->bit_counter++;
                if(decoder->bit_counter == BITSTREAM_PREAMBLE_LENGTH) {
                    // Preamble done
                    decoder->state = PACKET_DECODER_STATE_SOP;
                    decoder->state_counter = 0;
                    decoder->bit_counter = 0;
                }
            }
            else {
                // Error, preamble not correct
                decoder->state = PACKET_DECODER_STATE_ERROR;
                // TODO should rather reset decoder here, the error state is for debugging
            }
            break;
        case PACKET_DECODER_STATE_SOP:
            decoder->bit_buffer = (decoder->bit_buffer << 1) | bit;
            decoder->bit_counter++;
            if(decoder->bit_counter == 5) {
                K_Codes_t kcode = decoder->bit_buffer & 0b11111;
                if(decoder->state_counter < 3 && kcode == K_CODE_SYNC1) {
                    // K_CODE_SYNC1 received
                    decoder->bit_counter = 0;
                    decoder->state_counter++;
                }
                else if(decoder->state_counter == 3 && kcode == K_CODE_SYNC2) {
                    // K_CODE_SYNC2 received
                    decoder->bit_counter = 0;
                    decoder->state = PACKET_DECODER_STATE_HEADER;
                    decoder->state_counter = 0;
                }
                else {
                    // Error, SOP not correct
                    decoder->state = PACKET_DECODER_STATE_ERROR;
                }
            }
            break;
        case PACKET_DECODER_STATE_HEADER:
            decoder->bit_buffer = (decoder->bit_buffer << 1) | bit;
            decoder->bit_counter++;
            if(decoder->bit_counter == 10) {
                uint8_t kcode_l = (decoder->bit_buffer >> 0) & 0b11111;
                uint8_t kcode_h = (decoder->bit_buffer >> 5) & 0b11111;
                uint8_t byte = 0;
                byte |= K_CODE_VALUES[kcode_l] << 0;
                byte |= K_CODE_VALUES[kcode_h] << 4;

                decoder->header[decoder->state_counter] = byte;
                decoder->state_counter++;
                decoder->bit_counter = 0;
                if(decoder->state_counter == 2) {
                    // Header done
                    decoder->state = PACKET_DECODER_STATE_PAYLOAD;
                    decoder->state_counter = 0;
                    decoder->bit_counter = 0;
                }
            }
            break;
        case PACKET_DECODER_STATE_PAYLOAD:
            decoder->bit_buffer = (decoder->bit_buffer << 1) | bit;
            decoder->bit_counter++;
            if(decoder->bit_counter == 5) {
                uint8_t kcode = decoder->bit_buffer & 0b11111;
                if(kcode == K_CODE_EOP) {
                    // EOP received
                    decoder->state = PACKET_DECODER_STATE_DONE;
                    decoder->payload_length = decoder->state_counter;
                    decoder->bit_counter = 0;
                    decoder->state_counter = 0;
                    return true;
                }
            }
            if(decoder->bit_counter == 10) {
                uint8_t kcode_l = (decoder->bit_buffer >> 0) & 0b11111;
                uint8_t kcode_h = (decoder->bit_buffer >> 5) & 0b11111;
                uint8_t byte = 0;
                byte |= K_CODE_VALUES[kcode_l] << 0;
                byte |= K_CODE_VALUES[kcode_h] << 4;

                decoder->payload[decoder->state_counter] = byte;
                decoder->state_counter++;
                decoder->bit_counter = 0;
            }
            break;
        default:
            break;
    }
    return false;
}

bool decode_bmc_sample(BMC_Decoder_t *decoder, bool sample, bool *bit) {
    // TODO fix appended 0 at the end, its decoded as a 0 althought it's not part of the message
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

void decode_bmc(uint8_t *bmc, uint32_t bmc_length, uint8_t **bitstream, uint32_t *bitstream_length_bits) {
    int pos = 0;
    int num_preamble_bits = 0;
    BMC_Decoder_t bmc_decoder = bmc_decoder_init();
    Packet_Decoder_t packet_decoder = packet_decoder_init();

    int bitstream_length_bytes = (bmc_length)/8 + ((bmc_length)%8 != 0);
    *bitstream = malloc(bitstream_length_bytes);

    uint32_t bit_counter = 0;
    for(int i = 0; i < bmc_length; i++) {
        bool bit;
        if(decode_bmc_sample(&bmc_decoder, getbit(bmc, i), &bit)) {
            setbit(*bitstream, bit_counter, bit);
            bit_counter++;
            bool packet_decoded = decode_packet_bit(&packet_decoder, bit);
            if (packet_decoded) {
                printf("packet decoded\n");
                printf("header: %02x %02x\n", packet_decoder.header[0], packet_decoder.header[1]);
                printf("payload length: %d\n", packet_decoder.payload_length);
                printf("payload: ");
                for(int i = 0; i < packet_decoder.payload_length-4; i++) {
                    printf("%02x ", packet_decoder.payload[i]);
                }
                printf("\n");
                int crc_pos = packet_decoder.payload_length-4;
                printf("crc: %08x\n", packet_decoder.payload[crc_pos] | packet_decoder.payload[crc_pos+1] << 8 | packet_decoder.payload[crc_pos+2] << 16 | packet_decoder.payload[crc_pos+3] << 24);
            }
        }
    }
    *bitstream_length_bits = bit_counter;
}

BMC_Decoder_t bmc_decoder_init() {
    BMC_Decoder_t decoder = {
        .last_sample = 1,
        .last_transition = 100, // assume we start with no transitions
        .second_transition = false,
        .last_samples = 0xff,
    };
    return decoder;
}

Packet_Decoder_t packet_decoder_init() {
    Packet_Decoder_t decoder = {
        .state = PACKET_DECODER_STATE_PREAMBLE,
        .state_counter = 0,
        .bit_counter = 0,
        .bit_buffer = 0,
    };
    return decoder;
}