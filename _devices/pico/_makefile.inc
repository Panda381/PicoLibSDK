CSRC += ../../../_display/minivga/minivga.c
CSRC += ../../../_display/dvi/dvi.c
CSRC += ../../../_display/dvivga/dvivga.c
CSRC += ../../../_devices/pico/pico_init.c
CSRC += ../../../_devices/pico/pico_bat.c

ASRC += ../../../_display/dvi/dvi_enc.S
ASRC += ../../../_display/dvivga/dvivga_enc.S

DEFINE += -D USE_PICO=1
