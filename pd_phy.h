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

#define PD_TIMER_RX_PERIOD (49)
#define PD_TIMER_TX_PERIOD (80)


#ifndef __ASSEMBLER__



#endif // __ASSEMBLER__

#endif // _PD_PHY_H