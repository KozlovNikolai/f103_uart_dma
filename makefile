##############################################
# Настройки проекта
##############################################

# Имя выходного файла
TARGET = firmware

# Папки проекта
CORE_DIR     = Core
DRIVERS_DIR  = Drivers
STARTUP_DIR  = Startup
LINKER_DIR   = Linker

# Пути
INC_DIRS = \
  -I$(CORE_DIR)/Inc \
  -I$(DRIVERS_DIR)/CMSIS/Device/ST/STM32F1xx/Include \
  -I$(DRIVERS_DIR)/CMSIS/Include

SRC_DIRS = \
  $(CORE_DIR)/Src \
  $(DRIVERS_DIR)/CMSIS/Device/ST/STM32F1xx/Source/Templates \
  $(STARTUP_DIR)

# Компилятор
CC = arm-none-eabi-gcc
AS = arm-none-eabi-gcc -x assembler-with-cpp
LD = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# Флаги компилятора
CFLAGS  = -mcpu=cortex-m3 -mthumb -Wall -O2 -ffunction-sections -fdata-sections
CFLAGS += $(INC_DIRS)

# Флаги линковки
LDFLAGS = -T$(LINKER_DIR)/STM32F103XB_FLASH.ld -mcpu=cortex-m3 -mthumb -Wl,--gc-sections
LDFLAGS += --specs=nosys.specs --specs=nano.specs -u _printf_float

##############################################
# Файлы
##############################################

# Все C-файлы
C_SOURCES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
# Все ASM-файлы
ASM_SOURCES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))

# Объектные файлы
OBJ = $(C_SOURCES:.c=.o) $(ASM_SOURCES:.s=.o)

##############################################
# Правила
##############################################

all: $(TARGET).elf $(TARGET).bin

$(TARGET).elf: $(OBJ)
	$(LD) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)
	$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET).elf $(TARGET).bin

flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x8000000

flashreset: $(TARGET).bin
	st-flash --reset write $(TARGET).bin 0x8000000