#include "i2c.h"
#include "tremo_rcc.h"
#include "tremo_gpio.h"
#include "tremo_i2c.h"
#include "tremo_delay.h"
#include "tremo_dma.h"
#include "tremo_dma_handshake.h"
#include <stdbool.h>

void I2C_init(void)
{
    i2c_config_t config;

    // enable the clk
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_I2C0, true);
    // rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
    // Начеб-то не треба
    // gpio_init(GPIOA, GPIO_PIN_14, GPIO_MODE_OUTPUT_OD_HIZ);
    // gpio_init(GPIOA, GPIO_PIN_15, GPIO_MODE_OUTPUT_OD_HIZ);

    // set iomux
    gpio_set_iomux(GPIOA, GPIO_PIN_14, 3);
    gpio_set_iomux(GPIOA, GPIO_PIN_15, 3);

    // init
    i2c_config_init(&config);
#if defined(I2C_OVER_DMA)
    config.fifo_mode_en = true;
#endif
    // config.settings.master.speed = I2C_CR_BUS_MODE_FAST;
    i2c_init(I2C0, &config);
    i2c_cmd(I2C0, true);
}

// I2C start transmission, addr must contain R/W bit
void I2C_start(uint8_t addr)
{
    i2c_master_send_start(I2C0, addr, I2C_WRITE);
}

// I2C transmit one data byte via I2C
void I2C_write(uint8_t data)
{
    i2c_clear_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY);
    while (i2c_get_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY) != SET) continue;
    i2c_send_data(I2C0, data);
}

// I2C stop transmission
void I2C_stop(void)
{
    i2c_clear_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY);
    while (i2c_get_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY) != SET) continue;
    // stop
    i2c_master_send_stop(I2C0);
}

uint8_t I2C_read(uint8_t addr, uint8_t reg)
{
    // start
    i2c_master_send_start(I2C0, addr, I2C_WRITE);
    i2c_clear_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY);
    while (i2c_get_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY) != SET) continue;

    // write data
    i2c_send_data(I2C0, reg);
    i2c_clear_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY);
    while (i2c_get_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY) != SET) continue;

    // restart
    i2c_master_send_start(I2C0, addr, I2C_READ);
    i2c_clear_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY);
    while (i2c_get_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY) != SET) continue;

    // read data
    i2c_set_receive_mode(I2C0, I2C_NAK);
    while (i2c_get_flag_status(I2C0, I2C_FLAG_RECV_FULL) != SET) continue;
    i2c_clear_flag_status(I2C0, I2C_FLAG_RECV_FULL);
    uint8_t data = i2c_receive_data(I2C0);

    // stop
    i2c_master_send_stop(I2C0);
    return data;
}