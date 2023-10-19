CSRC += ../../../_display/minivga/minivga.c
CSRC += ../../../_display/dvi/dvi.c
CSRC += ../../../_devices/pico/pico_init.c
CSRC += ../../../_devices/pico/pico_bat.c

ASRC += ../../../_display/dvi/dvi_enc.S

DEFINE += -D USE_PICO=1

ifeq (${DEVICE},extdisp)
DEFINE += -D USE_EXTDISP=1
endif
