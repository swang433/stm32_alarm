#include "config.h"
#include <stdint.h>

/* start the RTC from the 32.768 kHz LSE crystal; the time itself is written by rtc_set().
   if the RTC was already initialised (e.g. after a reset-button press), leave it running 
*/
void rtc_init(void){
    RCC_APB1ENR |= (1U << 10);                  // RTC APB clock
    if (RTC_ICSR & (1U << 4)) return;           // INITS: calendar already set

    RCC_CSR1 |= (1U << 0);                      // LSEON
    while (!(RCC_CSR1 & (1U << 1)));            // wait LSERDY
    RCC_CSR1 = (RCC_CSR1 & ~(3U << 8)) | (1U << 8);  // RTCSEL = LSE
    RCC_CSR1 |= (1U << 15);                     // RTCEN
}

void rtc_set(uint8_t h, uint8_t m, uint8_t s){
    /* unlock, enter init mode, write time in BCD, leave init mode, relock.
       PRER reset value (127, 255) already divides 32768 Hz down to 1 Hz */
    RTC_WPR = 0xCA;
    RTC_WPR = 0x53;
    RTC_ICSR |= (1U << 7);                      // INIT
    while (!(RTC_ICSR & (1U << 6)));            // wait INITF
    RTC_TR = ((uint32_t)(h / 10) << 20) | ((uint32_t)(h % 10) << 16)
           | ((uint32_t)(m / 10) << 12) | ((uint32_t)(m % 10) << 8)
           | ((uint32_t)(s / 10) << 4)  |  (uint32_t)(s % 10);
    RTC_ICSR &= ~(1U << 7);
    RTC_WPR = 0xFF;
}

void rtc_read(uint8_t *h, uint8_t *m, uint8_t *s){
    uint32_t tr = RTC_TR;
    (void)RTC_DR;                               // unlock shadow registers
    *h = ((tr >> 20) & 0x3) * 10 + ((tr >> 16) & 0xF);
    *m = ((tr >> 12) & 0x7) * 10 + ((tr >> 8)  & 0xF);
    *s = ((tr >> 4)  & 0x7) * 10 + ( tr        & 0xF);
}
