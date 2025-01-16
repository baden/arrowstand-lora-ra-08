#include "i2c.h"
#include "tremo_rcc.h"
#include "tremo_gpio.h"
#include "tremo_i2c.h"
#include "tremo_delay.h"
#include "tremo_dma.h"
#include "tremo_dma_handshake.h"
#include <stdbool.h>

#define OLED_ADDR  0x3C

#define I2C_OVER_DMA

#if defined(I2C_OVER_DMA)
static volatile int is_tx_i2c_master_dma_done = 0;
// static uint32_t buf[11]              = { 0x0, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };
#endif
// static uint32_t buf[3]              = { 0x0, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };
// static uint8_t buf[4]              = { (OLED_ADDR << 1) | I2C_WRITE, 0x00, 0xAE, 0x00 }; // Address + 0x00 + 0xAE

#if defined(I2C_OVER_DMA)
void tx_i2c_master_dma_irq_handle(void)
{
    is_tx_i2c_master_dma_done = 1;
}
#endif

void I2C_init(void)
{
    i2c_config_t config;

    // enable the clk
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_I2C0, true);
    // rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
#if defined(I2C_OVER_DMA)
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_DMA0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_DMA1, true);
#endif
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
    // delay_us(100);
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

void I2C_writeDMA(const uint32_t* buf, size_t buf_size)
{
#if defined(I2C_OVER_DMA)
    dma_dev_t dma_dev;
    // uint32_t slave_addr = 0x3C;

/*
    buf[0] = (1 << 11) | (0 << 10) | (0 << 9) | (1 << 8) | (slave_addr << 1) | I2C_WRITE;
    for (int i = 1; i < 10; i++) {
        buf[i] |= (1 << 11) | (0 << 10) | (0 << 9) | (0 << 8);
    }
    buf[10] |= (1 << 11) | (0 << 10) | (1 << 9) | (0 << 8);
*/

    dma_dev.dma_num    = 0;
    dma_dev.ch         = 0;
    dma_dev.mode       = M2P_MODE;
    dma_dev.src        = (uint32_t)(buf);
    dma_dev.dest       = (uint32_t) & (I2C0->WFIFO);
    dma_dev.priv       = tx_i2c_master_dma_irq_handle;
    dma_dev.data_width = 2;
    dma_dev.block_size = buf_size;
    dma_dev.src_msize  = 0;
    dma_dev.dest_msize = 0;
    dma_dev.handshake  = DMA_HANDSHAKE_I2C_0_TX;

    dma_init(&dma_dev);
    dma_ch_enable(dma_dev.dma_num, 0);

    i2c_dma_cmd(I2C0, true);

    while (is_tx_i2c_master_dma_done == 0) continue;

    while (i2c_get_flag_status(I2C0, I2C_FLAG_TRANS_DONE) != SET) continue;
    i2c_clear_flag_status(I2C0, I2C_FLAG_TRANS_DONE);
#else
    #error Dont!
    // I2C_start(addr);
    // for(size_t i = 0; i < buf_size; i++)
    //     I2C_write(buf[i]);
    // I2C_stop();
#endif
}
