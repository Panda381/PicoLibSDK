CSRC += ../../../_display/st7789/st7789.c
CSRC += ../../../_display/minivga/minivga.c
CSRC += ../../../_devices/picopad/picopad_bat.c
CSRC += ../../../_devices/picopad/picopad_key.c
CSRC += ../../../_devices/picopad/picopad_led.c
CSRC += ../../../_devices/picopad/picopad_init.c
CSRC += ../../../_devices/picopad/picopad_ss.c

DEFINE += -D USE_PICOPAD=1

ifeq (${DEVICE},picopad08)
DEFINE += -D USE_PICOPAD08=1
MCU=RP2040
FLASHSIZE=0x200000
endif

ifeq (${DEVICE},picopad10)
DEFINE += -D USE_PICOPAD10=1
MCU=RP2040
FLASHSIZE=0x200000
endif

ifeq (${DEVICE},picopad20)
DEFINE += -D USE_PICOPAD20=1
MCU=RP2350A
FLASHSIZE=0x400000
endif

ifeq (${DEVICE},picopad20riscv)
DEFINE += -D USE_PICOPAD20=1
MCU=RP2350ARISCV
FLASHSIZE=0x400000
endif

ifeq (${DEVICE},picopadvga)
DEFINE += -D USE_PICOPADVGA=1
MCU=RP2040
FLASHSIZE=0x200000
endif
