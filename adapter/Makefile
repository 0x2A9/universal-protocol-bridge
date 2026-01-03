TARGET    := adapter
BUILD_DIR := build

# Toolchain
CC      := arm-none-eabi-gcc
CXX     := arm-none-eabi-g++
OBJCOPY := arm-none-eabi-objcopy
SIZE    := arm-none-eabi-size

# Optimization level: 0,1,2,3,s,fast
OPT ?= 0
ifneq ($(OPT),$(filter $(OPT),0 1 2 3 s))
  $(error Unsupported OPT='$(OPT)'. Allowed: 0, 1, 2, 3, s)
endif

# Mode-specific debug/release toggles
MODE ?= DEBUG
ifeq ($(MODE),DEBUG)
  MODE_CFLAGS := -g3
else ifeq ($(MODE),RELEASE)
  MODE_CFLAGS := -g0
else
  $(error Unknown MODE='$(MODE)'. Use MODE=DEBUG or MODE=RELEASE)
endif

# ---- Auto-discover all devices across all vendors ----
VENDORS := $(notdir $(wildcard devices/*))
SUPPORTED_DEV := $(sort $(foreach v,$(VENDORS),$(notdir $(wildcard devices/$(v)/*))))

ifeq ($(strip $(SUPPORTED_DEV)),)
  $(error No devices found under devices/*/*)
endif

# ---- Select device and mode (default = first discovered) ----
DEVICE ?= $(firstword $(SUPPORTED_DEV))

# ---- Locate DEVICE_DIR by searching vendors ----
DEVICE_DIR_CANDIDATES := $(foreach v,$(VENDORS),$(wildcard devices/$(v)/$(DEVICE)))

# If not found -> error
ifeq ($(strip $(DEVICE_DIR_CANDIDATES)),)
  $(error Unsupported DEVICE='$(DEVICE)'. Supported: $(SUPPORTED_DEV))
endif

# If found more than once -> error
ifneq ($(words $(DEVICE_DIR_CANDIDATES)),1)
  $(error DEVICE='$(DEVICE)' is ambiguous. Matches: $(DEVICE_DIR_CANDIDATES))
endif

DEVICE_DIR := $(firstword $(DEVICE_DIR_CANDIDATES))

# ---- Includes ----
INCS := -Icore/inc

# ---- Sources ----
SRCS := core/src/main.cpp

# ---- Configure params based on choosen device ----
ifeq ($(DEVICE),stm32f303vct6)
  FLASH_ADDR := 0x08000000

  MCUFLAGS   := -mcpu=cortex-m4 -mthumb
  DEFS       := -D$(MODE) -DUSE_HAL_DRIVER -DSTM32F303xC
  LDSCRIPT   := $(DEVICE_DIR)/linker/STM32F303VCTX_FLASH.ld

  INCS += -I$(DEVICE_DIR)/core/Inc \
          -I$(DEVICE_DIR)/core/Inc/usb \
          -I$(DEVICE_DIR)/hal/Inc \
          -I$(DEVICE_DIR)/hal/Inc/Legacy \
          -I$(DEVICE_DIR)/cmsis/Include \
          -I$(DEVICE_DIR)/cmsis/Device/ST/STM32F3xx/Include \
          -I$(DEVICE_DIR)/middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc \
          -I$(DEVICE_DIR)/middlewares/ST/STM32_USB_Device_Library/Core/Inc

  SRCS +=   $(DEVICE_DIR)/core/Src/system_init.c \
            $(DEVICE_DIR)/core/Src/device_stm32f3xx.cpp \
            $(DEVICE_DIR)/core/Src/stm32f3xx_hal_msp.c \
            $(DEVICE_DIR)/core/Src/stm32f3xx_it.c \
            $(DEVICE_DIR)/core/Src/syscalls.c \
            $(DEVICE_DIR)/core/Src/sysmem.c \
            $(DEVICE_DIR)/core/Src/usb/usb_device.c \
            $(DEVICE_DIR)/core/Src/usb/usbd_cdc_if.c \
            $(DEVICE_DIR)/core/Src/usb/usbd_conf.c \
            $(DEVICE_DIR)/core/Src/usb/usbd_desc.c \
            $(DEVICE_DIR)/hal/Src/stm32f3xx_hal.c \
            $(DEVICE_DIR)/hal/Src/stm32f3xx_hal_gpio.c \
            $(DEVICE_DIR)/hal/Src/stm32f3xx_hal_cortex.c \
            $(DEVICE_DIR)/hal/Src/stm32f3xx_hal_rcc.c \
            $(DEVICE_DIR)/hal/Src/stm32f3xx_hal_rcc_ex.c \
            $(DEVICE_DIR)/hal/Src/stm32f3xx_hal_pcd_ex.c \
            $(DEVICE_DIR)/hal/Src/stm32f3xx_hal_pcd.c \
            $(DEVICE_DIR)/hal/Src/stm32f3xx_ll_usb.c \
            $(DEVICE_DIR)/hal/Src/stm32f3xx_hal_dma.c \
            $(DEVICE_DIR)/hal/Src/stm32f3xx_hal_uart_ex.c \
            $(DEVICE_DIR)/hal/Src/stm32f3xx_hal_uart.c \
            $(DEVICE_DIR)/cmsis/Device/ST/STM32F3xx/Source/system_stm32f3xx.c \
            $(DEVICE_DIR)/middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c \
            $(DEVICE_DIR)/middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c \
            $(DEVICE_DIR)/middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c \
            $(DEVICE_DIR)/middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c \
            $(DEVICE_DIR)/startup/startup_stm32f303vctx.s
endif

# ---- Compiler/Assembler/Linker flags ----
CFLAGS   := $(MCUFLAGS) -O$(OPT) $(MODE_CFLAGS) -ffunction-sections -fdata-sections \
            -std=gnu18 \
            -Wall -Wextra $(DEFS) $(INCS)

CXXFLAGS := $(MCUFLAGS) -O$(OPT) $(MODE_CFLAGS) -ffunction-sections -fdata-sections \
            -fno-exceptions -fno-rtti -std=gnu++20 \
            -Wall -Wextra $(DEFS) $(INCS)

ASFLAGS  := $(MCUFLAGS) -x assembler-with-cpp $(DEFS) $(INCS)

LDFLAGS  := $(MCUFLAGS) \
            -T$(LDSCRIPT) \
            -Wl,--gc-sections \
            -Wl,-Map=$(BUILD_DIR)/$(TARGET).map \
            --specs=nano.specs --specs=nosys.specs

# Map each source to an object under build/ preserving subfolders
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(filter %.c,$(SRCS))) \
        $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(filter %.cpp,$(SRCS))) \
        $(patsubst %.s,$(BUILD_DIR)/%.o,$(filter %.s,$(SRCS)))

DEPS := $(OBJS:.o=.d)

# -------------------------
# Targets
# -------------------------
.PHONY: all clean size info

all: info $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin size

info:
	@echo "OPT=$(OPT)"
	@echo "MODE=$(MODE)"
	@echo "DEVICE=$(DEVICE)"
	@echo "DEVICE_DIR=$(DEVICE_DIR)"

# Link ELF
$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $^ $(LDFLAGS) -o $@

# Generate HEX/BIN
$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

size: $(BUILD_DIR)/$(TARGET).elf
	$(SIZE) $<

# Compile C -> object + depfile
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Compile C++ -> object + depfile
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Assemble .s -> object
$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(@D)
	$(CC) $(ASFLAGS) -MMD -MP -c $< -o $@

# Build and flash to an MCU
flash: info size $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex
	st-flash write $(BUILD_DIR)/$(TARGET).bin $(FLASH_ADDR)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
