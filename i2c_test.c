#include <stdint.h>
#include "config.h"

#define SCL_HI() (GPIOB_ODR |=  (1U << 6))
#define SCL_LO() (GPIOB_ODR &= ~(1U << 6))
#define SDA_HI() (GPIOB_ODR |=  (1U << 7))
#define SDA_LO() (GPIOB_ODR &= ~(1U << 7))
#define SDA_IN() (GPIOB_IDR &   (1U << 7))

#define OLED_ADDR 0x3C

static void delay(volatile int n) { while (n--); }
static void dly(void) { delay(50); }

static void bb_init(void) {
    RCC_APB2ENR |= (1U << 3);
    GPIOB_CRL &= ~(0xFF << 24);
    GPIOB_CRL |=  (0x77 << 24);   /* PB6=SCL PB7=SDA: GP open-drain 50MHz */
    SCL_HI(); SDA_HI();
    delay(10000);
}

static void bb_start(void) {
    SDA_HI(); dly(); SCL_HI(); dly();
    SDA_LO(); dly(); SCL_LO(); dly();
}

static void bb_stop(void) {
    SDA_LO(); dly(); SCL_HI(); dly(); SDA_HI(); dly();
}

static int bb_write(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        if (byte & (1U << i)) SDA_HI(); else SDA_LO();
        dly(); SCL_HI(); dly(); SCL_LO(); dly();
    }
    SDA_HI(); dly();
    SCL_HI(); dly();
    int nack = SDA_IN();
    SCL_LO(); dly();
    return nack ? 0 : 1;
}

static int bb_probe(uint8_t addr) {
    bb_start();
    int ack = bb_write(addr << 1);
    bb_stop();
    return ack;
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

static void oled_cmd(uint8_t cmd) {
    bb_start();
    bb_write(OLED_ADDR << 1);  /* slave address + write bit */
    bb_write(0x00);            /* control byte: Co=0 D/C#=0 = command */
    bb_write(cmd);
    bb_stop();
}

static const uint8_t colors[8] = {
    0xFF, 0xFF, 0xF0, 0xCC, 0x33, 0x0F, 0xAA, 0x55
};

static void stripes(void) {
    for (uint8_t page = 0; page < 8; page++) {
        oled_cmd(0xB0 + page);  /* set page */
        oled_cmd(0x02);         /* set low column (2-col hardware offset) */
        oled_cmd(0x10);         /* set high column */

        bb_start();
        bb_write(OLED_ADDR << 1);
        bb_write(0x40);         /* control byte: Co=0 D/C#=1 = data */
        for (int i = 0; i < 128; i++)
            bb_write(colors[page]);
        bb_stop();
    }
}

int main(void) {
    RCC_APB2ENR |= (1U << 4);   /* GPIOC clock */
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |=  (0x2 << 20);  /* PC13 output 2MHz push-pull */
    GPIOC_ODR |=  (1U << 13);   /* LED off */

    bb_init();
    delay(800000);               /* let display VCC stabilize */

    if (!bb_probe(OLED_ADDR)) {
        while (1) {              /* slow blink = display not found */
            GPIOC_ODR ^= (1U << 13);
            delay(400000);
        }
    }

    oled_init();
    stripes();

    while (1);
}
