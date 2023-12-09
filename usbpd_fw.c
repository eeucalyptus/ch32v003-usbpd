#include "ch32v003fun.h"

#include "usbpd.h"
#include "decoder.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __riscv
// Hidden to 
void TIM1_UP_IRQHandler(void) __attribute__((interrupt));
void EXTI7_0_IRQHandler(void) __attribute__((interrupt));
#endif

// Sampling frequency:
// min 3x max bitrate with osc tolerance => 3*330KHz/(1-0.012) <= 1.003 MHz
// max 4x min bitrate with osc tolerance => 4*270KHz*1.016 >= 1.097 MHz
// 47 cycles per sample @ 48 MHz fcpu -> fs = ~= 1.021 MHz 
#define SAMPLING_PERIOD (47)


void timer_init() {
    // TIMER
    printf("Initializing timer...\r\n");
    RCC->APB2PCENR |= RCC_APB2Periph_TIM1;
    TIM1->CTLR1 |= TIM_CounterMode_Up | TIM_CKD_DIV1;
    TIM1->CTLR2 = TIM_MMS_1;
    TIM1->ATRLR = SAMPLING_PERIOD; 
    TIM1->SWEVGR = TIM_PSCReloadMode_Immediate;

    NVIC_EnableIRQ(TIM1_UP_IRQn);
    TIM1->INTFR = ~TIM_FLAG_Update;
    TIM1->DMAINTENR |= TIM_IT_Update;
    //TIM1->CTLR1 |= TIM_CEN;
}

// max message len is 240 bits
// preamble, start of packet, header, payload, crc and 4b5b encoding overhead
// #bits = 50+20+20+240÷4×5+40+5 = 435
// at 4 samples per bit in the worst case: 1740 samples
#define MAX_NUM_SAMPLES (1740)
#define SAMPLE_BUFFER_SIZE (MAX_NUM_SAMPLES/8 + (MAX_NUM_SAMPLES%8 != 0))
volatile uint8_t samplebuffer[SAMPLE_BUFFER_SIZE];
volatile uint32_t samplebuffer_index = 0;

void TIM1_UP_IRQHandler() {
    bool sample = (GPIOC->INDR & (1<<4)) != 0;
    setbit(samplebuffer, samplebuffer_index, sample);
    if(samplebuffer_index == MAX_NUM_SAMPLES) {
        TIM1->CTLR1 &= ~TIM_CEN;
        TIM1->SWEVGR |= TIM_UG;
    }

    TIM1->INTFR = ~TIM_FLAG_Update;
}

void gpio_init() {
    printf("Initializing GPIO...\r\n");
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
    GPIOC->CFGLR &= ~(0xf<<(4*4));
    GPIOC->CFGLR |= ~(0b0100<<(4*4));	// CNF = 01: floating, MODE = 00: Input
}

void EXTI7_0_IRQHandler() {
    // enable timer
    TIM1->CTLR1 |= TIM_CEN;
    // sample gpio (should always be 0, because of the falling edge)
    bool sample = (GPIOC->INDR & (1<<4)) != 0;
    setbit(samplebuffer, samplebuffer_index, sample);
    EXTI->INTFR = EXTI_Line4;
}

int main()
{
    SystemInit();
    gpio_init();
    timer_init();

    return 0;
}