#include "config_c071.h" 
extern struct Curr_Time current_time; 

void delay(int cycles){
    for (volatile int i = 0; i < cycles; i++){
        __asm__("nop"); 
    }
}

/*button-driven interrupt functions*/
/* C071: EXTI0+1 share one handler */
void EXTI0_1_IRQHandler(void){
    if (EXTI_RPR1 & (1U << 0)){
        delay(8000 * DEBOUNCE_MS);
        if (!(GPIOA_IDR & (1U << 0)))
            button_event = BUTTON_INCREMENT;
        EXTI_RPR1 |= (1U << 0);
    }
    if (EXTI_RPR1 & (1U << 1)){
        delay(8000 * DEBOUNCE_MS);
        if (!(GPIOA_IDR & (1U << 1)))
            button_event = BUTTON_DECREMENT;
        EXTI_RPR1 |= (1U << 1);
    }
}

/* C071: EXTI2+3 share one handler — PA3 = BUTTON_ALARM_SET */
void EXTI2_3_IRQHandler(void){
    if (EXTI_RPR1 & (1U << 3)){
        delay(8000 * DEBOUNCE_MS);
        if (!(GPIOA_IDR & (1U << 3)))
            button_event = BUTTON_ALARM_SET;
        EXTI_RPR1 |= (1U << 3);
    }
}

/* C071: EXTI4-15 share one handler — PA4 = BUTTON_SNOOZE */
void EXTI4_15_IRQHandler(void){
    if (EXTI_RPR1 & (1U << 4)){
        delay(8000 * DEBOUNCE_MS);
        if (!(GPIOA_IDR & (1U << 4)))
            button_event = BUTTON_SNOOZE;
        EXTI_RPR1 |= (1U << 4);
    }
}

/* TODO: C071 RTC is different from F103 — no CNTH/CNTL/CRL registers.
   C071 RTC uses TR (time), DR (date), ISR/ICSR for status, and alarm via ALRMAR.
   Rewrite once I2C scan and GPIO are confirmed working. */
void RTC_IRQHandler(void){
}