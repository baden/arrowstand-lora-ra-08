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
// uint2_t g_test_pin   = GPIO_PIN_11;

// #define TEST_GPIOX GPIOA
// #define TEST_PIN   GPIO_PIN_9

// gpio_t *g_test_gpiox = GPIOA;
// uint8_t g_test_pin = GPIO_PIN_11;
// volatile uint32_t g_gpio_interrupt_flag = 0;

static void gptimer_simple_timer(timer_gp_t* TIMERx);

static uint8_t sencitive = 22;

static void init()
{
    board_init();
    lora_init();
    board_led_init();
    I2C_init();                             // initialize I2C first

    adxl345_init(sencitive);
    gpio_init(GPIOA, GPIO_PIN_11, GPIO_MODE_INPUT_PULL_UP);
    gpio_config_interrupt(GPIOA, GPIO_PIN_11, GPIO_INTR_FALLING_EDGE);
    /* NVIC config */
    NVIC_EnableIRQ(GPIO_IRQn);
    NVIC_SetPriority(GPIO_IRQn, 2);

    // Задіємо кнопку BOOT для керування режимами роботи
    gpio_init(GPIOA, GPIO_PIN_2, GPIO_MODE_INPUT_PULL_DOWN);
    gpio_config_interrupt(GPIOA, GPIO_PIN_2, GPIO_INTR_RISING_EDGE);

    gptimer_simple_timer(TIMER0);
    NVIC_EnableIRQ(TIMER0_IRQn);
    NVIC_SetPriority(TIMER0_IRQn, 2);
}

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
    Швидкий план роботи датчика:
    1. Чекаємо сигнал від акселерометра.
    2. При отриманні - передаємо посилку в LoRa.
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

static volatile bool activity_trigged = false;
static volatile bool boot_trigged = false;

volatile bool tx_in_progress = false;
volatile bool rx_in_progress = false;

enum mode {
    MODE_SENSOR = 0,
    MODE_TESTER = 1
};

enum mode mode = MODE_SENSOR;

// Протягом 0.5 сек після відправки може прийти корекція чутливості
#define RX_TIMEOUT_VALUE                            500

int main(void)
{
    static unsigned counter = 0;
    static unsigned trigged = 0;

    init();
    printf(
        "LoRa RA-08 ArrowStand sensor Start!\r\n"
        "Role: Sensor\r\n"
    );

    board_led_rgb(0, 1, 0);
    delay_ms(1000);
    board_led_rgb(0, 0, 0);

    Radio.Sleep();  // Чи треба?

    /* Infinite loop */
    while (1) {
        if(boot_trigged) {
            boot_trigged = false;

            mode = (mode==MODE_SENSOR) ? MODE_TESTER : MODE_SENSOR;
        }

        switch (mode)
        {
        case MODE_SENSOR:
            /* code */
            if(activity_trigged || (gpio_read(GPIOA, GPIO_PIN_11) == GPIO_LEVEL_LOW)) {
                adxl345_is_active();    // Clear INT flag
                activity_trigged = false;
                trigged = 30;

                if(!tx_in_progress && !rx_in_progress) {
                    tx_in_progress = true;
                    Buffer[0] = 'P';
                    Buffer[1] = 'I';
                    Buffer[2] = 'N';
                    Buffer[3] = 'G';
                    Buffer[4] = sencitive;
                    // srand( *ChipId );
                    // uint8_t random = ( rand() + 1 ) % 90;
                    // DelayMs( random );
                    Radio.Send( Buffer, BufferSize );
                    printf("Sent: PING\r\n");
                }
            }

            if(trigged) {
                trigged--;
                board_led_rgb(counter & (1<<0), counter & (1<<1), counter & (1<<2));
                counter++;
                if(trigged == 0) board_led_rgb(0, 0, 0);
            }
            break;
        
        case MODE_TESTER:
            if(tick) {
                tick--;

                if(!tx_in_progress && !rx_in_progress) {
                    tx_in_progress = true;
                    Buffer[0] = 'P';
                    Buffer[1] = 'I';
                    Buffer[2] = 'N';
                    Buffer[3] = 'G';
                    Buffer[4] = counter++;
                    // srand( *ChipId );
                    // uint8_t random = ( rand() + 1 ) % 90;
                    // DelayMs( random );
                    Radio.Send( Buffer, BufferSize );
                    printf("Sent: PING\r\n");
                }

                board_led_rgb(counter & (1<<0), counter & (1<<1), counter & (1<<2));
            }
            break;
        }

        switch( State ) {
        case RX:
            printf(" [RX]");
            rx_in_progress = false;
            if( BufferSize > 0 ) {
                // Отримання корекції чутливості?
                if(Buffer[0] == 'C') {
                    int correction = Buffer[1];
                    correction -= 64;
                    sencitive += correction;

                    adxl345_setSensitivity(sencitive);
                    // TODO: Save to NVM
                }
            //     printf("Received: [%s]\r\n", Buffer);
            //     Radio.Rx( RX_TIMEOUT_VALUE );
            }
            State = LOWPOWER;
            break;

        case TX:
            printf(" [TX]");
            rx_in_progress = true;
            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case RX_TIMEOUT:
            printf(" [RX_TIMEOUT]");
            rx_in_progress = false;
            // Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case RX_ERROR:
            printf(" [RX_ERROR]");
            rx_in_progress = false;
            // Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case TX_TIMEOUT:
            printf(" [TX_TIMEOUT]");
            // Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case LOWPOWER:
            break;

        default:
            break;
        }

        Radio.IrqProcess();

        // gpio_toggle(TEST_GPIOX, TEST_PIN);
    }
}

void OnTxDone( void )
{
    printf("OnTxDone\r\n");
    Radio.Sleep();
    State = TX;
    // State = LOWPOWER;
    tx_in_progress = false;
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    printf("OnRxDone %d [%02x,%02x]\r\n", size, payload[0], payload[1]);
    Radio.Sleep( );

    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    // RssiValue = rssi;
    // SnrValue = snr;
    State = RX;
}

void OnTxTimeout( void )
{
    printf("OnTxTimeout\r\n");
    Radio.Sleep();
    State = TX_TIMEOUT;
    tx_in_progress = false;
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
    timer_clear_status(TIMERx, TIMER_SR_UIF);

    timer_cmd(TIMERx, true);
}


#ifdef USE_FULL_ASSERT
void assert_failed(void* file, uint32_t line)
{
    (void)file;
    (void)line;

    while (1) { }
}
#endif

void GPIO_IRQHandler(void)
{
    if (gpio_get_interrupt_status(GPIOA, GPIO_PIN_11) == SET) {
        gpio_clear_interrupt(GPIOA, GPIO_PIN_11);
        activity_trigged = true;
    }

    // BOOT key
    if (gpio_get_interrupt_status(GPIOA, GPIO_PIN_2) == SET) {
        gpio_clear_interrupt(GPIOA, GPIO_PIN_2);
        boot_trigged = true;
    }
}
