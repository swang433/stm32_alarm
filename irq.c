#include "config.h" 
extern struct Curr_Time current_time; 

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

void RTC_IRQHandler(void){ //time tick interrupt
    if (RTC_CRL & RTC_CRL_SECF){
        uint32_t CNT_HIGH = (uint32_t)(RTC_CNTH & 0xFFFF) << 16;
        uint32_t CNT_LOW = (uint32_t)(RTC_CNTL & 0xFFFF);
        uint32_t cnt = CNT_HIGH | CNT_LOW;  

        uint32_t day_secs = cnt % 86400; 
        current_time.hour = day_secs / 3600;
        current_time.minute = (day_secs % 3600) / 60; 
        current_time.second = day_secs % 60; 
        
        time_changed = 1; //interrupt raises the time change flag

        while(!(RTC_CRL & RTC_CRL_RTOFF)){}

        /*
        enter config mode
        set clear second flag
        exit config mode
        */
        RTC_CRL |= RTC_CRL_CNF; 
        RTC_CRL &= ~(RTC_CRL_SECF);
        RTC_CRL &= ~(RTC_CRL_CNF);  
        while(!(RTC_CRL & RTC_CRL_RTOFF)){}
    }

    if (RTC_CRL & RTC_CRL_ALRF){
        /*
        raise global alarm flag variable
        enter config mode
        set alarm second flag
        exit config mode
        */
        alarm_sound = 1; 
        while(!(RTC_CRL & RTC_CRL_RTOFF)) {}
        RTC_CRL |= RTC_CRL_CNF; 
        RTC_CRL &= ~(RTC_CRL_ALRF); 
        RTC_CRL &= ~(RTC_CRL_CNF); 
        while(!(RTC_CRL & RTC_CRL_RTOFF)) {}
    }
}