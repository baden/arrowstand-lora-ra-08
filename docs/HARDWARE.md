# Hardware — розпіновка, периферія, SoC

> Доповнення до `AGENTS.md`. Деталі заліза: модуль RA-08, SoC ASR Tremo,
> зовнішні компоненти (ADXL345, OLED), розводи таймера/радіо.

---

## 1. Модуль AI-Thinker RA-08

Модуль інтегрує:

- **SoC**: ASR Microelectronics Tremo (ARM Cortex-M4F, FPU).
  - Flash 128 КБ @ `0x08000000`
  - RAM 16 КБ @ `0x20000000`, верх стека `0x20004000`
  - Default SystemCoreClock: 24 МГц (див. `platform/system/system_cm4.c`)
- **Радіо**: Semtech SX1262 (LoRa, 150–960 МГц).
- **Кварц**: 32 кГц (XO32K) для RTC.

Розпіновка модуля (з `README.md`):

| Pin | Назва         | Альтернативна функція                  | Використання в проєкті               |
|-----|---------------|---------------------------------------|--------------------------------------|
| 1,17| GND           | —                                     | —                                    |
| 2   | GPIO8         | ADC_IN1                               | KEY_LEFT (RX), GPIOA_8, PULL_UP       |
| 3   | GPIO11        | ADC_IN0                               | INT1 ADXL345 (TX), GPIOA_11, EXTI    |
| 4   | GPIO9         | DAC_OUT                               | KEY_RIGHT (RX), GPIOA_9, PULL_UP      |
| 5   | GPIO4         | —                                     | LED G, GPIOA_4                        |
| 6   | GPIO5         | SSPI_NSS                              | LED R, GPIOA_5                        |
| 7   | GPIO7         | SWCLK                                 | LED B, GPIOA_7                        |
| 8   | GPIO6         | SWDIO                                 | — (SWD debug)                         |
| 9   | VCC 3.3V      | —                                     | живлення                              |
| 10  | LPRXD         | GPIO60/LPUART_RX                      | —                                    |
| 11  | TXD           | GPIO17/PORTB_1, UART0_TXD (fun=1)     | UART0 TX для логів                    |
| 12  | RXD           | GPIO16/PORTB_0, UART0_RXD (fun=1)     | UART0 RX (не задіяний)                |
| 13  | GPIO15        | I2C0_SDA (alt=3)                      | I2C0 SDA → OLED + ADXL345            |
| 14  | GPIO14        | I2C0_SCL (alt=3)                      | I2C0 SCL                              |
| 15  | GPIO2         | BOOT                                  | BOOT кнопка (TX), GPIOA_2, PULL_DOWN |
| 16  | RST           | —                                     | апаратний reset (через RTS)           |
| 18  | ANT           | —                                     | антена LoRa                           |

> Увага: для I2C використовуються GPIOA_14/15, хоча в README фігурують GPIO14/15 —
> це той самий GPIOA (Port A), просто GPIO14 = pin 14 на порту A = GPIOA_14.

---

## 2. SoC ASR Tremo — периферія

| Модуль      | Регістр / IRQ            | Де у коді                                |
|-------------|---------------------------|-------------------------------------------|
| GPIO        | `RCC_PERIPHERAL_GPIOx`    | `tremo_gpio.c`, `tremo_gpio.h`            |
| UART0       | `CONFIG_DEBUG_UART`, log  | `init.c::uart_log_init`, 115200 8N1       |
| I2C0        | `RCC_PERIPHERAL_I2C0`     | `src/i2c.c`, polling, FAST mode (400 kHz) |
| TIMER0/1/2/3| `RCC_PERIPHERAL_TIMERx`   | `tremo_timer.c`, `main_*.c::gptim0_*`     |
| LORA (SPI)  | `RCC_PERIPHERAL_LORA`     | `lora/driver/sx1262-board.c`, `LORAC->...`|
| RTC         | `RCC_PERIPHERAL_RTC`      | `lora/driver/rtc-board.c`, `RtcInit()`    |
| PWR         | `tremo_pwr.h`             | `pwr_xo32k_lpm_cmd(true)`                 |
| SAC         | `RCC_PERIPHERAL_SAC`      | увімкнено в `board_init`, невідомо для чого |
| DMA         | `tremo_dma.c`             | не використовується (I2C на polling)      |
| LPTIMER     | `tremo_lptimer.h`         | потенціал для low-power wake-up           |

Wake-up / low-power режими:

- `tremo_pwr.h` — `pwr_xo32k_lpm_cmd`, deep sleep.
- Wake-up джерела: RTC, GPIO (EXTI), LPTIMER.

---

## 3. Зовнішні компоненти

### 3.1 ADXL345 (TX)

| Параметр          | Значення                          |
|-------------------|-----------------------------------|
| Адреса I2C        | `0x53` (7-bit, ADXL345_REG_DEVID) |
| INT1              | GPIOA_11, FALLING EDGE            |
| Activity threshold| `sencitive` (uint8, 0-255)        |
| Inactivity thresh | `sencitive - 1`                   |
| Час неактивності  | 3 с (`TIME_INACT`)                |
| Активні осі       | тільки Z                          |
| Інверсія INT      | active-low (`INT_INVERT = 1`)     |
| Low-power bit     | ON                                |

Див. `src/adxl345.c` — функції `adxl345_init(sensitivity)`,
`adxl345_setSensitivity(sensitivity)`, `adxl345_is_active()`.

### 3.2 OLED SSD1306 128×32 (RX)

| Параметр          | Значення                          |
|-------------------|-----------------------------------|
| Адреса I2C        | `0x3C` (7-bit) → `0x3C << 1 = 0x78` |
| Шина              | I2C0, FAST (400 kHz)              |
| Контролер         | SSD1306                           |
| Розмір            | 128×32 (використовується 128×32; код підтримує 128×64) |
| VCC               | internal charge pump (`0x14`)     |
| COM pins          | `0x02` (для 128×32)                |
| Contrast          | `0x8F`                            |
| Flip              | X-flipped, Y-flipped              |

API — `src/oled_min.c`:

- `OLED_init()` — ініціалізація.
- `OLED_fill(0x00)` / `OLED_fillU(0x00)` — очистка (повна / верхні 3 рядки).
- `OLED_prints(x, y, str)` — друк шрифтом `font[]` (24×32).
- `OLED_printsS(x, y, str)` — друк шрифтом `fontS[]` (5×8).
- `OLED_setpos(x, y)` / `OLED_draw_bmp(...)` — низький рівень.

### 3.3 RGB LED (на платі розширення)

| Колір | GPIO         | Pin |
|-------|--------------|-----|
| R     | GPIOA        | 5   |
| G     | GPIOA        | 4   |
| B     | GPIOA        | 7   |

API — `src/board_led.c`: `board_led_init()`, `board_led_rgb(r, g, b)`.
Активний рівень — HIGH.

---

## 4. Розводка радіо SX1262

У модулі RA-08 SX1262 підключений через внутрішній SPI-контролер
`LORAC` (не стандартний SSPx). Керування — пряме звертання до регістрів
`LORAC->CR0/1`, `LORAC->SSP_*`, `LORAC->SR`, `LORAC->NSS_CR`.

GPIO для RFSW (antenna switch):

| Сигнал      | GPIO  | Pin  | Файл визначення              |
|-------------|-------|------|-------------------------------|
| RFSW_CTRL   | GPIOD | 11   | `inc/lora_config.h` (iomux 6 для GPIO_PIN_10, інакше 3) |
| RFSW_VDD    | GPIOA | 10   | `inc/lora_config.h`            |

`SX126xAntSwOn()` / `SX126xAntSwOff()` у `lora/driver/sx1262-board.c` керують
живленням антенного світча.

---

## 5. Таймери

- **TIMER0** — програмний «тикач» в `main_*.c`:
  - TX: prescaler 23999, period 1000 → 1 тік / 1 с.
  - RX: prescaler `(24000/10)-1 = 2399`, period 1000 → 1 тік / 0.1 с
    (див. коментар «1 тік буде 1/10 сек» в `main_RX.c:301`).
- **RTC** — повільний таймер для LoRa-стека (див. `rtc-board.c`). Викликається
  з `RtcInit()` у `init.c`.
- **LPTIMER0/1** — для майбутніх режимів low-power (не використовується).

---

## 6. Діагностика / Debug

- **UART0** на 115200 8N1 (TX=PB1, RX=PB0). `printf` перенаправлено через
  кастомний `printf-stdarg.c` (бо libc викинуто з лінкера).
- **SWD** на GPIO6/GPIO7 (SWDIO/SWCLK). Не використовується у прод-прошивці,
  але доступний для зовнішнього дебагера.
- **Watchdog**: IWDG/WDG у SDK є, але в коді не активується.

---

## 7. Живлення

- **VCC**: 3.3 В через pin 9 модуля.
- **RST**: pin 16 — low-active.
- **BOOT**: pin 15 — керується DTR через завантажувач
  (`tremo_loader.py::hw_reset(mode=1)`: DTR=1→BOOT=1, RTS=0→RST=0).
- **XO32K** має бути увімкнений (`rcc_enable_oscillator(RCC_OSC_XO32K, true)`
  у `board_init`).

---

## 8. Регіони LoRa

Makefile визначає лише `-DREGION_EU433` і `RF_FREQUENCY = 433000000`.
Для зміни регіону:

1. `inc/lora_config.h` — `RF_FREQUENCY`, `TX_OUTPUT_POWER`.
2. `Makefile` — замінити `-DREGION_EU433` на потрібний.
3. Перевірити duty-cycle обмеження для регіону (EU433: 1%, EU868: 1%,
   US915: 400 мс тощо).
