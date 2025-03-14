
ifeq (${OS},Windows_NT)
CSRC += ${PICOLIBSDK_LIB_DIR}/lib.c
else
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_alarm.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_calendar.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_calendar64.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_canvas.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_color.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_config.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_crc.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_decnum.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_draw.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_drawcan.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_drawcan1.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_drawcan2.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_drawcan3.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_drawcan4.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_drawcan6.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_drawcan8.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_drawcan12.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_drawcan16.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_escpkt.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_event.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_fat.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_filesel.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_list.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_malloc.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_mat2d.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_miniring.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_print.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_pwmsnd.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_rand.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_rect.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_ring.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_ringrx.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_ringtx.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_sd.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_stream.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_text.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_textlist.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_textnum.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_tprint.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_tree.c
CSRC += ${PICOLIBSDK_LIB_DIR}/src/lib_video.c

CSRC += ${PICOLIBSDK_LIB_DIR}/mp3/lib_mp3.c

CSRC += ${PICOLIBSDK_LIB_DIR}/emu/emu.c

CSRC += ${PICOLIBSDK_LIB_DIR}/bigint/bigint.c
CSRC += ${PICOLIBSDK_LIB_DIR}/bigint/bernoulli.c

CSRC += ${PICOLIBSDK_LIB_DIR}/real/real.c
endif

# ASM sdk
ifeq (${CPUCLASS},RP2040)
ASRC += ${PICOLIBSDK_LIB_DIR}/src/vga_blitkey.S
endif
