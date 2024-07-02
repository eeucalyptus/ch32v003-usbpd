#include "ch32v003fun.h"

#include "pd_phy.h"
#include "pd_crc.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define PD_MESSAGETYPE_GOODCRC (0x01)
#define PD_MESSAGETYPE_GETSTATUS (0x12)
#define PD_MESSAGETYPE_GETSRCCAP (0x7)
#define PD_PORTDATAROLE_UFP (0x0000)
#define PD_PORTDATAROLE_DFP (0x0020)
#define PD_SPECREV1 (0x0000)
#define PD_SPECREV2 (0x0040)
#define PD_SPECREV3 (0x0080)
#define PD_POWERROLE_SOURCE (0x0100)
#define PD_POWERROLE_SINK (0x0000)
#define PD_MESSAGEID(x) (((x) << 9) & 0x0E00)
#define PD_NUMOBJ(x) (((x) << 12) & 0x7000)
#define PD_EXTENDEDMESSAGE (0x8000)


void gpio_init() {
    printf("Initializing GPIO...\r\n");

    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
    // high side
    PD_OH_GPIO->CFGLR &= ~(0xf<<(PD_OH_PIN*4));
    PD_OH_GPIO->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_ANALOG)<<(PD_OH_PIN*4); // CNF = 01: floating, MODE = 00: input
    PD_OH_GPIO->BSHR = 1<<PD_OH_PIN; // set high
    // // low side
    PD_OL_GPIO->CFGLR &= ~(0xf<<(PD_OL_PIN*4));
    PD_OL_GPIO->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_ANALOG)<<(PD_OL_PIN*4); // CNF = 01: floating, MODE = 00: input
    PD_OL_GPIO->BSHR = (1<<(16+PD_OL_PIN)); // set low

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

void send_packet(uint8_t *data, uint32_t len);

void send_test_message() {

    //uint8_t test_message[] = {0xC0, 0xFE, 0xDE, 0xAD, 0xBE, 0xEF,0,0,0,0};
    //uint8_t test_message[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,0,0,0,0};
    //uint8_t test_message[] = {0xa1, 0x41, 0x2c, 0x91, 0x01, 0x0a, 0x2c, 0xd1, 0x02, 0x00, 0x2c, 0xb1, 0x04, 0x00, 0x45, 0x41, 0x06, 0x00,0,0,0,0};

    // uint16_t header = PD_MESSAGETYPE_GOODCRC | PD_PORTDATAROLE_UFP | PD_SPECREV3 | PD_POWERROLE_SINK | PD_MESSAGEID(0);
    // uint8_t test_message[] = {(header >> 0) & 0xFF, (header >> 8) & 0xff, 0,0,0,0};

    uint16_t header = PD_MESSAGETYPE_GETSRCCAP | PD_PORTDATAROLE_UFP | PD_SPECREV3 | PD_POWERROLE_SINK | PD_MESSAGEID(0);
    uint8_t test_message[] = {(header >> 0) & 0xFF, (header >> 8) & 0xff, 0,0,0,0};

    pd_crc_init();
    for(int i = 0; i < sizeof(test_message)-4; i++) {
        pd_crc_update(test_message[i]);
    }
    int crc = pd_crc_finalize();
    printf("CRC: %08X\r\n", crc);
    test_message[sizeof(test_message)-4] = (crc >>  0) & 0xFF;
    test_message[sizeof(test_message)-3] = (crc >>  8) & 0xFF;
    test_message[sizeof(test_message)-2] = (crc >> 16) & 0xFF;
    test_message[sizeof(test_message)-1] = (crc >> 24) & 0xFF;
    
    PD_OH_GPIO->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(PD_OH_PIN*4); // CNF = 01: floating, MODE = 00: input
    PD_OL_GPIO->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(PD_OL_PIN*4); // CNF = 01: floating, MODE = 00: input
    send_packet(test_message, sizeof(test_message));
    PD_OH_GPIO->CFGLR &= ~(0xf<<(PD_OH_PIN*4)); // CNF = 01: floating, MODE = 00: input
    PD_OL_GPIO->CFGLR &= ~(0xf<<(PD_OL_PIN*4)); // CNF = 01: floating, MODE = 00: input
}

int main()
{
    opamp_init();
    SystemInit();
    gpio_init();

    for(int i = 10; i >= 0; i--) {
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