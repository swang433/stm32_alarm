#ifndef CONFIG_C071_H
#define CONFIG_C071_H

#include <stdint.h>

/*
ports used: 
PA: 
  - buttons (GPIOA)
  - pins: A0, A1, A3, A4
  - passive piezo buzzer (PWM)
  - pins: A6
PB: 
  - SSH1106 OLED (I2C peripheral)
  - pins: B6, B7 (CN10 pins 35 and 37)
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
EXTI: external interrupt registers for PA0, PA1, PA3, PA4
  - RTSR1:      rising edge trigger selection (lines 0,1,3,4)
  - FTSR1:      falling edge trigger selection (lines 0,1,3,4)
  - RPR1:       rising edge pending — write 1 to clear in ISR
  - FPR1:       falling edge pending — write 1 to clear in ISR
  - EXTICR[0]:  maps EXTI lines 0-3 to a GPIO port (0x0 = GPIOA)
  - EXTICR[1]:  maps EXTI lines 4-7 to a GPIO port (0x0 = GPIOA)
  - IMR1:       interrupt mask — unmask lines 0,1,3,4 to enable
  IRQ handlers needed:
    EXTI0_1_IRQn  (IRQ 5)  -> PA0, PA1
    EXTI2_3_IRQn  (IRQ 6)  -> PA3
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
TIM3: PWM output on PA6 (AF1 = TIM3_CH1)
  - CR1:   enable counter (bit0)
  - CCMR1: set CH1 to PWM mode 1 (bits 6:4 = 110)
  - CCER:  enable CH1 output (bit0)
  - PSC:   prescaler — divides system clock
  - ARR:   auto-reload — sets PWM period
  - CCR1:  compare value — sets duty cycle
*/
#define TIM3_CR1    (*(volatile uint32_t *)0x40000400)
#define TIM3_CCMR1  (*(volatile uint32_t *)0x40000418)
#define TIM3_CCER   (*(volatile uint32_t *)0x40000420)
#define TIM3_PSC    (*(volatile uint32_t *)0x40000428)
#define TIM3_ARR    (*(volatile uint32_t *)0x4000042C)
#define TIM3_CCR1   (*(volatile uint32_t *)0x40000434)

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

#endif