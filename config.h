#ifndef REGS_H
#define REGS_H

/* never include a .c file to avoid compile time duplication errors*/

#include <stdint.h>

/*
ports used: 
- A: base = 0x40010800
- B: base = 0x40010C00

SSD 1306 OLED Display: 
- I2C
- PB6 (SCL out)
- PB7 (SDA in and out)

Piezo Passive Buzzer: 
- PWM 
- PA6 (digital out)

Tactile switches: 
- PA0, 1, 2, 3 (digital in)
- digital input pullup
*/
#define RCC_APB2ENR (*(volatile uint32_t *) 0x40021018) //reset and clock control
#define RCC_APB1ENR (*(volatile uint32_t *) 0x4002101C) //I2C clock enable

/*
port A: 
- control regs
- input data regs
- output data regs
*/
#define GPIOA_CRL (*(volatile uint32_t *) 0x40010800)
#define GPIOA_IDR (*(volatile uint32_t *) 0x40010808)
#define GPIOA_ODR (*(volatile uint32_t *) 0x4001080C)

/*
port B:
- control regs
- input data regs
- output data regs
- !!! BSRR and BRR's obsolete for now !!!
*/
#define GPIOB_CRL (*(volatile uint32_t* ) 0x40010C00)
#define GPIOB_IDR (*(volatile uint32_t *) 0x40010C08)
#define GPIOB_ODR (*(volatile uint32_t *) 0x40010C0C)

//port C for testing
#define GPIOC_CRH    (*(volatile uint32_t *)0x40011004)
#define GPIOC_ODR    (*(volatile uint32_t *)0x4001100C)

/*AFIO - pin muxing for EXTI lines*/
#define AFIO_EXTICR1 (*(volatile uint32_t *) 0x40010008)

/*EXTI registers*/
#define EXTI_IMR  (*(volatile uint32_t *) 0x40010400)
#define EXTI_RTSR (*(volatile uint32_t *) 0x40010408)
#define EXTI_FTSR (*(volatile uint32_t *) 0x4001040C)
#define EXTI_PR   (*(volatile uint32_t *) 0x40010414)

/*NVIC - CPU-level interrupt enable*/
#define NVIC_ISER0 (*(volatile uint32_t *) 0xE000E100)

/*I2C peripheral regs*/
#define I2C_CR1   (*(volatile uint32_t *) 0x40005400) //I2C regs base address
#define I2C_CR2   (*(volatile uint32_t *) 0x40005404)
#define I2C_DR    (*(volatile uint32_t *) 0x40005410)
#define I2C_SR1   (*(volatile uint32_t *) 0x40005414)
#define I2C_SR2   (*(volatile uint32_t *) 0x40005418)
#define I2C_CCR   (*(volatile uint32_t *) 0x4000541C)
#define I2C_TRISE (*(volatile uint32_t *) 0x40005420)

/*clock-based registers*/
#define PWR_CR    (*(volatile uint32_t *) 0x40007000) //sleep, stop, standby
#define RCC_BDCR  (*(volatile uint32_t *) 0x40021020) //manages the oscillator crystal
#define RTC_CRH   (*(volatile uint32_t *) 0x40002800) //control regs 
#define RTC_CRL   (*(volatile uint32_t *) 0x40002804)
#define RTC_CNTH  (*(volatile uint32_t *) 0x40002810) //counter regs that actually track time
#define RTC_CNTL  (*(volatile uint32_t *) 0x40002814) 
#define RTC_ALRH  (*(volatile uint32_t *) 0x40002818) //alarm match conditions
#define RTC_ALRL  (*(volatile uint32_t *) 0x4000281C)

//note: 

/*RTC flags that help with ticking*/
#define RTC_CRL_SECF  (1U << 0) //second flag
#define RTC_CRL_ALRF  (1U << 1) //alarm flags that trigger interrupts on alarm times
#define RTC_CRL_OWF   (1U << 2) //overflow
#define RTC_CRL_RSF   (1U << 3) //registers synch flag
#define RTC_CRL_CNF   (1U << 4) //config mode flag
#define RTC_CRL_RTOFF (1U << 5) // real time clock off flag

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
volatile uint8_t current_mode = MODE_CLOCK;
volatile int8_t button_event = BUTTON_NONE; //which button pressed
volatile uint8_t time_changed = 0; 

struct Curr_Time{
    uint8_t hour; 
    uint8_t minute; 
    uint8_t second; 
};

volatile uint8_t alarm_sound = 0; 

#endif