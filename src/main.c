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
#include "init.h"
#include "role.h"
#include "radio.h"

// gpio_t*  g_test_gpiox = GPIOA;
// uint32_t g_test_pin   = GPIO_PIN_11;

// #define TEST_GPIOX GPIOA
// #define TEST_PIN   GPIO_PIN_9



static void init()
{
    board_init();
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
}

int main(void)
{
    static unsigned counter = 0;
    #ifdef USE_OLED
    char buf[32];
    #endif

    #ifdef USE_ACCEL
        static unsigned trigged = 0;
    #endif

    init();

    printf("LoRa RA-08 ArrowStand sensor Start!\r\n");

    #ifdef USE_OLED
        OLED_fill(0x00);
    #endif

    board_led_rgb(0, 1, 0);

    #if ROLE==ROLE_RX
    Radio.Rx( RX_TIMEOUT_VALUE );
    #endif

    /* Infinite loop */
    while (1) {
        lora_loop();

        // delay_ms(100);
        // printf("Tick: %d\r\n", counter++);

        #ifdef USE_ACCEL
            if(adxl345_is_active()) {
                trigged = 30;
            }

            if(trigged) {
                trigged--;
                board_led_rgb(counter & (1<<0), counter & (1<<1), counter & (1<<2));
                counter++;
                if(trigged == 0) board_led_rgb(0, 0, 0);
            }
        #endif


        #ifdef USE_OLED
            #ifdef USE_ACCEL
                snprintf(buf, sizeof(buf), "%s:%d", (trigged)?"*":"0", counter++);
            #else
                snprintf(buf, sizeof(buf), "%d", counter++);
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
