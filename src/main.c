#include <stdio.h>
#include "tremo_rcc.h"
#include "tremo_gpio.h"
#include "tremo_delay.h"
#include "tremo_uart.h"
#include "board_led.h"
#include "oled_min.h"
#include "i2c.h"

// #define TEST_GPIOX GPIOA
// #define TEST_PIN   GPIO_PIN_9

static void uart_log_init(void)
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);

    // uart0
    gpio_set_iomux(GPIOB, GPIO_PIN_0, 1);
    gpio_set_iomux(GPIOB, GPIO_PIN_1, 1);

    /* uart config struct init */
    uart_config_t uart_config;
    uart_config_init(&uart_config);

    uart_config.baudrate = UART_BAUDRATE_115200;
    uart_init(CONFIG_DEBUG_UART, &uart_config);
    uart_cmd(CONFIG_DEBUG_UART, ENABLE);
}

static void board_init()
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true);
    
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_SYSCFG, true);     // (З прикладу I2C master dma, треба глянути для чого це)
    // gpio_init(TEST_GPIOX, TEST_PIN, GPIO_MODE_OUTPUT_PP_HIGH);
    // rcc_enable_oscillator(RCC_OSC_XO32K, true);

    // LOG
    // rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
    // rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOC, true);
    // rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOD, true);
    // rcc_enable_peripheral_clk(RCC_PERIPHERAL_PWR, true);
    // rcc_enable_peripheral_clk(RCC_PERIPHERAL_RTC, true);
    // rcc_enable_peripheral_clk(RCC_PERIPHERAL_SAC, true);
    // rcc_enable_peripheral_clk(RCC_PERIPHERAL_LORA, true);

    // delay_ms(100);
    // pwr_xo32k_lpm_cmd(true);

    uart_log_init();

    board_led_init();
    OLED_init();
    // oled_init();

    // RtcInit();
}

uint32_t __attribute__ ((aligned(4))) OLED_buffer[OLED_WIDTH * OLED_HEIGHT / 8 + 2];

int main(void)
{
    static unsigned counter = 0;
    board_init();

    printf("LoRa RA-08 ArrowStand sensor Start!\r\n");

#if 0
    // Clear screen buffer
    OLED_buffer[0] = DMA_START | (OLED_ADDR << 1);
    OLED_buffer[1] = DMA_DATA  | OLED_DAT_MODE;
    for(size_t i = 2; i < OLED_WIDTH * OLED_HEIGHT / 8 + 1; i++) {
        OLED_buffer[1] = DMA_DATA;
    }
    OLED_buffer[OLED_WIDTH * OLED_HEIGHT / 8 + 1] = DMA_LAST;
    I2C_writeDMA(OLED_buffer, OLED_WIDTH * OLED_HEIGHT / 8 + 2);
#endif
    // OLED_clear();
    // OLED_print(0, 0, "Hello World", 1, 1);
    // OLED_fill(0xAA);

    /* Infinite loop */
    while (1) {
        delay_ms(1000);
        printf("Tick: %d\r\n", counter++);

        board_led_rgb(counter & (1<<0), counter & (1<<1), counter & (1<<2));

        #if 0
        // OLED_fill(counter);
        // OLED_draw_buffer();
        OLED_buffer[0] = OLED_DAT_MODE;
        OLED_buffer[1] = counter & 0xFF;
        OLED_buffer[2] = (counter >> 8) & 0xFF;
        I2C_writeBuf(OLED_ADDR, OLED_buffer, sizeof(OLED_buffer));
        #endif

        // gpio_toggle(TEST_GPIOX, TEST_PIN);
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(void* file, uint32_t line)
{
    (void)file;
    (void)line;

    while (1) { }
}
#endif
