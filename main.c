#include "config.h"

/*global event flags*/
volatile fsm_mode_t current_mode = MODE_CLOCK;
volatile button_t button_event = BUTTON_NONE;

/*time variables*/
static uint8_t _hour, _min;          // time being edited in SET_TIME / SET_ALARM
static uint8_t alarm_hour = 7, alarm_min = 0;
static uint8_t alarm_count; 
Time alarms[MAX_ALARMS]; 

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

void press(button_t button_pressed, fsm_mode_t fsm_mode){
    switch (fsm_mode){
        case MODE_CLOCK:{
            /*
            load the edit values once on entry, not on every press
                set:    edit the current RTC time
                snooze: edit the current alarm time
            the alarm check itself lives in main(), since press() only runs on button events
            */
            if (button_pressed == BUTTON_SET){
                uint8_t s;
                rtc_read(&_hour, &_min, &s);
                current_mode = MODE_SET_TIME;
            }
            else if (button_pressed == BUTTON_SNOOZE){
                _hour = alarm_hour; _min = alarm_min;
                current_mode = MODE_SET_ALARM;
            }
            break;
        }
        case MODE_SET_TIME:
        case MODE_SET_ALARM:
            /*
            both set modes edit the same h:m pair
                up/down step one minute, carrying into hours like an odometer
                hours wrap 23 <-> 0; +23 instead of -1 avoids uint8_t underflow
                set confirms: write the RTC or the alarm, then back to clock
            */
            if (button_pressed == BUTTON_UP){
                if (_min == 59){
                    _min = 0;
                    _hour = (_hour + 1) % 24;
                }
                else{
                    _min++;
                }
            }
            else if (button_pressed == BUTTON_DOWN){
                if (_min == 0){
                    _min = 59;
                    _hour = (_hour + 23) % 24;
                }
                else{
                    _min--;
                }
            }
            else if (button_pressed == BUTTON_SET){
                if (fsm_mode == MODE_SET_TIME)
                    rtc_set(_hour, _min, 0);
                else if (alarm_count >= MAX_ALARMS){
                    no_addr_sound(); 
                }
                else{
                    alarm_hour = _hour; alarm_min = _min;
                    Time new_alarm = {_hour, _min, 0};

                    if (same_time(alarms, &new_alarm, alarm_count)){
                        duplicate_alarms_sound();
                    }
                    else{
                        /* field-by-field: a whole-struct copy makes gcc call memcpy, which -nostdlib lacks */
                        alarms[alarm_count] = new_alarm; 
                        alarm_count++; 
                    }
                }
                current_mode = MODE_CLOCK; 
                }
            break;
        
        case MODE_RING:
            /* the tune is played by main() while in this mode; here we only listen for the dismiss */
            if (button_pressed == BUTTON_SNOOZE){
                buzzer_stop(); 
                current_mode = MODE_CLOCK;
            }
            break;
    }
}

bool same_time(const Time* arr, const Time* clock_time, int sz){
    for (int i = 0; i < sz; i++){
        if (arr[i].hour == clock_time->hour && arr[i].minute == clock_time->minute){
            return true; 
        }
    }
    return false; 
}

extern volatile uint8_t found_addr;

int main(){
    but_init(); 
    buzzer_init();  
    i2c_init();
    i2c_scan();
    if (!found_addr){
        while (1){
            no_addr_sound(); // warning sound when no I2C address is found 
        }
    }              

    oled_init();
    oled_fill(0x00);
    rtc_init();        // placeholder start time until set-time mode works

    uint8_t h, m, s, last_s = 0xFF;
    bool rung = false;                  // true while the current matching minute has already rung
    while (1){
        if (button_event != BUTTON_NONE){
            button_t curr_event = button_event;
            button_event = BUTTON_NONE;
            press(curr_event, current_mode);
        }

        if (current_mode == MODE_CLOCK){
            rtc_read(&h, &m, &s);
            if (s != last_s){           // only redraw when the second rolls over
                last_s = s;
                oled_time(h, m, s);
            }

            /* checked after press(), so a press that left clock mode this pass is never overridden */
            Time now = {h, m, s};
            if (!same_time(alarms, &now, alarm_count)){
                rung = false;           // minute moved on: re-arm for the next match
            }
            else if (!rung){
                rung = true;            // ring once per matching minute, even after a dismiss
                current_mode = MODE_RING;
            }
        }
        else if (current_mode == MODE_RING){
            wakey_wakey();              // blocks for the length of the tune; a press during it is held by the ISR
        }
    }
}