#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "usbpd.h"
#include "decoder.h"
#include "encoder.h"
#include "debug_usbpd.h"


USB_PD_Message_t input_msg = {
    .header = {0xBE, 0xEF},
    .payload = {0xDE, 0xAD, 0xBE, 0xEF},
    .payload_length = 4,
    .crc = 0xDAF00BAA,
};

int main(int argc, char *argv[])
{
    // Generate bitstream from message
    uint8_t *bitstream;
    uint32_t bitstream_length;
    convert_to_bitstream(&input_msg, &bitstream, &bitstream_length);

    // print bitstream
    printf("bitstream:\n");
    print_bitstream(bitstream, bitstream_length);

    // convert to bcm
    uint8_t *bcm;
    uint32_t bcm_length;
    convert_to_bcm(bitstream, bitstream_length, &bcm, &bcm_length);

    // resample bcm
    uint8_t *rbcm;
    uint32_t rbcm_length;
    resample_bcm(bcm, bcm_length, 1.5*1.22, &rbcm, &rbcm_length);

    // print bcm stream
    printf("bcm stream:\n");
    print_bitstream(rbcm, rbcm_length);

    // decode bcm
    uint8_t *decoded_bitstream;
    uint32_t decoded_bitstream_length;
    decode_bcm(rbcm, rbcm_length, &decoded_bitstream, &decoded_bitstream_length);

    // print decoded bitstream
    printf("decoded bitstream:\n");
    print_bitstream(decoded_bitstream, decoded_bitstream_length);

    // compare bitstreams
    bool eq = compare_bitstreams(bitstream, decoded_bitstream, bitstream_length);
    printf("bitstreams are %s\n", eq ? "equal" : "not equal");

    free(bcm);
    free(rbcm);
    free(decoded_bitstream);

    return 0;
}