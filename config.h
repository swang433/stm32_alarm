#ifndef CONFIG_H
#define CONFIG_H

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

/*
Reset and clock control registers:
  - RCC_IOPENR:  GPIO clock enable
  - RCC_APB1ENR: APB1 peripheral clock enable (TIM3, I2C1, RTC APB clock)
  - RCC_APB2ENR: APB2 peripheral clock enable
  - RCC_CSR1:    control/status register 1 (LSE on/ready, RTC clock source and enable)
*/
#define RCC_BASE      0x40021000U
#define RCC_IOPENR    (*(volatile uint32_t *)(RCC_BASE + 0x34))
#define RCC_APB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x3C))
#define RCC_APB2ENR   (*(volatile uint32_t *)(RCC_BASE + 0x40))
#define RCC_CSR1      (*(volatile uint32_t *)(RCC_BASE + 0x5C))

/*
GPIOA: 
  - MODER: mode registers
  - PUPDR: pullup/pulldown registers
  - GPIOA_AFR: alternate function register for PWM
*/
#define GPIOA_BASE   0x50000000U
#define GPIOA_MODER  (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_PUPDR  (*(volatile uint32_t *)(GPIOA_BASE + 0x0C))
#define GPIOA_IDR    (*(volatile uint32_t *)(GPIOA_BASE + 0x10))
#define GPIOA_ODR    (*(volatile uint32_t *)(GPIOA_BASE + 0x14))
#define GPIOA_AFR    (*(volatile uint32_t *)(GPIOA_BASE + 0x20))

/*
GPIOB: 
  - MODER: mode registers
  - OTYPER: output type register for open drain
  - OSPEEDR: output speed register
  - PUPDR: pullup/pulldown register
  - GPIOB_AFR: alternate function register for I2C
*/
#define GPIOB_BASE    0x50000400U
#define GPIOB_MODER   (*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_OTYPER  (*(volatile uint32_t *)(GPIOB_BASE + 0x04))
#define GPIOB_OSPEEDR (*(volatile uint32_t *)(GPIOB_BASE + 0x08))
#define GPIOB_PUPDR   (*(volatile uint32_t *)(GPIOB_BASE + 0x0C))
#define GPIOB_IDR     (*(volatile uint32_t *)(GPIOB_BASE + 0x10))
#define GPIOB_ODR     (*(volatile uint32_t *)(GPIOB_BASE + 0x14))
#define GPIOB_AFR     (*(volatile uint32_t *)(GPIOB_BASE + 0x20))

/* GPIOC: LD2 on PC9 */
#define GPIOC_BASE    0x50000800U
#define GPIOC_MODER   (*(volatile uint32_t *)(GPIOC_BASE + 0x00))
#define GPIOC_ODR     (*(volatile uint32_t *)(GPIOC_BASE + 0x14))

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
  - NVIC_ISER: nested vector interrupt controller interrupt set enable regsiter
*/
#define EXTI_BASE      0x40021800U
#define EXTI_RTSR1     (*(volatile uint32_t *)(EXTI_BASE + 0x00))
#define EXTI_FTSR1     (*(volatile uint32_t *)(EXTI_BASE + 0x04))
#define EXTI_RPR1      (*(volatile uint32_t *)(EXTI_BASE + 0x0C))
#define EXTI_FPR1      (*(volatile uint32_t *)(EXTI_BASE + 0x10))
#define EXTI_EXTICR0   (*(volatile uint32_t *)(EXTI_BASE + 0x60))
#define EXTI_EXTICR1   (*(volatile uint32_t *)(EXTI_BASE + 0x64))
#define EXTI_IMR1      (*(volatile uint32_t *)(EXTI_BASE + 0x80))
#define NVIC_BASE      0xE000E100U
#define NVIC_ISER      (*(volatile uint32_t *)(NVIC_BASE + 0x00))

/*
Real-time Clock registers: 
  - RTC_TR: time register
  - RTC_DR: date register
  - RTC_ICSR: initialization control and status register
  - RTC_PRER: prescaler register
  - RTC_CR: control register
  - RTC_WPR: write protect register
  - RTC_ALRMAR: alarm A register
  - RTC_SR: status register
  - RTC_SCR: status clear register
  (the RTC clock source and enable bits are in RCC_CSR1, defined with the RCC registers)
*/
#define RTC_BASE       0x40002800U
#define RTC_TR         (*(volatile uint32_t *)(RTC_BASE + 0x00))
#define RTC_DR         (*(volatile uint32_t *)(RTC_BASE + 0x04))
#define RTC_ICSR       (*(volatile uint32_t *)(RTC_BASE + 0x0C))
#define RTC_PRER       (*(volatile uint32_t *)(RTC_BASE + 0x10))
#define RTC_CR         (*(volatile uint32_t *)(RTC_BASE + 0x18))
#define RTC_WPR        (*(volatile uint32_t *)(RTC_BASE + 0x24))
#define RTC_ALRMAR     (*(volatile uint32_t *)(RTC_BASE + 0x40))
#define RTC_SR         (*(volatile uint32_t *)(RTC_BASE + 0x50))
#define RTC_SCR        (*(volatile uint32_t *)(RTC_BASE + 0x5C))

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
#define TIM3_BASE   0x40000400U
#define TIM3_CR1    (*(volatile uint32_t *)(TIM3_BASE + 0x00))
#define TIM3_CCMR1  (*(volatile uint32_t *)(TIM3_BASE + 0x18))
#define TIM3_CCER   (*(volatile uint32_t *)(TIM3_BASE + 0x20))
#define TIM3_PSC    (*(volatile uint32_t *)(TIM3_BASE + 0x28))
#define TIM3_ARR    (*(volatile uint32_t *)(TIM3_BASE + 0x2C))
#define TIM3_EGR    (*(volatile uint32_t *)(TIM3_BASE + 0x14))
#define TIM3_CCR2   (*(volatile uint32_t *)(TIM3_BASE + 0x38))

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
#define I2C1_BASE    0x40005400U
#define I2C1_CR1     (*(volatile uint32_t *)(I2C1_BASE + 0x00))
#define I2C1_CR2     (*(volatile uint32_t *)(I2C1_BASE + 0x04))
#define I2C1_TIMINGR (*(volatile uint32_t *)(I2C1_BASE + 0x10))
#define I2C1_ISR     (*(volatile uint32_t *)(I2C1_BASE + 0x18))
#define I2C1_ICR     (*(volatile uint32_t *)(I2C1_BASE + 0x1C))
#define I2C1_RXDR    (*(volatile uint32_t *)(I2C1_BASE + 0x24))
#define I2C1_TXDR    (*(volatile uint32_t *)(I2C1_BASE + 0x28))

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
#define AF5         830
#define BF5         932
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
void song(void); 

/*main_fsm.c: FSM button handling*/
void press(int8_t button_event, uint8_t fsm_mode);

/*irq.c: interrupt handlers, referenced by the vector table in startup.s*/
void EXTI0_1_IRQHandler(void);
void EXTI2_3_IRQHandler(void);
void EXTI4_15_IRQHandler(void);
void RTC_IRQHandler(void);

/*oled.c: i2c bus*/
void i2c_scan(void);
void i2c_init(void);
void oled_init(void);
void oled_fill(uint8_t val);

static inline void delay(int cycles) {
    for (volatile int i = 0; i < cycles; i++)
        __asm__("nop");
}

#endif