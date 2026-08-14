#include <stdint.h>

#define RCC_APB2ENR (*(volatile uint32_t *) 0x40021018)
#define RCC_APB1ENR (*(volatile uint32_t *) 0x4002101C)

#define GPIOB_CRL   (*(volatile uint32_t *) 0x40010C00)

#define I2C1_CR1    (*(volatile uint32_t *) 0x40005400)
#define I2C1_CR2    (*(volatile uint32_t *) 0x40005404)
#define I2C1_DR     (*(volatile uint32_t *) 0x40005410)
#define I2C1_SR1    (*(volatile uint32_t *) 0x40005414)
#define I2C1_SR2    (*(volatile uint32_t *) 0x40005418)
#define I2C1_CCR    (*(volatile uint32_t *) 0x4000541C)
#define I2C1_TRISE  (*(volatile uint32_t *) 0x40005420)

#define SSD1306_ADDR 0x3C

static void i2c_init(void) {
    /* enable GPIOB and I2C1 clocks */
    RCC_APB2ENR |= (1U << 3);
    RCC_APB1ENR |= (1U << 21);

    /* PB6 SCL, PB7 SDA: AF open-drain, 50MHz (CNF=10, MODE=11 -> 0xB) */
    GPIOB_CRL &= ~(0xFF << 24);
    GPIOB_CRL |=  (0xBB << 24);

    /* reset I2C1 */
    I2C1_CR1 |= (1U << 15);
    I2C1_CR1 &= ~(1U << 15);

    /* APB1 clock = 8MHz */
    I2C1_CR2 = 8;

    /* standard mode 100kHz: CCR = 8MHz / (2 * 100kHz) = 40 */
    I2C1_CCR = 40;

    /* TRISE = (8MHz / 1MHz) + 1 = 9 */
    I2C1_TRISE = 9;

    /* enable peripheral */
    I2C1_CR1 |= (1U << 0);
}

static void i2c_start(void) {
    /* wait until bus free */
    while (I2C1_SR2 & (1U << 1));
    /* generate START */
    I2C1_CR1 |= (1U << 8);
    /* wait for SB flag */
    while (!(I2C1_SR1 & (1U << 0)));
}

static void i2c_send_addr(uint8_t addr) {
    /* write address (7-bit shifted left, write = bit0 clear) */
    I2C1_DR = (addr << 1);
    /* wait for ADDR flag then clear by reading SR1 and SR2 */
    while (!(I2C1_SR1 & (1U << 1)));
    (void) I2C1_SR1;
    (void) I2C1_SR2;
}

static void i2c_write(uint8_t data) {
    /* wait for TXE */
    while (!(I2C1_SR1 & (1U << 7)));
    I2C1_DR = data;
}

static void i2c_stop(void) {
    /* wait for BTF */
    while (!(I2C1_SR1 & (1U << 2)));
    I2C1_CR1 |= (1U << 9);
}

static void oled_send_cmd(uint8_t cmd) {
    i2c_start();
    i2c_send_addr(SSD1306_ADDR);
    i2c_write(0x00); /* Co=0, D/C=0 -> command byte follows */
    i2c_write(cmd);
    i2c_stop();
}

static void oled_init(void) {
    oled_send_cmd(0xAE); /* display off */
    oled_send_cmd(0xD5); /* set display clock divide */
    oled_send_cmd(0x80);
    oled_send_cmd(0xA8); /* set multiplex ratio */
    oled_send_cmd(0x3F); /* 64 rows */
    oled_send_cmd(0xD3); /* set display offset */
    oled_send_cmd(0x00);
    oled_send_cmd(0x40); /* set start line = 0 */
    oled_send_cmd(0x8D); /* charge pump */
    oled_send_cmd(0x14); /* enable */
    oled_send_cmd(0x20); /* memory addressing mode */
    oled_send_cmd(0x00); /* horizontal */
    oled_send_cmd(0xA1); /* segment remap */
    oled_send_cmd(0xC8); /* COM scan direction */
    oled_send_cmd(0xDA); /* COM pins */
    oled_send_cmd(0x12);
    oled_send_cmd(0x81); /* contrast */
    oled_send_cmd(0xCF);
    oled_send_cmd(0xD9); /* precharge */
    oled_send_cmd(0xF1);
    oled_send_cmd(0xDB); /* VCOMH deselect */
    oled_send_cmd(0x40);
    oled_send_cmd(0xA4); /* display from RAM */
    oled_send_cmd(0xA6); /* normal (not inverted) */
    oled_send_cmd(0xAF); /* display on */
}

static void oled_fill(uint8_t pattern) {
    /* set column and page address to cover full 128x64 display */
    oled_send_cmd(0x21); /* column address */
    oled_send_cmd(0x00);
    oled_send_cmd(0x7F);
    oled_send_cmd(0x22); /* page address */
    oled_send_cmd(0x00);
    oled_send_cmd(0x07);

    /* stream all 1024 bytes as data */
    i2c_start();
    i2c_send_addr(SSD1306_ADDR);
    i2c_write(0x40); /* Co=0, D/C=1 -> data bytes follow */
    for (int i = 0; i < 1024; i++) {
        i2c_write(pattern);
    }
    i2c_stop();
}

void delay(int cycles){
    for (uint8_t i = 0; i < cycles; i++){
        __asm__("nop"); 
    }
}

#define GPIOC_CRH (*(volatile uint32_t *) 0x40011004)
#define GPIOC_ODR (*(volatile uint32_t *) 0x4001100C)

int main(void) {
    RCC_APB2ENR |= (1U << 4);
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |=  (0x2 << 20);
    GPIOC_ODR |= (1U << 13); /* LED off */

    i2c_init();
    GPIOC_ODR &= ~(1U << 13); /* LED on = got past i2c_init */
    delay(800000);
    GPIOC_ODR |= (1U << 13);

    oled_init();              /* if it hangs here LED never blinks again */
    GPIOC_ODR &= ~(1U << 13);
    delay(800000);
    GPIOC_ODR |= (1U << 13);

    oled_fill(0xFF);
    while (1);
}