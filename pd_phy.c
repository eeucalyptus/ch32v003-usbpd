#include "pd_phy.h"
#include <stdint.h>
#include <ch32v003fun.h>

#define PD_BIT_LUT_NONE 0
#define PD_BIT_LUT_1 1
#define PD_BIT_LUT_0 2
#define PD_BIT_LUT_INVALID 3
#define PD_BIT_LUT_IMPOSSIBLE 3

// sample 3-4 samples per bit (see write-up)
const uint8_t pd_bit_lut[64] = {
    /*0b000000*/PD_BIT_LUT_INVALID,
    /*0b000001X*/PD_BIT_LUT_NONE,
    /*0b000010X*/PD_BIT_LUT_NONE,
    /*0b000011*/PD_BIT_LUT_NONE,
    /*0b000100*/PD_BIT_LUT_NONE,
    /*0b000101X*/PD_BIT_LUT_NONE,
    /*0b000110X*/PD_BIT_LUT_NONE,
    /*0b000111*/PD_BIT_LUT_NONE,
    /*0b001000*/PD_BIT_LUT_NONE,
    /*0b001001X*/PD_BIT_LUT_1,
    /*0b001010X*/PD_BIT_LUT_INVALID,
    /*0b001011*/PD_BIT_LUT_NONE,
    /*0b001100*/PD_BIT_LUT_NONE,
    /*0b001101X*/PD_BIT_LUT_1,
    /*0b001110X*/PD_BIT_LUT_0,
    /*0b001111*/PD_BIT_LUT_NONE,
    /*0b010000*/PD_BIT_LUT_NONE,
    /*0b010001X*/PD_BIT_LUT_0,
    /*0b010010X*/PD_BIT_LUT_1,
    /*0b010011*/PD_BIT_LUT_NONE,
    /*0b010100*/PD_BIT_LUT_NONE,
    /*0b010101X*/PD_BIT_LUT_INVALID,
    /*0b010110X*/PD_BIT_LUT_1,
    /*0b010111*/PD_BIT_LUT_NONE,
    /*0b011000*/PD_BIT_LUT_NONE,
    /*0b011001X*/PD_BIT_LUT_1,
    /*0b011010X*/PD_BIT_LUT_NONE,
    /*0b011011*/PD_BIT_LUT_NONE,
    /*0b011100*/PD_BIT_LUT_NONE,
    /*0b011101X*/PD_BIT_LUT_NONE,
    /*0b011110X*/PD_BIT_LUT_0,
    /*0b011111*/PD_BIT_LUT_INVALID,
    /*0b100000*/PD_BIT_LUT_INVALID,
    /*0b100001X*/PD_BIT_LUT_0,
    /*0b100010X*/PD_BIT_LUT_NONE,
    /*0b100011*/PD_BIT_LUT_NONE,
    /*0b100100*/PD_BIT_LUT_NONE,
    /*0b100101X*/PD_BIT_LUT_NONE,
    /*0b100110X*/PD_BIT_LUT_NONE,
    /*0b100111*/PD_BIT_LUT_NONE,
    /*0b101000*/PD_BIT_LUT_NONE,
    /*0b101001X*/PD_BIT_LUT_NONE,
    /*0b101010X*/PD_BIT_LUT_INVALID,
    /*0b101011*/PD_BIT_LUT_NONE,
    /*0b101100*/PD_BIT_LUT_NONE,
    /*0b101101X*/PD_BIT_LUT_1,
    /*0b101110X*/PD_BIT_LUT_0,
    /*0b101111*/PD_BIT_LUT_NONE,
    /*0b110000*/PD_BIT_LUT_NONE,
    /*0b110001X*/PD_BIT_LUT_0,
    /*0b110010X*/PD_BIT_LUT_1,
    /*0b110011*/PD_BIT_LUT_NONE,
    /*0b110100*/PD_BIT_LUT_NONE,
    /*0b110101X*/PD_BIT_LUT_INVALID,
    /*0b110110X*/PD_BIT_LUT_1,
    /*0b110111*/PD_BIT_LUT_NONE,
    /*0b111000*/PD_BIT_LUT_NONE,
    /*0b111001X*/PD_BIT_LUT_NONE,
    /*0b111010X*/PD_BIT_LUT_INVALID,
    /*0b111011*/PD_BIT_LUT_NONE,
    /*0b111100*/PD_BIT_LUT_NONE,
    /*0b111101X*/PD_BIT_LUT_NONE,
    /*0b111110X*/PD_BIT_LUT_INVALID,
    /*0b111111*/PD_BIT_LUT_INVALID,
};

#define KCODE_DATA_MASK (0b1111)
typedef enum {
    KCODE_0 = 0,
    KCODE_1 = 1,
    KCODE_2 = 2,
    KCODE_3 = 3,
    KCODE_4 = 4,
    KCODE_5 = 5,
    KCODE_6 = 6,
    KCODE_7 = 7,
    KCODE_8 = 8,
    KCODE_9 = 9,
    KCODE_A = 10,
    KCODE_B = 11,
    KCODE_C = 12,
    KCODE_D = 13,
    KCODE_E = 14,
    KCODE_F = 15,
    KCODE_SYNC1 = 16,
    KCODE_SYNC2 = 17,
    KCODE_RST1 = 18,
    KCODE_RST2 = 19,
    KCODE_EOP = 20,
    KCODE_SYNC3 = 21,
    KCODE_RESERVED = 22,
} Kcodes_t;


#define KCODE_MASK (0b11111)
// KCODE LUT to convert 5-bit KCODE to KCODE enum (see Table 5.1, USB PD 3.2)
const uint8_t pd_kcode_lut[] = {
    /*0b00000*/KCODE_RESERVED,
    /*0b00001*/KCODE_RESERVED,
    /*0b00010*/KCODE_RESERVED,
    /*0b00011*/KCODE_RESERVED,
    /*0b00100*/KCODE_RESERVED,
    /*0b00101*/KCODE_RESERVED,
    /*0b00110*/KCODE_SYNC3,
    /*0b00111*/KCODE_RST1,
    /*0b01000*/KCODE_RESERVED,
    /*0b01001*/KCODE_1,
    /*0b01010*/KCODE_4,
    /*0b01011*/KCODE_5,
    /*0b01100*/KCODE_RESERVED,
    /*0b01101*/KCODE_EOP,
    /*0b01110*/KCODE_6,
    /*0b01111*/KCODE_7,
    /*0b10000*/KCODE_RESERVED,
    /*0b10001*/KCODE_SYNC2,
    /*0b10010*/KCODE_8,
    /*0b10011*/KCODE_9,
    /*0b10100*/KCODE_2,
    /*0b10101*/KCODE_3,
    /*0b10110*/KCODE_A,
    /*0b10111*/KCODE_B,
    /*0b11000*/KCODE_SYNC1,
    /*0b11001*/KCODE_RST2,
    /*0b11010*/KCODE_C,
    /*0b11011*/KCODE_D,
    /*0b11100*/KCODE_E,
    /*0b11101*/KCODE_F,
    /*0b11110*/KCODE_0,
    /*0b11111*/KCODE_RESERVED,
};

const uint8_t pd_5code_lut[] = {
    /*KCODE_0*/0b11110,
    /*KCODE_1*/0b01001,
    /*KCODE_2*/0b10100,
    /*KCODE_3*/0b10101,
    /*KCODE_4*/0b01010,
    /*KCODE_5*/0b01011,
    /*KCODE_6*/0b01110,
    /*KCODE_7*/0b01111,
    /*KCODE_8*/0b10010,
    /*KCODE_9*/0b10011,
    /*KCODE_A*/0b10110,
    /*KCODE_B*/0b10111,
    /*KCODE_C*/0b11010,
    /*KCODE_D*/0b11011,
    /*KCODE_E*/0b11100,
    /*KCODE_F*/0b11101,
    /*KCODE_SYNC1*/0b11000,
    /*KCODE_SYNC2*/0b10001,
    /*KCODE_RST1*/0b00111,
    /*KCODE_RST2*/0b11001,
    /*KCODE_EOP*/0b01101,
    /*KCODE_SYNC3*/0b00110,
};

void prototype_read_preamble();
void prototype_read_data();
void prototype_read_kcode(uint8_t *kcode);

// static variables
uint8_t shifty; // a0
uint8_t bit_count; // a1
uint8_t bits; // a2
uint8_t *data_ptr; // a3
uint8_t data_buffer[64]; // static
uint32_t pd_timer; // a4

void pd_invalid_bitsequence() {
    while(1);
}

void pd_invalid_preamble() {
    while(1);
}

void pd_invalid_sof() {
    while(1);
}

void wait_for_timer() {
    pd_timer += PD_TIMER_RX_PERIOD;
    while(pd_timer > SysTick->CNT);
}

void prototype_read_packet() {
    // reset timer
    pd_timer = SysTick->CNT;

    uint8_t kcode;
    
    // already known bits in shifty
    shifty = 0b110;

    // preamble
    prototype_read_preamble();

    // read SOF
    prototype_read_kcode(&kcode);
    if(kcode != KCODE_SYNC1) {
        pd_invalid_sof();
    }
    prototype_read_kcode(&kcode);
    if(kcode != KCODE_SYNC1) {
        pd_invalid_sof();
    }
    prototype_read_kcode(&kcode);
    if(kcode != KCODE_SYNC1) {
        pd_invalid_sof();
    }
    prototype_read_kcode(&kcode);
    if(kcode != KCODE_SYNC2) {
        pd_invalid_sof();
    }

    // read data
    prototype_read_data();
}

void prototype_read_preamble() {
    bit_count = 0;
    
    while(bit_count < 50) {
        wait_for_timer();

        // sample
        uint8_t sample_in = (PD_GPIO->INDR >> PD_PIN) & 1;
        shifty = (shifty << 1) | sample_in;

        // decode bmc bits
        uint8_t bit_event = pd_bit_lut[shifty & 0b111111];
        switch (bit_event) {
            case PD_BIT_LUT_1:
                if((bit_count & 1) == 1) {
                    bit_count++;
                }
                else {
                    pd_invalid_preamble();
                }
                break;
            case PD_BIT_LUT_0:
                if((bit_count & 1) == 0) {
                    bit_count++;
                }
                else {
                    pd_invalid_preamble();
                }
                break;
            case PD_BIT_LUT_INVALID:
                pd_invalid_bitsequence();
                break;
        }
    }
}

void prototype_read_kcode(uint8_t *kcode) {
    bit_count = 0;
    bits = 0;

    while(bit_count < 5) {
        wait_for_timer();

        // sample
        uint8_t sample_in = (PD_GPIO->INDR >> PD_PIN) & 1;
        shifty = (shifty << 1) | sample_in;

        // decode bmc bits
        uint8_t bit_event = pd_bit_lut[shifty & 0b111111];
        switch (bit_event) {
            case PD_BIT_LUT_1:
                bits = (bits << 1) | 1;
                break;
            case PD_BIT_LUT_0:
                bits = (bits << 1) | 0;
                break;
            case PD_BIT_LUT_INVALID:
                pd_invalid_bitsequence();
                break;
        }
    }
    *kcode = pd_kcode_lut[bits & KCODE_MASK];
}

void prototype_read_data() {
    data_ptr = data_buffer;
    while(1) {
        uint8_t kcode;
        prototype_read_kcode(&kcode);
        if(kcode == KCODE_EOP) {
            break;
        }
        *data_ptr = kcode & KCODE_DATA_MASK;
        prototype_read_kcode(&kcode);
        // second nibble can't contain EOP
        *data_ptr |= (kcode & KCODE_DATA_MASK) << 4;
        data_ptr++;
    }
}

void wait_for_tx_timer() {
    pd_timer += PD_TIMER_TX_PERIOD;
    while(pd_timer > SysTick->CNT);
}

void send_bit(uint8_t bit) {
    PD_GPIO->OUTDR ^= (1 << PD_PIN);
    wait_for_tx_timer();
    if(bit) {
        PD_GPIO->OUTDR ^= (1 << PD_PIN);
    }
    wait_for_tx_timer();
}

void send_kcode(Kcodes_t kcode) {
    uint8_t kcode_bits = pd_5code_lut[kcode];
    for(int i = 4; i >= 0; i--) {
        send_bit((kcode_bits >> (i)) & 1);
    }
}

void prototype_send_packet() {
    // reset timer
    pd_timer = SysTick->CNT;

    // preamble
    for(int i = 0; i < 50; i++) {
        send_bit(i & 1);
    }

    // SOF
    send_kcode(KCODE_SYNC1);
    send_kcode(KCODE_SYNC1);
    send_kcode(KCODE_SYNC1);
    send_kcode(KCODE_SYNC2);

    // data
    for(int i = 0; i < 64; i++) {
        send_kcode((data_buffer[i] >> 4) & KCODE_DATA_MASK);
        send_kcode((data_buffer[i] >> 0) & KCODE_DATA_MASK);
    }
}