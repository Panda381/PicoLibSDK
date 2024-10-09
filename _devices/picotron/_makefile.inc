CSRC += ../../../_display/minivga/minivga.c
CSRC += ../../../_devices/picotron/picotron_bat.c
CSRC += ../../../_devices/picotron/picotron_init.c
CSRC += ../../../_devices/picotron/picotron_key.c

DEFINE += -D USE_PICOTRON=1

MCU=RP2040
FLASHSIZE=0x200000
