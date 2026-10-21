#ifndef CONFIG_C071_H
#define CONFIG_C071_H

#include <stdint.h>

/*
ports used: 
PA:
  - buttons (GPIOA)
  - pins: PA0 (A0), PA1 (A1), PA4 (A2)
  - passive piezo buzzer (PWM)
  - pins: PA7 (D11)
PB:
  - button (GPIOB)
  - pin: PB3
  - SSH1106 OLED (I2C peripheral)
  - pins: PB6, PB7 (CN10 pins 35 and 37)
*/

//reset and clock enable registers
#define RCC_IOPENR   (*(volatile uint32_t *)0x40021034)
#define RCC_APB1ENR  (*(volatile uint32_t *)0x4002103C)
#define RCC_APB2ENR  (*(volatile uint32_t *)0x40021040)

/*
GPIOA: 
  - MODER: mode registers
  - PUPDR: pullup/pulldown registers
  - GPIOA_AFR: alternate function register for PWM
*/
#define GPIOA_MODER  (*(volatile uint32_t *)0x50000000)
#define GPIOA_PUPDR  (*(volatile uint32_t *)0x5000000C)
#define GPIOA_IDR    (*(volatile uint32_t *)0x50000010)
#define GPIOA_ODR    (*(volatile uint32_t *)0x50000014)
#define GPIOA_AFR    (*(volatile uint32_t *)0x50000020)

/*
GPIOB: 
  - MODER: mode registers
  - OTYPER: output type register for open drain
  - OSPEEDR: output speed register
  - PUPDR: pullup/pulldown register
  - GPIOB_AFR: alternate function register for I2C
*/
#define GPIOB_MODER   (*(volatile uint32_t *)0x50000400)
#define GPIOB_OTYPER  (*(volatile uint32_t *)0x50000404)
#define GPIOB_OSPEEDR (*(volatile uint32_t *)0x50000408)
#define GPIOB_PUPDR   (*(volatile uint32_t *)0x5000040C)
#define GPIOB_IDR     (*(volatile uint32_t *)0x50000410)
#define GPIOB_ODR     (*(volatile uint32_t *)0x50000414)
#define GPIOB_AFR     (*(volatile uint32_t *)0x50000420)

/* GPIOC: LD2 on PC9 */
#define GPIOC_MODER   (*(volatile uint32_t *)0x50000800)
#define GPIOC_ODR     (*(volatile uint32_t *)0x50000814)

/*
EXTI: external interrupt registers for PA0, PA1, PA4, PB3
  - RTSR1:      rising edge trigger selection (lines 0,1,3,4)
  - FTSR1:      falling edge trigger selection (lines 0,1,3,4)
  - RPR1:       rising edge pending — write 1 to clear in ISR
  - FPR1:       falling edge pending — write 1 to clear in ISR
  - EXTICR[0]:  maps EXTI lines 0-3 to a GPIO port (0x0 = GPIOA, 0x1 = GPIOB);
                one byte per line, so line 3 (PB3) is bits [26:24]
  - EXTICR[1]:  maps EXTI lines 4-7 to a GPIO port (0x0 = GPIOA)
  - IMR1:       interrupt mask — unmask lines 0,1,3,4 to enable
  IRQ handlers needed:
    EXTI0_1_IRQn  (IRQ 5)  -> PA0, PA1
    EXTI2_3_IRQn  (IRQ 6)  -> PB3
    EXTI4_15_IRQn (IRQ 7)  -> PA4
*/
#define EXTI_RTSR1     (*(volatile uint32_t *)0x40021800)
#define EXTI_FTSR1     (*(volatile uint32_t *)0x40021804)
#define EXTI_RPR1      (*(volatile uint32_t *)0x4002180C)
#define EXTI_FPR1      (*(volatile uint32_t *)0x40021810)
#define EXTI_EXTICR0   (*(volatile uint32_t *)0x40021860)
#define EXTI_EXTICR1   (*(volatile uint32_t *)0x40021864)
#define EXTI_IMR1      (*(volatile uint32_t *)0x40021880)

/*
TIM3: PWM output on PA7 / D11 (TIM3_CH2; confirm the AF number in the datasheet)
  - CR1:   control register, enable counter (bit0)
  - CCMR1: capture/compare mode register, set CH2 to PWM mode 1 (bits 14:12 = 110)
  - CCER:  capture/compare enable register, enable CH2 output (bit4)
  - PSC:   prescaler — divides system clock
  - ARR:   auto-reload — sets PWM period
  - EGR:   event generation
  - CCR2:  capture/compare register, compare value — sets duty cycle
*/
#define TIM3_CR1    (*(volatile uint32_t *)0x40000400)
#define TIM3_CCMR1  (*(volatile uint32_t *)0x40000418)
#define TIM3_CCER   (*(volatile uint32_t *)0x40000420)
#define TIM3_PSC    (*(volatile uint32_t *)0x40000428)
#define TIM3_ARR    (*(volatile uint32_t *)0x4000042C)
#define TIM3_EGR    (*(volatile uint32_t* )0x40000414)
#define TIM3_CCR2   (*(volatile uint32_t *)0x40000438)

/*
I2C1: SH1106 OLED on PB6 (SCL, AF6) and PB7 (SDA, AF6)
  - CR1:     enable peripheral, interrupt enables
  - CR2:     slave address, byte count, start/stop generation
  - TIMINGR: SCL timing (use ST CubeMX or AN4235 to calculate)
  - ISR:     status flags: TXIS (ready to send), RXNE (data ready), TC (transfer complete)
  - ICR:     write 1 to clear flags (STOPCF, NACKCF, etc.)
  - RXDR:    read received byte from here
  - TXDR:    write byte to transmit here
*/
#define I2C1_CR1     (*(volatile uint32_t *)0x40005400)
#define I2C1_CR2     (*(volatile uint32_t *)0x40005404)
#define I2C1_TIMINGR (*(volatile uint32_t *)0x40005410)
#define I2C1_ISR     (*(volatile uint32_t *)0x40005418)
#define I2C1_ICR     (*(volatile uint32_t *)0x4000541C)
#define I2C1_RXDR    (*(volatile uint32_t *)0x40005424)
#define I2C1_TXDR    (*(volatile uint32_t *)0x40005428)

#define OLED_ADDR    (const uint32_t)0x3C
#define CMD_CONTROL  0x00

#define DEBOUNCE_MS 20

/*FSM modes*/
#define MODE_CLOCK      0
#define MODE_SET_TIME   1
#define MODE_SET_ALARM  2

/*button event macros*/
#define BUTTON_NONE      -1
#define BUTTON_INCREMENT  0
#define BUTTON_DECREMENT  1
#define BUTTON_ALARM_SET  2
#define BUTTON_SNOOZE     3

/*buzzer notes*/
#define C6          1047
#define CS6         1109
#define DF6         1109
#define D6          1175
#define DS6         1245
#define EF6         1245
#define E6          1319
#define F6          1397
#define FS6         1480
#define GF6         1480
#define G6          1568
#define GS6         1661
#define AF6         1661
#define A6          1760
#define AS6         1865
#define BF6         1865
#define B6          1976
#define C7          2093
#define CS7         2217
#define DF7         2217
#define D7          2349
#define DS7         2489
#define EF7         2489
#define E7          2637
#define F7          2794
#define FS7         2960
#define GF7         2960
#define G7          3136
#define GS7         3322
#define AF7         3322
#define A7          3520
#define AS7         3729
#define BF7         3729
#define B7          3951
#define C8          4186

/*note lengths*/
#define SAME_NOTE          30
#define EIGHTH_NOTE        250  
#define QUARTER_NOTE      (EIGHTH_NOTE * 2)
#define HALF_NOTE         (EIGHTH_NOTE * 4)
#define WHOLE_NOTE        (EIGHTH_NOTE * 8)

/*PWM macros*/
#define BOARD_CLOCK       12000000U
#define GPIOA_PA7         7U
#define PRESCALE_FACTOR   0x0BU                             /* 12 MHz / (11 + 1) = 1 MHz tick */
#define TIMER_TICK_HZ     (BOARD_CLOCK / (PRESCALE_FACTOR + 1U))
#define AUTO_RELOAD_INIT  0x08E0U                           /* 440 Hz at a 1 MHz tick */
#define CPS               1200

/*global event flags*/
extern volatile uint8_t current_mode;
extern volatile int8_t button_event;
extern volatile uint8_t time_changed;
extern volatile uint8_t alarm_sound;

struct Curr_Time{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

/*buzz.c: passive piezo buzzer, TIM3_CH2 PWM on PA7*/
void buzzer_init(void);
void buzzer_sound(uint32_t freq);
void buzzer_stop(void);
void buzzer_scale(void); 
void note_len(int len); 

/*main_fsm.c: FSM button handling*/
void press(int8_t button_event, uint8_t fsm_mode);

/*irq.c: interrupt handlers, referenced by the vector table in startup.s*/
void EXTI0_1_IRQHandler(void);
void EXTI2_3_IRQHandler(void);
void EXTI4_15_IRQHandler(void);
void RTC_IRQHandler(void);

/*oled.c: i2c bus*/
void i2c_scan(void);

static inline void delay(int cycles) {
    for (volatile int i = 0; i < cycles; i++)
        __asm__("nop");
}

#endif