CSRC += ../../../_display/minivga/minivga.c
CSRC += ../../../_devices/demovga/demovga_bat.c
CSRC += ../../../_devices/demovga/demovga_init.c
CSRC += ../../../_devices/demovga/demovga_ss.c
CSRC += ../../../_devices/demovga/demovga_key.c

DEFINE += -D USE_DEMOVGA=1

MCU=RP2040
FLASHSIZE=0x200000
