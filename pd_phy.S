#include "ch32v003fun.h"
#include "pd_phy.h"

#define CFGLR_OFFSET 0
#define INDR_OFFSET 8
#define OUTDR_OFFSET 12
#define BSHR_OFFSET 16

//#define SysTick_BASE (0xE000F000)
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
	sw a5, 20(sp) // work register
    sw t0, 24(sp)
    sw t1, 28(sp)
    sw t2, 32(sp)

retry_recv_packet:
    // initialize state
    li a0, 0b110 // shifty
    li a1, 0 // bit count
    li a2, 0 // bits
    
    lui a3, %hi(data_buffer) // data ptr  
    addi a3, a3, %lo(data_buffer)

wait_for_low:
	la a5, PD_GPIO_BASE // GPIO ptr
    lw a5, INDR_OFFSET(a5)
    c.srli a5, PD_PIN
    c.andi a5, 1
    c.beqz a5, wait_for_low

    la t0, SysTick_BASE
    lw a4, SYSTICK_CNT_OFFSET(t0) // timer
    //addi a4, a4, -10 // tune timer to start in the past

preamble:
    // wait for sample
    addi a4, a4, PD_TIMER_RX_PERIOD
    la t0, SysTick_BASE
sync_loop1:
    lw t1, SYSTICK_CNT_OFFSET(t0)
    blt t1, a4, sync_loop1

    // sample input
	la a5, PD_GPIO_BASE // GPIO ptr
    lw a5, INDR_OFFSET(a5)
    c.srli a5, PD_PIN
    c.andi a5, 1
    // shift in sample
    c.slli a0, 1
    c.or a0, a5

    // check bit event from lut
    li a5, 0b111111
    c.and a5, a0
    la t1, pd_bit_lut
    c.add t1, a5
    lb a5, (t1)

    // cases
    c.beqz a5, preamble // NONE
    li t1, PD_BIT_LUT_INVALID
    c.beqz a5, preamble_invalid // INVALID
    li t1, PD_BIT_LUT_0
    c.beqz a5, preamble_bit0 // 0
    
preamble_bit1:
    // check preamble bit
    mv a5, a1
    c.andi a5, 1
    // invalid if unexpected 0
    c.beqz a5, preamble_invalid 
    // count up
    addi a1, a1, 1
    // check for full preamble
    li a5, 50 // TODO is probably 49 instead because of bgt, or use bge instead
    bgt a5, a1, preamble_done
    c.j preamble

preamble_bit0:
    // check preamble bit
    mv a5, a1
    c.andi a5, 1
    // invalid if unexpected 1
    c.bnez a5, preamble_invalid 
    // count up
    addi a1, a1, 1
    // no need to check for full preamble (last bit has to be 1)
    c.j preamble


preamble_invalid:
    // end in invalid state
    li a5, 0b111111
    c.and a5, a0
    bnez a5, bad_preamble
    bnez a1, bad_preamble
    c.j retry_recv_packet
bad_preamble:
    // return bad_preamble 
    c.j bad_preamble


preamble_done:
    // check for SOF
    call recv_kcode
    c.addi a2, -PD_KCODE_SYNC1
    c.bnez a2, sof_invalid
    call recv_kcode
    c.addi a2, -PD_KCODE_SYNC1
    c.bnez a2, sof_invalid
    call recv_kcode
    c.addi a2, -PD_KCODE_SYNC1
    c.bnez a2, sof_invalid
    call recv_kcode
    c.addi a2, -PD_KCODE_SYNC2
    c.bnez a2, sof_invalid

recv_data:
    call recv_kcode
    mv a5, a2
    c.addi a5, -PD_KCODE_EOP
    c.beqz a5, recv_data_end
    // store data
    mv a5, a2
    call recv_kcode
    c.slli a5, 4
    // no need to mask kcode, only 4 bits of data expected
    c.or a5, a2
    sb a5, (a3)
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
    lw a0, (sp)
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
    la t0, SysTick_BASE
sync_loop2:
    lw t1, SYSTICK_CNT_OFFSET(t0)
    blt t1, a4, sync_loop2

    // sample input
    lw a5, INDR_OFFSET(a5)
    c.srli a5, PD_PIN
    c.andi a5, 1
    // shift in sample 0(
    c.slli a0, 1
    c.or a0, a5

    // check bit event from lut
    li a5, 0b111111
    c.and a5, a0
    la t1, pd_bit_lut
    c.add t1, a5
    lb a5, (t1)

    // cases
    c.beqz a5, recv_kcode_loop // NONE
    li t1, PD_BIT_LUT_INVALID
    c.beqz a5, recv_kcode_invalid // INVALID
    li t1, PD_BIT_LUT_0
    c.beqz a5, recv_kcode_bit0 // 0

recv_kcode_bit1:
    c.slli a2, 1
    c.addi a2, 1
    c.j recv_kcode_bit
recv_kcode_bit0:
    c.slli a2, 1
recv_kcode_bit:
    // count up
    addi a1, a1, 1
    // check for full preamble
    li t0, 5
    bge t0, a1, recv_kcode_end
    c.j recv_kcode_loop

recv_kcode_invalid:
    // end in invalid state
    c.j recv_kcode_invalid

recv_kcode_end:
    // c.andi a2, 0b11111 // no need to mask, only 5 bits
    la t0, pd_kcode_lut
    c.add t0, a2
    lb a2, (t0)
    ret

.global send_packet
.balign 4
send_packet:
    addi sp,sp,-80
	sw a0, 0(sp) // message byte array
	sw a1, 4(sp) // message length
	sw a2, 8(sp)
	sw a3, 12(sp)
	sw a4, 16(sp)
	sw a5, 20(sp) // work register
    sw t0, 24(sp)
    sw t1, 28(sp)
    sw t2, 32(sp)
    sw ra, 36(sp) 

    // initialize state
    //li a0, 0 // bit to send
    //li a1, 0 // kcode to send
    la a3, PD_OH_GPIO_BASE // GPIO ptr  

    la t0, SysTick_BASE
    lw a4, SYSTICK_CNT_OFFSET(t0) // timer
    sw a4, 40(sp)// push timer to stack
    //addi a4, a4, -10 // tune timer to start in the past

send_preamble:
    li a5, 50
send_preamble_loop:
    sw a5, 44(sp)
    andi a0, a5, 1
send_preamble_bit:
    // wait for sample
    lw a4, 40(sp)
    addi a4, a4, PD_TIMER_TX_PERIOD
    sw a4, 40(sp)
    la t0, SysTick_BASE
preamble_bit_sync1:
    lw a5, SYSTICK_CNT_OFFSET(t0)
    blt a5, a4, preamble_bit_sync1

    // first half of bit
    li a4, 1
    c.slli a4, PD_OH_PIN
    lw a5, OUTDR_OFFSET(a3)
    c.xor a5, a4
    sw a5, OUTDR_OFFSET(a3)

    // wait for sample
    lw a4, 40(sp)
    addi a4, a4, PD_TIMER_TX_PERIOD
    sw a4, 40(sp)
    la t0, SysTick_BASE
preamble_bit_sync2:
    lw a5, SYSTICK_CNT_OFFSET(t0)
    blt a5, a4, preamble_bit_sync2

    // second half of bit
preamble_bit_second_half:
    c.beqz a0, preamble_bit_skip_second_transistion
    // transition for 1
    li a4, 1
    c.slli a4, PD_OH_PIN
    lw a5, OUTDR_OFFSET(a3)
    c.xor a5, a4
    sw a5, OUTDR_OFFSET(a3)
preamble_bit_skip_second_transistion:

    lw a5, 44(sp)
    c.addi a5, -1
    c.bnez a5, send_preamble_loop

    // send SOF
    li a1, PD_KCODE_SYNC1
    call send_kcode
    li a1, PD_KCODE_SYNC1
    call send_kcode
    li a1, PD_KCODE_SYNC1
    call send_kcode
    li a1, PD_KCODE_SYNC2
    call send_kcode


    lw a0, 0(sp) // message byte array
    lw a1, 4(sp) // message length
send_data_loop:
    c.beqz a1, send_data_end
    // send first nibble
    //li a1, 0xBA
    lw a0, 0(sp) // message byte array
    lb a1, (a0)
    c.andi a1, 0b1111 
    call send_kcode
    
    // send second nibble
    //li a1, 0xBA
    lw a0, 0(sp) // message byte array
    lb a1, (a0)
    c.srli a1, 4
    c.andi a1, 0b1111
    call send_kcode

    // loop over bytes
    lw a0, 0(sp) // message byte array
    lw a1, 4(sp) // message length
    c.addi a0, 1
    c.addi a1, -1
    sw a0, 0(sp)
    sw a1, 4(sp) 
    c.j send_data_loop

send_data_end:
    // EOP
    li a1, PD_KCODE_EOP
    call send_kcode

    // wait for sample
    lw a4, 40(sp)
    addi a4, a4, PD_TIMER_TX_PERIOD
    sw a4, 40(sp)
    la t0, SysTick_BASE
eop_bit_sync1:
    lw a5, SYSTICK_CNT_OFFSET(t0)
    blt a5, a4, eop_bit_sync1

    // toggle pin once more
    li a4, 1
    c.slli a4, PD_OH_PIN
    lw a5, OUTDR_OFFSET(a3)
    c.xor a5, a4
    sw a5, OUTDR_OFFSET(a3)
    nop
    nop
    nop
    nop
    nop
    nop
    c.or a5, a4
    sw a5, OUTDR_OFFSET(a3)

    // set high
    li a4, 1
    c.slli a4, PD_OH_PIN
    lw a5, OUTDR_OFFSET(a3)
    c.or a5, a4
    sw a5, OUTDR_OFFSET(a3)

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
    c.add t0, a1
    lb a1, (t0)

    // send 5 bits
    li a5, 4
send_kcode_loop:
    sw a5, 44(sp)
    mv a0, a1
    c.andi a0, 1
    c.srli a1, 1
send_kcode_bit:
    // wait for sample
    lw a4, 40(sp)
    addi a4, a4, PD_TIMER_TX_PERIOD
    sw a4, 40(sp)
    la t0, SysTick_BASE
kcode_bit_sync1:
    lw a5, SYSTICK_CNT_OFFSET(t0)
    blt a5, a4, kcode_bit_sync1

    // first half of bit
    li a4, 1
    c.slli a4, PD_OH_PIN
    lw a5, OUTDR_OFFSET(a3)
    c.xor a5, a4
    sw a5, OUTDR_OFFSET(a3)

    // wait for sample
    lw a4, 40(sp)
    addi a4, a4, PD_TIMER_TX_PERIOD
    sw a4, 40(sp)
    la t0, SysTick_BASE
kcode_bit_sync2:
    lw a5, SYSTICK_CNT_OFFSET(t0)
    blt a5, a4, kcode_bit_sync2

    // second half of bit
kcode_bit_second_half:
    c.beqz a0, kcode_bit_skip_second_transistion
    // transition for 1
    li a4, 1
    c.slli a4, PD_OH_PIN
    lw a5, OUTDR_OFFSET(a3)
    c.xor a5, a4
    sw a5, OUTDR_OFFSET(a3)
kcode_bit_skip_second_transistion:

    lw a5, 44(sp)
    c.addi a5, -1
    bgez a5, send_kcode_loop
    ret

; // sends bit in a0
; send_bit:
;     // first half of bit
;     li a4, 1
;     c.slli a4, PD_OH_PIN
;     lw a5, OUTDR_OFFSET(a3)
;     c.xor a5, a4
;     sw a5, OUTDR_OFFSET(a3)

;     // wait for sample
;     lw a4, 40(sp)
;     addi a4, a4, PD_TIMER_TX_PERIOD
;     sw a4, 40(sp)
;     la t0, SysTick_BASE
; sync_loop3:
;     lw a5, SYSTICK_CNT_OFFSET(t0)
;     blt a5, a4, sync_loop3

;     // second half of bit
; second_half:
;     c.beqz a0, skip_second_transistion
;     // transition for 1
;     li a4, 1
;     c.slli a4, PD_OH_PIN
;     lw a5, OUTDR_OFFSET(a3)
;     c.xor a5, a4
;     sw a5, OUTDR_OFFSET(a3)
; skip_second_transistion:
;     // wait for sample
;     lw a4, 40(sp)
;     addi a4, a4, PD_TIMER_TX_PERIOD
;     sw a4, 40(sp)
;     la t0, SysTick_BASE
; sync_loop4:
;     lw a5, SYSTICK_CNT_OFFSET(t0)
;     blt a5, a4, sync_loop4
;     ret


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