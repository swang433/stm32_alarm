#include "config_c071.h"
#include <stdint.h>

extern volatile int8_t button_event; 
extern volatile uint8_t current_mode;

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

int main(void){
    return 0; 
}