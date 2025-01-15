#include "i2c.h"
#include "tremo_rcc.h"
#include "tremo_gpio.h"
#include "tremo_i2c.h"
#include "tremo_delay.h"

void I2C_init(void)
{
    i2c_config_t config;

    // enable the clk
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_I2C0, true);
    // rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);

    // set iomux
    gpio_set_iomux(GPIOA, GPIO_PIN_14, 3);
    gpio_set_iomux(GPIOA, GPIO_PIN_15, 3);

    // init
    i2c_config_init(&config);
    config.settings.master.speed = I2C_CR_BUS_MODE_FAST;
    i2c_init(I2C0, &config);
    i2c_cmd(I2C0, true);

}

// I2C start transmission, addr must contain R/W bit
void I2C_start(uint8_t addr)
{
    i2c_master_send_start(I2C0, addr, I2C_WRITE);
    delay_us(100);
}

// I2C transmit one data byte via I2C
void I2C_write(uint8_t data)
{
    i2c_clear_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY);
    while (i2c_get_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY) != SET)
        ;

    i2c_send_data(I2C0, data);
}

// I2C stop transmission
void I2C_stop(void)
{
    i2c_clear_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY);
    while (i2c_get_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY) != SET)
        ;
    // stop
    i2c_master_send_stop(I2C0);
}
