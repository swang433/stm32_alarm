#include "config_c071.h"
#include <stdint.h>

static void i2c_init(void){
    /* enable GPIOB and I2C1 clocks*/
    RCC_IOPENR |= (1U << 1); 
    RCC_APB1ENR |= (1U << 21); 

    /* 
    clear control bits for pins 6 and 7 
    and enable pins 6 and 7 as open drain AF open drain 
    */ 
    GPIOB_MODER  &= ~((0x3 << 12) | (0x3 << 14)); 
    GPIOB_MODER  |=  ((0x2 << 12) | (0x2 << 14)); 
    GPIOB_OTYPER |=  (1U << 6)    | (1U << 7); 
    GPIOB_PUPDR  |=  (1U << 12)   | (1U << 14); 
    GPIOB_AFR    |=  (6U << 24)   | (6U << 28); 

    /*
    clear peripheral enable bit
    set timing for 400khz I2C slave clock
    enable peripherl
    */
    I2C1_CR1 &= ~(1U << 0);  
    I2C1_TIMINGR = 3598; 
    I2C1_CR1 |= (1U << 0); 
}

static void start(void){
    /*
    bit 15 of the interrupt status reg 
    checks of the bus is busy
    */
    while(I2C1_ISR & (1U << 15)); 
}

// dead code since AUTOEND is enabled
// static void stop(void){
//     /*
//     wait for byte transfer finished (BTF)
//     stop signal
//     */
//     while(!(I2C1_ISR & (1U << 6))); 
//     I2C1_CR2 |= (1U << 14); 
// }

static int write(uint8_t byte){
    /*
    I2C_ISR bit 1 only goes high when previous byte is ACK'd
    set transfer register to the actual command byte
    check if the byte sent out was ACK'd or not
    */
    while(!(I2C1_ISR & (1U << 1))); 
    I2C1_TXDR = byte; 
    return !(I2C1_ISR & (1U << 4)); 
}

static int oled_cmd(uint8_t cmd){
    /*
    start condition
    confirm 7 bit slave address via SADD bits in C2
        set transmission size to 2 bytes
        enable AUTOEND to automatically send stop condition
        start condition
    command control bytes
    actual command
    stop condition done via AUTOEND
    */
    start();  
    I2C1_CR2 = (OLED_ADDR << 1) 
            |  (2U << 16)
            |  (1U << 25)
            |  (1U << 13); 
    write(CMD_CONTROL); 
    int res = write(cmd); 
    
    while(!(I2C1_ISR & (1U << 5))); 
    I2C1_ICR |= (1U << 5); 
    return res; 
}

static void oled_init(void){
    oled_cmd(0xAE);        // display off
    oled_cmd(0xD5); oled_cmd(0x80);  // clock divide ratio / oscillator
    oled_cmd(0xA8); oled_cmd(0x3F);  // multiplex ratio (64 rows)
    oled_cmd(0xD3); oled_cmd(0x00);  // display offset = 0
    oled_cmd(0x40);        // start line = 0
    oled_cmd(0xAD); oled_cmd(0x8B);  // DC-DC on (SH1106 internal pump)
    oled_cmd(0xA1);        // segment remap (col 127 -> SEG0)
    oled_cmd(0xC8);        // COM scan reversed
    oled_cmd(0xDA); oled_cmd(0x12);  // COM pins config
    oled_cmd(0x81); oled_cmd(0x80);  // contrast
    oled_cmd(0xD9); oled_cmd(0x1F);  // precharge period
    oled_cmd(0xDB); oled_cmd(0x40);  // VCOMH deselect level
    oled_cmd(0xA4);        // display follows RAM
    oled_cmd(0xA6);        // normal display (not inverted)
    oled_cmd(0xAF);        // display on
}

/* fill all 8 pages with val — 0xFF = all pixels on, 0x00 = all off */
static void oled_fill(uint8_t val){
    for (uint8_t page = 0; page < 8; page++){
        /* set page and column address (SH1106 has 2-pixel offset) */
        oled_cmd(0xB0 | page);  // page address
        oled_cmd(0x02);         // column low nibble (offset 2)
        oled_cmd(0x10);         // column high nibble

        /* send 128 data bytes: 1 control byte + 128 pixel bytes = 129 total */
        start();
        I2C1_CR2 = (OLED_ADDR << 1)
                 | (129U << 16)
                 | (1U << 25)
                 | (1U << 13);
        write(0x40);            // data control byte
        for (uint8_t col = 0; col < 128; col++)
            write(val);
        while (!(I2C1_ISR & (1U << 5)));
        I2C1_ICR |= (1U << 5);
    }
}

volatile uint8_t found_addr = 0;

void i2c_scan(void) {
    /*
    for every possible 7 bit slave address: 
        set SADD, 
        0 byte transfer size with bit 16, 
        enable AUTOEND via bit 25, 
        start via bit 13 in control reg bits
        if NACK or STOP flag bits go high that means the bus is no longer blocked
        if nack bit (4) is low that means address found
        clear stop and nack flags via ICR
    */
    for (uint8_t addr = 1; addr < 128; addr++) {
        I2C1_CR2 = (addr << 1) | (0U << 16) | (1U << 25) | (1U << 13);
        while (!(I2C1_ISR & ((1U << 5) | (1U << 4))));
        if (!(I2C1_ISR & (1U << 4)))
            found_addr = addr;
        I2C1_ICR |= (1U << 5) | (1U << 4);
    }
}

static void led_init(void){
    RCC_IOPENR |= (1U << 0) | (1U << 2);  // GPIOA and GPIOC clocks
    GPIOA_MODER &= ~(0x3 << 10);
    GPIOA_MODER |=  (0x1 << 10);           // PA5 = LD1 output
    GPIOC_MODER &= ~(0x3 << 18);
    GPIOC_MODER |=  (0x1 << 18);           // PC9 = LD2 output
}

static void delay(volatile int cycles){
    while(cycles--){
        __asm__("nop"); 
    }
}

int main(void){
    i2c_init();
    i2c_scan();
    led_init();

    if (found_addr){
        oled_init();
        oled_fill(0xFF);  // all pixels on — screen should go fully white
    }

    while (1){
        if (found_addr){
            GPIOA_ODR ^= (1U << 5);
            GPIOC_ODR ^= (1U << 9);
            delay(100000);
        } else {
            GPIOA_ODR ^= (1U << 5);
            delay(250000);
        }
    }
}