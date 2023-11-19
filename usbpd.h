#ifndef _USBPD_H_
#define _USBPD_H_

#include <stdint.h>

#define KCODE_BITS (5)
#define KCODE_BYTE_BITS (2*KCODE_BITS)

#define BITSTREAM_PREAMBLE_LENGTH (50)
#define BITSTREAM_SOP_LENGTH (KCODE_BITS*4)
#define BITSTREAM_HEADER_LENGTH (KCODE_BYTE_BITS*2)
#define BITSTREAM_CRC_LENGTH (KCODE_BYTE_BITS*4)
#define BITSTREAM_EOP_LENGTH (KCODE_BITS*1)
#define BITSTREAM_PAYLOAD_LENGTH(payload_length) (KCODE_BYTE_BITS*payload_length)

#define getbit(buffer, pos) (((buffer)[pos/8] & (1 << (pos%8))) != 0)
#define setbit(buffer, pos, value) ((buffer)[(pos)/8] = ((buffer)[(pos)/8] & ~(1 << ((pos)%8))) | ((value) << ((pos)%8)))

#define BITSTREAM_MESSAGE_LENGTH(payload_length) (BITSTREAM_PREAMBLE_LENGTH + \
  BITSTREAM_SOP_LENGTH + BITSTREAM_HEADER_LENGTH + \
  BITSTREAM_PAYLOAD_LENGTH(payload_length) + BITSTREAM_CRC_LENGTH + \
  BITSTREAM_EOP_LENGTH)

#ifndef MAX_PAYLOAD_LENGTH
#define MAX_PAYLOAD_LENGTH 1024
#endif

// See USB PD Table 5-1 4b5b Symbol Encoding Table
typedef enum {
  K_CODE_0 = 0b11110,
  K_CODE_1 = 0b01001,
  K_CODE_2 = 0b10100,
  K_CODE_3 = 0b10101,
  K_CODE_4 = 0b01010,
  K_CODE_5 = 0b01011,
  K_CODE_6 = 0b01110,
  K_CODE_7 = 0b01111,
  K_CODE_8 = 0b10010,
  K_CODE_9 = 0b10011,
  K_CODE_A = 0b10110,
  K_CODE_B = 0b10111,
  K_CODE_C = 0b11010,
  K_CODE_D = 0b11011,
  K_CODE_E = 0b11100,
  K_CODE_F = 0b11101,
  K_CODE_SYNC1 = 0b11000,
  K_CODE_SYNC2 = 0b10001,
  K_CODE_RST1 = 0b00111,
  K_CODE_RST2 = 0b11001,
  K_CODE_EOP = 0b01101,
  K_CODE_SYNC3 = 0b00110,
} K_Codes_t;

// Values for the codes See USB PD Table 5-1 4b5b Symbol Encoding Table
extern const uint8_t K_CODE_VALUES[32];
extern const uint8_t K_CODE_FOR_VALUE[16];

// USB PD Message contents (see USB PD 5.6 Packet Format)
typedef struct {
    uint8_t header[2];
    uint8_t payload[MAX_PAYLOAD_LENGTH];
    uint32_t payload_length;
    uint32_t crc;
} USB_PD_Message_t;


#endif // _USBPD_H_