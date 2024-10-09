#   make clean ... clean
#   make all ... compile TARGET

# Options:
# TARGET ... source file name (without extension)
# MCU ... mcu RP2040, RP2350A (60 pin), RP2350B (80 pin)
# CPUTYPE ... cortex-m0plus, cortex-m33, risc-v

ifeq (${MCU},RP2040)
CPUCLASS=RP2040
else
CPUCLASS=RP2350
endif

##############################################################################
# Input files

# ASM source files
ASRC +=	${TARGET}.S

##############################################################################
# Configuration

# destination filename
#TARGET = boot2

# stack size of one CPU core (should be 2KB to fit into half of SRAM banks 4 and 5)
STACK_SIZE = 0x0800

# temporary build directory
TEMP = ./build

# Base makefile (to ensure recompilation after change)
MAKEFILE = ./Makefile

# linker script
LDSCRIPT = ./boot_stage2_${CPUCLASS}.ld

# CPU
# CPU Architecture
ifeq (${CPUCLASS},RP2040)
ARCHCFG = -march=armv6-m -mcpu=cortex-m0plus -mthumb -mfpu=auto -mfloat-abi=soft
else
ifeq (${CPUTYPE},risc-v)
ARCHCFG = -march=rv32imac_zicsr_zifencei_zba_zbb_zbkb_zbs  -D __riscv=1
else
ARCHCFG = -mcpu=cortex-m33 -march=armv8-m.main+fp+dsp -mthumb -mfloat-abi=softfp -mcmse
endif
endif

##############################################################################
# Compilation flags

# === ASM flags

# MCU
AFLAGS += ${ARCHCFG}

# === C flags

# MCU
CFLAGS += ${ARCHCFG}

# Optimization level
#CFLAGS += -O0	# do no optimization
#CFLAGS += -O1	# optimize minimally
CFLAGS += -O2	# optimize more
#CFLAGS += -O3	# optimize even more (do not use, compilation may be incorrect)
#CFLAGS += -Ofast # optimize for speed (do not use, compilation may be incorrect)
#CFLAGS += -Og -g3 # optimize for debugger (use together with -g0..-g3, level of debugging information)
#CFLAGS += -Os	# optimize for size

# do not use builtin functions (memset)
CFLAGS += -Wl,--build-id=none
#CFLAGS += -fno-builtin

# do not use standard includes
#CFLAGS += -nostdinc

# create separate sections (to enable eliminate unused sections)
CFLAGS += -fdata-sections -ffunction-sections

# use signed char
CFLAGS += -fsigned-char

# do not assume usual definitions
#CFLAGS += -ffreestanding

# do not use exceptions and unwind frames
CFLAGS += -fno-unwind-tables -fno-exceptions

# --specs=nosys.specs ... do not use _exit
#CFLAGS += --specs=nosys.specs

# --specs=nano.specs ... use small library without printf float
#CFLAGS += --specs=nano.specs

# generate debug informations
#CFLAGS += -g3

# === C++ flags

CPPFLAGS = $(CFLAGS) -std=c++14 -Wno-aligned-new -fno-rtti -nostdinc++

# === Linker flags

# Entry point
LDFLAGS += --entry _stage2_boot

# Linker script
LDFLAGS += -Wl,-script=$(LDSCRIPT)

# do not link startup code and libs
LDFLAGS += -Wl,-nostdlib
# LDFLAGS += -Wl,-nostartfiles

# libc without system functions
LDFLAGS += --specs=nosys.specs 

# do not use builtin functions (memset)
LDLAGS += -Wl,--build-id=none

# symbol map
LDFLAGS += -Wl,-Map=./$(TARGET).map

# eliminate unused sections
LDFLAGS += -Wl,--gc-sections,--relax

# no start files
LDFLAGS += -nostartfiles

##############################################################################
# Compiler

# compiler base
ifeq (${CPUTYPE},risc-v)
COMP = riscv32-unknown-elf-
else
COMP = arm-none-eabi-
endif

# C compiler name
CC = ${COMP}gcc

# C++ compiler name
CPP = ${COMP}g++

# ASM compiler name
AS = ${COMP}gcc -x assembler-with-cpp

# Linker
LD = ${COMP}gcc

# Object dump
OD = ${COMP}objdump

# Object copy
OC = ${COMP}objcopy

# Names
NM = ${COMP}nm

# Size
SZ = ${COMP}size

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
	@../_tools/rm.exe -rf $(TEMP)
	@../_tools/rm.exe -f ./$(TARGET).bin ./$(TARGET).hex ./$(TARGET).lst ./$(TARGET).map ./$(TARGET).sym ./$(TARGET).siz

##############################################################################
# Compile all (use "make all")

all: createdirs elf bin hex lst sym siz

##############################################################################
# Create temporary directory

createdirs:
	@../_tools/mkdir.exe -p $(TEMP)

##############################################################################
# Dependencies of output files

elf: $(TEMP)/$(TARGET).elf
bin: ./$(TARGET).bin
hex: ./$(TARGET).hex
lst: ./$(TARGET).lst
sym: ./$(TARGET).sym
siz: ./$(TARGET).siz

##############################################################################
# Create .bin file from .elf file

./$(TARGET).bin: $(TEMP)/$(TARGET).elf
	@echo "    bin		 $@"
	@$(OC) -O binary $< $@

##############################################################################
# Create .hex file from .elf file

./$(TARGET).hex: $(TEMP)/$(TARGET).elf
	@echo "    hex		 $@"
	@$(OC) -O ihex $< $@

##############################################################################
# Create .lst file from .elf file

./$(TARGET).lst: $(TEMP)/$(TARGET).elf
	@echo "    lst		 $@"
	@$(OD) -d -S $< > $@

##############################################################################
# Create .sym file from .elf file

./$(TARGET).sym: $(TEMP)/$(TARGET).elf
	@echo "    sym		 $@"
	@$(NM) -n $< > $@

##############################################################################
# Create .siz file from .elf file

./$(TARGET).siz: $(TEMP)/$(TARGET).elf
	@echo "    siz		 $@"
	@$(SZ) $< > $@

##############################################################################
# Create .o files from .c files

define CC_TEMP
$(TEMP)/$(notdir $(basename $(1))).o : $(1) $(MAKEFILE)
	@echo "    CC		 $$<"
	@$(CC) $$(CFLAGS) -std=gnu99 -c $$< -o $$@
endef

$(foreach src, $(CSRC), $(eval $(call CC_TEMP, $(src))))

##############################################################################
# Create .o files from .cpp files

define CPP_TEMP
$(TEMP)/$(notdir $(basename $(1))).o : $(1) $(MAKEFILE)
	@echo "    C++		 $$<"
	@$(CPP) $$(CPPFLAGS) -c $$< -o $$@
endef

$(foreach src, $(SRC), $(eval $(call CPP_TEMP, $(src))))

##############################################################################
# Create .o files from .S files

define AS_TEMP
$(TEMP)/$(notdir $(basename $(1))).o : $(1) $(MAKEFILE)
	@echo "    ASM		 $$<"
	@$(AS) $$(CFLAGS) $$(AFLAGS) -c $$< -o $$@
endef

$(foreach src, $(ASRC), $(eval $(call AS_TEMP, $(src))))

##############################################################################
# Create .elf from .o files

$(TEMP)/$(TARGET).elf: $(ALLOBJ) $(MAKEFILE) $(LDSCRIPT)
	@echo "    ld		 $@"
	@$(LD) $(LDFLAGS) $(ALLOBJ) -o $@

##############################################################################
# Head dependencies, to recompile C after changing H

ifneq (${MAKECMDGOALS},clean)
-include $(wildcard $(TEMP)/*.d)
endif

##############################################################################
# List of dependencies not creating files

.PHONY: clean all createdirs elf bin hex lst sym siz
