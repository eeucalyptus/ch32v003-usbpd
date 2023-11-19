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

    // convert to bmc
    uint8_t *bmc;
    uint32_t bmc_length;
    convert_to_bmc(bitstream, bitstream_length, &bmc, &bmc_length);

    // resample bmc
    uint8_t *rbmc;
    uint32_t rbmc_length;
    resample_bmc(bmc, bmc_length, 1.5*1.22, &rbmc, &rbmc_length);

    // print bmc stream
    printf("bmc stream:\n");
    print_bitstream(rbmc, rbmc_length);

    // decode bmc
    uint8_t *decoded_bitstream;
    uint32_t decoded_bitstream_length;
    decode_bmc(rbmc, rbmc_length, &decoded_bitstream, &decoded_bitstream_length);

    // print decoded bitstream
    printf("decoded bitstream:\n");
    print_bitstream(decoded_bitstream, decoded_bitstream_length);

    // compare bitstreams
    bool eq = compare_bitstreams(bitstream, decoded_bitstream, bitstream_length);
    printf("bitstreams are %s\n", eq ? "equal" : "not equal");

    free(bmc);
    free(rbmc);
    free(decoded_bitstream);

    return 0;
}