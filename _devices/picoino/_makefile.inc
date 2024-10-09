CSRC += ../../../_display/minivga/minivga.c
CSRC += ../../../_devices/picoino/picoino_bat.c
CSRC += ../../../_devices/picoino/picoino_init.c
CSRC += ../../../_devices/picoino/picoino_ss.c

DEFINE += -D USE_PICOINO=1

MCU=RP2040
FLASHSIZE=0x200000

ifeq (${DEVICE},picoino10)
DEFINE += -D USE_PICOINO10=1
CSRC += ../../../_devices/picoino/picoino_key.c
else
DEFINE += -D USE_PICOINOMINI=1
CSRC += ../../../_devices/picoino/picoino_keymini.c
endif
