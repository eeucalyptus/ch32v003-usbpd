#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "decoder.h"
#include "usbpd.h"
#include "encoder.h"


USB_PD_Message_t input_msg = {
    .header = {0xBE, 0xEF},
    .payload = {0xDE, 0xAD, 0xBE, 0xEF},
    .payload_length = 4,
    .crc = 0xDAF00BAA,
};

uint8_t test_bitstream[] = {0b10110101, 0b11000111};

int main(int argc, char *argv[])
{
    //uint8_t *bitstream;
    //convert_to_bitstream(&input_msg, &bitstream);
    uint8_t *bcm;
    uint32_t bcm_length;
    uint8_t *rbcm;
    uint32_t rbcm_length;
    convert_to_bcm(test_bitstream, 16, &bcm, &bcm_length);
    resample_bcm(bcm, bcm_length, 1.5*1.22, &rbcm, &rbcm_length);
    printf("rbcm_length: %d\n", rbcm_length);
    print_bitstream(rbcm, rbcm_length);

    uint8_t *decoded_bitstream;
    uint32_t decoded_bitstream_length;
    decode_bcm(rbcm, rbcm_length, &decoded_bitstream, &decoded_bitstream_length);
    print_bitstream(test_bitstream, 16);
    print_bitstream(decoded_bitstream, decoded_bitstream_length);

    free(bcm);
    free(rbcm);

    return 0;
}