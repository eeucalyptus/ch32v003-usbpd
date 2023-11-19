#include "usbpd.h"

const uint8_t K_CODE_VALUES[32] = {
    [0] = 0xff,       [1] = 0xff,       [2] = 0xff,       [3] = 0xff,
    [4] = 0xff,       [5] = 0xff,       [6] = 0xff,       [7] = 0xff,
    [8] = 0xff,       [K_CODE_1] = 0x1, [K_CODE_4] = 0x4, [K_CODE_5] = 0x5,
    [12] = 0xff,      [13] = 0xff,      [K_CODE_6] = 0x6, [K_CODE_7] = 0x7,
    [16] = 0xff,      [17] = 0xff,      [K_CODE_8] = 0x8, [K_CODE_9] = 0x9,
    [K_CODE_2] = 0x2, [K_CODE_3] = 0x3, [K_CODE_A] = 0xa, [K_CODE_B] = 0xb,
    [24] = 0xff,      [25] = 0xff,      [K_CODE_C] = 0xc, [K_CODE_D] = 0xd,
    [K_CODE_E] = 0xe, [K_CODE_F] = 0xf, [K_CODE_0] = 0x0, [31] = 0xff,
};

const uint8_t K_CODE_FOR_VALUE[16] = {
    [0] = K_CODE_0, [1] = K_CODE_1, [2] = K_CODE_2, [3] = K_CODE_3,
    [4] = K_CODE_4, [5] = K_CODE_5, [6] = K_CODE_6, [7] = K_CODE_7,
    [8] = K_CODE_8, [9] = K_CODE_9, [10] = K_CODE_A, [11] = K_CODE_B,
    [12] = K_CODE_C, [13] = K_CODE_D, [14] = K_CODE_E, [15] = K_CODE_F,
};