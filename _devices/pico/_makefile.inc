CSRC += ../../../_display/minivga/minivga.c
CSRC += ../../../_display/dvi/dvi.c
CSRC += ../../../_display/dvivga/dvivga.c
CSRC += ../../../_devices/pico/pico_init.c
CSRC += ../../../_devices/pico/pico_bat.c

ifneq (${DEVICE},pico2riscv)
ASRC += ../../../_display/dvi/dvi_enc.S
ASRC += ../../../_display/dvivga/dvivga_enc.S
endif

DEFINE += -D USE_PICO=1

ifeq (${DEVICE},pico)
MCU=RP2040
FLASHSIZE=0x200000
endif

ifeq (${DEVICE},pico2)
DEFINE += -D USE_PICO2=1
MCU=RP2350A
FLASHSIZE=0x400000
endif

ifeq (${DEVICE},pico2riscv)
DEFINE += -D USE_PICO2=1
MCU=RP2350ARISCV
FLASHSIZE=0x400000
endif
