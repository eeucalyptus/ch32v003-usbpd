#ifndef _ENCODER_H_
#define _ENCODER_H_

#include "usbpd.h"
#include <stdint.h>

// converts a USB_PD_Message_t to a bit stream
void convert_to_bitstream(USB_PD_Message_t *msg, uint8_t **bitstream, uint32_t *bitstream_length);

// converts a bit stream to a BCM stream
void convert_to_bcm(uint8_t *bitstream, uint32_t bitstream_length,
                    uint8_t **bcm, uint32_t *bcm_length);

// resamples a BCM stream
void resample_bcm(uint8_t *bcm, uint32_t bcm_length, float resampling_factor, 
                  uint8_t **rbcm, uint32_t *rbcm_length);

#endif // _ENCODER_H_