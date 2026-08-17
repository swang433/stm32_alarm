#include "config.h"
#include <stdint.h>

//manually toggle via bit bang
#define SCL_HI() (GPIOB_ODR |=  (1U << 6))
#define SCL_LO() (GPIOB_ODR &= ~(1U << 6)) 
#define SDA_HI() (GPIOB_ODR |=  (1U << 7))
#define SDA_LO() (GPIOB_ODR &= ~(1U << 7)) 
#define SDA_IN() (GPIOB_IDR &   (1U << 7))

#define OLED_ADDR   0x3C
#define CMD_CONTROL 0x00

static void delay(volatile unsigned int cycles){
    for (int i = 0; i < cycles; i++){
        __asm__("nop"); 
    }
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
    SDA_HI(); dly(); 
    SCL_HI(); dly(); 
    SDA_LO(); dly(); 
    SCL_LO(); 
}

//stop condition
static void stop(void){
    SDA_LO(); dly();
    SCL_HI(); dly(); 
    SDA_HI(); dly(); 
}

static int write(uint8_t byte){
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
    return nack ? 0: 1; //if nack return write fail(0)
}

static void oled_cmd(uint8_t cmd){
    start(); 
    write(OLED_ADDR << 1);
    write(CMD_CONTROL); 
    write(cmd); 
    stop(); 
}

static void oled_init(void) {
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

static const uint8_t heart_top[16] = {
    0x00, 0x1C, 0x3E, 0x7E, 0xFE, 0xFE, 0x7E, 0x3E,
    0x3E, 0x7E, 0xFE, 0xFE, 0x7E, 0x3E, 0x1C, 0x00
};

static const uint8_t heart_bot[16] = {
    0x00, 0x1C, 0x3E, 0x7E, 0x7C, 0x38, 0x10, 0x00,
    0x00, 0x10, 0x38, 0x7C, 0x7E, 0x3E, 0x1C, 0x00
};

static void oled_data_buf(uint8_t page, uint8_t col, const uint8_t *buf, uint8_t len) {
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

static void draw_heart(uint8_t page, uint8_t col) {
    oled_data_buf(page + 1, col, heart_top, 16);
    oled_data_buf(page,     col, heart_bot, 16);   
}

int main(void){
    bb_init(); 
    delay(800000); 
    oled_init(); 
    draw_heart(3, 56); 
    
    while(1); 
}