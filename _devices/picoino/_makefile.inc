CSRC += ${PICOLIBSDK_DEVICES_DIR}/picoino/picoino_bat.c
CSRC += ${PICOLIBSDK_DEVICES_DIR}/picoino/picoino_init.c
CSRC += ${PICOLIBSDK_DEVICES_DIR}/picoino/picoino_ss.c

DEFINE += -D USE_PICOINO=1

MCU=RP2040
FLASHSIZE=0x200000

ifeq (${DEVICE},picoino10)
DEFINE += -D USE_PICOINO10=1
CSRC += ${PICOLIBSDK_DEVICES_DIR}/picoino/picoino_key.c
else
DEFINE += -D USE_PICOINOMINI=1
CSRC += ${PICOLIBSDK_DEVICES_DIR}/picoino/picoino_keymini.c
endif
