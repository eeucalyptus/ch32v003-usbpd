#include "ch32v003fun.h"

#include "pd_phy.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

void gpio_init() {
    printf("Initializing GPIO...\r\n");

    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
    // high side
    PD_OH_GPIO->CFGLR &= ~(0xf<<(PD_OH_PIN*4));
    PD_OH_GPIO->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(PD_OH_PIN*4); // CNF = 01: floating, MODE = 00: input
    PD_OH_GPIO->BSHR = 1<<PD_OH_PIN; // set high
    // // low side
    // PD_OL_GPIO->CFGLR &= ~(0xf<<(PD_OL_PIN*4));
    // PD_OL_GPIO->CFGLR |= 0b0011<<(PD_OL_PIN*4); // CNF = 01: floating, MODE = 00: input
    // PD_OL_GPIO->BSHR = (1<<(16+PD_OL_PIN)); // set low

    // input OP-output
    // RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
    // GPIOD->CFGLR &= ~(0xf<<(4*4));
    // GPIOD->CFGLR |= 0b0100<<(4*4); // CNF = 00: analog, MODE = 00: input
    //EXTI->INTENR |= EXTI_Line4;
    //EXTI->FTENR |= EXTI_Line4;

    // disable uart pin
    GPIOD->CFGLR &= ~(0xf<<(5*4));
    GPIOD->CFGLR |= 0b0100<<(5*4); // CNF = 00: analog, MODE = 00: input

    // LED	
    GPIOC->CFGLR &= ~(0xf<<(4*0));
	//GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);
    GPIOC->CFGLR |= 0b0001<<(4*0); // CNF = 00: push-pull, MODE = 01: out 10MHz
}

void opamp_init( void )
{
	// enable op and select default inputs
	EXTEN->EXTEN_CTR |= EXTEN_OPA_EN; // alt inputs: EXTEN_OPA_PSEL | EXTEN_OPA_NSEL;
}

void send_packet();

void send_test_message() {
    uint8_t test_message[] = {0xDE, 0xAD, 0xBE, 0xEF};
    send_packet();
}

int main()
{
    opamp_init();
    SystemInit();
    gpio_init();

    for(int i = 5; i >= 0; i--) {
        printf("Starting in %d...\r\n", i);
        Delay_Ms(1000);
    }

    while(1) {
        printf("Sending...\r\n");
        send_test_message();
        Delay_Ms(1000);
    }

    return 0;
}