#include <stdint.h>
#include <stdio.h>

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