CSRC += ${PICOLIBSDK_DEVICES_DIR}/demovga/demovga_bat.c
CSRC += ${PICOLIBSDK_DEVICES_DIR}/demovga/demovga_init.c
CSRC += ${PICOLIBSDK_DEVICES_DIR}/demovga/demovga_ss.c
CSRC += ${PICOLIBSDK_DEVICES_DIR}/demovga/demovga_key.c

DEFINE += -D USE_DEMOVGA=1

MCU=RP2040
FLASHSIZE=0x200000
