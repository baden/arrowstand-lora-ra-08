# AGENTS.md — Інструкції для AI агентів

> Робочий каталог: `/home/baden/SDK/lora/arrowstand-lora-ra-08`
> Цей файл — головний орієнтир для будь-якого AI агента, який працює з проєктом.
> Перш ніж редагувати код, обов'язково прочитай усі супутні файли в `docs/`.

---

## 1. Що це за проєкт

**`arrowstand-lora-ra-08`** — це embedded-прошивка (bare-metal C, без RTOS) для модуля
**AI-Thinker RA-08** (LoRa-модуль на SoC ASR Microelectronics Tremo з вбудованим
радіо Semtech SX1262).

Проєкт реалізує два пристрої «стрілкової стійки» (ArrowStand) для стрільби з лука:

- **TX (передавач, `main_TX.c`)** — датчик-акселерометр ADXL345, що виявляє активність
  (постріл) і передає пакет по LoRa. Має RGB-світлодіод і кнопку BOOT для перемикання
  режимів (`MODE_SENSOR` / `MODE_TESTER`).
- **RX (приймач, `main_RX.c`)** — постійно слухає ефір, відображає лічильник влучань,
  RSSI/SNR та поточну чутливість датчика на OLED SSD1306 128×32. Кнопками
  `KEY_LEFT`/`KEY_RIGHT` можна змінити корекцію чутливості, яка відправляється на TX.

Протокол обміну — фірмовий «PING»/«C» (5 байт):
```
TX → RX: 'P','I','N','G', <sensitivity>
RX → TX: 'C', <correction+64>, x,x,x
```

Вибір пристрою здійснюється через `ROLE=TX` або `ROLE=RX` під час `make`.

---

## 2. Структура репозиторію

```
arrowstand-lora-ra-08/
├── AGENTS.md                      ← цей файл (інструкції для AI агентів)
├── README.md                      ← людська документація, нотатки автора
├── Makefile                       ← основний Makefile (збірка + flash)
├── Makefile.orig                  ← оригінальний Makefile від Ai-Thinker SDK
├── .editorconfig                  ← стильові правила
├── cfg/
│   ├── gcc.ld                     ← linker script (128K Flash, 16K RAM)
│   └── ram.ini                    ← конфіг для RAM-debug (Keil/uVision)
├── src/                           ← власний код проєкту
│   ├── main_TX.c                  ← точка входу для передавача
│   ├── main_RX.c                  ← точка входу для приймача
│   ├── init.c / .h                ← board_init(): RCC, живлення, RTC, UART
│   ├── lora.c / .h                ← ініціалізація радіо, RadioEvents
│   ├── adxl345.c / .h             ← драйвер акселерометра через I2C
│   ├── i2c.c / .h                 ← простий polling-драйвер I2C0
│   ├── oled_min.c / .h            ← SSD1306 I2C 128×32
│   ├── board_led.c / .h           ← RGB LED GPIOA 4,5,7
│   ├── tremo_it.c / .h            ← обробники переривань (GPIO, TIMER0)
│   └── role.h                     ← визначення ROLE_TX / ROLE_RX
├── inc/                           ← публічні заголовки власного коду + шрифти
│   ├── lora_config.h              ← частота, потужність, SF, BW, CR (EU433)
│   ├── boom.h, font*.h, ...       ← шрифти для OLED
├── drivers/                       ← SDK від Ai-Thinker (ASR Tremo)
│   ├── peripheral/                ← HAL: GPIO, UART, I2C, SPI, ADC, RTC, TIM...
│   │   ├── inc/tremo_*.h          ← заголовки периферії
│   │   └── src/tremo_*.c          ← реалізація
│   └── crypto/
│       ├── inc/                   ← AES, SHA, RSA, ECC, RNG
│       └── lib/libcrypto.a        ← статична криптобібліотека
├── lora/                          ← LoRa-стек
│   ├── radio/
│   │   ├── radio.h                ← API Radio (Semtech-style)
│   │   └── sx126x/                ← драйвер SX126x + .c/.h
│   ├── driver/                    ← board-level: rtc-board, sx1262-board, utilities
│   ├── system/                    ← timer.c/.h, delay.c/.h, log.h, crypto/cmac.c
│   ├── mac/                       ← LoRaMac.c (Semtech LoRaWAN MAC) + всі регіони
│   └── linkwan/                   ← LinkWAN (Alibaba LoRaWAN) + region CN470A
├── platform/                      ← CMSIS + startup
│   ├── CMSIS/core_cm4.h, ...      ← ARM CMSIS для Cortex-M4 з FPU
│   └── system/
│       ├── startup_cm4.S          ← вектор переривань + Reset_Handler
│       ├── system_cm4.c           ← SystemInit()
│       ├── tremo_cm4.h            ← визначення IRQn для Tremo
│       └── printf-stdarg.c        ← реалізація printf/sprintf (бо libc викинуто)
└── scripts/
    └── tremo_loader.py            ← завантажувач прошивки через UART (DTR/RTS reset)
```

---

## 3. Стек технологій

| Рівень              | Технологія / компонент                                   |
|---------------------|----------------------------------------------------------|
| Мова програмування  | C99 (`-std=gnu99`), іноді C++-сумісні заголовки          |
| Збірка              | GNU Make + `arm-none-eabi-gcc` (Cortex-M4, Thumb)        |
| Лінкер              | `arm-none-eabi-gcc` + кастомний `cfg/gcc.ld`            |
| Прошивка            | Python 3 (`pyserial`) + `scripts/tremo_loader.py`        |
| Залежності Python   | `pyserial`, `configparser` (див. README.md)              |
| Мікроконтролер      | ASR Tremo (ARM Cortex-M4F, 128K Flash, 16K RAM)          |
| Радіо               | Semtech SX1262 (LoRa, EU 433 MHz у проєкті)              |
| Сенсори / вивід     | ADXL345 (I2C, INT1→GPIOA_11), SSD1306 OLED 128×32 (I2C)  |
| Стек                | Bare-metal (без RTOS), власний event-loop + переривання  |
| LoRa-стек (наявний) | Semtech `LoRaMac` (LoRaWAN) + LinkWAN від Alibaba + всі регіони (EU868, US915, AS923, AU915, CN470, CN779, EU433, IN865, KR920) |
| Крипто (наявне)     | AES, DES, SHA1/224/256/384/512, RSA, ECC, RNG, CMAC      |
| Друк                | Власний `printf-stdarg.c` (libc викинуто з лінкера)      |

**Зверни увагу**: LoRaWAN-стек (`lora/mac`, `lora/linkwan`) та крипто-бібліотека
(`drivers/crypto`) **присутні в репозиторії, але не використовуються** в основній
прошивці. Застосунок працює через «голий» LoRa-драйвер Semtech
(`lora/radio/sx126x` + `lora/driver/sx1262-board.c`). Не видаляй і не рефактор ці
підсистеми без явного завдання — вони можуть знадобитись для LoRaWAN-міграції.

---

## 4. Як зібрати і прошити

> Повна інструкція з поясненнями — у `docs/BUILD_AND_FLASH.md`.
> Тут — лише скорочений «happy path».

```bash
# Залежності (один раз)
sudo apt-get install gcc-arm-none-eabi git python3 python3-pip python3-virtualenv

# Python venv (один раз)
python3 -m venv .venv
source .venv/bin/activate
pip install pyserial configparser

# Збірка під передавач або приймач
make clean
ROLE=TX make
ROLE=RX make

# Прошивка через UART (DTR/RTS керують BOOT+RST модуля)
make clean && PORT=/dev/ttyUSB0 ROLE=RX make flash
make clean && PORT=/dev/ttyUSB0 ROLE=TX make flash
```

**Застереження з README**:
- На macOS прошивка через `make flash` не працює стабільно.
- Рекомендована версія тулчейну: `arm-none-eabi-gcc 13.2.1` (Linux apt) або
  `10.3-2021.07` (Homebrew). Теоретично SDK підтримує 9-2020q2.

---

## 5. Ключові файли — на що звертати увагу

| Файл                          | Що там і чому важливий                                            |
|-------------------------------|--------------------------------------------------------------------|
| `Makefile`                    | Список джерел, CFLAGS, регіон (`-DREGION_EU433`), modem, ROLE      |
| `cfg/gcc.ld`                  | Flash 128K @ 0x08000000, RAM 16K; entry = `Reset_Handler`          |
| `platform/system/startup_cm4.S` | Таблиця векторів + копіювання `.data`, очистка `.bss`, `bl main` |
| `inc/lora_config.h`           | `RF_FREQUENCY`, `TX_OUTPUT_POWER`, SF/BW/CR (LoRa-параметри)      |
| `inc/role.h`                  | `USE_OLED` (RX) vs `USE_ACCEL` (TX) через `-DROLE`                 |
| `src/main_TX.c`               | Логіка передавача: ACCEL → LoRa TX                                 |
| `src/main_RX.c`               | Логіка приймача: LoRa RX → OLED                                    |
| `src/lora.c`                  | `lora_init()` — налаштування Radio.SetTxConfig / SetRxConfig        |
| `src/init.c`                  | `board_init()` — тактування, живлення, RTC, LORA-peripheral clk    |
| `lora/driver/sx1262-board.c`  | Доступ до LoRa-радіо через вбудований LORAC-контролер              |
| `scripts/tremo_loader.py`     | Завантажувач: DTR=BOOT, RTS=RST, протокол 0xFE…/…0xEF              |

---

## 6. Правила редагування коду для AI агентів

1. **Не додавай коментарі** — код мінімальний, у стилі автора. Якщо потрібно
   пояснити рішення, додай коментар лише у відповідь автору в чаті, не у файл.
2. **Не перейменовуй існуючі символи** без явного прохання — багато ідентифікаторів
   (`Buffer`, `State`, `OnTxDone`, `Radio`) збігаються з шаблонами Semtech.
3. **Мова відповідей і коментарів у коді** — українська (як у `README.md` і
   коментарях основних файлів). Уникай російської.
4. **Стиль** — див. `.editorconfig`: K&R-подібний, табуляція в Makefile,
   `cpp_space_before_block_open_brace=true`, фігурні дужки на тому ж рядку.
5. **`#pragma once`** — для нових заголовків. Не використовуй `#ifndef`/`#define`.
6. **Стандарт мови** — `gnu99`. Без `__STDC_LIMIT_MACROS`, без C++.
7. **Floating-point** — FPU є (`-mfpu=fpv4-sp-d16 -mfloat-abi=softfp`), але
   намагайся уникати `float`/`double` для embedded-логіки.
8. **Розмір RAM — 16K**, Flash — 128K. Не видічай великі буфери в стеку.
9. **Переривання** — див. та表ці у `startup_cm4.S`. Слабкі аліаси вже оголошені,
   просто визнач handler з тим самим іменем.
10. **Зміни в Makefile** — обережно: `EXCLUDES := src/main_TX.c src/main_RX.c`
    спеціально відкидає обидва `main_*.c` і додає лише `src/main_$(ROLE).c`.

---

## 7. Типові задачі, які можуть прийти від користувача

| Запит                                      | На що звернути увагу                                    |
|--------------------------------------------|----------------------------------------------------------|
| «Додай deep sleep на TX»                   | `tremo_pwr.h`, RTC wakeup, режим низького енергоспоживання |
| «Зроби RX з акумулятором»                 | Живлення, LPTIMER, відключення OLED у idle              |
| «Додай LoRaWAN замість raw LoRa»           | `lora/mac/LoRaMac.c` + регіон (CN470A для RA-08)        |
| «Показуй температуру на OLED»              | ADXL345 має temperature reg; додати в `adxl345.c`        |
| «Перенеси код на інший модуль ASR»         | Зміни в `lora/driver/sx1262-board.c` (виводи RFSW)      |
| «Зміни частоту на 868 МГц»                 | `inc/lora_config.h` + Makefile `-DREGION_EU868`          |
| «Зроби свій payload (замість PING)»        | Зміни в `main_TX.c`/`main_RX.c`, узгодь розмір Buffer    |

---

## 8. Що категорично заборонено

- Змінювати ABI, не повідомивши користувача: `Makefile` (CFLAGS/LDFLAGS),
  `cfg/gcc.ld`, `platform/system/startup_cm4.S`, `platform/CMSIS/*`.
- Видаляти файли з `lora/mac`, `lora/linkwan`, `drivers/crypto` — це частина
  SDK, потрібна для майбутніх LoRaWAN-розширень.
- Змінювати `Makefile.orig` — це еталонний Makefile від Ai-Thinker, тримаємо
  для звірки.
- Додавати залежності в Python або C без явного дозволу користувача.
- Комітити код без явного прохання (`NEVER commit changes unless the user
  explicitly asks you to`).

---

## 9. Додаткова документація

Перед серйозними змінами прочитай:

- `docs/PROJECT_OVERVIEW.md` — архітектура і потік даних
- `docs/HARDWARE.md` — розпіновка RA-08, адреси I2C, регіони
- `docs/CODING_CONVENTIONS.md` — детальний style-guide
- `docs/BUILD_AND_FLASH.md` — всі нюанси збірки та прошивки
- `README.md` — нотатки автора, відомі проблеми, посилання

---

## 10. Шпаргалка для швидкого старту агента

```text
Платформа:      ASR Tremo (ARM Cortex-M4F, FPU, 128K Flash / 16K RAM)
SDK в репо:     drivers/ + platform/ + lora/  (усе bundled, без зовнішніх SDK)
Тулчейн:        arm-none-eabi-gcc (-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16)
Збірка:         ROLE=TX|RX make
Прошивка:       PORT=/dev/ttyUSB0 ROLE=TX|RX make flash
Завантажувач:   scripts/tremo_loader.py (pyserial, DTR=BOOT, RTS=RST)
LoRa:           SX1262 на 433 МГц, SF7/BW125/CR4/5 (див. lora_config.h)
Дев-прошивка:   TX має ADXL345; RX має OLED SSD1306 128x32
```
