#include "i2c.h"
#include "tremo_rcc.h"
#include "tremo_gpio.h"
#include "tremo_i2c.h"
#include "tremo_delay.h"
#include "tremo_dma.h"
#include "tremo_dma_handshake.h"
#include <stdint.h>
#include <stdbool.h>

#define OLED_ADDR  0x3C
static volatile int is_tx_i2c_master_dma_done = 0;
static uint32_t buf[11]              = { 0x0, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };
// static uint32_t buf[3]              = { 0x0, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };
// static uint8_t buf[4]              = { (OLED_ADDR << 1) | I2C_WRITE, 0x00, 0xAE, 0x00 }; // Address + 0x00 + 0xAE

void tx_i2c_master_dma_irq_handle(void)
{
    is_tx_i2c_master_dma_done = 1;
}


#define SDA_PIN GPIOA, GPIO_PIN_15
#define SCL_PIN GPIOA, GPIO_PIN_14

#define SW_I2C_WAIT_TIME    10 // 10us = 100kHz
#define I2C_READ            0x01
#define READ_CMD            1
#define WRITE_CMD           0

#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

static void i2c_clk_data_out()
{
    gpio_write(SCL_PIN, GPIO_LEVEL_HIGH);
    delay_us(SW_I2C_WAIT_TIME);
    gpio_write(SCL_PIN, GPIO_LEVEL_LOW);
}

static void sda_out(uint8_t out)
{
    if(out) {
        gpio_write(SDA_PIN, GPIO_LEVEL_HIGH);
    } else {
        gpio_write(SDA_PIN, GPIO_LEVEL_LOW);
    }
}

static void i2c_start_condition()
{
    gpio_write(SDA_PIN, GPIO_LEVEL_HIGH);
    gpio_write(SCL_PIN, GPIO_LEVEL_HIGH);
    delay_us(SW_I2C_WAIT_TIME);
    gpio_write(SDA_PIN, GPIO_LEVEL_LOW);
    delay_us(SW_I2C_WAIT_TIME);
    gpio_write(SCL_PIN, GPIO_LEVEL_LOW);
    delay_us(SW_I2C_WAIT_TIME << 1);
}

static void i2c_stop_condition()
{
    gpio_write(SDA_PIN, GPIO_LEVEL_LOW);
    gpio_write(SCL_PIN, GPIO_LEVEL_HIGH);
    delay_us(SW_I2C_WAIT_TIME);
    gpio_write(SDA_PIN, GPIO_LEVEL_HIGH);
    delay_us(SW_I2C_WAIT_TIME);
}

static uint8_t SW_I2C_ReadVal_SDA()
{   
    // return d->hal_io_ctl(HAL_IO_OPT_GET_SDA_LEVEL, NULL);
    return gpio_read(SDA_PIN);
}

static uint8_t i2c_check_ack()
{
    uint8_t ack;
    int i;
    unsigned int temp;
    // d->hal_io_ctl(HAL_IO_OPT_SET_SDA_INPUT, NULL);
    gpio_init(SDA_PIN, GPIO_MODE_INPUT_PULL_UP);
    delay_us(SW_I2C_WAIT_TIME);
    gpio_write(SCL_PIN, GPIO_LEVEL_HIGH);
    ack = 0;
    delay_us(SW_I2C_WAIT_TIME);
    for (i = 10; i > 0; i--)
    {
        temp = !(SW_I2C_ReadVal_SDA());
        if (temp)
        {
            ack = 1;
            break;
        }
    }
    gpio_write(SCL_PIN, GPIO_LEVEL_LOW);
    // d->hal_io_ctl(HAL_IO_OPT_SET_SDA_OUTPUT, NULL);
    gpio_init(SDA_PIN, GPIO_MODE_OUTPUT_OD_HIZ);
    delay_us(SW_I2C_WAIT_TIME);
    return ack;
}

// static void i2c_check_not_ack()
// {
//     // TODO
//     return 0;
// }

static void i2c_register_address(uint8_t addr)
{
    int x;

    gpio_write(SCL_PIN, GPIO_LEVEL_LOW);

    for (x = 7; x >= 0; x--)
    {
        sda_out(addr & (1 << x));
        delay_us(SW_I2C_WAIT_TIME);
        i2c_clk_data_out();
    }
}

static void i2c_slave_address(uint8_t IICID, uint8_t readwrite)
{
    int x;

    if (readwrite) {
        IICID |= I2C_READ;
    } else {
        IICID &= ~I2C_READ;
    }

    gpio_write(SCL_PIN, GPIO_LEVEL_LOW);

    for (x = 7; x >= 0; x--) {
        sda_out(IICID & (1 << x));
        delay_us(SW_I2C_WAIT_TIME);
        i2c_clk_data_out();
    }
}

static void SW_I2C_Write_Data(uint8_t data)
{
    int x;
    gpio_write(SCL_PIN, GPIO_LEVEL_LOW);
    for (x = 7; x >= 0; x--)
    {
        sda_out(data & (1 << x));
        delay_us(SW_I2C_WAIT_TIME);
        i2c_clk_data_out();
    }
}

uint8_t SW_I2C_Write_8addr(uint8_t IICID, uint8_t regaddr, uint8_t *pdata, uint8_t rcnt)
{
    uint8_t returnack = TRUE;
    uint8_t index;
    if(!rcnt) return FALSE;
    i2c_start_condition();
    i2c_slave_address(IICID, WRITE_CMD);
    if (!i2c_check_ack()) { returnack = FALSE; }
    delay_us(SW_I2C_WAIT_TIME);
    i2c_register_address(regaddr);
    if (!i2c_check_ack()) { returnack = FALSE; }
    delay_us(SW_I2C_WAIT_TIME);
    for ( index = 0 ; index < rcnt ; index++)
    {
        SW_I2C_Write_Data(pdata[index]);
        if (!i2c_check_ack()) { returnack = FALSE; }
        delay_us(SW_I2C_WAIT_TIME);
    }
    i2c_stop_condition();
    return returnack;    
}

void I2C_init(void)
{
    gpio_write(SDA_PIN, GPIO_LEVEL_HIGH);
    gpio_write(SCL_PIN, GPIO_LEVEL_HIGH);
    gpio_init(GPIOA, GPIO_PIN_14, GPIO_MODE_OUTPUT_OD_HIZ);
    gpio_init(GPIOA, GPIO_PIN_15, GPIO_MODE_OUTPUT_OD_HIZ);
    gpio_write(SDA_PIN, GPIO_LEVEL_HIGH);
    gpio_write(SCL_PIN, GPIO_LEVEL_HIGH);
    delay_ms(1);

    uint8_t data = 1;
    SW_I2C_Write_8addr(0x3C<<1, 0, &data, sizeof(data));
}

void I2C_init2(void)
{
    dma_dev_t dma_dev;
    uint32_t slave_addr = 0x3C;
    i2c_config_t config;

    // enable the clk
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_I2C0, true);
    // rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_DMA0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_DMA1, true);

    gpio_init(GPIOA, GPIO_PIN_14, GPIO_MODE_OUTPUT_OD_HIZ);
    gpio_init(GPIOA, GPIO_PIN_15, GPIO_MODE_OUTPUT_OD_HIZ);

    // set iomux
    gpio_set_iomux(GPIOA, GPIO_PIN_14, 3);
    gpio_set_iomux(GPIOA, GPIO_PIN_15, 3);

    buf[0] = (1 << 11) | (0 << 10) | (0 << 9) | (1 << 8) | (slave_addr << 1) | I2C_WRITE;
    for (int i = 1; i < 10; i++) {
        buf[i] |= (1 << 11) | (0 << 10) | (0 << 9) | (0 << 8);
    }
    buf[10] |= (1 << 11) | (0 << 10) | (1 << 9) | (0 << 8);

    // init
    i2c_config_init(&config);
    config.fifo_mode_en = true;
    // config.settings.master.speed = I2C_CR_BUS_MODE_FAST;
    i2c_init(I2C0, &config);
    i2c_cmd(I2C0, true);


    // buf[0] = (1 << 11) | (0 << 10) | (0 << 9) | (1 << 8) | (slave_addr << 1) | I2C_WRITE;


    dma_dev.dma_num    = 0;
    dma_dev.ch         = 0;
    dma_dev.mode       = M2P_MODE;
    dma_dev.src        = (uint32_t)(buf);
    dma_dev.dest       = (uint32_t) & (I2C0->WFIFO);
    dma_dev.priv       = tx_i2c_master_dma_irq_handle;
    dma_dev.data_width = 2;
    dma_dev.block_size = 11;
    dma_dev.src_msize  = 0;
    dma_dev.dest_msize = 0;
    dma_dev.handshake  = DMA_HANDSHAKE_I2C_0_TX;

    dma_init(&dma_dev);
    dma_ch_enable(dma_dev.dma_num, 0);

    i2c_dma_cmd(I2C0, true);

    while (is_tx_i2c_master_dma_done == 0) {
        ;
    }

    while (i2c_get_flag_status(I2C0, I2C_FLAG_TRANS_DONE) != SET)
        ;
    i2c_clear_flag_status(I2C0, I2C_FLAG_TRANS_DONE);

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
