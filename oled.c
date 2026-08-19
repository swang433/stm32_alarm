#include "config.h"
#include <stdint.h>

static void i2c_init(void){
    /* enable GPIOB and I2C1 clocks*/
    RCC_APB2ENR |= (1U << 3); 
    RCC_APB1ENR |= (1U << 21); 

    /* 
    clear control bits for pins 6 and 7 
    and enable pins 6 and 7 as open drain AF open drain 
    */
    GPIOB_CRL &= ~(0xFF << 24); 
    GPIOB_CRL |=  (0xBB << 24); 

    /*clear and config I2C bus*/
    I2C_CR1 |=  (1U << 15); 
    I2C_CR1 &= ~(1U << 15); 
    
    /*
    set APB bus speed
    set actual I2C bus speed 
    set rise time
    enable peripheral for the I2C bus
    */
    I2C_CR2 = 8; 
    I2C_CCR = (1U << 15) | 7;
    I2C_TRISE = 3; 
    I2C_CR1 |= (1U << 0); 
}

static void start(void){
    /*
    wait til the bus is not busy
    start
    wait for start bit
    */
    while(I2C_SR2 & (1U << 1)); 
    I2C_CR1 |= (1U << 8); 
    while(!(I2C_SR1 & (1U << 0))); 
}

static void stop(void){
    /*
    wait for byte transfer finished (BTF)
    stop signal
    */
    while(!(I2C_SR1 & (1U << 2))); 
    I2C_CR1 |= (1U << 9); 
}

static int write(uint8_t byte){
    /*
    write byte to data reg
    wait til transfer register empty
    return 0 on NACK, vice versa
    */
    I2C_DR = byte; 
    while(!(I2C_SR1 & (1U << 7))); 
    return !(I2C_SR1 & (1U << 10)); 
}

static void send_addr(uint8_t addr){
    /*
    set data reg to the slave address
    check if address matched via status reg
    read status register and discarding value 
    */
    I2C_DR = (addr << 1); 
    while(!(I2C_SR1 & (1U << 1)));
    (void) I2C_SR1; 
    (void) I2C_SR2;  
}

static int oled_cmd(uint8_t cmd){
    /*
    start condition
    confirm slave address
    command control bytes
    actual command
    stop condition 
    */
    start(); 
    send_addr(OLED_ADDR); 
    write(CMD_CONTROL); 
    write(cmd); 
    stop(); 
}

int main(void){
    return 0; 
}