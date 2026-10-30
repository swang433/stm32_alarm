#include "config.h"
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

extern volatile uint8_t found_addr;

int main(){
    buzzer_init(); 
    song(); 
    buzzer_stop(); 

    i2c_init();
    i2c_scan();
    if (!found_addr)
        while (1);              // no OLED found, stop here

    oled_init();
    oled_fill(0x00);
    rtc_init(12, 0, 0);         // placeholder start time until set-time mode works

    uint8_t h, m, s, last_s = 0xFF;
    while (1){
        rtc_read(&h, &m, &s);
        if (s != last_s){       // only redraw when the second rolls over
            last_s = s;
            oled_time(h, m, s);
        }
    }
}