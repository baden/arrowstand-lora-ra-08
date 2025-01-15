

PROJECT := lora-ra-08


SOURCES := $(wildcard src/*.c)

CFLAGS  := -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf
LDFLAGS := -Wl,--gc-sections -Wl,--wrap=printf -Wl,--wrap=sprintf -Wl,--wrap=snprintf

LINK_LD := cfg/gcc.ld

