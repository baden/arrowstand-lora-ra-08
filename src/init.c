#include "timer.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"
#include "tremo_pwr.h"
#include "tremo_delay.h"
#include "tremo_uart.h"
#include "rtc-board.h"

#if defined(CONFIG_DEBUG_UART)
static void uart_log_init(void)
{
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
#endif

void board_init()
{
    rcc_enable_oscillator(RCC_OSC_XO32K, true);

#if defined(CONFIG_DEBUG_UART)
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
#endif
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOD, true);
    // rcc_enable_peripheral_clk(RCC_PERIPHERAL_SYSCFG, true);     // (З прикладу I2C master dma, треба глянути для чого це)
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_PWR, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_RTC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_SAC, true);    // WTF is this?
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_LORA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_TIMER0, true);

    delay_ms(100);
    pwr_xo32k_lpm_cmd(true);
    RtcInit();

    // gpio_init(TEST_GPIOX, TEST_PIN, GPIO_MODE_OUTPUT_PP_HIGH);
#if defined(CONFIG_DEBUG_UART)
    uart_log_init();
#endif
}
