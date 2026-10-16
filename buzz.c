#include "config_c071.h"
#include <stdint.h>

#define BOARD_CLOCK       12000000U
#define GPIOA_PA7         7U
#define PRESCALE_FACTOR   0x0BU                             /* 12 MHz / (11 + 1) = 1 MHz tick */
#define TIMER_TICK_HZ     (BOARD_CLOCK / (PRESCALE_FACTOR + 1U))
#define AUTO_RELOAD_INIT  0x08E0U                           /* 440 Hz at a 1 MHz tick */

void buzzer_init(void);
void buzzer_sound(uint32_t freq);
void buzzer_stop(void);

void buzzer_init(void){
    /*
    enable GPIOA and TIM3 clocks
    PA7: clear mode bits, set alternate function (0b10)
    PA7: clear alternate function bits 28-31, set AF1 (0b0001, TIM3_CH2)
    set tim3 prescaler and auto reload
    set initial duty in tim3_ccr2; duty cycle = auto reload value / 2
    CH2: clear mode and preload bits, set PWM mode 1 (0b110) and enable preload
    load prescaler and auto reload through the event generation register
    enable auto reload preload (ARPE)
    output and counter stay off until buzzer_sound() is called
    */
    RCC_IOPENR  |=  (1U << 0);
    RCC_APB1ENR |=  (1U << 1);
    GPIOA_MODER &= ~(3U   << 2 * GPIOA_PA7);
    GPIOA_MODER |=  (2U   << 2 * GPIOA_PA7);
    GPIOA_AFR   &= ~(0xFU << 4 * GPIOA_PA7);
    GPIOA_AFR   |=  (1U   << 4 * GPIOA_PA7);
    TIM3_PSC    =   PRESCALE_FACTOR;
    TIM3_ARR    =   AUTO_RELOAD_INIT;
    TIM3_CCR2   =   TIM3_ARR >> 1;
    TIM3_CCMR1  &= ~((7U << 12) | (1U << 11));
    TIM3_CCMR1  |=  (6U << 12) | (1U << 11);
    TIM3_EGR    =   (1U << 0);
    TIM3_CR1    |=  (1U << 7);
}

void buzzer_sound(uint32_t freq){
    if (freq == 0){
        buzzer_stop();
        return;
    }
    /*
    auto reload = timer tick rate / frequency - 1
    duty cycle  = (auto reload + 1) / 2, a 50% square wave
    */
    TIM3_ARR    =   TIMER_TICK_HZ / freq - 1;
    TIM3_CCR2   =   (TIM3_ARR + 1) / 2;
    TIM3_CCER   |=  (1U << 4);
    TIM3_CR1    |=  (1U << 0);
}

void buzzer_stop(void){
    /* disable the channel 2 output and stop the counter */
    TIM3_CCER   &= ~(1U << 4);
    TIM3_CR1    &= ~(1U << 0);
}
