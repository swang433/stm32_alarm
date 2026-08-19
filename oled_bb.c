#include "config_stm32f.h"
#include <stdint.h>

//manually toggle via bit bang
#define SCL_HI() (GPIOB_ODR |=  (1U << 6))
#define SCL_LO() (GPIOB_ODR &= ~(1U << 6)) 
#define SDA_HI() (GPIOB_ODR |=  (1U << 7))
#define SDA_LO() (GPIOB_ODR &= ~(1U << 7)) 
#define SDA_IN() (GPIOB_IDR &   (1U << 7))

static void delay(volatile unsigned int cycles){
    for (int i = 0; i < cycles; i++)__asm__("nop"); 
}

void bb_init(void){
    RCC_APB2ENR |= (1U << 3); 
    GPIOB_CRL &= ~(0xFF << 24);
    GPIOB_CRL |= (0x77 << 24);  /* PB6=SCL PB7=SDA: GP open-drain 50MHz */
    SCL_HI(); SDA_HI(); 
    delay(10000); 
}

static void dly(){delay(50);}

//start condition
static void start(void){
    /* pulling SDA low when SCL is high  */
    SDA_HI(); dly(); 
    SCL_HI(); dly(); 
    SDA_LO(); dly(); 
    SCL_LO(); 
}

//stop condition
static void stop(void){
    /* pulling SDA high when SCL is high */
    SDA_LO(); dly();
    SCL_HI(); dly(); 
    SDA_HI(); dly(); 
}

static int write(uint8_t byte){
    /*
    iterate through every bit of the byte: 
        manually toggle data wire 1 = high, low = 0
        tick clock wire once to signal the end of the bit
    reset data wire high (default)
    tick clock once, during the clock cycle
    check if the SDA wire is low signaling a successful write
    return 1 if write is successful
    */
    for (int i = 7; i >= 0; i--){
        if (byte & (1U << i)){
            SDA_HI(); 
        }
        else{
            SDA_LO(); 
        }
        dly(); SCL_HI();
        dly(); SCL_LO(); 
        dly(); 
    }
    SDA_HI(); dly(); 
    SCL_HI(); dly(); 
    int nack = SDA_IN(); 
    SCL_LO(); dly(); 
    return nack ? 0: 1; 
}

static void oled_cmd(uint8_t cmd){
    /*
    start condition
    write 7-bit slave address
    command control byte
    actual command
    stop condition
    */
    start(); 
    write(OLED_ADDR << 1);
    write(CMD_CONTROL); 
    write(cmd); 
    stop(); 
}

static void oled_init(void) {
    /* OLED init sequence */
    oled_cmd(0xAE);            /* display off */
    oled_cmd(0xD5); oled_cmd(0x80);  /* clock divide */
    oled_cmd(0xA8); oled_cmd(0x3F);  /* multiplex ratio: 64 rows */
    oled_cmd(0xD3); oled_cmd(0x00);  /* display offset: 0 */
    oled_cmd(0x40);            /* start line: 0 */
    oled_cmd(0xAD); oled_cmd(0x8B);  /* charge pump: internal VCC (SSH1106) */
    oled_cmd(0xA1);            /* segment remap */
    oled_cmd(0xC8);            /* COM scan direction: reversed */
    oled_cmd(0xDA); oled_cmd(0x12);  /* COM pins */
    oled_cmd(0x81); oled_cmd(0xCF);  /* contrast */
    oled_cmd(0xD9); oled_cmd(0xF1);  /* precharge period */
    oled_cmd(0xDB); oled_cmd(0x40);  /* VCOMH deselect */
    oled_cmd(0xA4);            /* display from RAM */
    oled_cmd(0xA6);            /* normal (not inverted) */
    oled_cmd(0xAF);            /* display on */
}

static void oled_data_buf(uint8_t page, uint8_t col, const uint8_t *buf, uint8_t len) {
    /*
    0xB0 => set page command
    set low bits for the column
    set high bits for the column
    start condition 
    select slave address
    control byte for the pixel data
    write each pixel column byte one at a time
    stop condition
    */
    oled_cmd(0xB0 | page);
    oled_cmd(0x00 | ((col + 2) & 0x0F));
    oled_cmd(0x10 | ((col + 2) >> 4));
    start();
    write(OLED_ADDR << 1);
    write(0x40);
    for (uint8_t i = 0; i < len; i++)
        write(buf[i]);
    stop();
}

static void oled_clear(void) {
    for (uint8_t page = 0; page < 8; page++) {
        oled_cmd(0xB0 | page);
        oled_cmd(0x02);
        oled_cmd(0x10);
        start();
        write(OLED_ADDR << 1);
        write(0x40);
        for (uint8_t i = 0; i < 128; i++)
            write(0x00);
        stop();
    }
}

static const uint8_t font5x7[10][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, /* 0 */
    {0x00, 0x42, 0x7F, 0x40, 0x00}, /* 1 */
    {0x42, 0x61, 0x51, 0x49, 0x46}, /* 2 */
    {0x21, 0x41, 0x45, 0x4B, 0x31}, /* 3 */
    {0x18, 0x14, 0x12, 0x7F, 0x10}, /* 4 */
    {0x27, 0x45, 0x45, 0x45, 0x39}, /* 5 */
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, /* 6 */
    {0x01, 0x71, 0x09, 0x05, 0x03}, /* 7 */
    {0x36, 0x49, 0x49, 0x49, 0x36}, /* 8 */
    {0x06, 0x49, 0x49, 0x29, 0x1E}, /* 9 */
};

static void digit(uint8_t page, uint8_t col, uint8_t digit){
    oled_data_buf(page, col, font5x7[digit], 5); 
}

static void display_digit(uint8_t num){
    if (num < 10){
        digit(3, 60, num); 
    }
    else{
        digit(3, 56, num / 10); 
        digit(3, 63, num % 10); 
    }
}

int main(void){
    bb_init(); 
    delay(100000); 
    oled_init(); 
    oled_clear(); 
    int i = 0;   
    while(1){
        oled_clear(); 
        if (i > 20){
            i = 0; 
        }
        display_digit(i); 
        i++; 
    } 
}