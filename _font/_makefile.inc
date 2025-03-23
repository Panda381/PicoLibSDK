ifeq (${OS},Windows_NT)
CSRC += ${PICOLIBSDK_FONT_DIR}/font.c
else
CSRC += ${PICOLIBSDK_FONT_DIR}/font_boldB_8x14.c
CSRC += ${PICOLIBSDK_FONT_DIR}/font_boldB_8x16.c
CSRC += ${PICOLIBSDK_FONT_DIR}/font_bold_8x14.c
CSRC += ${PICOLIBSDK_FONT_DIR}/font_bold_8x16.c
CSRC += ${PICOLIBSDK_FONT_DIR}/font_bold_8x8.c
CSRC += ${PICOLIBSDK_FONT_DIR}/font_cond_6x8.c
CSRC += ${PICOLIBSDK_FONT_DIR}/font_game_8x8.c
CSRC += ${PICOLIBSDK_FONT_DIR}/font_ibmtiny_8x8.c
CSRC += ${PICOLIBSDK_FONT_DIR}/font_ibm_8x14.c
CSRC += ${PICOLIBSDK_FONT_DIR}/font_ibm_8x16.c
CSRC += ${PICOLIBSDK_FONT_DIR}/font_ibm_8x8.c
CSRC += ${PICOLIBSDK_FONT_DIR}/font_italic_8x8.c
CSRC += ${PICOLIBSDK_FONT_DIR}/font_thin_8x8.c
CSRC += ${PICOLIBSDK_FONT_DIR}/font_tiny_5x8.c

CSRC += ${PICOLIBSDK_FONT_DIR}/font_bold_8x16_win1250.c
endif
