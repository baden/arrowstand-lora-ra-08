#include <stdio.h>
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
// #define TEST_PIN   GPIO_PIN_9

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

const char* progress = "/-\\|";

int main(void)
{
    static unsigned counter = 0;
    static unsigned receives = 0;

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
            counter++;
            snprintf(buf, sizeof(buf),
                "%c:%d"
                , progress[counter & 3]
                , receives
            );
            OLED_prints(0, 0, buf);
        }

        switch( State ) {
        case RX:
            printf(" [RX]");
            receives++;
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
    printf("OnTxDone\r\n");
    Radio.Sleep();
    State = TX;
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    printf("OnRxDone\r\n");
    Radio.Sleep( );

/*
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
*/
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


static void gptimer_simple_timer(timer_gp_t* TIMERx)
{
    timer_init_t timerx_init;

    timer_config_interrupt(TIMERx, TIMER_DIER_UIE, ENABLE);

    timerx_init.prescaler          = 23999;  //sysclock defaults to 24M, is divided by (prescaler + 1) to 1k
    timerx_init.counter_mode       = TIMER_COUNTERMODE_UP;
    timerx_init.period             = 1000;   //time period is ((1 / 1k) * 1000) 
    timerx_init.clock_division     = TIMER_CKD_FPCLK_DIV1;
    timerx_init.autoreload_preload = false;
    timer_init(TIMERx, &timerx_init);

    timer_generate_event(TIMERx, TIMER_EGR_UG, ENABLE);
    timer_clear_status(TIMER0, TIMER_SR_UIF);

    timer_cmd(TIMERx, true);

}
