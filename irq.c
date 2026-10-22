#include "config.h" 
extern struct Curr_Time current_time; 

/*button-driven interrupt functions*/
/* C071: EXTI0+1 share one handler */
void EXTI0_1_IRQHandler(void){
    if (EXTI_FPR1 & (1U << 0)){
        delay(8000 * DEBOUNCE_MS);
        if (!(GPIOA_IDR & (1U << 0)))
            button_event = BUTTON_INCREMENT;
        EXTI_FPR1 |= (1U << 0);
    }
    if (EXTI_FPR1 & (1U << 1)){
        delay(8000 * DEBOUNCE_MS);
        if (!(GPIOA_IDR & (1U << 1)))
            button_event = BUTTON_DECREMENT;
        EXTI_FPR1 |= (1U << 1);
    }
}

/* C071: EXTI2+3 share one handler — PB3 = BUTTON_ALARM_SET */
void EXTI2_3_IRQHandler(void){
    if (EXTI_FPR1 & (1U << 3)){
        delay(8000 * DEBOUNCE_MS);
        if (!(GPIOB_IDR & (1U << 3)))
            button_event = BUTTON_ALARM_SET;
        EXTI_FPR1 |= (1U << 3);
    }
}

/* C071: EXTI4-15 share one handler — PA4 = BUTTON_SNOOZE */
void EXTI4_15_IRQHandler(void){
    if (EXTI_FPR1 & (1U << 4)){
        delay(8000 * DEBOUNCE_MS);
        if (!(GPIOA_IDR & (1U << 4)))
            button_event = BUTTON_SNOOZE;
        EXTI_FPR1 |= (1U << 4);
    }
}

void RTC_IRQHandler(void){
    //TODO: implement real time clock ISR
}