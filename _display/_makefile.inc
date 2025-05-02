ifeq (${MCU},RP2350A)
ASRC += ${PICOLIBSDK_DISPLAY_DIR}/disphstx/disphstx_dvi_m33.S
ASRC += ${PICOLIBSDK_DISPLAY_DIR}/disphstx/disphstx_vga_m33.S
endif

ifeq (${MCU},RP2350ARISCV)
ASRC += ${PICOLIBSDK_DISPLAY_DIR}/disphstx/disphstx_dvi_riscv.S
ASRC += ${PICOLIBSDK_DISPLAY_DIR}/disphstx/disphstx_vga_riscv.S
endif

ifeq (${MCU},RP2040)
ASRC += ${PICOLIBSDK_DISPLAY_DIR}/dvi/dvi_enc.S
ASRC += ${PICOLIBSDK_DISPLAY_DIR}/dvivga/dvivga_enc.S
endif

ifeq (${OS},Windows_NT)
CSRC += ${PICOLIBSDK_DISPLAY_DIR}/display.c
else
CSRC += ${PICOLIBSDK_DISPLAY_DIR}/disphstx/disphstx_dvi.c
CSRC += ${PICOLIBSDK_DISPLAY_DIR}/disphstx/disphstx_dvi_render.c
CSRC += ${PICOLIBSDK_DISPLAY_DIR}/disphstx/disphstx_vga.c
CSRC += ${PICOLIBSDK_DISPLAY_DIR}/disphstx/disphstx_vga_render.c
CSRC += ${PICOLIBSDK_DISPLAY_DIR}/disphstx/disphstx_vmode.c
CSRC += ${PICOLIBSDK_DISPLAY_DIR}/disphstx/disphstx_vmode_simple.c
CSRC += ${PICOLIBSDK_DISPLAY_DIR}/disphstx/disphstx_vmode_format.c
CSRC += ${PICOLIBSDK_DISPLAY_DIR}/disphstx/disphstx_vmode_time.c
CSRC += ${PICOLIBSDK_DISPLAY_DIR}/disphstx/disphstx_picolibsk.c

CSRC += ${PICOLIBSDK_DISPLAY_DIR}/disphstxmini/disphstxmini.c
CSRC += ${PICOLIBSDK_DISPLAY_DIR}/dvi/dvi.c
CSRC += ${PICOLIBSDK_DISPLAY_DIR}/dvivga/dvivga.c
CSRC += ${PICOLIBSDK_DISPLAY_DIR}/minivga/minivga.c
CSRC += ${PICOLIBSDK_DISPLAY_DIR}/st7789/st7789.c
endif
