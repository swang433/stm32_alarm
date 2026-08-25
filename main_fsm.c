#include "config_c071.h"
#include <stdint.h>

/*global event flags*/
volatile uint8_t current_mode = MODE_CLOCK;
volatile int8_t button_event = BUTTON_NONE;
volatile uint8_t time_changed = 0;
volatile uint8_t alarm_sound = 0;

void press(int8_t button_event, uint8_t fsm_mode){
    switch (fsm_mode){
        case MODE_CLOCK:
            /* logic */
            break;
        
        case MODE_SET_ALARM: 
            /* logic */
            break;
        
        case MODE_SET_TIME:
            /* logic */
            break; 
    }
}