//
// USB PD CRC calculation as in example code from USB_PD_R3_2 V1.0 2023-10.pdf
//
#include <stdio.h>
#include <stdint.h>

#define CRC_POLY 0x04C11DB7
#define CRC_INIT 0xFFFFFFFF

static int current_crc = 0xffffffff;

void pd_crc_init() {
    current_crc = 0xffffffff;
}

void pd_crc_update(uint8_t data) {
    int newbit, newword, rl_crc;
    for(int i=0; i<8; i++) {
        newbit = ((current_crc>>31) ^ ((data>>i)&1)) & 1;
        if(newbit) newword=CRC_POLY-1; else newword=0;
        rl_crc = (current_crc<<1) | newbit;
        current_crc = rl_crc ^ newword;
    }
}

uint32_t pd_crc_finalize() {
    int ret = 0;
    int j, bit;
    current_crc = ~current_crc;
    for(int i=0;i<32;i++) {
        j = 31-i;
        bit = (current_crc>>i) & 1;
        ret |= bit<<j;
    }
    return ret;
}


// This message should have a CRC of 0xea4afa09:
// const char msg[] = {
//     0xa1, 0x41,
//     0x2c, 0x91, 0x01, 0x0a,
//     0x2c, 0xd1, 0x02, 0x00,
//     0x2c, 0xb1, 0x04, 0x00,
//     0x45, 0x41, 0x06, 0x00,
// };
//
// int main() {
//     pd_crc_init();
//     for(int i = 0; i < sizeof(msg); i++) {
//         pd_crc_update(msg[i]);
//     }
//     int crc = pd_crc_finalize();
//     printf("CRC: 0x%08X\r\n", crc);
//     return 0;
// }