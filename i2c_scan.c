#include <stdint.h>

#define RCC_APB2ENR (*(volatile uint32_t *) 0x40021018)
#define RCC_APB1ENR (*(volatile uint32_t *) 0x4002101C)

#define GPIOA_CRH   (*(volatile uint32_t *) 0x40010804)
#define GPIOB_CRL   (*(volatile uint32_t *) 0x40010C00)

#define USART1_SR   (*(volatile uint32_t *) 0x40013800)
#define USART1_DR   (*(volatile uint32_t *) 0x40013804)
#define USART1_BRR  (*(volatile uint32_t *) 0x40013808)
#define USART1_CR1  (*(volatile uint32_t *) 0x4001380C)

#define I2C1_CR1    (*(volatile uint32_t *) 0x40005400)
#define I2C1_CR2    (*(volatile uint32_t *) 0x40005404)
#define I2C1_DR     (*(volatile uint32_t *) 0x40005410)
#define I2C1_SR1    (*(volatile uint32_t *) 0x40005414)
#define I2C1_SR2    (*(volatile uint32_t *) 0x40005418)
#define I2C1_CCR    (*(volatile uint32_t *) 0x4000541C)
#define I2C1_TRISE  (*(volatile uint32_t *) 0x40005420)

static void delay(int cycles) {
    for (volatile int i = 0; i < cycles; i++)
        __asm__("nop");
}

static void uart_init(void) {
    RCC_APB2ENR |= (1U << 14) | (1U << 2);

    /* PA9 AF push-pull 50MHz (CNF=10, MODE=11 -> 0xB) */
    GPIOA_CRH &= ~(0xF << 4);
    GPIOA_CRH |=  (0xB << 4);

    USART1_BRR = 833; /* 9600 baud at 8MHz */
    USART1_CR1 = (1U << 13) | (1U << 3);
}

static void uart_putc(char c) {
    while (!(USART1_SR & (1U << 7)));
    USART1_DR = c;
}

static void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

static void uart_print_hex(uint8_t val) {
    const char hex[] = "0123456789ABCDEF";
    uart_putc('0');
    uart_putc('x');
    uart_putc(hex[val >> 4]);
    uart_putc(hex[val & 0xF]);
}

static void i2c_init(void) {
    RCC_APB2ENR |= (1U << 0); /* AFIO clock */
    RCC_APB2ENR |= (1U << 3); /* GPIOB clock */
    RCC_APB1ENR |= (1U << 21); /* I2C1 clock */

    GPIOB_CRL &= ~(0xFF << 24);
    GPIOB_CRL |=  (0xBB << 24);

    I2C1_CR1 |= (1U << 15);
    I2C1_CR1 &= ~(1U << 15);

    I2C1_CR2   = 8;
    I2C1_CCR   = 40;
    I2C1_TRISE = 9;

    I2C1_CR1 |= (1U << 0);
}

static int i2c_probe(uint8_t addr) {
    while (I2C1_SR2 & (1U << 1));

    I2C1_CR1 |= (1U << 8);
    while (!(I2C1_SR1 & (1U << 0)));

    I2C1_DR = (addr << 1);

    uint32_t sr1;
    while (1) {
        sr1 = I2C1_SR1;
        if (sr1 & (1U << 1))  break;  /* ADDR = ACK */
        if (sr1 & (1U << 10)) break;  /* AF = NACK */
    }

    int acked = (sr1 & (1U << 1)) ? 1 : 0;

    if (acked) {
        (void) I2C1_SR1;
        (void) I2C1_SR2;
    } else {
        I2C1_SR1 &= ~(1U << 10);
    }

    I2C1_CR1 |= (1U << 9);
    delay(10000);

    return acked;
}

int main(void) {
    uart_init();
    i2c_init();

    uart_puts("I2C scan:\r\n");

    int found = 0;
    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        if (i2c_probe(addr)) {
            uart_puts("  found: ");
            uart_print_hex(addr);
            uart_puts("\r\n");
            found++;
        }
    }

    if (!found)
        uart_puts("  no devices found\r\n");

    uart_puts("done\r\n");

    while (1);
}
