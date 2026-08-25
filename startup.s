    .syntax unified
    .cpu cortex-m0plus
    .thumb

    .section .isr_vector, "a"
    .word   _estack
    .word   Reset_Handler
    .word   0           /* NMI */
    .word   0           /* HardFault */
    .word   0           /* Reserved */
    .word   0           /* Reserved */
    .word   0           /* Reserved */
    .word   0, 0, 0, 0  /* Reserved */
    .word   0           /* SVCall */
    .word   0           /* Reserved */
    .word   0           /* Reserved */
    .word   0           /* PendSV */
    .word   0           /* SysTick */
    /* peripheral IRQs */
    .word   0           /* WWDG */
    .word   0           /* PVM_VDDIO2 */
    .word   0           /* RTC */
    .word   0           /* FLASH */
    .word   0           /* RCC_CRS */
    .word   0           /* EXTI0_1 */
    .word   0           /* EXTI2_3 */
    .word   0           /* EXTI4_15 */
    .word   0           /* USB_DRD_FS */
    .word   0           /* DMA1_Ch1 */
    .word   0           /* DMA1_Ch2_3 */
    .word   0           /* DMAMUX_DMA1_Ch4_5 */
    .word   0           /* ADC1 */
    .word   0           /* TIM1_BRK_UP_TRG_COM */
    .word   0           /* TIM1_CC */
    .word   0           /* TIM2 */
    .word   0           /* TIM3 */
    .word   0           /* Reserved */
    .word   0           /* Reserved */
    .word   0           /* TIM14 */
    .word   0           /* Reserved */
    .word   0           /* TIM16 */
    .word   0           /* TIM17 */
    .word   0           /* I2C1 */
    .word   0           /* I2C2 */
    .word   0           /* SPI1 */
    .word   0           /* SPI2 */
    .word   0           /* USART1 */
    .word   0           /* USART2 */

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
    ldr r3, [r2]
    adds r2, r2, #4
    str r3, [r0]
    adds r0, r0, #4
    b copy_data

zero_bss:
    ldr r0, =_sbss
    ldr r1, =_ebss
    movs r2, #0
clear_bss:
    cmp r0, r1
    bge start_main
    str r2, [r0]
    adds r0, r0, #4
    b clear_bss

start_main:
    bl main
    b .
