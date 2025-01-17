## LoRa на модулі AI-Thinker RA-08

Для Linux:

```bash
sudo apt-get install gcc-arm-none-eabi git python3 python3-pip python3-venv
```

Для Mac мабуть (я не перевіряв, в мене вже було все встановлено):

```bash
brew install install gcc-arm-none-eabi git python3 python3-pip python3-venv
```

Скачуємо репозиторій.

```bash
git clone https://github.com/baden/arrowstand-lora-ra-08.git
cd arrowstand-lora-ra-08
```

Створюємо віртуальне оточення шоб не мусорити:

```bash
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install pyserial configparser
```

Збираємо.

```bash
make
```

Прошиваємо

```bash
PORT=/dev/ttyUSB0 make flash
```

От, сюка. Под Linux прошивається, а от під Мас не хоче.


## Распіновка виводів модуля RA-08 (і відлагоджувальної плати)

1,17 - GND


2 - GPIO8/ADC_IN1
3 - GPIO11/ADC_IN0          (вхід INT1 від акселерометра. PORTA_11, Просинання з глубокого сну.)

4 - GPIO9/DAC_OUT - Toggle each 1 sec

5 - GPIO4/SSPI_CLK - Internal LED G
6 - GPIO5/SSPI_NSS - Internal LED R
7 - SWCLK, GPIO7 - Internal LED B (?)
8 - SWDIO, GPIO6/SWD_DATA

9 - VCC 3,3V

10 - LPRXD, GPIO60/LPUART-RX
11 - TXD, GPIO17 | GPIO_PORTB_1 | UART0_TXD (fun=1)
12 - GPIO16 | GPIO_PORTB_0 | UART0_RXD (fun=1)       Я так розумію це або RXD0, або LPRXD

13 - GPIO15 - I2C0_SDA (alt=3)
14 - GPIO14 - I2C0_SCL (alt=3)

15 - GPIO2, BOOT
16 - RST, low power is effective

18 - ANT

## Посилання

Документація (https://drive.google.com/drive/folders/1TeFu02b9EZpESxnhLrE_o-G0U0cJ4ivc)

[MUX table](https://drive.google.com/drive/folders/1ph9s0ZU6HYJ4AYP4xbFndDFbg8pSCYrn)


## Проба №1

По https://github.com/Ai-Thinker-Open/Ai-Thinker-LoRaWAN-Ra-08 пройшов. З умови деяких дрібних поправок, приклади зібрались та запустились.
Які відмінності:

- python3 замість python
- десь ругнулось на #include<stdint-gcc.h>, прибрав ото "-gcc" - зібралось і працює.

## Так, спробуємо це зібрати одразу на MAC

1. Треба визначитись з arm gcc.

В орігіналі встановився

```
arm-none-eabi-gcc (15:13.2.rel1-2) 13.2.1 20231009
```

Це через apt-get install gcc-arm-none-eabi.

На MAC в мене, через homebrew, встановлена

```
arm-none-eabi-gcc (GNU Arm Embedded Toolchain 10.3-2021.07) 10.3.1 20210621 (release)
```

Бачу шо якшо немає системної, SDK завантажує (шо дивно це для вінди)

```
https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-win32.zip?revision=95631fd0-0c29-41f4-8d0c-3702650bdd74&hash=60FBF84A2ADC7B1F508C2D625E831E1F1184F509
```

Це взагалі гілка 9. Так шо буду пробувати на моїй 10ці.


Вставляю тут вивід "make"


```
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf src/main.c -o out/main.o
echo Compiling tremo_it.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf src/tremo_it.c -o out/tremo_it.o
echo Compiling system_cm4.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system/system_cm4.c -o out/system_cm4.o
echo Compiling tremo_adc.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_adc.c -o out/tremo_adc.o
echo Compiling tremo_bstimer.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_bstimer.c -o out/tremo_bstimer.o
echo Compiling tremo_crc.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_crc.c -o out/tremo_crc.o
echo Compiling tremo_dac.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_dac.c -o out/tremo_dac.o
echo Compiling tremo_delay.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_delay.c -o out/tremo_delay.o
echo Compiling tremo_dma.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_dma.c -o out/tremo_dma.o
echo Compiling tremo_flash.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_flash.c -o out/tremo_flash.o
echo Compiling tremo_gpio.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_gpio.c -o out/tremo_gpio.o
echo Compiling tremo_i2c.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_i2c.c -o out/tremo_i2c.o
echo Compiling tremo_i2s.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_i2s.c -o out/tremo_i2s.o
echo Compiling tremo_iwdg.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_iwdg.c -o out/tremo_iwdg.o
echo Compiling tremo_lcd.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_lcd.c -o out/tremo_lcd.o
echo Compiling tremo_lptimer.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_lptimer.c -o out/tremo_lptimer.o
echo Compiling tremo_lpuart.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_lpuart.c -o out/tremo_lpuart.o
echo Compiling tremo_pwr.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_pwr.c -o out/tremo_pwr.o
echo Compiling tremo_rcc.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_rcc.c -o out/tremo_rcc.o
echo Compiling tremo_rtc.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_rtc.c -o out/tremo_rtc.o
echo Compiling tremo_spi.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_spi.c -o out/tremo_spi.o
echo Compiling tremo_system.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_system.c -o out/tremo_system.o
echo Compiling tremo_timer.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_timer.c -o out/tremo_timer.o
echo Compiling tremo_uart.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_uart.c -o out/tremo_uart.o
echo Compiling tremo_wdg.c...
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb -I inc -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/CMSIS -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/common -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system -I /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/inc  -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/drivers/peripheral/src/tremo_wdg.c -o out/tremo_wdg.o
"arm-none-eabi-gcc" -c -mcpu=cortex-m4 -mthumb /home/baden/SDK/arrowstand-projects/lora-ra-08/Ai-Thinker-LoRaWAN-Ra-08/platform/system/startup_cm4.S -o out/startup_cm4.o
echo out/gpio_toggle.elf
"arm-none-eabi-gcc" -mcpu=cortex-m4 -mthumb -mthumb-interwork -Wl,--gc-sections -Wl,--wrap=printf -Wl,--wrap=sprintf -Wl,--wrap=snprintf -Wl,-Map=out/gpio_toggle.map -Tcfg/gcc.ld -o out/gpio_toggle.elf out/main.o out/tremo_it.o out/system_cm4.o out/tremo_adc.o out/tremo_bstimer.o out/tremo_crc.o out/tremo_dac.o out/tremo_delay.o out/tremo_dma.o out/tremo_flash.o out/tremo_gpio.o out/tremo_i2c.o out/tremo_i2s.o out/tremo_iwdg.o out/tremo_lcd.o out/tremo_lptimer.o out/tremo_lpuart.o out/tremo_pwr.o out/tremo_rcc.o out/tremo_rtc.o out/tremo_spi.o out/tremo_system.o out/tremo_timer.o out/tremo_uart.o out/tremo_wdg.o out/startup_cm4.o 
echo out/gpio_toggle.bin
"arm-none-eabi-objcopy" -O binary -R .eh_frame -R .init -R .fini -R .comment -R .ARM.attributes out/gpio_toggle.elf out/gpio_toggle.bin
"arm-none-eabi-objcopy" -O ihex -R .eh_frame -R .init -R .fini -R .comment -R .ARM.attributes out/gpio_toggle.elf out/gpio_toggle.hex
echo Build completed.



arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mthumb-interwork -Wl,--gc-sections -Wl,--wrap=printf -Wl,--wrap=sprintf -Wl,--wrap=snprintf -Wl,-Map=build/lora-ra-08.map -Tcfg/gcc.ld -o build/src/main.o build/src/main.o build/src/tremo_it.o build/platform/system/system_cm4.o build/drivers/peripheral/src/tremo_adc.o build/drivers/peripheral/src/tremo_bstimer.o build/drivers/peripheral/src/tremo_crc.o build/drivers/peripheral/src/tremo_dac.o build/drivers/peripheral/src/tremo_delay.o build/drivers/peripheral/src/tremo_dma.o build/drivers/peripheral/src/tremo_flash.o build/drivers/peripheral/src/tremo_gpio.o build/drivers/peripheral/src/tremo_i2c.o build/drivers/peripheral/src/tremo_i2s.o build/drivers/peripheral/src/tremo_iwdg.o build/drivers/peripheral/src/tremo_lcd.o build/drivers/peripheral/src/tremo_lptimer.o build/drivers/peripheral/src/tremo_lpuart.o build/drivers/peripheral/src/tremo_pwr.o build/drivers/peripheral/src/tremo_rcc.o build/drivers/peripheral/src/tremo_rtc.o build/drivers/peripheral/src/tremo_spi.o build/drivers/peripheral/src/tremo_system.o build/drivers/peripheral/src/tremo_timer.o build/drivers/peripheral/src/tremo_uart.o build/drivers/peripheral/src/tremo_wdg.o build/platform/system/startup_cm4.o
```