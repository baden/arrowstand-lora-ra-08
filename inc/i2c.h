#pragma once

#include <stdlib.h>
#include <stdint.h>

// I2C Functions
void I2C_init(void);            // I2C init function
void I2C_start(uint8_t addr);   // I2C start transmission, addr must contain R/W bit
void I2C_write(uint8_t data);   // I2C transmit one data byte via I2C
void I2C_stop(void);            // I2C stop transmission
uint8_t I2C_read(uint8_t addr, uint8_t reg);    // I2C read one byte from register
