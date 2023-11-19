#ifndef _ENCODER_H_
#define _ENCODER_H_

#include "usbpd.h"
#include <stdint.h>

// converts a USB_PD_Message_t to a bit stream
void convert_to_bitstream(USB_PD_Message_t *msg, uint8_t **bitstream, uint32_t *bitstream_length);

// converts a bit stream to a BMC stream
void convert_to_bmc(uint8_t *bitstream, uint32_t bitstream_length,
                    uint8_t **bmc, uint32_t *bmc_length);

// resamples a BMC stream
void resample_bmc(uint8_t *bmc, uint32_t bmc_length, float resampling_factor, 
                  uint8_t **rbmc, uint32_t *rbmc_length);

#endif // _ENCODER_H_