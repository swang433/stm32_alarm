    .syntax unified
    .cpu cortex-m3
    .thumb

    .section .isr_vector, "a"
    .word   _estack
    .word   Reset_Handler
    .word   0  /* NMI */
    .word   0  /* HardFault */
    .word   0  /* MemManage */
    .word   0  /* BusFault */
    .word   0  /* UsageFault */
    .word   0, 0, 0, 0
    .word   0  /* SVC */
    .word   0  /* DebugMon */
    .word   0
    .word   0  /* PendSV */
    .word   0  /* SysTick */

    .section .text
    .global Reset_Handler
    .type   Reset_Handler, %function
Reset_Handler:
    /* copy .data from flash to RAM */
    ldr r0, =_sdata
    ldr r1, =_edata
    ldr r2, =_sidata
copy_data:
    cmp r0, r1
    bge zero_bss
    ldr r3, [r2], #4
    str r3, [r0], #4
    b copy_data

zero_bss:
    ldr r0, =_sbss
    ldr r1, =_ebss
    mov r2, #0
clear_bss:
    cmp r0, r1
    bge start_main
    str r2, [r0], #4
    b clear_bss

start_main:
    bl main
    b .
