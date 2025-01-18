#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "tremo_uart.h"
#include "tremo_gpio.h"
#include "tremo_pwr.h"
#include "tremo_rcc.h"
#include "delay.h"
#include "timer.h"
#include "sx126x-board.h"
#include "sx126x.h"
#include "radio.h"
#include "rtc-board.h"
#include "tremo_delay.h"


void lora_init()
{
    // TODO: Можливо тут не місце для цього
    rcc_enable_oscillator(RCC_OSC_XO32K, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_RTC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_LORA, true);

    delay_ms(100);
    pwr_xo32k_lpm_cmd(true);
    RtcInit();
}