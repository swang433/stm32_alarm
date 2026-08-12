#include "config.h"

/*global event flags*/
volatile uint8_t current_mode = MODE_CLOCK;
volatile int8_t button_event = BUTTON_NONE; //which button pressed

void delay(int cycles){
    for (volatile int i = 0; i < cycles; i++){
        __asm__("nop"); 
    }
}

/*button-driven interrupt functions*/
void EXTI0_IRQHandler(void){
    if (EXTI_PR & (1U << 0)){
        delay(8000 * DEBOUNCE_MS);
        if (!(GPIOA_IDR & (1U << 0))){ //detect when input is pulled low
            button_event = BUTTON_INCREMENT; 
        } 
        EXTI_PR |= (1U << 0); 
    }
}

void EXTI1_IRQHandler(void){
    if (EXTI_PR & (1U << 1)){
        delay(8000 * DEBOUNCE_MS);
        if (!(GPIOA_IDR & (1U << 1))){ //detect when input is pulled low
            button_event = BUTTON_DECREMENT; 
        } 
        EXTI_PR |= (1U << 1); 
    }
}

void EXTI2_IRQHandler(void){
    if (EXTI_PR & (1U << 2)){
        delay(8000 * DEBOUNCE_MS);
        if (!(GPIOA_IDR & (1U << 2))){ //detect when input is pulled low
            button_event = BUTTON_ALARM_SET; 
        } 
        EXTI_PR |= (1U << 2); 
    }
}

void EXTI3_IRQHandler(void){
    if (EXTI_PR & (1U << 3)){
        delay(8000 * DEBOUNCE_MS);
        if (!(GPIOA_IDR & (1U << 3))){ //detect when input is pulled low
            button_event = BUTTON_SNOOZE; 
        } 
        EXTI_PR |= (1U << 3); 
    }
}