
# Includes from main Makefile.
# Set parameters TARGET, DEVICE and DEVCLASS, as follows.

# Command to setup target project name (i.e. name of output file).
# Name "LOADER" is reserved for boot loader.
#    set TARGET=HELLO

# Command to select target device (use names from the _setup.bat file).
#    set DEVICE=picopad10

# Command to select class of target device (to use common compilation setup).
#    set DEVCLASS=picopad

# Command to select linker script NO_FLASH ('noflash' = use only RAM, #define flag NO_FLASH)
#    set MEMMAP=noflash

# Setup MCU: RP2040, RP2350A (60 pin), RP2350B (80 pin)
#    MCU=RP2040

# Setup Flash memory (usually RP2040: 0x200000, RP2350: 0x400000)
#    FLASHSIZE=0x200000

# Path to root directory from the project directory (without trailing '/' delimiter)
ifeq ($(PICO_ROOT_PATH),)
PICO_ROOT_PATH = ../../..
endif

# Path to PicoLibSDK _boot2 directory (without trailing '/' delimiter)
ifeq ($(PICOLIBSDK_BOOT2_DIR),)
PICOLIBSDK_BOOT2_DIR = ${PICO_ROOT_PATH}/_boot2
endif

# Path to PicoLibSDK _devices directory (without trailing '/' delimiter)
ifeq ($(PICOLIBSDK_DEVICES_DIR),)
PICOLIBSDK_DEVICES_DIR = ${PICO_ROOT_PATH}/_devices
endif

# Path to PicoLibSDK _display directory (without trailing '/' delimiter)
ifeq ($(PICOLIBSDK_DISPLAY_DIR),)
PICOLIBSDK_DISPLAY_DIR = ${PICO_ROOT_PATH}/_display
endif

# Path to PicoLibSDK _font directory (without trailing '/' delimiter)
ifeq ($(PICOLIBSDK_FONT_DIR),)
PICOLIBSDK_FONT_DIR = ${PICO_ROOT_PATH}/_font
endif

# Path to PicoLibSDK _lib directory (without trailing '/' delimiter)
ifeq ($(PICOLIBSDK_LIB_DIR),)
PICOLIBSDK_LIB_DIR = ${PICO_ROOT_PATH}/_lib
endif

# Path to PicoLibSDK _loader directory (without trailing '/' delimiter)
ifeq ($(PICOLIBSDK_LOADER_DIR),)
PICOLIBSDK_LOADER_DIR = ${PICO_ROOT_PATH}/_loader
endif

# Path to PicoLibSDK _sdk directory (without trailing '/' delimiter)
ifeq ($(PICOLIBSDK_SDK_DIR),)
PICOLIBSDK_SDK_DIR = ${PICO_ROOT_PATH}/_sdk
endif

# Path to PicoLibSDK _tools directory (without trailing '/' delimiter)
ifeq ($(PICOLIBSDK_TOOLS_DIR),)
PICOLIBSDK_TOOLS_DIR = ${PICO_ROOT_PATH}/_tools
endif

# C code define path to root directory
DEFINE += -D PICO_ROOT_PATH=${PICO_ROOT_PATH}

# C code define path to _display directory
DEFINE += -D PICOLIBSDK_DISPLAY_DIR=${PICOLIBSDK_DISPLAY_DIR}

# C code define path to _font directory
DEFINE += -D PICOLIBSDK_FONT_DIR=${PICOLIBSDK_FONT_DIR}

# C code define path to _lib directory
DEFINE += -D PICOLIBSDK_LIB_DIR=${PICOLIBSDK_LIB_DIR}

# C code define path to _sdk directory
DEFINE += -D PICOLIBSDK_SDK_DIR=${PICOLIBSDK_SDK_DIR}

# C code define include base "includes.h" file
DEFINE += -D INCLUDES_H=\"${PICO_ROOT_PATH}/includes.h\"

# C code define include base "config_def.h" file
DEFINE += -D CONFIG_DEF_H=\"${PICO_ROOT_PATH}/config_def.h\"

##############################################################################

# Debug configuration flag: 1=use float from libc, 0=use optimised float
USE_FLOATLIBC=0

# Debug configuration flag: 1=use double from libc, 0=use optimised double
USE_DOUBLELIBC=0

ifeq (${USE_FLOATLIBC},1)
DEFINE += -D USE_FLOATLIBC=1
else
DEFINE += -D USE_FLOATLIBC=0
endif

ifeq (${USE_DOUBLELIBC},1)
DEFINE += -D USE_DOUBLELIBC=1
else
DEFINE += -D USE_DOUBLELIBC=0
endif

##############################################################################
# Input files

# Do not include stdio.h file
DEFINE += -D _STDIO_H_

# Device class
include ${PICOLIBSDK_DEVICES_DIR}/${DEVCLASS}/_makefile.inc

# MCU base setup
ifeq (${MCU},RP2040)
CPUCLASS=RP2040
CPUARCH=M0
RAMSIZE=0x40000
DEFINE += -D RP2040=1
else
CPUCLASS=RP2350
CPUARCH=M33
RAMSIZE=0x80000
DEFINE += -D RP2350=1
ifeq (${MCU},RP2350A)
DEFINE += -D RP2350A=1
else
ifeq (${MCU},RP2350ARISCV)
CPUCLASS=RP2350RISCV
CPUARCH=RISCV
DEFINE += -D RP2350A=1 -D RISCV=1 -D __riscv=1
else
ifeq (${MCU},RP2350BRISCV)
CPUCLASS=RP2350RISCV
CPUARCH=RISCV
DEFINE += -D RP2350B=1 -D RISCV=1 -D __riscv=1
else
DEFINE += -D RP2350B=1
endif
endif
endif
endif

# SDK
include ${PICOLIBSDK_SDK_DIR}/_makefile.inc

# Libraries
include ${PICOLIBSDK_LIB_DIR}/_makefile.inc

# fonts
include ${PICOLIBSDK_FONT_DIR}/_makefile.inc

# Displays
include ${PICOLIBSDK_DISPLAY_DIR}/_makefile.inc

# Includes
IPATH += -I. -I${PICOLIBSDK_SDK_DIR}/

# ASM source files
ifeq (${MEMMAP},noflash)
# NOFLASH - no loader if located in RAM
DEFINE += -D NO_FLASH=1 -D PICO_NO_FLASH=1
else
ifeq (${TARGET},LOADER)
ASRC += ${PICOLIBSDK_BOOT2_DIR}/boot2_w25q080_${CPUCLASS}_bin.S
DEFINE += -D BOOT3_LOADER=1
else
ifeq (${DEVCLASS},pico)
ASRC += ${PICOLIBSDK_BOOT2_DIR}/boot2_w25q080_${CPUCLASS}_bin.S
else
ASRC += ${PICOLIBSDK_LOADER_DIR}/loader_${DEVICE}.S
endif
endif
endif

##############################################################################
# Configuration

# temporary build directory
TEMP = ./build

# Base makefile (to ensure recompilation after change)
MAKEFILE = ./Makefile

# Base include file (to ensure recompilation after change)
INCLUDEFILE = ./include.h

# Base config file (to ensure recompilation after change)
CONFIGFILE = ./config.h

# Memory setup
DEFINE += -D FLASHSIZE=${FLASHSIZE}
DEFINE += -D RAMSIZE=${RAMSIZE}

# linker script
ifeq (${CPUCLASS},RP2040)

ifeq (${TARGET},LOADER)
LDSCRIPT = ${PICO_ROOT_PATH}/memmap_rp2040_boot.ld
else
ifeq (${MEMMAP},noflash)
LDSCRIPT = ${PICO_ROOT_PATH}/memmap_rp2040_noflash.ld
else
ifeq (${DEVCLASS},pico)
LDSCRIPT = ${PICO_ROOT_PATH}/memmap_rp2040_default.ld
else
LDSCRIPT = ${PICO_ROOT_PATH}/memmap_rp2040_app.ld
endif
endif
endif

else

ifeq (${CPUCLASS},RP2350RISCV)

ifeq (${TARGET},LOADER)
LDSCRIPT = ${PICO_ROOT_PATH}/memmap_rp2350riscv_boot.ld
else
ifeq (${MEMMAP},noflash)
LDSCRIPT = ${PICO_ROOT_PATH}/memmap_rp2350riscv_noflash.ld
else
ifeq (${DEVCLASS},pico)
LDSCRIPT = ${PICO_ROOT_PATH}/memmap_rp2350riscv_default.ld
else
LDSCRIPT = ${PICO_ROOT_PATH}/memmap_rp2350riscv_app.ld
endif
endif
endif

else

ifeq (${TARGET},LOADER)
LDSCRIPT = ${PICO_ROOT_PATH}/memmap_rp2350_boot.ld
else
ifeq (${MEMMAP},noflash)
LDSCRIPT = ${PICO_ROOT_PATH}/memmap_rp2350_noflash.ld
else
ifeq (${DEVCLASS},pico)
LDSCRIPT = ${PICO_ROOT_PATH}/memmap_rp2350_default.ld
else
LDSCRIPT = ${PICO_ROOT_PATH}/memmap_rp2350_app.ld
endif
endif
endif

endif

endif

# CPU Architecture
ifeq (${CPUCLASS},RP2040)
ARCHCFG = -march=armv6-m -mcpu=cortex-m0plus -mthumb
# -mfpu=auto
else
ifeq (${CPUCLASS},RP2350)
ifeq (${USE_FLOATLIBC},1)
ARCHCFG = -mcpu=cortex-m33 -march=armv8-m.main+dsp -msoft-float -mfloat-abi=soft -mthumb -mcmse
else
ARCHCFG = -mcpu=cortex-m33 -march=armv8-m.main+fp+dsp -mfloat-abi=softfp -mthumb -mcmse
endif
else
# GCC 13:
ARCHCFG = -march=rv32imac_zicsr_zifencei_zba_zbb_zbs_zbkb
# GCC 14 with older assembler (Zcb: Basic additional compressed instructions; Zca is alias for "c" subset):
# ARCHCFG = -march=rv32ima_zicsr_zifencei_zba_zbb_zbs_zbkb_zca_zcb
# GCC 14 (Zcmp: Push/pop and double-move compressed instructions):
# ARCHCFG = -march=rv32ima_zicsr_zifencei_zba_zbb_zbs_zbkb_zca_zcb_zcmp
endif
endif

##############################################################################
# Compilation flags

# === ASM flags

# MCU
AFLAGS = $(ARCHCFG)

# === C flags

# MCU
CFLAGS = $(ARCHCFG)

# Optimization level
#CFLAGS += -O0	# do no optimization
#CFLAGS += -O1	# optimize minimally
#CFLAGS += -O2	# optimize more
#CFLAGS += -O3	# optimize even more
#CFLAGS += -Ofast # optimize for speed
#CFLAGS += -Og -g3 # optimize for debugger (use together with -g0..-g3, level of debugging information)
CFLAGS += -Os	# optimize for size

# Do not use built-in functions. This is case when compiller changes printf("x") to putchar('x').
#CFLAGS += -fno-builtin

# create separate sections (to enable eliminate unused sections)
CFLAGS += -ffunction-sections -fdata-sections 

# use unsigned char (this is default for ARM cpu)
CFLAGS += -funsigned-char

# use fast math
# CFLAGS += -ffast-math -fno-math-errno -funsafe-math-optimizations -fno-signed-zeros -fno-signaling-nans

# pedantic - to report error when 'return' is missing ... cannot be used, it does not support noname structures
#CFLAGS += -Wall -fms-extensions -ansi -pedantic

# generate debug informations
#CFLAGS += -g3

# warning as errors
CFLAGS += -Werror

# === C++ flags

CPPFLAGS = $(CFLAGS)

# do not use exceptions and unwind frames
CPPFLAGS += -fno-exceptions -fno-unwind-tables
CPPFLAGS += -fno-rtti -fno-use-cxa-atexit -std=gnu++17

# === Linker flags

# MCU
LDFLAGS = $(ARCHCFG)

# no built in
LDFLAGS += -Wl,--build-id=none

# libc without system functions
LDFLAGS += --specs=nosys.specs

# generate map file
LDFLAGS += -Wl,-Map=$(TARGET).map

# Linker script
LDFLAGS += -Wl,-script=$(LDSCRIPT)

# eliminate unused sections
LDFLAGS += -Wl,--gc-sections

# no start files
LDFLAGS += -nostartfiles

# no warning "*.elf has a LOAD segment with RWX permissions"
LDFLAGS += -Wl,--no-warn-rwx-segment

# set memory
LDFLAGS += -Wl,'--defsym=FLASHSIZE=${FLASHSIZE}'
LDFLAGS += -Wl,'--defsym=RAMSIZE=${RAMSIZE}'

# === wrap library functions

# memory
LDFLAGS+=-Wl,--wrap=malloc
LDFLAGS+=-Wl,--wrap=calloc
LDFLAGS+=-Wl,--wrap=free
LDFLAGS+=-Wl,--wrap=realloc

# memory: sdk_memops_asm.S
ifeq (${CPUCLASS},RP2040)
LDFLAGS+=-Wl,--wrap=memset
LDFLAGS+=-Wl,--wrap=__aeabi_memset
LDFLAGS+=-Wl,--wrap=__aeabi_memset4
LDFLAGS+=-Wl,--wrap=__aeabi_memset8
LDFLAGS+=-Wl,--wrap=memcpy
LDFLAGS+=-Wl,--wrap=__aeabi_memcpy
LDFLAGS+=-Wl,--wrap=__aeabi_memcpy4
LDFLAGS+=-Wl,--wrap=__aeabi_memcpy8
endif

# printf
LDFLAGS+=-Wl,--wrap=printf
LDFLAGS+=-Wl,--wrap=vprintf
LDFLAGS+=-Wl,--wrap=sprintf
LDFLAGS+=-Wl,--wrap=snprintf
LDFLAGS+=-Wl,--wrap=vsnprintf
LDFLAGS+=-Wl,--wrap=vsprintf
LDFLAGS+=-Wl,--wrap=puts
LDFLAGS+=-Wl,--wrap=putchar
LDFLAGS+=-Wl,--wrap=getchar

# bit operations
# LDFLAGS+=-Wl,--wrap=__clz
# LDFLAGS+=-Wl,--wrap=__clzl
# LDFLAGS+=-Wl,--wrap=__clzll
# LDFLAGS+=-Wl,--wrap=__clzsi2
# LDFLAGS+=-Wl,--wrap=__clzdi2
# LDFLAGS+=-Wl,--wrap=__ctzsi2
# LDFLAGS+=-Wl,--wrap=__ctzdi2
# LDFLAGS+=-Wl,--wrap=__popcountsi2
# LDFLAGS+=-Wl,--wrap=__popcountdi2

# single float - common functions, faster alternatives to libc
ifneq (${USE_FLOATLIBC},1)
LDFLAGS+=-Wl,--wrap=ldexpf
LDFLAGS+=-Wl,--wrap=truncf
LDFLAGS+=-Wl,--wrap=roundf
LDFLAGS+=-Wl,--wrap=floorf
LDFLAGS+=-Wl,--wrap=ceilf
LDFLAGS+=-Wl,--wrap=asinf
LDFLAGS+=-Wl,--wrap=acosf
# RISC-V: atanf cannot be implemented this way because atanf is internally called from atan2f in libc
#ifneq (${CPUCLASS},RP2350RISCV)
LDFLAGS+=-Wl,--wrap=atanf
#endif
LDFLAGS+=-Wl,--wrap=sinhf
LDFLAGS+=-Wl,--wrap=coshf
LDFLAGS+=-Wl,--wrap=tanhf
LDFLAGS+=-Wl,--wrap=asinhf
LDFLAGS+=-Wl,--wrap=acoshf
LDFLAGS+=-Wl,--wrap=atanhf
LDFLAGS+=-Wl,--wrap=exp2f
LDFLAGS+=-Wl,--wrap=log2f
LDFLAGS+=-Wl,--wrap=exp10f
LDFLAGS+=-Wl,--wrap=log10f
LDFLAGS+=-Wl,--wrap=expm1f
LDFLAGS+=-Wl,--wrap=log1pf
LDFLAGS+=-Wl,--wrap=fmaf
LDFLAGS+=-Wl,--wrap=powintf
LDFLAGS+=-Wl,--wrap=powf
LDFLAGS+=-Wl,--wrap=hypotf
LDFLAGS+=-Wl,--wrap=cbrtf
LDFLAGS+=-Wl,--wrap=fmodf
LDFLAGS+=-Wl,--wrap=remquof
LDFLAGS+=-Wl,--wrap=remainderf
LDFLAGS+=-Wl,--wrap=dremf
endif

# double float - common functions, faster alternatives to libc
ifneq (${USE_DOUBLELIBC},1)
LDFLAGS+=-Wl,--wrap=ldexp
LDFLAGS+=-Wl,--wrap=trunc
LDFLAGS+=-Wl,--wrap=round
LDFLAGS+=-Wl,--wrap=floor
LDFLAGS+=-Wl,--wrap=ceil

# libc is faster
ifneq (${CPUCLASS},RP2350RISCV)
LDFLAGS+=-Wl,--wrap=asin
LDFLAGS+=-Wl,--wrap=acos
endif

# RISC-V: atanf cannot be implemented this way because atanf is internally called from atan2f in libc
ifneq (${CPUCLASS},RP2350RISCV)
LDFLAGS+=-Wl,--wrap=atan
endif

# libc is faster
ifneq (${CPUCLASS},RP2350RISCV)
LDFLAGS+=-Wl,--wrap=sinh
LDFLAGS+=-Wl,--wrap=cosh
endif

LDFLAGS+=-Wl,--wrap=tanh
LDFLAGS+=-Wl,--wrap=asinh
LDFLAGS+=-Wl,--wrap=acosh
LDFLAGS+=-Wl,--wrap=atanh
LDFLAGS+=-Wl,--wrap=exp2
LDFLAGS+=-Wl,--wrap=log2
LDFLAGS+=-Wl,--wrap=exp10
LDFLAGS+=-Wl,--wrap=log10
LDFLAGS+=-Wl,--wrap=expm1
LDFLAGS+=-Wl,--wrap=log1p
LDFLAGS+=-Wl,--wrap=fma
LDFLAGS+=-Wl,--wrap=powint
LDFLAGS+=-Wl,--wrap=pow

# libc is faster
ifneq (${CPUCLASS},RP2350RISCV)
LDFLAGS+=-Wl,--wrap=hypot
LDFLAGS+=-Wl,--wrap=cbrt
endif

LDFLAGS+=-Wl,--wrap=fmod
LDFLAGS+=-Wl,--wrap=remquo
LDFLAGS+=-Wl,--wrap=remainder
LDFLAGS+=-Wl,--wrap=drem
endif

# single float - RP2040 functions
ifeq (${CPUCLASS},RP2040)
ifneq (${USE_FLOATLIBC},1)
LDFLAGS+=-Wl,--wrap=__aeabi_fcmpun
LDFLAGS+=-Wl,--wrap=copysignf
LDFLAGS+=-Wl,--wrap=__aeabi_fadd
LDFLAGS+=-Wl,--wrap=__aeabi_fsub
LDFLAGS+=-Wl,--wrap=__aeabi_frsub
LDFLAGS+=-Wl,--wrap=__aeabi_fmul
LDFLAGS+=-Wl,--wrap=__aeabi_fdiv
LDFLAGS+=-Wl,--wrap=__aeabi_cfrcmple
LDFLAGS+=-Wl,--wrap=__aeabi_cfcmple
LDFLAGS+=-Wl,--wrap=__aeabi_cfcmpeq
LDFLAGS+=-Wl,--wrap=__aeabi_fcmpeq
LDFLAGS+=-Wl,--wrap=__aeabi_fcmplt
LDFLAGS+=-Wl,--wrap=__aeabi_fcmple
LDFLAGS+=-Wl,--wrap=__aeabi_fcmpge
LDFLAGS+=-Wl,--wrap=__aeabi_fcmpgt
LDFLAGS+=-Wl,--wrap=__aeabi_ui2f
LDFLAGS+=-Wl,--wrap=__aeabi_i2f
LDFLAGS+=-Wl,--wrap=__aeabi_ul2f
LDFLAGS+=-Wl,--wrap=__aeabi_l2f
LDFLAGS+=-Wl,--wrap=__aeabi_f2iz
LDFLAGS+=-Wl,--wrap=__aeabi_f2uiz
LDFLAGS+=-Wl,--wrap=__aeabi_f2lz
LDFLAGS+=-Wl,--wrap=__aeabi_f2ulz
LDFLAGS+=-Wl,--wrap=sqrtf
LDFLAGS+=-Wl,--wrap=sinf
LDFLAGS+=-Wl,--wrap=cosf
LDFLAGS+=-Wl,--wrap=sincosf
LDFLAGS+=-Wl,--wrap=tanf
LDFLAGS+=-Wl,--wrap=atan2f
LDFLAGS+=-Wl,--wrap=expf
LDFLAGS+=-Wl,--wrap=logf
LDFLAGS+=-Wl,--wrap=__aeabi_f2d
endif
endif

# double float - RP2040 functions
ifeq (${CPUCLASS},RP2040)
ifneq (${USE_DOUBLELIBC},1)
LDFLAGS+=-Wl,--wrap=__aeabi_dcmpun
LDFLAGS+=-Wl,--wrap=copysign
LDFLAGS+=-Wl,--wrap=__aeabi_dadd
LDFLAGS+=-Wl,--wrap=__aeabi_dsub
LDFLAGS+=-Wl,--wrap=__aeabi_drsub
LDFLAGS+=-Wl,--wrap=__aeabi_dmul
LDFLAGS+=-Wl,--wrap=__aeabi_ddiv
LDFLAGS+=-Wl,--wrap=__aeabi_cdrcmple
LDFLAGS+=-Wl,--wrap=__aeabi_cdcmple
LDFLAGS+=-Wl,--wrap=__aeabi_cdcmpeq
LDFLAGS+=-Wl,--wrap=__aeabi_dcmpeq
LDFLAGS+=-Wl,--wrap=__aeabi_dcmplt
LDFLAGS+=-Wl,--wrap=__aeabi_dcmple
LDFLAGS+=-Wl,--wrap=__aeabi_dcmpge
LDFLAGS+=-Wl,--wrap=__aeabi_dcmpgt
LDFLAGS+=-Wl,--wrap=__aeabi_ui2d
LDFLAGS+=-Wl,--wrap=__aeabi_i2d
LDFLAGS+=-Wl,--wrap=__aeabi_ul2d
LDFLAGS+=-Wl,--wrap=__aeabi_l2d
LDFLAGS+=-Wl,--wrap=__aeabi_d2iz
LDFLAGS+=-Wl,--wrap=__aeabi_d2uiz
LDFLAGS+=-Wl,--wrap=__aeabi_d2lz
LDFLAGS+=-Wl,--wrap=__aeabi_d2ulz
LDFLAGS+=-Wl,--wrap=sqrt
LDFLAGS+=-Wl,--wrap=sin
LDFLAGS+=-Wl,--wrap=cos
LDFLAGS+=-Wl,--wrap=sincos
LDFLAGS+=-Wl,--wrap=tan
LDFLAGS+=-Wl,--wrap=atan2
LDFLAGS+=-Wl,--wrap=exp
LDFLAGS+=-Wl,--wrap=log
LDFLAGS+=-Wl,--wrap=__aeabi_d2f
endif
endif

# single float - RP2350-ARM functions
ifeq (${CPUCLASS},RP2350)
ifneq (${USE_FLOATLIBC},1)
LDFLAGS+=-Wl,--wrap=__aeabi_fcmpun
LDFLAGS+=-Wl,--wrap=copysignf
LDFLAGS+=-Wl,--wrap=__aeabi_fadd
LDFLAGS+=-Wl,--wrap=__aeabi_fsub
LDFLAGS+=-Wl,--wrap=__aeabi_frsub
LDFLAGS+=-Wl,--wrap=__aeabi_fmul
LDFLAGS+=-Wl,--wrap=__aeabi_fdiv
LDFLAGS+=-Wl,--wrap=__aeabi_l2f
LDFLAGS+=-Wl,--wrap=__aeabi_ul2f
LDFLAGS+=-Wl,--wrap=__aeabi_f2lz
LDFLAGS+=-Wl,--wrap=__aeabi_f2ulz
LDFLAGS+=-Wl,--wrap=__aeabi_f2d
LDFLAGS+=-Wl,--wrap=__aeabi_i2f
LDFLAGS+=-Wl,--wrap=__aeabi_ui2f
LDFLAGS+=-Wl,--wrap=__aeabi_f2iz
LDFLAGS+=-Wl,--wrap=__aeabi_f2uiz
LDFLAGS+=-Wl,--wrap=__aeabi_fcmp
LDFLAGS+=-Wl,--wrap=__aeabi_cfrcmple
LDFLAGS+=-Wl,--wrap=__aeabi_cfcmple
LDFLAGS+=-Wl,--wrap=__aeabi_cfcmpeq
LDFLAGS+=-Wl,--wrap=__aeabi_fcmpeq
LDFLAGS+=-Wl,--wrap=__aeabi_fcmplt
LDFLAGS+=-Wl,--wrap=__aeabi_fcmple
LDFLAGS+=-Wl,--wrap=__aeabi_fcmpge
LDFLAGS+=-Wl,--wrap=__aeabi_fcmpgt
LDFLAGS+=-Wl,--wrap=sqrtf
LDFLAGS+=-Wl,--wrap=expf
LDFLAGS+=-Wl,--wrap=logf
LDFLAGS+=-Wl,--wrap=atan2f
LDFLAGS+=-Wl,--wrap=tanf
LDFLAGS+=-Wl,--wrap=sincosf
LDFLAGS+=-Wl,--wrap=sinf
LDFLAGS+=-Wl,--wrap=cosf
endif
endif

# double float - RP2350-ARM functions
ifeq (${CPUCLASS},RP2350)
ifneq (${USE_DOUBLELIBC},1)
LDFLAGS+=-Wl,--wrap=__aeabi_dcmpun
LDFLAGS+=-Wl,--wrap=copysign
LDFLAGS+=-Wl,--wrap=__aeabi_dadd
LDFLAGS+=-Wl,--wrap=__aeabi_dsub
LDFLAGS+=-Wl,--wrap=__aeabi_drsub
LDFLAGS+=-Wl,--wrap=__aeabi_dmul
LDFLAGS+=-Wl,--wrap=__aeabi_ddiv
LDFLAGS+=-Wl,--wrap=__aeabi_d2f
LDFLAGS+=-Wl,--wrap=__aeabi_i2d
LDFLAGS+=-Wl,--wrap=__aeabi_ui2d
LDFLAGS+=-Wl,--wrap=__aeabi_d2iz
LDFLAGS+=-Wl,--wrap=__aeabi_d2uiz
LDFLAGS+=-Wl,--wrap=__aeabi_d2i_r
LDFLAGS+=-Wl,--wrap=__aeabi_d2ui_r
LDFLAGS+=-Wl,--wrap=__aeabi_l2f
LDFLAGS+=-Wl,--wrap=__aeabi_ul2f
LDFLAGS+=-Wl,--wrap=__aeabi_f2lz
LDFLAGS+=-Wl,--wrap=__aeabi_f2ulz
LDFLAGS+=-Wl,--wrap=__aeabi_cdrcmple
LDFLAGS+=-Wl,--wrap=__aeabi_cdcmple
LDFLAGS+=-Wl,--wrap=__aeabi_cdcmpeq
LDFLAGS+=-Wl,--wrap=__aeabi_dcmpeq
LDFLAGS+=-Wl,--wrap=__aeabi_dcmplt
LDFLAGS+=-Wl,--wrap=__aeabi_dcmple
LDFLAGS+=-Wl,--wrap=__aeabi_dcmpge
LDFLAGS+=-Wl,--wrap=__aeabi_dcmpgt
LDFLAGS+=-Wl,--wrap=sqrt
LDFLAGS+=-Wl,--wrap=exp
LDFLAGS+=-Wl,--wrap=log
LDFLAGS+=-Wl,--wrap=atan2
LDFLAGS+=-Wl,--wrap=tan
LDFLAGS+=-Wl,--wrap=sincos
LDFLAGS+=-Wl,--wrap=sin
LDFLAGS+=-Wl,--wrap=cos
endif
endif

# single float - RP2350-RISCV functions
ifeq (${CPUCLASS},RP2350RISCV)
ifneq (${USE_FLOATLIBC},1)
LDFLAGS+=-Wl,--wrap=__addsf3
LDFLAGS+=-Wl,--wrap=__subsf3
LDFLAGS+=-Wl,--wrap=__mulsf3
LDFLAGS+=-Wl,--wrap=__divsf3

LDFLAGS+=-Wl,--wrap=__clzsi2
LDFLAGS+=-Wl,--wrap=__clz2si

LDFLAGS+=-Wl,--wrap=sqrtf

LDFLAGS+=-Wl,--wrap=expf
LDFLAGS+=-Wl,--wrap=exp2f
LDFLAGS+=-Wl,--wrap=exp12f

LDFLAGS+=-Wl,--wrap=logf
LDFLAGS+=-Wl,--wrap=log2f
LDFLAGS+=-Wl,--wrap=log10f

LDFLAGS+=-Wl,--wrap=sinf
LDFLAGS+=-Wl,--wrap=cosf
LDFLAGS+=-Wl,--wrap=sincosf
LDFLAGS+=-Wl,--wrap=tanf
LDFLAGS+=-Wl,--wrap=cotanf

LDFLAGS+=-Wl,--wrap=asinf
LDFLAGS+=-Wl,--wrap=acosf
LDFLAGS+=-Wl,--wrap=atanf
LDFLAGS+=-Wl,--wrap=acotanf
LDFLAGS+=-Wl,--wrap=atan2f

LDFLAGS+=-Wl,--wrap=__fixsfsi
LDFLAGS+=-Wl,--wrap=__fixunssfsi
LDFLAGS+=-Wl,--wrap=__fixsfdi
LDFLAGS+=-Wl,--wrap=__fixunssfdi
LDFLAGS+=-Wl,--wrap=__floatsisf
LDFLAGS+=-Wl,--wrap=__floatunsisf
LDFLAGS+=-Wl,--wrap=__floatdisf
LDFLAGS+=-Wl,--wrap=__floatundisf
endif
endif

# double float - RP2350-RISCV functions
ifeq (${CPUCLASS},RP2350RISCV)
ifneq (${USE_DOUBLELIBC},1)
LDFLAGS+=-Wl,--wrap=__adddf3
LDFLAGS+=-Wl,--wrap=__subdf3
LDFLAGS+=-Wl,--wrap=__muldf3
LDFLAGS+=-Wl,--wrap=__divdf3

LDFLAGS+=-Wl,--wrap=sqrt

LDFLAGS+=-Wl,--wrap=exp
LDFLAGS+=-Wl,--wrap=exp2
LDFLAGS+=-Wl,--wrap=exp10

LDFLAGS+=-Wl,--wrap=log
LDFLAGS+=-Wl,--wrap=log2
LDFLAGS+=-Wl,--wrap=log10

LDFLAGS+=-Wl,--wrap=sin
LDFLAGS+=-Wl,--wrap=cos
LDFLAGS+=-Wl,--wrap=sincos
LDFLAGS+=-Wl,--wrap=tan
LDFLAGS+=-Wl,--wrap=cotan

LDFLAGS+=-Wl,--wrap=asin
LDFLAGS+=-Wl,--wrap=acos
LDFLAGS+=-Wl,--wrap=atan
LDFLAGS+=-Wl,--wrap=acotan
LDFLAGS+=-Wl,--wrap=atan2

LDFLAGS+=-Wl,--wrap=sinh
LDFLAGS+=-Wl,--wrap=cosh
LDFLAGS+=-Wl,--wrap=tanh
LDFLAGS+=-Wl,--wrap=asinh
LDFLAGS+=-Wl,--wrap=acosh
LDFLAGS+=-Wl,--wrap=atanh

LDFLAGS+=-Wl,--wrap=powint
LDFLAGS+=-Wl,--wrap=pow
LDFLAGS+=-Wl,--wrap=hypot
LDFLAGS+=-Wl,--wrap=cbrt

endif
endif

# integer operations: sdk_divider.S
ifeq (${CPUCLASS},RP2040)
LDFLAGS+=-Wl,--wrap=__aeabi_idiv
LDFLAGS+=-Wl,--wrap=__aeabi_idivmod
LDFLAGS+=-Wl,--wrap=__aeabi_uidiv
LDFLAGS+=-Wl,--wrap=__aeabi_uidivmod
LDFLAGS+=-Wl,--wrap=__aeabi_ldiv
LDFLAGS+=-Wl,--wrap=__aeabi_ldivmod
LDFLAGS+=-Wl,--wrap=__aeabi_uldiv
LDFLAGS+=-Wl,--wrap=__aeabi_uldivmod
LDFLAGS+=-Wl,--wrap=__aeabi_lmul
endif

##############################################################################
# Compiler

# compiler base
ifeq (${CPUCLASS},RP2350RISCV)
COMP = riscv32-unknown-elf-
else
COMP = arm-none-eabi-
endif

# C compiler name
CC = ${COMP}gcc

# C++ compiler name
CPP = ${COMP}g++

# ASM compiler name
AS = ${COMP}gcc

# Linker
LD = ${COMP}g++

# Object dump
OD = ${COMP}objdump

# Object copy
OC = ${COMP}objcopy

# Names
NM = ${COMP}nm

# Size
SZ = ${COMP}size

# uf2
ifeq (${OS},Windows_NT)
UF = ${PICOLIBSDK_TOOLS_DIR}/elf2uf2.exe
else
UF = ${PICOLIBSDK_TOOLS_DIR}/elf2uf2/elf2uf2
endif

# uf2 family_id
ifeq (${CPUCLASS},RP2040)
FAMILY_ID = 1
else
ifeq (${CPUCLASS},RP2350RISCV)
FAMILY_ID = 3
else
FAMILY_ID = 2
endif
endif

##############################################################################
# File list

# list of all source files
ALLSRC = $(ASRC) $(CSRC) $(SRC)

# list of all files without directory and without extension
ALLBASE = $(notdir $(basename $(ALLSRC)))

# list of all object files *.o
ALLOBJ = $(addprefix $(TEMP)/, $(addsuffix .o, $(ALLBASE)))

##############################################################################
# Clear temporary directory (use "make clean")

clean:
	@${PICOLIBSDK_TOOLS_DIR}/rm -rf $(TEMP)
	@${PICOLIBSDK_TOOLS_DIR}/rm -f ./$(TARGET).bin ./$(TARGET).hex ./$(TARGET).lst ./$(TARGET).map ./$(TARGET).sym ./$(TARGET).siz ./$(TARGET).uf2

##############################################################################
# Compile all (use "make all")

all: createdirs elf bin hex lst sym siz uf2

##############################################################################
# Create temporary directory

createdirs:
ifeq (${OS},Windows_NT)
	@${PICOLIBSDK_TOOLS_DIR}/mkdir.exe -p $(TEMP)
else
	@mkdir -p $(TEMP)
endif

##############################################################################
# Dependencies of output files

elf: $(TEMP)/$(TARGET).elf
bin: ./$(TARGET).bin
hex: ./$(TARGET).hex
lst: ./$(TARGET).lst
sym: ./$(TARGET).sym
siz: ./$(TARGET).siz
uf2: ./$(TARGET).uf2

##############################################################################
# Create .bin file from .elf file

./$(TARGET).bin: $(TEMP)/$(TARGET).elf
	@echo     bin		 $@
	@$(OC) -O binary $< $@

##############################################################################
# Create .hex file from .elf file

./$(TARGET).hex: $(TEMP)/$(TARGET).elf
	@echo     hex		 $@
	@$(OC) -O ihex $< $@

##############################################################################
# Create .lst file from .elf file

./$(TARGET).lst: $(TEMP)/$(TARGET).elf
	@echo     lst		 $@
	@$(OD) -d -S $< > $@

##############################################################################
# Create .sym file from .elf file

./$(TARGET).sym: $(TEMP)/$(TARGET).elf
	@echo     sym		 $@
	@$(NM) -n $< > $@

##############################################################################
# Create .siz file from .elf file

./$(TARGET).siz: $(TEMP)/$(TARGET).elf
	@echo     siz		 $@
	@$(SZ) $< > $@

##############################################################################
# Create .uf2 file from .elf file

./$(TARGET).uf2: $(TEMP)/$(TARGET).elf
	@echo     uf2		 $@
	@$(UF) $< $@ $(FAMILY_ID)

##############################################################################
# Create .o files from .c files

define CC_TEMP
$(TEMP)/$(notdir $(basename $(1))).o : $(1) $(MAKEFILE) $(INCLUDEFILE) $(CONFIGFILE)
	@echo     CC		 $$<
	@$(CC) $$(DEFINE) $$(IPATH) $$(CFLAGS) -std=gnu11 -c $$< -o $$@
endef

$(foreach src, $(CSRC), $(eval $(call CC_TEMP, $(src))))

##############################################################################
# Create .o files from .cpp files

define CPP_TEMP
$(TEMP)/$(notdir $(basename $(1))).o : $(1) $(MAKEFILE) $(INCLUDEFILE) $(CONFIGFILE)
	@echo     C++		 $$<
	@$(CPP) $$(DEFINE) $$(IPATH) $$(CPPFLAGS) -c $$< -o $$@
endef

$(foreach src, $(SRC), $(eval $(call CPP_TEMP, $(src))))

##############################################################################
# Create .o files from .S files

define AS_TEMP
$(TEMP)/$(notdir $(basename $(1))).o : $(1) $(MAKEFILE) $(INCLUDEFILE) $(CONFIGFILE)
	@echo     ASM		 $$<
	@$(CC) $$(DEFINE) $$(IPATH) $$(AFLAGS) -c $$< -o $$@
endef

$(foreach src, $(ASRC), $(eval $(call AS_TEMP, $(src))))

##############################################################################
# Create .elf from .o files

$(TEMP)/$(TARGET).elf: $(ALLOBJ) $(MAKEFILE) $(LDSCRIPT)
	@echo     ld		 $@
	@$(LD) $(LDFLAGS) $(ALLOBJ) -o $@

##############################################################################
# Head dependencies, to recompile C after changing H

ifneq (${MAKECMDGOALS},clean)
-include $(wildcard $(TEMP)/*.d)
endif

##############################################################################
# List of dependencies not creating files

.PHONY: clean all createdirs elf bin hex lst sym siz uf2
