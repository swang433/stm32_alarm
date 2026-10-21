#include "config_c071.h"
#include <stdint.h> 

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

void note_len(int len){
    delay(len * CPS); 
}

void buzzer_scale(void){
    // call buzzer_init() in main 
    // buzzer_sound() calls
    buzzer_sound(_BUZZ_C6); note_len(EIGHTH);
    buzzer_sound(_BUZZ_D6); note_len(EIGHTH);
    buzzer_sound(_BUZZ_E6); note_len(EIGHTH);
    buzzer_sound(_BUZZ_F6); note_len(EIGHTH);
    buzzer_sound(_BUZZ_G6); note_len(EIGHTH);
    buzzer_sound(_BUZZ_A6); note_len(EIGHTH);
    buzzer_sound(_BUZZ_B6); note_len(EIGHTH);
    buzzer_sound(_BUZZ_C7); note_len(EIGHTH); 
    buzzer_stop();  
}

void play(void){
    buzzer_sound(_BUZZ_BF6); note_len(QUARTER_NOTE); 
    delay(10000);  
    buzzer_sound(_BUZZ_BF6); note_len(EIGHTH);  
    buzzer_sound(_BUZZ_AF6); note_len(EIGHTH);  
    buzzer_sound(_BUZZ_BF6); note_len(QUARTER_NOTE);  
    delay(10000);
    buzzer_sound(_BUZZ_BF6); note_len(EIGHTH);  
    buzzer_sound(_BUZZ_DF7); note_len(EIGHTH);  
    delay(10000);
    buzzer_sound(_BUZZ_DF7); note_len(QUARTER_NOTE);  
    buzzer_sound(_BUZZ_EF7); note_len(EIGHTH);  
    buzzer_sound(_BUZZ_DF7); note_len(EIGHTH);
    buzzer_sound(_BUZZ_BF6); note_len(QUARTER_NOTE);  
    
    // buzzer_sound(_BUZZ_BF6); note_len(QUARTER_NOTE);  
    // buzzer_sound(_BUZZ_BF6); note_len(QUARTER_NOTE);  
    // buzzer_sound(_BUZZ_BF6); note_len(QUARTER_NOTE);  
    // buzzer_sound(_BUZZ_BF6); note_len(QUARTER_NOTE);  
    // buzzer_sound(_BUZZ_BF6); note_len(QUARTER_NOTE);  
    // buzzer_sound(_BUZZ_BF6); note_len(QUARTER_NOTE);  
    // buzzer_sound(_BUZZ_BF6); note_len(QUARTER_NOTE);  
    // buzzer_sound(_BUZZ_BF6); note_len(QUARTER_NOTE);  
    // buzzer_sound(_BUZZ_BF6); note_len(QUARTER_NOTE);  
    // buzzer_sound(_BUZZ_BF6); note_len(QUARTER_NOTE);  
    // buzzer_sound(_BUZZ_BF6); note_len(QUARTER_NOTE);   
}