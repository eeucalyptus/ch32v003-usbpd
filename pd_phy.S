#include "ch32v003fun.h"
#include "pd_phy.h"

global pd_bit_lut

#define CFGLR_OFFSET 0
#define INDR_OFFSET 8
#define OUTDR_OFFSET 12
#define BSHR_OFFSET 16

#define SYSTICK_CNT_OFFSET 0x8

.section .text.vector_handler
.global EXTI7_0_IRQHandler
.balign 4
EXTI7_0_IRQHandler:
recv_packet:
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
    //addi a4, a4, -10 // tune timer to start in the past

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
    call recv_kcode
    addi a2, a2, -KCODE_SYNC1
    c.bnez a2, sof_invalid
    call recv_kcode
    addi a2, a2, -KCODE_SYNC1
    c.bnez a2, sof_invalid
    call recv_kcode
    addi a2, a2, -KCODE_SYNC1
    c.bnez a2, sof_invalid
    call recv_kcode
    addi a2, a2, -KCODE_SYNC2
    c.bnez a2, sof_invalid

recv_data:
    call recv_kcode
    mv t0, a2
    addi t0, t0, -KCODE_EOP
    c.beqz t0, recv_data_end
    // store data
    mv t0, a2
    call recv_kcode
    c.slli t0, 4
    // no need to mask kcode, only 4 bits of data expected
    c.or t0, a2
    sb t0, 0(a3)
    addi a3, a3, 1

recv_data_end:
    // restore state
    lw t2, 32(sp)
    lw t1, 28(sp)
    lw t0, 24(sp)
    lw a5, 20(sp)
    lw a4, 16(sp)
    lw a3, 12(sp)
    lw a2, 8(sp)
    lw a1, 4(sp)
    lw a0, 0(sp)
    addi sp,sp,80
    mret


sof_invalid:
    // end in invalid state
    c.j sof_invalid


// recvs a kcode to a2
recv_kcode:
    li a2, 0 // bits
recv_kcode_loop:
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
    c.bez t0, recv_kcode_loop // NONE
    li t1, PD_BIT_LUT_1
    c.beq t0, t1, recv_kcode_bit1 // 1
    li t1, PD_BIT_LUT_0
    c.beq t0, t1, recv_kcode_bit0 // 0
    li t1, PD_BIT_LUT_INVALID
    c.beq t0, t1, recv_kcode_invalid // INVALID

recv_kcode_bit1:
    c.slli a2, 1
    c.ori a2, 1
    c.j recv_kcode_bit
recv_kcode_bit0:
    c.slli a2, 1
    c.ori a2, 0
recv_kcode_bit:
    // count up
    addi a1, a1, 1
    // check for full preamble
    li t0, 5
    c.bge t0, a1, recv_kcode_end
    c.j recv_kcode_loop

recv_kcode_invalid:
    // end in invalid state
    c.j recv_kcode_invalid

recv_kcode_end:
    // c.andi a2, 0b11111 // no need to mask, only 5 bits
    la t0, pd_kcode_lut
    add t0, a2
    lb a2, t0
    ret

send_packet:
    addi sp,sp,-80
	sw a0, 0(sp)
	sw a1, 4(sp)
	//sw a2, 8(sp)
	sw a3, 12(sp)
	sw a4, 16(sp)
	sw a5, 20(sp)
    sw t0, 24(sp)
    sw t1, 28(sp)
    sw t2, 32(sp)
    sw ra, 36(sp) // save ra

    // initialize state
    //li a0, 0 // bit to send
    //li a1, 0 // kcode to send
    li a3, data_buffer // data ptr  
	la a5, PD_GPIO_BASE // GPIO ptr

    la t0, SysTick
    li a4, SYSTICK_CNT_OFFSET(t0) // timer
    //addi a4, a4, -10 // tune timer to start in the past

send_preamble:
    ldi t0, 25
send_preamble_loop:
    // send bit 0
    li a0, 0
    call send_bit
    li a0, 1
    call send_bit
    addi t0, t0, -1
    c.bnez t0, send_preamble_loop

    // send SOF
    li a1, KCODE_SYNC1
    call send_kcode
    li a1, KCODE_SYNC1
    call send_kcode
    li a1, KCODE_SYNC1
    call send_kcode
    li a1, KCODE_SYNC2
    call send_kcode

send_data:
    // TODO implement

    // restore state
    lw ra, 36(sp)
    lw t2, 32(sp)
    lw t1, 28(sp)
    lw t0, 24(sp)
    lw a5, 20(sp)
    lw a4, 16(sp)
    lw a3, 12(sp)
    //lw a2, 8(sp)
    lw a1, 4(sp)
    lw a0, 0(sp)
    addi sp,sp,80
    ret

// send kcode in a1
send_kcode:
    // get 5code for kcode from lut
    la t0, pd_5code_lut
    add t0, a1
    lb a1, t0

    // send 5 bits
    ldi t0, 4
send_kcode_loop:
    mv a0, a1
    c.srli a0, t0
    c.andi a0, 1
    mv t2, ra // send_bit doesn't use t2
    call send_bit
    mv ra, t2
    addi t0, t0, -1
    bge t0, zero, send_kcode_loop
    ret

// sends bit in a0
send_bit:
    // first half of bit
    ldi t0, 1
    c.slli t0, PD_PIN
    lw t1, OUTDR_OFFSET(a5)
    c.xor t1, t0
    sw t1, OUTDR_OFFSET(a5)
    c.xor t1, t0

    // wait for sample
    addi a4, a4, PD_TIMER_TX_PERIOD
    la t0, SysTick
sync_loop3:
    lw t1, SYSTICK_CNT_OFFSET(t0)
    blt t1, a4, sync_loop3

    // second half of bit
    c.beqz a0, skip_second_transistion
    sw t1, OUTDR_OFFSET(a5)

skip_second_transistion:
    // wait for sample
    addi a4, a4, PD_TIMER_TX_PERIOD
    la t0, SysTick
sync_loop4:
    lw t1, SYSTICK_CNT_OFFSET(t0)
    blt t1, a4, sync_loop3


/*
    Notes to myself
    - Receiving ISR is completely translated
        - initialize code and run decoding!
        - Use gdb for quick testing
    - Sending routine partially translated
        - send_data is not implemented
        - should afterwards be tested by sending out data
    - Unchecked:
        - Bit order
        - Nibble-Order
    - Should clean up a lot!
        - remove collisions with c code
*/