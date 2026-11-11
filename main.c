#include "config.h"
#include <stdint.h>

/*global event flags*/
volatile uint8_t current_mode = MODE_CLOCK;
volatile int8_t button_event = BUTTON_NONE;
volatile uint8_t time_changed = 0;
volatile uint8_t alarm_sound = 0;

static uint8_t edit_h, edit_m;          // time being edited in SET_TIME / SET_ALARM
static uint8_t alarm_h = 7, alarm_m = 0;
#define EXTI_MASK ((1 << 0) | (1 << 1) | (1 << 3) | (1 << 4))

void but_init(){
    /*
    TODO: 
    initialize GPIO ports A and B 
        - enable GPIO on ports A and B
        - set the button pins as inputs
        - set pins PA0, PA1, PA4, PB3 to input pullup
    
    initialize external interrupt lines 0, 1, 3, 4
        - route EXTI line 3 to port B in EXTICR (lines 0, 1, 4 default to port A)
        - listen to falling edges on EXTI lines 0, 1, 3, and 4 (press pulls the pin low)
        - unmask lines 0, 1, 3, 4 in IMR1
        - clear any pending flags in FPR1 left over from setup

    enable the interrupts in the NVIC last
        - EXTI0_1 (IRQ 5), EXTI2_3 (IRQ 6), EXTI4_15 (IRQ 7) in NVIC_ISER
    */

    // GPIO
    RCC_IOPENR  |=  (1 << 0) | (1 << 1);
    GPIOA_MODER &= ~((3 << 0) | (3 << 2) | (3 << 8));      GPIOB_MODER &= ~(3 << 6); 
    GPIOA_PUPDR |=  (1 << 0) | (1 << 2 * 1) | (1 << 2 * 4); GPIOB_PUPDR |= (1 << 2 * 3);  

    // EXTI
    EXTI_EXTICR0 |= (1U << 24); // route EXTI line 3 to port B
    EXTI_FTSR1 |=  EXTI_MASK; 
    EXTI_IMR1  |=  EXTI_MASK; 
    EXTI_FPR1  |=  EXTI_MASK; 

    // NVIC 
    NVIC_ISER |= (1U << 5) | (1U << 6) | (1U << 7);
}

void press(int8_t button_pressed, uint8_t fsm_mode){
    switch (fsm_mode){
        case MODE_CLOCK:
            /*
            load the edit values once on entry, not on every press
                set:    edit the current RTC time
                snooze: edit the current alarm time
            */
            if (button_pressed == BUTTON_SET){
                uint8_t s;
                rtc_read(&edit_h, &edit_m, &s);
                current_mode = MODE_SET_TIME;
            }
            else if (button_pressed == BUTTON_SNOOZE){
                edit_h = alarm_h; edit_m = alarm_m;
                current_mode = MODE_SET_ALARM;
            }
            break;

        case MODE_SET_TIME:
        case MODE_SET_ALARM:
            /*
            both set modes edit the same h:m pair
                up/down step one minute, carrying into hours like an odometer
                hours wrap 23 <-> 0; +23 instead of -1 avoids uint8_t underflow
                set confirms: write the RTC or the alarm, then back to clock
            */
            if (button_pressed == BUTTON_UP){
                if (edit_m == 59){
                    edit_m = 0;
                    edit_h = (edit_h + 1) % 24;
                }
                else{
                    edit_m++;
                }
            }
            else if (button_pressed == BUTTON_DOWN){
                if (edit_m == 0){
                    edit_m = 59;
                    edit_h = (edit_h + 23) % 24;
                }
                else{
                    edit_m--;
                }
            }
            else if (button_pressed == BUTTON_SET){
                if (fsm_mode == MODE_SET_TIME)
                    rtc_set(edit_h, edit_m, 0);
                else{
                    alarm_h = edit_h; alarm_m = edit_m;
                }
                current_mode = MODE_CLOCK;
            }
            break;
        
        case MODE_RING:
            error_sound(); 
            if (button_pressed == BUTTON_SNOOZE){
                buzzer_stop(); 
                current_mode = MODE_CLOCK;
            }
    }
}

extern volatile uint8_t found_addr;

int main(){
    but_init(); 
    buzzer_init();  
    i2c_init();
    i2c_scan();
    if (!found_addr){
        while (1){
            error_sound(); // warning sound when no I2C address is found 
        }
    }              

    oled_init();
    oled_fill(0x00);
    rtc_init(); 
    rtc_set(13, 10, 20);         // placeholder start time until set-time mode works

    uint8_t h, m, s, last_s = 0xFF;
    while (1){
        rtc_read(&h, &m, &s);
        if (s != last_s){       // only redraw when the second rolls over
            last_s = s;
            oled_time(h, m, s);
        }
    }
}