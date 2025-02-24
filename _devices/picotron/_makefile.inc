CSRC += ${PICOLIBSDK_DEVICES_DIR}/picotron/picotron_bat.c
CSRC += ${PICOLIBSDK_DEVICES_DIR}/picotron/picotron_init.c
CSRC += ${PICOLIBSDK_DEVICES_DIR}/picotron/picotron_key.c

DEFINE += -D USE_PICOTRON=1

MCU=RP2040
FLASHSIZE=0x200000
