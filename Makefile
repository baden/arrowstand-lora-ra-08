default: all

PROJECT := lora-ra-08

BUILD_DIR := build

SOURCES := $(wildcard src/*.c) \
	platform/system/system_cm4.c  \
	platform/system/startup_cm4.S \
	platform/system/printf-stdarg.c \
	$(wildcard drivers/peripheral/src/*.c) \
    $(wildcard lora/system/*.c)  \
    $(wildcard lora/system/crypto/*.c)  \
    $(wildcard lora/radio/sx126x/*.c)  \
    $(wildcard lora/driver/*.c)

CFLAGS  := -mcpu=cortex-m4 -mthumb
CFLAGS  += -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant
CFLAGS  += -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf
CFLAGS  += -I inc -I platform/CMSIS -I platform/common -I platform/system -I drivers/peripheral/inc
CFLAGS  += -I drivers/crypto/inc -I lora/driver -I lora/system -I lora/radio -I lora/radio/sx126x
CFLAGS  += -DROLE=ROLE_$(ROLE) -DCONFIG_DEBUG_UART=UART0 -DUSE_MODEM_LORA -DREGION_EU433

LDFLAGS := -mcpu=cortex-m4 -mthumb -mthumb-interwork
LDFLAGS += -Wl,--gc-sections -Wl,--wrap=printf -Wl,--wrap=sprintf -Wl,--wrap=snprintf -Wl,--print-memory-usage

LIBS := drivers/crypto/lib/libcrypto.a

LINK_LD := cfg/gcc.ld

CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy

C_SOURCES := $(filter %.c,$(SOURCES))
S_SOURCES := $(filter %.S,$(SOURCES))

C_O_FILES := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
S_O_FILES := $(patsubst %.S,$(BUILD_DIR)/%.o,$(S_SOURCES))

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	@$(CC) -c -mcpu=cortex-m4 -mthumb $< -o $@

$(BUILD_DIR)/$(PROJECT)-$(ROLE).elf: $(C_O_FILES) $(S_O_FILES)
	@$(CC) $(LDFLAGS) -Wl,-Map=$(BUILD_DIR)/$(PROJECT)-$(ROLE).map -T$(LINK_LD) -o $@ $(C_O_FILES) $(S_O_FILES) $(LIBS)

$(BUILD_DIR)/$(PROJECT)-$(ROLE).bin: $(BUILD_DIR)/$(PROJECT)-$(ROLE).elf
	@$(OBJCOPY) -O binary -R .eh_frame -R .init -R .fini -R .comment -R .ARM.attributes $< $@

all: $(BUILD_DIR)/$(PROJECT)-$(ROLE).bin

flash: $(BUILD_DIR)/$(PROJECT)-$(ROLE).bin
	python3 scripts/tremo_loader.py -p ${PORT} -b 921600 flash 0x08000000 $<

flash1: $(BUILD_DIR)/$(PROJECT)-$(ROLE).bin
	python3 scripts/tremo_loader.py -p ${PORT} -b 115200 flash 0x08000000 $<

clean:
	@rm -rf $(BUILD_DIR)

print-%:
	@echo $* is $($*)

