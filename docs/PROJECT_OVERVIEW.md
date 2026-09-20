# Project Overview — Архітектура і потік даних

> Доповнення до `AGENTS.md`. Описує логічну архітектуру прошивки, потоки даних
> та контракти між підсистемами.

---

## 1. Дві ролі пристроїв

Проєкт підтримує дві бінарні прошивки, що збираються з одного й того самого
вихідного коду, але з різним файлом точки входу (`main_TX.c` чи `main_RX.c`).

| Властивість | `ROLE=TX` (передавач)        | `ROLE=RX` (приймач)             |
|-------------|------------------------------|---------------------------------|
| Файл main   | `src/main_TX.c`              | `src/main_RX.c`                 |
| Датчик      | ADXL345 (I2C 0x53)           | —                               |
| Дисплей     | —                            | SSD1306 OLED 128×32 (I2C 0x3C)  |
| Кнопки      | BOOT (GPIOA_2)               | LEFT (GPIOA_8) / RIGHT (GPIOA_9) |
| RGB LED     | Індикація режимів            | Індикація влучань               |
| LoRa        | Передача за тригером         | Постійний RX                    |
| Визначення  | `#define USE_ACCEL`          | `#define USE_OLED`               |

`inc/role.h` обирає правильний макрос на етапі компіляції через `-DROLE=ROLE_$(ROLE)`
з Makefile.

---

## 2. Потік даних — TX

```
[ADXL345 INT1] → [GPIOA_11 EXTI FALLING] → [GPIO_IRQHandler]
                                            ↓ activity_trigged = true
[main_TX loop] → бачить activity_trigged
                 ↓
                 формує Buffer = 'P','I','N','G', <sensitivity>
                 ↓
                 Radio.Send(Buffer, 5)
                 ↓
                 [OnTxDone] → перемикає на RX 500 мс
                              ↓
                 [OnRxDone] якщо RX у віконі
                              ↓
                 'C' → коригує sencitive, перезаписує ADXL345 THRESH_ACT
```

Таймер TIMER0 (1 кГц) використовується і для лічильника `tick` (в режимі
`MODE_TESTER`), і для дебаунсу/затримок.

Два режими роботи (перемикаються кнопкою BOOT):

- `MODE_SENSOR` — очікуємо тригер від ADXL345, передаємо PING.
- `MODE_TESTER` — кожен тік таймера передаємо пакет з лічильником.
  Використовується для перевірки радіоканалу.

---

## 3. Потік даних — RX

```
[main_RX init] → Radio.Rx(0)         // постійний прийом
[main_RX loop] → Radio.IrqProcess()  // poll IRQ
[OnRxDone] → Buffer = payload, State = RX
[main_RX switch State]:
    if RX:
        receives++
        друкуємо "<sensitivity> <receives>" та "RSSI:<x> SNR:<y>"
        якщо correction != 0:
            Buffer = 'C', <correction+64>, x, x, x
            Radio.Send(Buffer)
            correction = 0
        Radio.Rx(0)                // повертаємось у режим прийому
```

Таймер TIMER0x опитує кнопки (фільтр PULL_UP → LOW = натиснута). Лічильник
`off_after` гасить екран через N тіків (≈ 100 мс/тік у RX).

---

## 4. Формат пейлоада (потрібно узгоджувати)

| Поле          | TX → RX (`PING`)              | RX → TX (`C`)             |
|---------------|--------------------------------|---------------------------|
| Byte 0        | `'P'` (0x50)                   | `'C'` (0x43)              |
| Byte 1        | `'I'` (0x49)                   | `<correction + 64>`       |
| Byte 2        | `'N'` (0x4E)                   | `0` (unused)              |
| Byte 3        | `'G'` (0x47)                   | `0` (unused)              |
| Byte 4        | `<sensitivity>` (uint8)        | `0` (unused)              |

`BUFFER_SIZE = 5` визначено в обох `main_*.c` і `src/lora.c` (через
`RadioEvents_t` callback).

**Важливо**: при зміні `BUFFER_SIZE` треба синхронізувати і TX, і RX — інакше
`OnRxDone` прочитає лише частину пейлоада.

---

## 5. Радіо — підсистема LoRa

`src/lora.c::lora_init()` виконує:

1. `Radio.Init(&RadioEvents)` — ініціалізує SX126x через SPI (вбудований LORAC).
2. `Radio.SetChannel(RF_FREQUENCY)` — 433 МГц для EU433.
3. `Radio.SetTxConfig(...)` — `MODEM_LORA`, `TX_OUTPUT_POWER = 14 dBm`,
   `LORA_BANDWIDTH = 0 (125 kHz)`, `LORA_SPREADING_FACTOR = 7`,
   `LORA_CODINGRATE = 1 (4/5)`, таймаут TX = 3000 мс.
4. `Radio.SetRxConfig(...)` — симетрично до TX, але `RxContinuous = true`.

Файли реалізації:

- `lora/radio/radio.h` + `lora/radio/sx126x/radio.c` — Semtech Radio API
  (структура `Radio_s` з вказівниками на функції).
- `lora/radio/sx126x/sx126x.c` + `.h` — низькорівневий драйвер SX126x
  (SPI-команди, регістри, IRQ).
- `lora/driver/sx1262-board.c` — board-level обгортка: працює з `LORAC->...`
  регістрами (вбудований SPI-контролер для SX1262 на RA-08), RFSW_VDD на
  GPIOA_10, RFSW_CTRL на GPIOD_11.
- `lora/driver/rtc-board.c` — RTC + низька потужність для LoRa-стека.
- `lora/system/timer.c` + `delay.c` — програмні таймери для стека.

---

## 6. Точка входу та ініціалізація

```
Power-On → Reset_Handler (startup_cm4.S)
            ↓ копіює .data з Flash → RAM
            ↓ обнуляє .bss
            ↓ bl system_init  → SystemInit() → SystemCoreClock = 24 MHz
            ↓ bl __libc_init_array (порожній, libc викинуто)
            ↓ bl main
                ↓ static void init():
                       board_init()        → RCC, PWR, RTC, UART0 115200 8N1
                       lora_init()         → Radio.Init, SetChannel, Set{tx,rx}Config
                       board_led_init()    → GPIOA 4/5/7 як RGB
                       I2C_init()          → I2C0 400 кГц (FAST), SDA GPIOA_15, SCL GPIOA_14
                       OLED_init() [RX]    або adxl345_init(sensitivity) [TX]
                       + GPIO IRQ для ACCEL/BOOT [TX]
                       + TIMER0 1 кГц [TX] / 100 Гц [RX]
```

Стек = 4 КБ (`_STACK_SIZE = 0x1000`), починається з `0x20004000`.

---

## 7. Стани (state machine)

Спільний для обох ролей `enum States_t`:

```c
typedef enum { LOWPOWER, RX, RX_TIMEOUT, RX_ERROR, TX, TX_TIMEOUT } States_t;
```

Це копія Semtech PingPong-демки. Логіка:

- `LOWPOWER` → просто крутимо цикл, дозволяємо `Radio.IrqProcess()`.
- `TX` / `RX` → спрацювала подія радіо, обробляємо.
- `*_TIMEOUT` → таймаут, повертаємось у `LOWPOWER` (у RX автоматично
  перезапускаємо `Radio.Rx(0)`).

---

## 8. Переривання

Вектори визначені в `platform/system/startup_cm4.S`. Слабкі аліаси ведуть на
`Default_Handler` (нескінченний цикл). Щоб замінити handler — визнач функцію
з тим самим ім'ям у своєму `.c` файлі.

Використовувані в проєкті:

| IRQ             | Обробник              | Файл             |
|-----------------|------------------------|------------------|
| `GPIO_IRQn`     | `GPIO_IRQHandler`      | `main_TX.c`      |
| `TIMER0_IRQn`   | `gptim0_IRQHandler`    | `main_TX.c` / `main_RX.c` |
| `LORA_IRQn`     | (intrinsic in driver)  | `lora/driver/sx1262-board.c` |

`RTC_IRQHandler`, `UART0_IRQHandler`, `I2C0_IRQHandler` не використовуються
вручну (I2C опитується, UART polling-режимі).

---

## 9. LoRaWAN-стек — наявний, але не задіяний

У репозиторії є повний LoRaWAN-стек Semtech (`lora/mac/LoRaMac.c`) та
LinkWAN від Alibaba (`lora/linkwan/linkwan.c`) з регіонами
CN470A, EU868, US915, AS923, AU915, CN779, EU433, IN865, KR920.

`Makefile` цих файлів **не компілює** — див. `SOURCES := ...`, лише
`lora/radio`, `lora/driver`, `lora/system` додаються до збірки.

Для міграції на LoRaWAN потрібно:

1. Додати `lora/mac/LoRaMac.c` + `LoRaMacCrypto.c` + потрібний регіон до `SOURCES`.
2. Додати `lora/linkwan/linkwan.c` + `lwan_config.c` (опційно).
3. Замінити виклики `Radio.Send/Rx` на `LoRaMac.Send/Receive`.
4. Увімкнути крипто: додати `drivers/crypto/lib/libcrypto.a` (вже в `LIBS`).
5. Перевірити розмір Flash/RAM — LoRaWAN стек додає ≈ 30-40 КБ.

---

## 10. Зв'язок з GPIO

Детальний розпис виводів — у `docs/HARDWARE.md`. Тут — лише як код працює з
GPIO:

```c
gpio_set_iomux(GPIOA, GPIO_PIN_14, 3);   // I2C0_SCL
gpio_set_iomux(GPIOA, GPIO_PIN_15, 3);   // I2C0_SDA
gpio_set_iomux(GPIOA, GPIO_PIN_4, 0);    // LED G
gpio_set_iomux(GPIOB, GPIO_PIN_0, 1);    // UART0_TXD
gpio_set_iomux(GPIOB, GPIO_PIN_1, 1);    // UART0_RXD
gpio_set_iomux(GPIOD, GPIO_PIN_11, 6|3); // LORA RFSW_CTRL (див. sx1262-board.c)
```

Альтернативна функція (iomux) обирається числом `0..7`. Конкретні значення —
див. `inc/lora_config.h` та документацію Tremo.
