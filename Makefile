# ######                  #######                           #######  #####  
# #     # # #    # ######    #    # #    # ######     ####  #     # #     # 
# #     # # ##   # #         #    # ##  ## #         #    # #     # #       
# ######  # # #  # #####     #    # # ## # #####     #      #     #  #####  
# #       # #  # # #         #    # #    # #         #      #     #       # 
# #       # #   ## #         #    # #    # #         #    # #     # #     # 
# #       # #    # ######    #    # #    # ######     ####  #######  #####  

PROJECT_NAME     := pinetime-cos
TARGETS          := pinetime-cos
OUTPUT_DIRECTORY := _build

NRFUTIL := d:/Work/Pinetime/nRF52832/nrfutil
#SDK_ROOT := /mnt/d/Work/PineTime/nRF5_SDK_17.1.0_ddde560
SDK_ROOT := d:/Work/PineTime/nRF5_SDK_17.1.0_ddde560

PEM_FILE := d:/Work/PineTime/nordic_pem_keys/pinetime.pem

SOFT_DEVICE := s112
SOFT_DEVICE_HEX := s112_nrf52_7.2.0_softdevice.hex

SOFT_DEVICE_UC := S112

PROJ_DIR := ./src

$(OUTPUT_DIRECTORY)/pinetime-cos.out: \
  LINKER_SCRIPT  := gcc_nrf52.ld

# --------------------------------------------------------------------

INC_FOLDERS += \
  $(PROJ_DIR) \
  ./config \

include Makefile.nrf52
include MakefileV8.lvgl

# Source
SRC_FILES += \
  $(PROJ_DIR)/sys/sys.c \
  $(PROJ_DIR)/sys/utils.c \
  $(PROJ_DIR)/sys/lvgl_init.c \
  $(PROJ_DIR)/hardware/watchdog.c \
  $(PROJ_DIR)/hardware/backlight.c \
  $(PROJ_DIR)/hardware/battery.c \
  $(PROJ_DIR)/hardware/rtc.c \
  $(PROJ_DIR)/hardware/st7789.c \
  $(PROJ_DIR)/hardware/cst816.c \
  $(PROJ_DIR)/hardware/flash.c \
  $(PROJ_DIR)/theme/lv_theme_pinetime.c \
  $(PROJ_DIR)/theme/lv_font_roboto_24.c \
  $(PROJ_DIR)/theme/lv_font_sys_20.c \
  $(PROJ_DIR)/theme/lv_font_clock_42.c \
  $(PROJ_DIR)/theme/lv_font_clock_90.c \
  $(PROJ_DIR)/apps/app.c \
  $(PROJ_DIR)/nrf_ble/nrf_ble.c \
  $(PROJ_DIR)/nrf_ble/ble_cmd.c \
  $(PROJ_DIR)/main.c \

# APPS
SRC_FILES += \
  $(PROJ_DIR)/apps/app/clock.c \
  $(PROJ_DIR)/apps/app/info.c \
  $(PROJ_DIR)/apps/app/menu.c \

# Resources
SRC_FILES += \
  $(PROJ_DIR)/resources/msg.c \
  


# Include
INC_FOLDERS += \
  $(PROJ_DIR)/sys \
  $(PROJ_DIR)/nrf_ble \
  $(PROJ_DIR)/hardware \
  $(PROJ_DIR)/theme \
  $(PROJ_DIR)/apps \
  $(PROJ_DIR)/apps/app \

  

# --------------------------------------------------------------------

# Libraries common to all targets
LIB_FILES += \

# Optimization flags
OPT = -Os -g3 -fno-exceptions -fno-non-call-exceptions
# Debug
#OPT = -Og -g3
# Uncomment the line below to enable link time optimization
#OPT += -flto

# C flags common to all targets
CFLAGS += $(OPT)
#CFLAGS += -DDEBUG 
CFLAGS += -DAPP_TIMER_V2
CFLAGS += -DAPP_TIMER_V2_RTC1_ENABLED
CFLAGS += -DBOARD_PCA10040
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DFLOAT_ABI_HARD
CFLAGS += -DFREERTOS
CFLAGS += -DNRF52
CFLAGS += -DNRF52832_XXAA
CFLAGS += -DNRF52_PAN_74
CFLAGS += -DNRF_SD_BLE_API_VERSION=7
CFLAGS += -D$(SOFT_DEVICE_UC)
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall -Werror
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums
#CFLAGS += -Wstack-usage=48
# remove error from unused functions
CFLAGS += -Wno-unused-function

# C++ flags common to all targets
CXXFLAGS += $(OPT)
#ASMFLAGS += -DDEBUG
# Assembler flags common to all targets
#ASMFLAGS += -g3
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DAPP_TIMER_V2
ASMFLAGS += -DAPP_TIMER_V2_RTC1_ENABLED
ASMFLAGS += -DBOARD_PCA10040
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DFLOAT_ABI_HARD
ASMFLAGS += -DFREERTOS
ASMFLAGS += -DNRF52
ASMFLAGS += -DNRF52832_XXAA
ASMFLAGS += -DNRF52_PAN_74
ASMFLAGS += -DNRF_SD_BLE_API_VERSION=7
ASMFLAGS += -D$(SOFT_DEVICE_UC)
ASMFLAGS += -DSOFTDEVICE_PRESENT

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs
LDFLAGS += -fstack-usage
LDFLAGS += -Wl,--print-memory-usage

pinetime-cos: CFLAGS += -D__HEAP_SIZE=0x1500
pinetime-cos: CFLAGS += -D__STACK_SIZE=0x1500
pinetime-cos: ASMFLAGS += -D__HEAP_SIZE=0x1500
pinetime-cos: ASMFLAGS += -D__STACK_SIZE=0x1500

# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIB_FILES += -lc -lnosys -lm

.PHONY: default help

# Default target - first one defined
default: all

all: pinetime-cos

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo		pinetime-cos
	@echo		softdevice - make .hex from bootloader, softdevice and app
	@echo		prog - Program Pinetime via GDB

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc
include $(TEMPLATE_PATH)/Makefile.common
$(foreach target, $(TARGETS), $(call define_target, $(target)))

softdevice: default
	@echo	** Creating $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).app.hex
	@$(NRFUTIL) settings generate --family NRF52 --application $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex --app-boot-validation VALIDATE_GENERATED_CRC --application-version 0x00 --bootloader-version 0x01 --bl-settings-version 2 --softdevice $(SDK_ROOT)/components/softdevice/$(SOFT_DEVICE)/hex/$(SOFT_DEVICE_HEX) $(OUTPUT_DIRECTORY)/dfu_settings.hex
	@python scripts/hexmerge.py --overlap=replace $(SDK_ROOT)/components/softdevice/$(SOFT_DEVICE)/hex/$(SOFT_DEVICE_HEX) bootloader/bootloader_pinetime-cos.hex $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex $(OUTPUT_DIRECTORY)/dfu_settings.hex -o $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).app.hex
	
prog: softdevice
	@echo	** Program Pinetime with $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).app.hex
	@arm-none-eabi-gdb.exe --batch -ex="target extended-remote 192.168.1.20:3333" -ex "load" -ex "monitor reset" $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).app.hex

pkg:
	@$(NRFUTIL) pkg generate --key-file $(PEM_FILE) --app-boot-validation VALIDATE_GENERATED_CRC --hw-version 52 --sd-req 0x0103 --application-version-string "0.1.0" --application $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).zip