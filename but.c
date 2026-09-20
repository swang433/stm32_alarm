#include "config_c071.h"
#include <stdint.h>

/*
 * Test: PA0 (A0), PA1 (A1), PA4 (A2), PB3 (D9) buttons each toggle
 * LD1 (PA5) and LD2 (PC9). Buttons are active-low with internal
 * pull-ups; interrupt on falling edge.
 */

static void delay(int cycles) {
    for (volatile int i = 0; i < cycles; i++)
        __asm__("nop");
}

static void toggle_leds(void) {
    GPIOA_ODR ^= (1U << 5);   /* LD1 on PA5 */
    GPIOC_ODR ^= (1U << 9);   /* LD2 on PC9 */
}

static void debounce_and_toggle(volatile uint32_t *idr, uint32_t pin) {
    delay(400000);
    EXTI_FPR1 |= (1U << pin);   /* drop any bounce that re-armed during the delay */
    if (!(*idr & (1U << pin))) /* still low -> real press, not a bounce */
        toggle_leds();
}

/* PA0 (A0), PA1 (A1) */
void EXTI0_1_IRQHandler(void) {
    if (EXTI_FPR1 & (1U << 0)) {
        EXTI_FPR1 |= (1U << 0);
        debounce_and_toggle(&GPIOA_IDR, 0);
    }
    if (EXTI_FPR1 & (1U << 1)) {
        EXTI_FPR1 |= (1U << 1);
        debounce_and_toggle(&GPIOA_IDR, 1);
    }
}

/* PB3 (D9) */
void EXTI2_3_IRQHandler(void) {
    if (EXTI_FPR1 & (1U << 3)) {
        EXTI_FPR1 |= (1U << 3);
        debounce_and_toggle(&GPIOB_IDR, 3);
    }
}

/* PA4 (A2) */
void EXTI4_15_IRQHandler(void) {
    if (EXTI_FPR1 & (1U << 4)) {
        EXTI_FPR1 |= (1U << 4);
        debounce_and_toggle(&GPIOA_IDR, 4);
    }
}

int main(void) {
    /* enable clocks: GPIOA (bit0), GPIOB (bit1), GPIOC (bit2) */
    RCC_IOPENR |= (1U << 0) | (1U << 1) | (1U << 2);

    /* PA5 output (LD1): MODER bits [11:10] = 01 */
    GPIOA_MODER &= ~(3U << 10);
    GPIOA_MODER |=  (1U << 10);

    /* PC9 output (LD2): MODER bits [19:18] = 01 */
    GPIOC_MODER &= ~(3U << 18);
    GPIOC_MODER |=  (1U << 18);

    /* PA0, PA1, PA4 input (MODER = 00) */
    GPIOA_MODER &= ~((3U << 0) | (3U << 2) | (3U << 8));

    /* pull-up on PA0, PA1, PA4: PUPDR bits = 01 */
    GPIOA_PUPDR &= ~((3U << 0) | (3U << 2) | (3U << 8));
    GPIOA_PUPDR |=  ((1U << 0) | (1U << 2) | (1U << 8));

    /* PB3 input (MODER = 00) with pull-up (PUPDR = 01) */
    GPIOB_MODER &= ~(3U << 6);
    GPIOB_PUPDR &= ~(3U << 6);
    GPIOB_PUPDR |=  (1U << 6);

    /* EXTI source: lines 0, 1, 4 -> GPIOA (code 0x0); line 3 -> GPIOB (code 0x1).
       On this EXTI generation each EXTICRx line field is 8 bits wide (3 used),
       at bit offsets 0/8/16/24 for lines 0/1/2/3 -- not the old 4-bit nibble
       layout, so line 3's port-select field lives at bits [26:24]. */
    EXTI_EXTICR0 = (1U << 24);
    EXTI_EXTICR1 = 0;

    /* falling edge trigger on lines 0, 1, 3, 4 (active-low buttons) */
    EXTI_FTSR1 |=  (1U << 0) | (1U << 1) | (1U << 3) | (1U << 4);
    EXTI_RTSR1 &= ~((1U << 0) | (1U << 1) | (1U << 3) | (1U << 4));

    /* unmask lines 0, 1, 3, 4 */
    EXTI_IMR1 |= (1U << 0) | (1U << 1) | (1U << 3) | (1U << 4);

    /* enable NVIC IRQs: EXTI0_1=5, EXTI2_3=6, EXTI4_15=7 */
    volatile uint32_t *NVIC_ISER = (volatile uint32_t *)0xE000E100;
    *NVIC_ISER |= (1U << 5) | (1U << 6) | (1U << 7);

    while (1) {}
}