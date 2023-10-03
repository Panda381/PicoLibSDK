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
endif

ifeq (${DEVICE},picopad10)
DEFINE += -D USE_PICOPAD10=1
endif

ifeq (${DEVICE},picopadvga)
DEFINE += -D USE_PICOPADVGA=1
endif
