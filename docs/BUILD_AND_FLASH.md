# Build & Flash — збірка і прошивка

> Доповнення до `AGENTS.md`. Повна покрокова інструкція: встановлення
> залежностей, збірка, прошивка через UART-завантажувач, типові проблеми.

---

## 1. Залежності

### Linux (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install gcc-arm-none-eabi git python3 python3-pip python3-venv
```

Рекомендована версія: **13.2.1** (`15:13.2.rel1-2` в apt). Теоретично SDK
підтримує 9-2020q2 і новіше.

### macOS

```bash
brew install gcc-arm-none-eabi git python3 python3-pip python3-venv
```

Версія з Homebrew: **10.3-2021.07**. Зверни увагу: **прошивка через
`make flash` на macOS нестабільна** (див. README.md).

### Windows (WSL або окремо)

WSL рекомендовано — повторює Linux-flow. Оригінальний SDK підтримує також
Toolchain 9-2020q2 для Windows, але в проєкті використовується GNU Make.

---

## 2. Python venv

```bash
cd /home/baden/SDK/lora/arrowstand-lora-ra-08
python3 -m venv .venv
source .venv/bin/activate
pip install pyserial configparser
```

Залежності `pyserial` + `configparser` потрібні для `scripts/tremo_loader.py`.

---

## 3. Збірка

### Базові цілі

```bash
make              # дефолт — all (залежить від $(ROLE))
make clean        # видалити build/
make all          # build/lora-ra-08-$(ROLE).bin
make flash        # завантажити .bin через UART
make flash1       # старіший режим 115200 (див. нижче)
make print-%      # @echo $* is $($*) — друк змінних Makefile
```

### Збірка під конкретну роль

```bash
make clean
ROLE=TX make           # передавач: src/main_TX.c, USE_ACCEL
ROLE=RX make           # приймач:  src/main_RX.c, USE_OLED
```

Артефакти:

- `build/lora-ra-08-TX.elf` / `build/lora-ra-08-TX.bin`
- `build/lora-ra-08-TX.map` (linker map)
- аналогічно для RX

### Прапорці компіляції (з Makefile)

```
-mcpu=cortex-m4 -mthumb
-Wall -Os -ffunction-sections
-mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant
-std=gnu99
-fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf
-I inc -I platform/CMSIS -I platform/common -I platform/system
-I drivers/peripheral/inc -I drivers/crypto/inc
-I lora/driver -I lora/system -I lora/radio -I lora/radio/sx126x
-DROLE=ROLE_$(ROLE) -DCONFIG_DEBUG_UART=UART0 -DUSE_MODEM_LORA -DREGION_EU433
```

### Лінкер

```
-mcpu=cortex-m4 -mthumb -mthumb-interwork
-Wl,--gc-sections
-Wl,--wrap=printf -Wl,--wrap=sprintf -Wl,--wrap=snprintf
-Wl,--print-memory-usage
-Wl,-Map=build/lora-ra-08-$(ROLE).map
-Tcfg/gcc.ld
drivers/crypto/lib/libcrypto.a
```

Пам'ять:

- **FLASH** 128 КБ @ `0x08000000`
- **RAM** 16 КБ @ `0x20000000`
- **STACK** 4 КБ (`_STACK_SIZE = 0x1000`)
- **HEAP** 0 (`_HEAP_SIZE = 0x0000`)

---

## 4. Прошивка через UART

### Завантажувач `scripts/tremo_loader.py`

Використовує фірмовий bootloader Tremo з пропрієтарним протоколом
(0xFE … 0xEF, CRC32). Керує RESET та BOOT через DTR/RTS.

Підключення:

| Сигнал CH340 / FTDI | GPIO RA-08 | Pin модуля |
|---------------------|------------|------------|
| DTR                 | BOOT       | 15         |
| RTS                 | RST        | 16         |
| TX                  | RX (PB0)   | 12         |
| RX                  | TX (PB1)   | 11         |
| GND                 | GND        | 1, 17      |

### Команди

```bash
# Перевірити підключення
PORT=/dev/ttyUSB0 python3 scripts/tremo_loader.py -p $PORT read_sn

# Стерти область (адреса, розмір)
PORT=/dev/ttyUSB0 python3 scripts/tremo_loader.py -p $PORT erase 0x08000000 0x20000

# Прошити (адреса + файл)
PORT=/dev/ttyUSB0 python3 scripts/tremo_loader.py -p $PORT -b 921600 flash 0x08000000 build/lora-ra-08-TX.bin

# Або через make
make clean && PORT=/dev/ttyUSB0 ROLE=TX make flash
```

### Швидкості (baud)

- `flash`: 921600 (дефолт, швидко).
- `flash1`: 115200 (для нестабільних каналів, наприклад USB-hub).

Зверни увагу: в `Makefile` рядок `python3 scripts/tremo_loader.py ... flash`
закоментовано другий варіант з `-b 19200` для дуже повільних каналів.

---

## 5. Типові проблеми

### 5.1 `make flash` на macOS — не працює стабільно

Див. README.md:
> От, сюка. Под Linux прошивається, а от під Мас не хоче.

Workaround: прошивати з Linux або WSL.

### 5.2 `Permission denied: /dev/ttyUSB0`

```bash
sudo usermod -aG dialout $USER
# перелогін
sudo chmod 666 /dev/ttyUSB0   # одноразово
```

### 5.3 `arm-none-eabi-gcc: command not found`

Не встановлено toolchain. Див. крок 1.

### 5.4 Помилка `unknown MCU 'cortex-m4'`

Стара версія GCC. Потрібен ≥ 9-2020q2.

### 5.5 `tremo_loader.py: Connect failed`

- Перевір живлення модуля (3.3 В стабільно).
- Перевір DTR/RTS у схемі (DTR↔BOOT, RTS↔RST).
- Спробуй `make flash1` (115200 замість 921600).
- Затисни кнопку BOOT під час увімкнення, потім відпусти (ручний режим).

### 5.6 Перевірка розміру бінарника

```bash
arm-none-eabi-size build/lora-ra-08-TX.elf
```

Або в Makefile додано `-Wl,--print-memory-usage` — побачиш в кінці лінкера.

### 5.7 Зависає на `RX_TIMEOUT` (відомий баг)

Із комітів (`11f8cdb Падла! Зависає на RX timeout.`) видно, що RX-таймаут був
проблемним. Поточне рішення: `Radio.Rx(0 /*RX_TIMEOUT_VALUE*/)` —
безкінечний прийом без таймауту. Не повертай `RX_TIMEOUT_VALUE` у
`Radio.Rx(500)` без перевірки.

---

## 6. Зневадження

### UART-логи

`printf` через `printf-stdarg.c` → UART0 (PB0/PB1) на 115200 8N1.

```bash
# Linux
sudo minicom -D /dev/ttyUSB0 -b 115200
# або
sudo screen /dev/ttyUSB0 115200
```

### SWD

Для глибшого зневадження доступний SWD на GPIO6 (SWDIO) / GPIO7 (SWCLK) —
потрібен зовнішній програматор (J-Link, ST-Link, CMSIS-DAP).

### RAM-debug через Keil/uVision

`cfg/ram.ini` містить скрипт завантаження `.elf` у RAM для дебага
без стирання Flash. Потребує uVision — не використовується в основному
процесі розробки.

---

## 7. Версії SDK і toolchain

| Компонент       | Версія / примітка                                |
|-----------------|--------------------------------------------------|
| ASR Tremo SDK   | Bundled у `drivers/` + `platform/`              |
| LoRa стек       | Semtech SX126x driver (bundled)                 |
| arm-none-eabi   | 13.2.1 (Linux apt) / 10.3-2021.07 (Homebrew)    |
| Python          | 3.7+                                             |
| pyserial        | остання стабільна                                |
| Make            | GNU Make 4.x                                     |

Перед внесенням змін у SDK-файли — перевір, що в репозиторії збережено
оригінальний `Makefile.orig` як референс.
