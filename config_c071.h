#ifndef CONFIG_C071_H
#define CONFIG_C071_H

/* never include a .c file to avoid compile time duplication errors*/

#include <stdint.h>

/*
ports used:
- A: base = 0x50000000  (AHB, not APB like F1)
- B: base = 0x50000400

SH1106 OLED Display:
- I2C1
- PB6 (SCL), AF6, open-drain
- PB7 (SDA), AF6, open-drain

Piezo Passive Buzzer:
- TIM3_CH1 PWM
- PA6, AF1

Tactile switches:
- PA0, 1, 2, 3 (digital in)
- digital input pull-up
*/

/*
RCC - base 0x40021000
C071 splits clock enables into:
  IOPENR  - GPIO ports  (AHB)
  APBENR1 - APB1: I2C1, TIM3, RTC, PWR
  APBENR2 - APB2: SYSCFG (needed for EXTI mux)
  BDCR    - backup domain: LSE, RTC clock source
verify offsets against RM0490 table 7
*/
#define RCC_IOPENR  (*(volatile uint32_t *) 0x40021054) //GPIO clock enable
#define RCC_APBENR1 (*(volatile uint32_t *) 0x4002105C) //APB1 peripheral clock enable
#define RCC_APBENR2 (*(volatile uint32_t *) 0x40021060) //APB2 peripheral clock enable
#define RCC_BDCR    (*(volatile uint32_t *) 0x40021090) //backup domain control

/*
port A:
- mode, output type, pull-up/down, data regs
- no CRL/CRH on C0 - replaced by MODER + OTYPER + PUPDR
*/
#define GPIOA_MODER  (*(volatile uint32_t *) 0x50000000) //2 bits per pin: 00=in 01=out 10=AF 11=analog
#define GPIOA_OTYPER (*(volatile uint32_t *) 0x50000004) //0=push-pull 1=open-drain
#define GPIOA_PUPDR  (*(volatile uint32_t *) 0x5000000C) //2 bits per pin: 00=none 01=pull-up 10=pull-down
#define GPIOA_IDR    (*(volatile uint32_t *) 0x50000010)
#define GPIOA_ODR    (*(volatile uint32_t *) 0x50000014)
#define GPIOA_AFRL   (*(volatile uint32_t *) 0x50000020) //alternate function, pins 0-7, 4 bits per pin

/*
port B:
- I2C1 pins PB6/PB7 need AF6 + open-drain
*/
#define GPIOB_MODER  (*(volatile uint32_t *) 0x50000400)
#define GPIOB_OTYPER (*(volatile uint32_t *) 0x50000404)
#define GPIOB_PUPDR  (*(volatile uint32_t *) 0x5000040C)
#define GPIOB_IDR    (*(volatile uint32_t *) 0x50000410)
#define GPIOB_ODR    (*(volatile uint32_t *) 0x50000414)
#define GPIOB_AFRL   (*(volatile uint32_t *) 0x50000420) //PB6=bits[27:24] PB7=bits[31:28]

/*port C for testing - only ODR needed for LED*/
#define GPIOC_MODER  (*(volatile uint32_t *) 0x50000800)
#define GPIOC_ODR    (*(volatile uint32_t *) 0x50000814)

/*
EXTI - new IP on C0, base 0x40021800
no AFIO: pin-to-EXTI mux is inside EXTI itself via EXTICR
EXTICR uses 8 bits per line (vs 4 bits on F1): 0=PA, 1=PB, 2=PC...
lines 0+1 share one IRQ, lines 2+3 share one IRQ (unlike F1 which had individual IRQs)
pending flags split into rising (RPR1) and falling (FPR1)
*/
#define EXTI_RTSR1   (*(volatile uint32_t *) 0x40021800) //rising trigger select
#define EXTI_FTSR1   (*(volatile uint32_t *) 0x40021804) //falling trigger select
#define EXTI_RPR1    (*(volatile uint32_t *) 0x4002180C) //rising edge pending (clear by writing 1)
#define EXTI_FPR1    (*(volatile uint32_t *) 0x40021810) //falling edge pending
#define EXTI_EXTICR1 (*(volatile uint32_t *) 0x40021860) //pin mux for lines 0-3
#define EXTI_EXTICR2 (*(volatile uint32_t *) 0x40021864) //pin mux for lines 4-7
#define EXTI_IMR1    (*(volatile uint32_t *) 0x40021880) //interrupt mask

/*NVIC - same Cortex-M core address, but IRQ numbers differ from F1*/
#define NVIC_ISER0 (*(volatile uint32_t *) 0xE000E100)

/*
IRQ positions in ISER0 for C071 (verify against RM0490 table 58):
EXTI lines 0-1 share IRQ 5, lines 2-3 share IRQ 6
*/
#define IRQ_EXTI0_1  (1U << 5)
#define IRQ_EXTI2_3  (1U << 6)
#define IRQ_RTC      (1U << 3)
#define IRQ_TIM3     (1U << 16)

/*
I2C1 - base address same (0x40005400) but completely different register layout
new IP: no CCR/TRISE, uses TIMINGR for all timing
no separate DR: split into TXDR (write) and RXDR (read)
no SR1/SR2: replaced by ISR (status) + ICR (clear flags)
address + byte count + start all packed into CR2
*/
#define I2C1_CR1     (*(volatile uint32_t *) 0x40005400)
#define I2C1_CR2     (*(volatile uint32_t *) 0x40005404)
#define I2C1_TIMINGR (*(volatile uint32_t *) 0x40005410) //replaces CCR + TRISE
#define I2C1_ISR     (*(volatile uint32_t *) 0x40005418) //status flags
#define I2C1_ICR     (*(volatile uint32_t *) 0x4000541C) //clear flags (write 1 to clear)
#define I2C1_RXDR    (*(volatile uint32_t *) 0x40005424) //receive data
#define I2C1_TXDR    (*(volatile uint32_t *) 0x40005428) //transmit data

/*I2C1 ISR flag bits*/
#define I2C_ISR_TXE   (1U << 0)  //transmit data reg empty
#define I2C_ISR_TXIS  (1U << 1)  //transmit interrupt (need to write next byte)
#define I2C_ISR_RXNE  (1U << 2)  //receive data not empty
#define I2C_ISR_NACKF (1U << 4)  //NACK received
#define I2C_ISR_STOPF (1U << 5)  //stop detected
#define I2C_ISR_TC    (1U << 6)  //transfer complete (AUTOEND=0)
#define I2C_ISR_BUSY  (1U << 15) //bus busy

/*I2C1 CR2 field positions - pack address, count, direction, start/stop into one write*/
#define I2C_CR2_SADD_SHIFT   1   //7-bit slave address sits at bits [7:1]
#define I2C_CR2_NBYTES_SHIFT 16  //number of bytes to transfer
#define I2C_CR2_RD_WRN       (1U << 10) //1=read 0=write
#define I2C_CR2_START        (1U << 13)
#define I2C_CR2_STOP         (1U << 14)
#define I2C_CR2_AUTOEND      (1U << 25) //auto stop after NBYTES transferred

/*I2C1 ICR bits - write 1 to clear the corresponding ISR flag*/
#define I2C_ICR_NACKCF (1U << 4)
#define I2C_ICR_STOPCF (1U << 5)

#define OLED_ADDR   0x3C
#define CMD_CONTROL 0x00

/*
TIM3 - base 0x40000400 (same as F1)
used for PA6 PWM output (TIM3_CH1, AF1)
register layout identical to F1 timers
*/
#define TIM3_CR1   (*(volatile uint32_t *) 0x40000400)
#define TIM3_DIER  (*(volatile uint32_t *) 0x4000040C) //interrupt/DMA enable
#define TIM3_SR    (*(volatile uint32_t *) 0x40000410)
#define TIM3_EGR   (*(volatile uint32_t *) 0x40000414) //event gen (write UG to reload PSC/ARR)
#define TIM3_CCMR1 (*(volatile uint32_t *) 0x40000418) //CH1+CH2 capture/compare mode
#define TIM3_CCER  (*(volatile uint32_t *) 0x40000420) //CH output enable
#define TIM3_PSC   (*(volatile uint32_t *) 0x40000428) //prescaler
#define TIM3_ARR   (*(volatile uint32_t *) 0x4000042C) //auto-reload (sets period)
#define TIM3_CCR1  (*(volatile uint32_t *) 0x40000434) //compare value CH1 (sets duty cycle)

/*
RTC - base 0x40002800, new calendar IP (NOT the F1 32-bit counter)
time is stored in BCD in TR, date in DR
alarm A compares against TR fields
write protection: write 0xCA then 0x53 to WPR to unlock, any other value locks
*/
#define RTC_TR     (*(volatile uint32_t *) 0x40002800) //time: [22:20]=HT [19:16]=HU [14:12]=MNT [11:8]=MNU [6:4]=ST [3:0]=SU
#define RTC_DR     (*(volatile uint32_t *) 0x40002804) //date
#define RTC_ICSR   (*(volatile uint32_t *) 0x4000280C) //init and status (replaces CRL)
#define RTC_PRER   (*(volatile uint32_t *) 0x40002810) //prescaler: [22:16]=async [14:0]=sync
#define RTC_CR     (*(volatile uint32_t *) 0x40002818) //control
#define RTC_WPR    (*(volatile uint32_t *) 0x40002824) //write protection (write 0xCA, 0x53 to unlock)
#define RTC_ALRMAR (*(volatile uint32_t *) 0x40002840) //alarm A match register
#define RTC_ALRMASSR (*(volatile uint32_t *) 0x40002844)

/*RTC ICSR bits*/
#define RTC_ICSR_ALRAWF (1U << 0) //alarm A write flag (poll before writing ALRMAR)
#define RTC_ICSR_RSF    (1U << 4) //register sync flag (poll after wakeup before reading TR/DR)
#define RTC_ICSR_INITF  (1U << 5) //in init mode (poll this after setting INIT)
#define RTC_ICSR_INIT   (1U << 6) //set to enter init mode, clear to exit

/*RTC CR bits*/
#define RTC_CR_ALRAE  (1U << 8)  //alarm A enable
#define RTC_CR_ALRAIE (1U << 12) //alarm A interrupt enable

/*RTC ALRMAR mask bits - set to ignore that field in the match*/
#define RTC_ALRMAR_MSK1 (1U << 7)  //ignore seconds
#define RTC_ALRMAR_MSK2 (1U << 15) //ignore minutes
#define RTC_ALRMAR_MSK3 (1U << 23) //ignore hours
#define RTC_ALRMAR_MSK4 (1U << 31) //ignore day/date

/*PWR*/
#define PWR_CR1 (*(volatile uint32_t *) 0x40007000)
#define PWR_CR1_DBP (1U << 8) //disable backup domain write protection

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
