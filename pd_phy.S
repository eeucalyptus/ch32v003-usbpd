#include "ch32v003fun.h"
#include "pd_phy.h"

global pd_bit_lut

#define CFGLR_OFFSET 0
#define INDR_OFFSET 8
#define BSHR_OFFSET 16

#define SYSTICK_CNT_OFFSET 0x8

.section .text.vector_handler
.global EXTI7_0_IRQHandler
.balign 4
EXTI7_0_IRQHandler:
    addi sp,sp,-80
	sw a0, 0(sp)
	sw a1, 4(sp)
	sw a2, 8(sp)
	sw a3, 12(sp)
	sw a4, 16(sp)
	sw a5, 20(sp)
    sw t0, 24(sp)
    sw t1, 28(sp)
    sw t2, 32(sp)

    // initialize state
    li a0, 0b110 // shifty
    li a1, 0 // bit count
    li a2, 0 // bits
    li a3, data_buffer // data ptr  
	la a5, PD_GPIO_BASE // GPIO ptr

    la t0, SysTick
    li a4, SYSTICK_CNT_OFFSET(t0) // timer
    addi a4, a4, -10 // tune timer to start in the past

preamble:
    // wait for sample
    addi a4, a4, PD_TIMER_RX_PERIOD
    la t0, SysTick
sync_loop1:
    lw t1, SYSTICK_CNT_OFFSET(t0)
    blt t1, a4, sync_loop1

    // sample input
    lw t0, INDR_OFFSET(a5)
    c.srli t0, PD_PIN
    c.andi t0, 1
    // shift in sample
    c.slli a0, 1
    c.or a0, t0

    // check bit event from lut
    mv a0, t0
    c.andi t0, 0b111111
    la t1, pd_bit_lut
    add t1, t0
    lb t0, t1

    // cases
    c.bez t0, preamble // NONE
    li t1, PD_BIT_LUT_1
    c.beq t0, t1, preamble_bit1 // 1
    li t1, PD_BIT_LUT_0
    c.beq t0, t1, preamble_bit0 // 0
    li t1, PD_BIT_LUT_INVALID
    c.beq t0, t1, preamble_invalid // INVALID
    
preamble_bit1:
    // check preamble bit
    mv t0, a1
    c.andi t0, 1
    // invalid if unexpected 0
    c.bez t0, preamble_invalid 
    // count up
    addi a1, a1, 1
    // check for full preamble
    li t0, 50
    c.bge t0, a1, preamble_done
    c.j preamble

preamble_bit0:
    // check preamble bit
    mv t0, a1
    c.andi t0, 1
    // invalid if unexpected 1
    c.bnez t0, preamble_invalid 
    // count up
    addi a1, a1, 1
    // no need to check for full preamble (last bit has to be 1)
    c.j preamble


preamble_invalid:
    // end in invalid state
    c.j preamble_invalid

preamble_done:
    // check for SOF
    call read_kcode
    addi a2, a2, -KCODE_SYNC1
    c.bnez a2, sof_invalid
    call read_kcode
    addi a2, a2, -KCODE_SYNC1
    c.bnez a2, sof_invalid
    call read_kcode
    addi a2, a2, -KCODE_SYNC1
    c.bnez a2, sof_invalid
    call read_kcode
    addi a2, a2, -KCODE_SYNC2
    c.bnez a2, sof_invalid

read_data:
    call read_kcode
    mv t0, a2
    addi t0, t0, -KCODE_EOP
    c.beqz t0, read_data_end
    // store data
    mv t0, a2
    call read_kcode
    c.slli t0, 4
    // no need to mask kcode, only 4 bits of data expected
    c.or t0, a2
    sb t0, 0(a3)
    addi a3, a3, 1

read_data_end:
    c.j read_data_end

sof_invalid:
    // end in invalid state
    c.j sof_invalid


// reads a kcode to a2
read_kcode:
    li a2, 0 // bits
read_kcode_loop:
    // wait for sample
    addi a4, a4, PD_TIMER_RX_PERIOD
    la t0, SysTick
sync_loop2:
    lw t1, SYSTICK_CNT_OFFSET(t0)
    blt t1, a4, sync_loop2

    // sample input
    lw t0, INDR_OFFSET(a5)
    c.srli t0, PD_PIN
    c.andi t0, 1
    // shift in sample
    c.slli a0, 1
    c.or a0, t0

    // check bit event from lut
    mv a0, t0
    c.andi t0, 0b111111
    la t1, pd_bit_lut
    add t1, t0
    lb t0, t1

    // cases
    c.bez t0, read_kcode_loop // NONE
    li t1, PD_BIT_LUT_1
    c.beq t0, t1, read_kcode_bit1 // 1
    li t1, PD_BIT_LUT_0
    c.beq t0, t1, read_kcode_bit0 // 0
    li t1, PD_BIT_LUT_INVALID
    c.beq t0, t1, read_kcode_invalid // INVALID

read_kcode_bit1:
    c.slli a2, 1
    c.ori a2, 1
    c.j read_kcode_bit
read_kcode_bit0:
    c.slli a2, 1
    c.ori a2, 0
read_kcode_bit:
    // count up
    addi a1, a1, 1
    // check for full preamble
    li t0, 5
    c.bge t0, a1, read_kcode_end
    c.j read_kcode_loop

read_kcode_invalid:
    // end in invalid state
    c.j read_kcode_invalid

read_kcode_end:
    // c.andi a2, 0b11111 // no need to mask, only 5 bits
    la t0, pd_kcode_lut
    add t0, a2
    lb a2, t0
    ret