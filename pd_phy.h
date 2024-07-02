#ifndef _PD_PHY_H
#define _PD_PHY_H

#define LOCAL_CONCAT_BASE(A, B) A##B##_BASE
#define LOCAL_EXP_BASE(A, B) LOCAL_CONCAT_BASE(A,B)

#define LOCAL_CONCAT(A, B) A##B
#define LOCAL_EXP(A, B) LOCAL_CONCAT(A,B)

#define USB_GPIO_BASE LOCAL_EXP_BASE( GPIO, USB_PORT )

#define PD_PORT D
#define PD_PIN 4
#define PD_GPIO LOCAL_EXP(GPIO, PD_PORT)
#define PD_GPIO_BASE LOCAL_EXP_BASE( GPIO, PD_PORT )

#define PD_OH_PORT C
#define PD_OH_PIN 4
#define PD_OH_GPIO LOCAL_EXP(GPIO, PD_OH_PORT)
#define PD_OH_GPIO_BASE LOCAL_EXP_BASE( GPIO, PD_OH_PORT )

#define PD_OL_PORT C
#define PD_OL_PIN 3
#define PD_OL_GPIO LOCAL_EXP(GPIO, PD_OL_PORT)
#define PD_OL_GPIO_BASE LOCAL_EXP_BASE( GPIO, PD_OL_PORT )

#define PD_TIMER_RX_PERIOD (49)
#define PD_TIMER_TX_PERIOD (80)

#define PD_BIT_LUT_NONE 0
#define PD_BIT_LUT_1 1
#define PD_BIT_LUT_0 2
#define PD_BIT_LUT_INVALID 3
#define PD_BIT_LUT_IMPOSSIBLE 3

#define KCODE_DATA_MASK (0b1111)
#define PD_KCODE_0 (0)
#define PD_KCODE_1 (1)
#define PD_KCODE_2 (2)
#define PD_KCODE_3 (3)
#define PD_KCODE_4 (4)
#define PD_KCODE_5 (5)
#define PD_KCODE_6 (6)
#define PD_KCODE_7 (7)
#define PD_KCODE_8 (8)
#define PD_KCODE_9 (9)
#define PD_KCODE_A (10)
#define PD_KCODE_B (11)
#define PD_KCODE_C (12)
#define PD_KCODE_D (13)
#define PD_KCODE_E (14)
#define PD_KCODE_F (15)
#define PD_KCODE_SYNC1 (16)
#define PD_KCODE_SYNC2 (17)
#define PD_KCODE_RST1 (18)
#define PD_KCODE_RST2 (19)
#define PD_KCODE_EOP (20)
#define PD_KCODE_SYNC3 (21)
#define PD_KCODE_RESERVED (22)

#ifndef __ASSEMBLER__



#endif // __ASSEMBLER__

#endif // _PD_PHY_H