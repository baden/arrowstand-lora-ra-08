#include <stdio.h>
#include <string.h>
#include "tremo_rcc.h"
#include "tremo_gpio.h"
#include "tremo_delay.h"
#include "tremo_uart.h"
#include "tremo_timer.h"
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
#define KEY_LEFT_PIN    GPIOA, GPIO_PIN_8
#define KEY_RIGHT_PIN   GPIOA, GPIO_PIN_9

static void gptimer_simple_timer(timer_gp_t* TIMERx);

static void init()
{
    board_init();
    lora_init();
    board_led_init();
    I2C_init();                             // initialize I2C first
    OLED_init();

    gptimer_simple_timer(TIMER0);
    NVIC_EnableIRQ(TIMER0_IRQn);
    NVIC_SetPriority(TIMER0_IRQn, 2);

    gpio_init(KEY_LEFT_PIN, GPIO_MODE_INPUT_PULL_UP);
    gpio_init(KEY_RIGHT_PIN, GPIO_MODE_INPUT_PULL_UP);
}

// static uint8_t fac_us  = 0;
// static uint16_t fac_ms = 0;

volatile unsigned tick = 0;

void gptim0_IRQHandler(void)
{
    bool state;

    timer_get_status(TIMER0, TIMER_SR_UIF, &state);

    if (state) {
        timer_clear_status(TIMER0, TIMER_SR_UIF);

        tick++;
    }
}

/*
    Швидкий план роботи приймача:
    1. Чекаємо посилку від датчика.
    2. При отриманні посилки від датчика, відображаємо шось на єкрані.
*/

typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT
} States_t;

volatile States_t State = LOWPOWER;

#define BUFFER_SIZE                                 5 // Define the payload size here
uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];
int8_t RssiValue = 0;
int8_t SnrValue = 0;

const char* progress = "/-\\|";

int main(void)
{
    // static unsigned counter = 0;
    static unsigned receives = 0;
    static unsigned off_after = 0;
    static gpio_level_t key_left_prev = GPIO_LEVEL_HIGH;
    static gpio_level_t key_right_prev = GPIO_LEVEL_HIGH;
    static int correction = 0;

    // uint32_t clk_freq = rcc_get_clk_freq(RCC_HCLK);
    // uint32_t tick_rate = 1000;
    // fac_us = clk_freq / 1000000;
    // fac_ms = 1000 / tick_rate;

    char buf[32];
    init();
    printf(
        "LoRa RA-08 ArrowStand sensor Start!\r\n"
        "Role: Receiver\r\n"
    );

    OLED_fill(0x00);
    board_led_rgb(0, 0, 0);

    // Запускаємо приймач. Він буде постійно у режимі прийому. Замість випадків коли треба керувати датчиком.
    Radio.Rx( 0 /*RX_TIMEOUT_VALUE*/ );

    while (1) {
        // lora_loop();

        // delay_ms(100);
        // printf("Tick: %d\r\n", counter++);

        if(tick) {
            tick--;

            if(off_after) {
                off_after--;
                if(off_after == 0) {
                    OLED_fillU(0x00);
                    board_led_rgb(0, 0, 0);
                }
            }
            // counter++;
            // snprintf(buf, sizeof(buf),
            //     "%c:%d"
            //     , progress[counter & 3]
            //     , receives
            // );
            // OLED_prints(0, 0, buf);

            if(gpio_read(KEY_LEFT_PIN) == GPIO_LEVEL_LOW) {
                if(key_left_prev == GPIO_LEVEL_HIGH) {
                    key_left_prev = GPIO_LEVEL_LOW;
                    // Натиснута ліва кнопка

                    correction--;
                    snprintf(buf, sizeof(buf),
                        "<%3d >"
                        , correction
                    );
                    OLED_prints(0, 0, buf);
                    off_after = 10;      // Через 1 сек погасить


                }
            } else {
                key_left_prev = GPIO_LEVEL_HIGH;
            }

            if(gpio_read(KEY_RIGHT_PIN) == GPIO_LEVEL_LOW) {
                if(key_right_prev == GPIO_LEVEL_HIGH) {
                    key_right_prev = GPIO_LEVEL_LOW;
                    // Натиснута права кнопка

                    correction++;
                    snprintf(buf, sizeof(buf),
                        "<%3d >"
                        , correction
                    );
                    OLED_prints(0, 0, buf);
                    off_after = 10;      // Через 1 сек погасить

                }
            } else {
                key_right_prev = GPIO_LEVEL_HIGH;
            }

        }
        // board_led_rgb(0, 0, 0);

        switch( State ) {
        case RX:
            printf(" [RX]");
            receives++;
            board_led_rgb(1, 1, 1);

            if( BufferSize > 0 ) {
                snprintf(buf, sizeof(buf),
                    "%3d %d"
                    , Buffer[4]     // Sensitivity?
                    , receives
                );
                OLED_prints(0, 0, buf);

                // OLED_setpos(0, 3);
                snprintf(buf, sizeof(buf),
                    "RSSI:%4d SNR:%d"
                    , RssiValue
                    , SnrValue
                );
                OLED_printsS(0, 3, buf);

                off_after = 5;      // Через 0.5 сек погасить
            }

            // if( BufferSize > 0 ) {
            //     printf("Received: [%s]\r\n", Buffer);
            //     Radio.Rx( RX_TIMEOUT_VALUE );
            // }
            Radio.Rx( 0 /*RX_TIMEOUT_VALUE*/ );
            State = LOWPOWER;
            break;

        case TX:
            printf(" [TX]");
            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case RX_TIMEOUT:
            printf(" [RX_TIMEOUT]");
            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case RX_ERROR:
            printf(" [RX_ERROR]");
            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case TX_TIMEOUT:
            printf(" [TX_TIMEOUT]");
            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case LOWPOWER:
            break;

        default:
        }

        Radio.IrqProcess();
    }
}

void OnTxDone( void )
{
    Radio.Sleep();
    printf("OnTxDone\r\n");
    State = TX;
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.Sleep( );
    printf("OnRxDone (%d, %d, %d)\r\n", size, rssi, snr);
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
    State = RX;
}

void OnTxTimeout( void )
{
    printf("OnTxTimeout\r\n");
    Radio.Sleep();
    State = TX_TIMEOUT;
}

void OnRxTimeout( void )
{
    printf("OnRxTimeout\r\n");
    Radio.Sleep( );
    State = RX_TIMEOUT;
}

void OnRxError( void )
{
    printf("OnRxError\r\n");
    Radio.Sleep( );
    State = RX_ERROR;
}


#ifdef USE_FULL_ASSERT
void assert_failed(void* file, uint32_t line)
{
    (void)file;
    (void)line;

    while (1) { }
}
#endif

// 1 тік буде 1/10 сек

static void gptimer_simple_timer(timer_gp_t* TIMERx)
{
    timer_init_t timerx_init;

    timer_config_interrupt(TIMERx, TIMER_DIER_UIE, ENABLE);

    timerx_init.prescaler          = (24000/10)-1 /*23999*/;  //sysclock defaults to 24M, is divided by (prescaler + 1) to 1k
    timerx_init.counter_mode       = TIMER_COUNTERMODE_UP;
    timerx_init.period             = 1000;   //time period is ((1 / 1k) * 1000) 
    timerx_init.clock_division     = TIMER_CKD_FPCLK_DIV1;
    timerx_init.autoreload_preload = false;
    timer_init(TIMERx, &timerx_init);

    timer_generate_event(TIMERx, TIMER_EGR_UG, ENABLE);
    timer_clear_status(TIMER0, TIMER_SR_UIF);

    timer_cmd(TIMERx, true);

}
