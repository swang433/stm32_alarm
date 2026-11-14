#include "config.h"
#include <stdint.h>

#define SCL_PIN 6U
#define SDA_PIN 7U

void i2c_init(void){
    /* enable GPIOB and I2C1 clocks*/
    RCC_IOPENR |= (1U << 1); 
    RCC_APB1ENR |= (1U << 21); 

    /* 
    clear control bits for pins 6 and 7 
    and set pins 6 and 7 to alternate function, open drain, pull-up, AF6 
    */ 
    GPIOB_MODER  &= ~((0x3 << 2 * SCL_PIN)          | (0x3 << 2 * SDA_PIN)); 
    GPIOB_MODER  |=  ((0x2 << 2 * SCL_PIN)          | (0x2 << 2 * SDA_PIN)); 
    GPIOB_OTYPER |=  (1U << SCL_PIN)                | (1U << SDA_PIN); 
    GPIOB_PUPDR  |=  (1U << 2 * SCL_PIN)            | (1U << 2 * SDA_PIN); 
    GPIOB_AFR    |=  (6U << 4 * SCL_PIN)            | (6U << 4 * SDA_PIN); 

    /*
    clear peripheral enable bit
    set timing for 400khz I2C slave clock
    enable peripheral
    */
    I2C1_CR1 &= ~(1U << 0);  
    I2C1_TIMINGR = 3598; 
    I2C1_CR1 |= (1U << 0); 
}

static void i2c_start(void){
    /*
    bit 15 of the interrupt status reg 
    checks if the bus is busy
    */
    while(I2C1_ISR & (1U << 15)); 
}

static int i2c_write(uint8_t byte){
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
    i2c_start();  
    I2C1_CR2 = (OLED_ADDR << 1) 
            |  (2U << 16)
            |  (1U << 25)
            |  (1U << 13); 
    i2c_write(CMD_CONTROL); 
    int res = i2c_write(cmd); 
    
    while(!(I2C1_ISR & (1U << 5))); 
    I2C1_ICR |= (1U << 5); 
    return res; 
}

void oled_init(void){
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
void oled_fill(uint8_t val){
    for (uint8_t page = 0; page < 8; page++){
        /* set page and column address (SH1106 has 2-pixel offset) */
        oled_cmd(0xB0 | page);  // page address
        oled_cmd(0x02);         // column low nibble (offset 2)
        oled_cmd(0x10);         // column high nibble

        /* send 128 data bytes: 1 control byte + 128 pixel bytes = 129 total */
        i2c_start();
        I2C1_CR2 = (OLED_ADDR << 1)
                 | (129U << 16)
                 | (1U << 25)
                 | (1U << 13);
        i2c_write(0x40);            // data control byte
        for (uint8_t col = 0; col < 128; col++)
            i2c_write(val);
        while (!(I2C1_ISR & (1U << 5)));
        I2C1_ICR |= (1U << 5);
    }
}

volatile uint8_t found_addr; 

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

static const uint8_t font_digits[12][5] = {
    {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00},
    {0x42,0x61,0x51,0x49,0x46}, {0x21,0x41,0x45,0x4B,0x31},
    {0x18,0x14,0x12,0x7F,0x10}, {0x27,0x45,0x45,0x45,0x39},
    {0x3C,0x4A,0x49,0x49,0x30}, {0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36}, {0x06,0x49,0x49,0x29,0x1E},
    {0x00,0x36,0x36,0x00,0x00}, {0x00,0x00,0x00,0x00,0x00}
};

static void oled_set_cursor(uint8_t page, uint8_t col){
    col += 2; 
    oled_cmd(0xB0 | page); 
    oled_cmd(0x00 | (col & 0x0F)); 
    oled_cmd(0x10 | (col >> 4)); 
}

static void oled_data(const uint8_t *buf, uint8_t len){
    i2c_start(); 
    I2C1_CR2 = (OLED_ADDR << 1) | ((uint32_t)(len + 1) << 16) | (1U << 25) | (1U << 13);
    i2c_write(0x40);  
    for (uint8_t i = 0; i < len; i++) i2c_write(buf[i]); 
    while (!(I2C1_ISR & (1U << 5))); 
    I2C1_ICR |= (1U << 5); 
}

/* double each bit vertically: bit i -> bits 2i and 2i+1 */
static uint16_t stretch(uint8_t b){
    uint16_t r = 0;
    for (uint8_t i = 0; i < 8; i++)
        if (b & (1U << i)) r |= 3U << (2 * i);
    return r;
}

static void oled_char2x(uint8_t page, uint8_t col, char c){
    uint8_t idx = (c == ' ') ? 11 : (c == ':') ? 10 : (uint8_t)(c - '0');
    uint8_t top[12], bot[12];
    top[10] = top[11] = bot[10] = bot[11] = 0;   // spacing columns; avoids memset under -nostdlib
    for (uint8_t i = 0; i < 5; i++){
        uint16_t s = stretch(font_digits[idx][i]);
        top[2*i] = top[2*i + 1] = s & 0xFF;
        bot[2*i] = bot[2*i + 1] = s >> 8;
    }
    oled_set_cursor(page, col);     oled_data(top, 12);
    oled_set_cursor(page + 1, col); oled_data(bot, 12);
}

/* "HH:MM:SS" centred: 8 chars x 12px = 96px, starting at column 16, pages 3-4 */
void oled_time(uint8_t h, uint8_t m, uint8_t s, bool hide){
    char str[8] = { '0' + h/10, '0' + h%10, ':', '0' + m/10, '0' + m%10,
                    ':', '0' + s/10, '0' + s%10 };
     
    if (hide){
        for (uint8_t i = 0; i < 8; i++){
            str[i] = ' '; 
        }
    }

    for (uint8_t i = 0; i < 8; i++)
        oled_char2x(3, 16 + 12*i, str[i]);
}