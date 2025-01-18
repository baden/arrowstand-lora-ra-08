#include <stdio.h>
#include "tremo_rcc.h"
#include "tremo_gpio.h"
#include "tremo_delay.h"
#include "tremo_uart.h"
#include "board_led.h"
#include "oled_min.h"
#include "i2c.h"
#include "adxl345.h"
#include "lora.h"

gpio_t*  g_test_gpiox = GPIOA;
uint32_t g_test_pin   = GPIO_PIN_11;

// #define TEST_GPIOX GPIOA
// #define TEST_PIN   GPIO_PIN_9

#define USE_OLED
// #define USE_ACCEL

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
    lora_init();

    board_led_init();

    I2C_init();                             // initialize I2C first

    #ifdef USE_OLED
        OLED_init();
    #endif
    // oled_init();

    #ifdef USE_ACCEL
        adxl345_init(22);
    #endif

    // RtcInit();
}

#include "boom.h"

int main(void)
{
    static unsigned counter = 0;
    #ifdef USE_OLED
    char buf[32];
    #endif

    #ifdef USE_ACCEL
        static unsigned trigged = 0;
    #endif

    board_init();

    printf("LoRa RA-08 ArrowStand sensor Start!\r\n");

    // OLED_clear();
    // OLED_print(0, 0, "Hello World", 1, 1);
    #ifdef USE_OLED
        OLED_fill(0);
    #endif

    board_led_rgb(0, 0, 0);

    /* Infinite loop */
    while (1) {
        delay_ms(50);
        printf("Tick: %d\r\n", counter++);

        #ifdef USE_ACCEL
            if(adxl345_is_active()) {
                trigged = 50;
            }

            if(trigged) {
                trigged--;
                board_led_rgb(counter & (1<<0), counter & (1<<1), counter & (1<<2));
                if(trigged == 0) board_led_rgb(0, 0, 0);
            }
        #endif


        #ifdef USE_OLED
            #ifdef USE_ACCEL
                snprintf(buf, sizeof(buf), "%s:%d", (trigged)?"*":"0", counter);
            #else
                snprintf(buf, sizeof(buf), "%d", counter);
            #endif
            // OLED_setpos(0, 0);
            // OLED_print(buf);

            // OLED_setpos(0, 1);
            // OLED_print(buf);
            // OLED_plotChar('A');

            OLED_prints(0, 0, buf);
        #endif

        // OLED_draw_bmp(0,0,32,4,boom);

        // OLED_draw_bmp(0, 0, uint8_t x1, uint8_t y1, const uint8_t* bmp);
        #if 0
        OLED_fill(counter);
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
