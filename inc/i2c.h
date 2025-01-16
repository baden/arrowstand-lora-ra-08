#pragma once

#include <stdlib.h>
#include <stdint.h>

// I2C Functions
void I2C_init(void);            // I2C init function
void I2C_start(uint8_t addr);   // I2C start transmission, addr must contain R/W bit
void I2C_write(uint8_t data);   // I2C transmit one data byte via I2C
void I2C_stop(void);            // I2C stop transmission

void I2C_writeDMA(const uint32_t* buf, size_t buf_size);


#define DMA_START (1 << 11) | (0 << 10) | (0 << 9) | (1 << 8)
#define DMA_DATA  (1 << 11) | (0 << 10) | (0 << 9) | (0 << 8)
#define DMA_LAST  (1 << 11) | (0 << 10) | (1 << 9) | (0 << 8)
