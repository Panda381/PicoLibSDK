
PicoLibSDK - Alternative SDK library for Raspberry Pico, RP2040 and RP2350
==========================================================================
SDK Programmer's Guide, Version 2.08, May 2025

Copyright (c) 2023-2025 Miroslav Nemecek

Panda38@seznam.cz
hardyplotter2@gmail.com
https://github.com/Panda381/PicoLibSDK
https://www.breatharian.eu/hw/picolibsdk/index_en.html
https://github.com/pajenicko/picopad
https://picopad.eu/en/


About
-----
PicoLibSDK is an alternative extended C/C++ SDK library for the Raspberry Pico
or Pico 2 module with the RP2040 or RP2350 processor (in the ARM or RISC-V
mode), but it can also be used for other modules which use these processors.
Compared to the original SDK library, officially provided with the Raspberry
Pico, the PicoLibSDK library tries to extend the functionality of the original
library and especially to make the SDK library easier to use. But most of
functions of original SDK are provided here as well, for backwards
compatibility. What you can find in the PicoLibSDK library:

- Boot Loader: Boot loader allowing to select and run UF2 programs from SD card.

- SDK hardware control: ADC, boot ROM, clocks control, CPU control, hardware
  divider, DMA, doorbells, double and float arithmetics, FIFO mailboxes, flash
  programming, GPIO, HSTX, I2C, hardware interpolator, IRQ, multicore, PIO, PLL,
  PWM, QSPI, reset and power control, ROSC, RTC, SHA256, SPI, spinlocks,
  SysTick, alarm timer, TMDS, TRNG, watchdog, XOSC. The RP2350 processor can be
  used in ARM or RISC-V mode.

- Tool library: alarm, 32-bit Unix calendar, long 64-bit astronomic calendar,
  canvas drawing, RGBA color vector, CRC check with DMA support, decode numbers,
  emulators, escape packet protocol, event ring buffer, FAT file system, doubly
  linked list, memory allocator, 2D transformation matrix, mini-ring buffer,
  formatted print, PWM sound output with ADPCM, random generator, rectangle,
  ring buffer, DMA ring buffer, SD card, streams, text strings, text list, text
  print, tree list, VGA drawing, video player, MP3 player.

- USB library: multiplayer mini-port, CDC device and host - serial communication,
  HID device and host - including external keyboard and mouse.

- Big intergers: calculations with large integers, calculation of Bernoulli
  numbers.

- Real numbers: calculations with floating-point numbers with optional precision
  up to 3690 digits and 30-bit exponent. Scientific functions with optional
  calculation method - Ln, Exp, Sqrt, Sin, Cos, Tan, arcus, hyperbolic functions
  and many more. Linear factorials with accurate and fast calculation.

- Display drivers: Prepared support of TFT, VGA and HDMI display with resolution
  320x240 up to 800x600, with 4, 8, 15 or 16 bits output.

- Devices: Support of Picoino/PicoinoMini with 8-bit QVGA display, DemoVGA with
  16-bit VGA display, Picotron with 4-bit VGA display and PicoPad with 16-bit
  TFT display. Some samples are also prepared for the basic Raspberry Pico
  without additional hardware.


License
-------
The library source code is, with a few exceptions, completely free to use for
any purpose, including commercial use. It is possible to use and modify all or
parts of the library source code without restriction. Some libraries (mainly
single- and double-floating-point mathematics) are mostly the copyrighted work
of Raspberry Pi and are therefore subject to the licensing terms of the original
authors.


Building
--------
The compilation of the PicoLibSDK library and its sample examples is mainly
prepared for easy use on Windows. It does not require any additional programs
and utilities, only the installation of the GCC ARM compiler.

GCC ARM Installation in Windows

Download the GCC ARM compiler from
https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads. You can use
Windows Installer. To compile RP2350 (Pico 2), you will need GCC-ARM compiler
version 13.3 or more. To compile RP2350 in RISC-V mode, you will need Hazard3
RISC-V compiler. You can use GCC-RISCV32 compiler version 13.2 or more:
https://www.embecosm.com/resources/tool-chain-downloads/#corev. This version
does not support compressed instructions, resulting code is a litle bigger. If
you will use GCC version 14.2, you will need to build it yourself, but you can
also use compressed RISC-V instructions, the resulting code will be smaller. In
such case, edit Makefile.inc in base directory of PicoLibSDK, and change
compilation switch to GCC 14 (search “CPU Architecture”). You can install both
compilers in the same directory, e.g. C:\ARM.

In the last stage of the installation, enable the "Add path to environment
variable" option. This will ensure that the compiler files will be found.
Note: If you are using a shell program such as FAR, you must exit and restart
the program to update the PATH system variable.

How to compile in Windows

The compilation of the PicoLibSDK library was prepared primarily for ease of use
in cooperation with the FAR file manager or another file manager that supports
the console window. For each sample application, there are several BAT command
files that can be easily run from the FAR by pressing a letter and Enter. The
compilation is done using c.bat (in FAR press letter C and Enter). The d.bat
file is used to clean up the compilation and delete generated intermediate
files. Use e.bat to send the compiled program to the Raspberry Pico. The a.bat
file performs all 3 operations at once - cleaning up old compilation, compile
the program and send it to the Pico.

The target device for which the compilation is performed is passed as
a parameter via the command line. A default target device is prepared in the
c.bat file in case you do not specify it as a parameter. In the Pico folder, the
default compilation is for the target device 'pico' (the Raspberry Pico module
itself without any added hardware), in the DemoVGA folder the compilation is for
'demovga', in the Picoino folder the compilation is for 'picoino10', in the
Picotron folder the compilation is for 'picotron' and in the PicoPad folder
the default compilation is for 'picopad20' (or select 'picopad10' or
'picopad20riscv' batch file).

If the target device is not passed to the compilation files, it is compiled for
the default device set in the _setup.bat file. This also applies if you compile
all programs using c_all.bat. You can change the default target device by moving
the :default label in the _setup.bat file.

Sending the program to Pico using e.bat is done by copying the file to the Pico
access disk. This assumes that the Pico access disk is labeled R: (like
Raspberry). If your disk has a different name, correct the disk name in the
_e1.bat file (in the base folder of the PicoLibSDK library) or rename the disk
in the system disk management (This Computer - Service - Disk Management -
Change Drive Letter and Paths).

Cleaning up the compilation can be useful not only to clean up the application
directory, but also during application development. Normally, all files are
compiled during the first compilation. When recompiling, only the changed files
are compiled. However, this only applies to *.c and *.cpp and *.s files, not *.h
header files. If you make a change to a *.h file, you may need to either modify
the C file that uses that *.h file, or clean up the compilation with d.bat
before recompiling to do a complete compilation.

The Windows compilation does not use CMake to optimize the compilation, all
files are always compiled. This can be tedious, however - the programmer usually
spends most of the time thinking and writing the program, and only a small part
on compiling. Therefore, it may be preferable to handle the compilation more
easily, rather than saving some compilation time. The final code is not affected
- the linker will only use the parts of the program that are really needed in
the final code. If you want to use some part of the library, you usually don't
need to activate its compilation (with exceptions such as USB or real numbers),
you just use necessary functions in the program.

How to compile in Linux

The PicoLibSDK library contains basic script files for program compilation.
After downloading the PicoLibSDK library to the Linux, it is necessary to set
the EXEC attribute of the execution scripts - the scripts are exported from the
Windows environment and therefore do not have the EXEC attribute set. You can
use the following command to set the script attribute in a batch:

find . -type f -name *.sh -exec chmod +x {} \;

To compile, you need at least the elf2uf2 program, which you can get from the
original SDK library. Place the program in the _tools folder. Second, you will
need the LoaderCrc program, which is used to calculate the checksum of
applications run by the boot3 loader. You can find the source code for this
program in the _tools/PicoPadLoaderCrc folder where you can compile it.

As a working environment, it is a good idea to use Midnight Commander, with
which you can edit programs and immediately compile and run them using prepared
scripts.

To compile the program, run the c.sh script. Compilation is done implicitly for
the target device PicoPad 1.0. If you want to compile for a different device,
you can either pass the device name as a parameter via the command line, or edit
the parameters for the default target device in the _setup.ch script, where you
can also find the names of possible target devices.

The d.sh script is used to clean up the compilation and delete generated
intermediate files. Use e.sh to send the compiled program to the Raspberry Pico
- to use it, edit mount point in _e1.sh. The a.sh file performs all 3 operations
at once - cleaning up old compilation, compile the program and send it to the
Pico. You will also need to delete the compiled files using d.sh if you change
the contents of the *.h header file. Without clearing the compilation, the
recompilation of dependent *.c files would not take place (dependency method is
not used here).

The compilation does not use CMake to optimize the compilation, all files are
always compiled. The final code is not affected - the linker will only use the
parts of the program that are really needed in the final code. This method is
used to make the compilation easier to use. If you want to use some part of the
library, you usually don't need to activate its compilation (with exceptions
such as USB or real numbers), you just use necessary functions in the program.


Configuration
-------------
To configure application compilation, switches defined by #define C preprocessor
commands are used. In most cases, there is no need to change the compilation
configuration - it is predefined optimally for the target device for which the
application is being compiled. Changing the configuration may be necessary in
cases where a module is disabled and needs to be activated. This is for example
activating the STDIO output on a USB port that is disabled by default.

The default configuration file is config.h, which is located in the base folder
of the application. For example, by entering the command:

#define USE_USB_STDIO 1 // use USB stdio (UsbPrint function)

activates the USB STDIO output (the output from printf() will go to the COM
console on the host system).

Most configuration switches use a value of 1 to turn the module on, and a value
of 0 to turn it off.

If a configuration switch is not specified at all in the config.h file, its
default setting is used. In the first step, the default setting is used in the
_config.h file, which is found in the home folder of the files for the target
device (e.g. _devices\picopad).

If the configuration switch is not defined there either, the default switch
setting is used, which is specified in the config_def.h file located in the base
folder of the PicoLibSDK library.

Some configuration switches can be automatically reconfigured. This applies when
a module requires another module - in that case, the other module is activated
even if the user has requested that it be deactivated. An example would be the
activation of USB device. If following command is given:

#define USE_USB_DEV_CDC 1

to activate the USB communication UART interface, the USE_USB_DEV (USB device)
and USE_USB (USB interface) switches are also automatically activated.


How to create new project
-------------------------
To start a new project, the easiest way is to go to the NEW folder and copy the
NEW project folder under a new name (e.g. TEST). In the new folder, open the
c.bat, d.bat and e.bat files and change the settings of the TARGET system
variable to the name of the new project (e.g. "set TARGET=TEST"). If you want
the compiled project to be uploaded to another folder in the target SD card
image, change the setting of the GRPDIR parameter in c.bat - this is the name
of the project group folder.

Do not use the name LOADER for the project - this name is used to distinguish
the boot loader during compilation.

Use only names with a maximum length of 8 characters (uppercase), as the boot3
loader only supports short names 8.3 of the FAT file system.

You can add configuration switches to the file config.h in your new project. For
example, if you want the printf output to run to a USB virtual serial port, set
the parameter:

#define USE_USB_STDIO	1	// use USB stdio (UsbPrint function)

You can add your *.h header files to the include.h file. These files will be
visible to all your source code. Add include files at the end of the file, after
the main include , e.g.:

#include "src/main.h"		// main code
#include "src/game.h"		// game

You can add your source files to the Makefile file. Add ASM files to the group
ASRC, add C source files to the group CSRC and add C++ source files to the group
SRC. For example add source of image:

SRC += src/main.cpp
SRC += img/logo.cpp

Remember that if you have some functions in a C or S (ASM) file and you want to
call them from a CPP file, you have to mark the functions in the *.h header file
with the extern "C" tag so that their names are visible in the CPP file. For
example:

#ifdef __cplusplus
extern "C" {
#endif

void MyFunction();

#ifdef __cplusplus
}
#endif
 

Select or create new target device
----------------------------------
PicoLibSDK can be compiled with miscellaneous target devices. In the current
version of the library, programs are compiled implicitly for PicoPad version
1.0. To change the default target device, or to create a new target device,
open the _setup.bat file in the base PicoLibSDK folder. You will see several
settings there. Move the ":default" label in front of another parameter set to
change the default target device.

You create a new device by creating a new parameter group and changing the
parameters in the meaning: DEVICE is the name of the target device, including
the version. It is used in the source code to distinguish device variants.
DEVCLASS is the class of the target device - used primarily in Makefile to
distinguish the compilation type. DEVDIR is the path to the SD card image.
It is used when copying a compiled file to the target SD card.

You can also select the target device to be compiled in the c.bat file. In
multiple compilation, the name of the target device is passed here as the %1
parameter, which is passed forward on the last row of the file as the parameter
for the _c1.bat file. You can prepend another row before the last row, in which
you set the name of the target device as a parameter, according to the list of
names given in the _setup.bat file. For example, if you want to compile for
picopad10 (you can leave the last row, the row added before it will take
precedence):

..\..\_c1.bat picopad10
..\..\_c1.bat %1

The following text refers only to the creation of a new device.

As you can see in the Makefile.inc in the base folder of the library, the
_makefile.inc file from the device folder is included in the Makefile, named
according to the DEVCLASS parameter. So, for a new device, you must create a
folder in the _devices folder named by DEVCLASS, and create the _makefile.inc
file there. This will contain a list of source files and possibly also DEFINE
parameters. First of all, you need to add a DEFINE specifying that it is
compiled for this device. For example:

DEFINE += -D USE_PICOINO=1

Add the include file _config.h from the device folder to the config_def.h file,
conditioned on the switch for the device. It will contain additional
configuration parameter settings. For example:

#if USE_PICOINO // use Picoino device configuration
#include "_devices/picoino/_config.h"
#endif

In the includes.h file in the base library folder, add a call of the _include.h
file from the device folder, conditioned on the switch for the device. It will
contain the included header files for the device. For example:

#if USE_PICOINO // use Picoino device configuration
#include "_devices/picoino/_include.h"
#endif


Conventions
-----------
The library source files use the Windows style notation for function names and
global variables - the initial letters of words are uppercase, other letters are
lowercase. Constants #define are uppercase. Local variables are lowercase. Types
of structures start with the letter 's'.

Structure definitions of hardware are not used to access the peripheral
registers, but direct access via register address definitions with #define is
used. Similarly, the library usually does not use predefined constants to
manipulate registers, but uses directly numeric values according to the
datasheet. These practices are used for simplicity of access. The usual
conventions using definitions require searching constantly between the datasheet
and the definition files, and so rather than making the job easier, they make
the job more complicated. This simplified approach only requires the use of the
datasheet. This does not affect the SDK library user, as most operations are
covered by functions, without having to work directly with registers.

Base Types:

s8	signed 8-bit integer
u8	unsigned 8-bit integer
s16	signed 16-bit integer
u16	unsigned 16-bit integer
s32	signed 32-bit integer
u32	unsigned 32-bit integer
s64	signed 64-bit integer
u64	unsigned 64-bit integer

int	signed integer
uint	unsigned integer

Bool	logical Boolean type with 8-bit size
True	logical Boolean true value
False	logical Boolean false value

NULL	invalid pointer

Note: 'char' can be signed or unsigned. Better to use s8 or u8 if you need
      to be sure.

Useful Constants and Macros:

count_of(a)	count of array entries
INLINE		inline function
NOINLINE	no-inline function
ALIGNED		array aligned to 4-bytes
PACKED		packed structure with unaligned entries
NOFLASH(fnc)	time critical function placed into RAM
ENDIAN16(k)	change 16-bit endian (little endian <-> big endian)
ABS(val)	absolute value
MAX(val1,val2)	maximal value
MIN(val1,val2)	minimal value
B0..B31		bit 0..bit 31 (= 0x00000001 .. 0x80000000)
BIT(pos)	bit at given position
BIGINT		big integer value (=0x40000000)
KHZ		kHz multiply (= 1000)
MHZ		MHz multiply (= 1000000)
NOCHAR		no character from keyboard (= 0)
NOKEY		no key from keyboard (= 0)

Control characters

symbolic name / HEX code / text code / ASCII name / Control-letter / usage
CH_NUL		0x00	'\0'	NUL	^@ no character, end of text
CH_ALL		0x01	'\1'	SOH	^A select [A]ll
CH_BLOCK	0x02	'\2'	STX	^B mark [B]lock
CH_STX = CH_BLOCK (start of packet)
CH_COPY		0x03	'\3'	ETX	^C [C]opy block, copy file
CH_ETX = CH_COPY (end of packet)
CH_END		0x04	'\4'	EOT	^D en[D] of row, end of files
CH_MOVE		0x05	'\5'	ENQ	^E rename files, mov[E] block
CH_FIND		0x06	'\6'	ACK	^F [F]ind
CH_NEXT		0x07	'\a'	BEL	^G [G]o next, repeat find
CH_BS		0x08	'\b'	BS	^H backspace
CH_TAB		0x09	'\t'	HT	^I tabulator
CH_LF		0x0A	'\n'	LF	^J line feed
CH_PGUP		0x0B	'\v'	VT	^K page up
CH_PGDN		0x0C	'\f'	FF	^L page down
CH_FF = CH_PGDN (form feed, new page)
CH_CR		0x0D	'\r'	CR	^M enter, next row, run file
CH_NEW		0x0E	'\16'	SO	^N [N]ew file
CH_OPEN		0x0F	'\17'	SI	^O [O]pen file, edit file
CH_PRINT	0x10	'\20'	DLE	^P [P]rint file, send file
CH_QUERY	0x11	'\21'	DC1	^Q [Q]uery, display help
CH_REPLACE	0x12	'\22'	DC2	^R find and [R]eplace
CH_SAVE		0x13	'\23'	DC3	^S [S]ave file
CH_INS		0x14	'\24'	DC4	^T [T]oggle Insert switch, mark file
CH_HOME		0x15	'\25'	NAK	^U Home, begin of row, begin of files
CH_PASTE	0x16	'\26'	SYN	^V paste from clipboard
CH_SYN = CH_PASTE (synchronise transfer)
CH_CLOSE	0x17	'\27'	ETB	^W close file
CH_CUT		0x18	'\30'	CAN	^X cut selected text
CH_REDO		0x19	'\31'	EM	^Y redo previously undo action
CH_UNDO		0x1A	'\32'	SUB	^Z undo action
CH_ESC		0x1B	'\e'	ESC	^[ Esc, break, menu
CH_RIGHT	0x1C	'\34'	FS	^\ Right (+Shift: End, +Ctrl: Word right)
CH_UP		0x1D	'\35'	GS	^] Up (+Shift: PageUp, +Ctrl: Text start)
CH_LEFT		0x1E	'\36'	RS	^^ Left (+Shift: Home, +Ctrl: Word left)
CH_DOWN		0x1F	'\37'	US	^_ Down (+Shift: PageDn, +Ctrl: Text end)
CH_SPC		0x20	' '	SPC	space
CH_DEL		0x7F	'\x7F'	DEL	delete character on cursor


Directory structure
-------------------
How PicoLibSDK library files and directories are organized:

!DemoVGA - SD card contents with sample programs and loader for DemoVGA.

!Pico - sample programs for base Raspberry Pico module.

!Picoino10 - SD card contents with sample programs and loader for Picoino.

!PicoinoMini - SD card contents with sample programs and loader for PicoinoMini.

!PicoPad10 - SD card contents with sample programs and loader for PicoPad
	version 1.0.

!PicoPad20 - SD card contents with sample programs and loader for PicoPad
	version 2.0 in ARM mode.

!PicoPad20riscv - SD card contents with sample programs and loader for PicoPad
	version 2.0 in RISC-V mode.

!PicoPadVGA - SD card contents with sample programs and loader for PicoPadVGA.

!Picotron - SD card contents with sample programs and loader for Picotron.

_boot2 - boot2 loader stage 2, which is placed at the beginning of each UF2
	program and is used to initialize the Flash memory interface. The boot2
	loader is compiled using the prepared batch file c.bat and requires the
	checksum program boot2crc.exe.

_devices - device definitions and drivers. Currently, the ready devices are
	DemoVGA, Pico, Picoino 1.0, PicoinoMini, PicoPad 0.8, PicoPad 1.0,
	PicoPadVGA, Picotron.  The target device is selected at compile time
	with the c.bat file parameter ('demovga', 'pico', 'picoino10',
	'picoinomini', 'picopad08', 'picopad10', 'picopad20', 'picopad20riscv',
        'picopadvga', 'picotron'). If the target device is not specified when
        compiling the application, the default device is selected by the
        _setup.bat file.

_display - drivers for the displays. Currently, there is a mini-VGA display
	driver for 4/8/15/16-bit output to a VGA monitor with 320x240 up to
	800x600 pixel resolution. And TFT display driver for 16-bit RGB565
	format at 320x240 pixel resolution - this driver is used by PicoPad.

_doc - documentation, SDK Programmer's Guide.

_font - prepared fonts. These are non-proportional fonts with a fixed cell width
	for a character of 8 pixels. The file is in BMP 2-color format. The
	RaspPicoImg.exe program exports the font images to the C source file.

_lib - library files of utilities, including library for large real numbers and
	for large integers.

_loader - boot3 program loader, with the ability to run applications from the
	SD card.

_sdk - SDK library for controlling the RP2040 hardware.

_tools - support programs:
	- AS4040 - Intel 4004/4040 assembler compiler
	- BinC - export binary file into C/C++ byte array
	- Boot2CRC - boot2 boot loader checksum calculation
	- DviTms - generator of DVI TMDS 16-bit 2-symbol table
	- HidComp - compiler and decompiler of HID descriptors
	- PicoPadImg - converting BMP images to PicoPad format
	- PicoPadImg2 - converting BMP images to DispHSTX format
	- PicoPadLoaderBin - export boot3 loader to C source code
	- PicoPadLoaderCrc - calculate (and set) application checksum
	- PicoPadVideo - video converter to PicoPad/Picoino format
	- RaspPicoClock - calculation of PLL generator settings combinations
	- RaspPicoImg - convert BMP images to Picoino format
	- RaspPicoPal332 - palette generator for Picoino and QVGA output
	- RaspPicoRle - RLE compression of BMP images for PicoVGA library
	- RaspPicoSnd - export WAV audio files to C source code
	- elf2uf2 - convert ELF file to UF2 file
	- pioasm - PIO program assembler

DemoVGA - sample programs for DemoVGA board

Pico - sample programs for Raspberry Pico

Picoino - sample programs for Picoino and PicoinoMini

PicoPad - sample programs for PicoPad

Picotron - sample programs for Picotron

_setup.bat - setting parameters for compilation according to the selected target
	device

c_all.bat - compilation of all sample applications for the selected target
	device

c_all_devices.bat - compilation of all sample applications for all target
	devices

config_def.h - setting the default compilation configuration

d_all.bat - clean up the compilation from temporary files

global.h - global definitions (types, constants)

includes.h - global include of all header *.h files to compile

Makefile.inc - main compilation script for make.exe

memmap_*.ld - scripts for linker


Boot3 loader
------------
The PicoLibSDK boot3 loader is a 32 KB (or 64 KB on RP2350 Pico 2) resident
program that is permanently loaded at the beginning of the Flash memory. The
boot3 loader can be loaded into the processor's memory only by programming via
USB cable, just like the classic procedure for programming regular programs for
the Raspberry Pico.

The Boot3 loader contains a standard boot2 loader (256 bytes) at its start.
During system boot, the boot2 loader is started first, which initializes the
Flash memory and passes on control to the boot3 loader.

The RP2350 Pico 2 can run in either ARM or RISC-V processor mode. The Boot3
loader must be of the same type as the program being run. If you change the mode
between ARM and RISC-V, you must load a new boot3 loader from the PC via USB.

The boot3 loader first initializes the base devices. In the Picoino the output
is to the QVGA display, implemented by the PIO, in the PicoPad the output is to
the TFT display. The boot3 loader detects whether an SD card is inserted. If no
SD card is inserted, it checks if there is a valid application loaded in the
memory from the next address up to 32 or 64 KB (it must have a valid checksum)
and if the application is OK, it runs it. If an SD card is inserted, it will not
run the application, but will display the contents of the SD card and allows the
user to select a program to run.

The applications on the SD card are in UF2 format. They differ from the standard
format for Pico only in that they contain a boot3 loader at the beginning and
that they are protected by a checksum. This allows applications to be loaded
into the Pico in the classic way via USB cable, like other common applications,
because the boot3 loader will be loaded into memory along with them.

When the boot3 loader starts an application from the SD card, it skips the
initial 32 KB (or 64 KB) that the boot3 loader contains and loads the rest of
the file into Flash memory. It then checks the validity of the application - it
must have a valid header and a valid checksum. If everything is OK, the boot3
loader will run the application.

It is also possible to immediately start an application that is already loaded
in memory - this is possible with the 'Y' button (the 'Back' button in Picoino),
which is otherwise used to exit the application. The 'X' button adjusts the
volume and backlight of the display.

When selecting an application to run, the boot3 loader displays a preview image
and description text in addition to the file name. Whether the application has
an image and text available is indicated by the TXT and BMP shortcuts next to
the file name.

The description text is displayed in the right half of the screen. The text can
have a line length of up to 26 characters. The number of lines can be either 14,
if an image is also present, or 30 lines. The following control characters can
be used in the text:

^	is prefix of control characters
^^	print ^ character
^1..^9	print character with code 1..9
^A..^V	print character with code 10..31
^0	normal gray text
^W	white text
^X	green text
^Y	yellow text
^Z	red text
^[	start invert
^]	stop invert

The preview image is a BMP file. For Picoino it is an 8-bit format with RGB332
palettes (that are generated by RaspPicoPal332). For PicoPad, it is a 16-bit
RGB565 format - if you save such an image from Photoshop, choose the extended
formats when saving and select the R5G6B5 format. For both formats, turn on the
"Flip row order" option. You can get the preview images from the application as
a screenshot by turning on the configuration option USE_SCREENSHOT = 1.


Booting process
---------------
After the power is turned on and the reset sequence is completed, the processor
starts its operation in the BootROM (16 KB or 32 KB from address 0x00000000),
which is a fixed part of the RP2040/RP2350 and cannot be changed by the user.
The start address ("_start") is stored in the BootROM in the second entry of
the interrupt vector. The first entry is the address of the stack pointer.

Both processor cores start in the BootROM in the same way. If it is core 1
(detected via CpuID()), core 1 goes into sleep mode in a waiting loop, waiting
to be woken up by the core 0 via the mailbox FIFO. Only core 0 continues its
activity (main()).

The processor will first check if the BOOTSEL button is pressed. If so, it will
switch to booting the program from USB. If it is not pressed, it continues
booting from the external Flash memory (_flash_boot).

Activates the connection for the external Flash memory (the Flash memory is
mapped from address 0x10000000 and has a size of 2 to 16 MB) and reads 256 bytes
from the beginning of the Flash memory to end of RAM memory, to address
0x20041F00. These 256 bytes contain the stage 2 boot loader and are appended to
the beginning of each application for the RP2040.

The processor checks the boot loader checksum - the calculated CRC32A for the
first 252 bytes must match the checksum in the last 4 bytes. If the checksum
matches, the program jumps to the beginning of the boot2 loader. The RP2350 does
not run the boot2 module, but the beginning of the program. The program itself
provides the boot2 call and initializes the Flash memory.

The BootROM program initialized only the basic interface for the Flash memory -
in order to be able to connect basically any Flash memory. Boot2 loader sets a
faster Flash memory interface. Different boot2 loaders may be needed for
different Flash memories.

Boot2 loader continues by jumping to the main application program. It reads the
stack pointer settings and start address from the vector table at address
0x10000100, and jumps into the application at the start address.

Applications compiled for the PicoLibSDK library may contain another boot loader
at the beginning, stage 3. It is 32 KB or 64 KB in size and is used to load
programs from the SD card into Flash memory.

Each program prepared in this way contains the entire 32 KB or 64 KB boot3
loader at the beginning. This allows the program to be loaded into the Flash
memory using both the classic USB cable procedure and the boot3 loader from the
SD card. When booting a program from an SD card, the boot3 loader does not load
the first 32 KB or 64 KB of the file, but only the following part, with the
program itself. The program itself thus starts at address 0x10008000 with its
vector table.

Before starting the application, the Boot3 loader first checks the special
application header, which is located after the vector table. It calculates the
CRC32A checksum of the entire application in Flash memory, and if the checksum
is OK, it jumps to the application entry point according to the vector table.

If the system starts via reset only, without loading the application into
memory, the boot3 loader checks if an SD card is inserted into the device. If
it is inserted, it is assumed that the user will want to start another
application by selecting it, and the contents of the SD card will be displayed.
If the SD card is not inserted, it is assumed to just restart the application,
in which case the boot3 loader will jump to start the application.

Whether the application starts via the boot3 loader or just via the boot2
loader, in both cases it starts with the start address stored in the second
entry of the vector table "_reset_handler" (the first entry is the stack
pointer). This code is located in the crt0.S file.

If core1 is detected, it jumps back to BootROM to go sleep, waiting to be
activated. Only core0 will continue. The program copies the read-only data from
Flash memory to RAM memory - this initializes the global variables. It also
transfers to RAM the codes of functions that are to be executed from RAM and not
from Flash.

Next, the program clears the BSS segment, which is an area that does not require
initialization, but can be expected to contain zero variables.

The program calls the RuntimeInit() function (in the sdk_runtime.c file), which
contains the basic initialization of the system without burdening the programmer
of it. The function initializes peripherals, object constructors, ROM functions,
floating math, interrupt controller, system clock, memory allocator, and finally
target device. After it, program starts with main() function with main program
code.

When the program wants to return to the boot3 loader (exiting the program with
the Y key), it does so by calling the ResetToBootLoader() function. The function
sets the magic value in the scratch register of the watchdog and activates the
watchdog. Watchdog will reset the processor. From the magic value boot3 loader
recognizes that it is an application exit, so it displays the contents of the SD
card instead of jumping into the application.


FAR File Manager
----------------
I highly recommend that you use the FAR file manager when developing
applications for the microchips. FAR includes everything that is needed to
develop software for microchips - it includes quality text editor, manipulation
with files, searching, comparison. And above all, it includes a console window
where you can still see the output from the compiler. The entire PicoLibSDK
library was developed using the FAR manager. You can download FAR manager from
the web site https://www.farmanager.com/ .

Most important controls of the file window

Tab			change active panel
Ctrl+O			hide/show both panels
Ctrl+U			swap panels
Ctrl+P			hide/show inactive panel
Alft+F1			change disk in left panel
Alf+F2			change disk in right panel
Insert			select file
[Numeric +]		select files by mask
Shift+[Numeric +]	select all files (and directories)
Shift+[Numeric -]	unselect all files (and directories)
Ctrl+F			full path to command line
F3			view file
F4			edit file
F5			copy selected files and directories
F6			move selected files and directories
F7			create directory
F8			delete selected files and directories
F11			modules (compare files and their contents)
Alt+F7			search files and their contents
Alt+F8			command history
Shift+F4		create new file and edit

You can preset the window size in the shortcut properties.

Most important controls of text editor (F4)

F2			save file
Shift+F2		save file As
F7			search
Ctrl+F7			search and replace
Shift+F8		select code page
Esc			quit editor
Ctrl+Z			undo
Ctrl+Shift+Z		redo
Shift+arrows		select block
Ctrl+C			copy to clipboard
Ctrl+X			cut to clipboard
Ctrl+V			paste from clipboard
Delete			delete selection


Original-SDK interface
----------------------
For backward compatibility, the PicoLibSDK library provides most of the
functions of the original SDK library. The functions of the original SDK are
mostly implemented by inline functions, redirecting the code to native
PicoLibSDK library functions. Main differences:

The PicoLibSDK library simplifies some features and extends others. One
simplification is that PicoLibSDK adheres less strictly to locking down
functions for multitasking environments. Due to limited processor memory,
multitasking is not expected. Excessive locking can slow down program
functionality, so locking is concentrated on only the most necessary operations.

Functions from the original SDK for working with USB are not implemented. The
USB interface is handled differently in the PicoLibSDK, in a simpler and faster
way (instead of USB tasks, everything is handled more efficiently in
interrupts). Interfaces are incompatible.

The functions for WiFi and Bluetooth are not implemented - these modules are not
yet implemented in PicoLibSDK either.

SDK functions for accessing processor devices are preferentially addressed by
device indexes in the PicoLibSDK library, while the original SDK library uses a
pointer to device registers. Addressing via device indexes is more simple, but
in some cases addressing via pointers may generate slightly more efficient code.
For this reason the ability to address devices via pointers has been added to
the PicoLibSDK library. Many functions are therefore available in 2 formats. If
you do not require more code efficiency, we recommend using the simpler
addressing via indexes.


History of versions
-------------------
03/08/2023 PicoPad prototype with pre-alpha PicoLibSDK version 0.8.
04/28/2023 PicoPad full version with alpha PicoLibSDK version 0.90
06/28/2023 alpha PicoLibSDK version 0.91, improved battery measurement
07/30/2023 version 1.00: first final release
08/06/2023 version 1.01: printf() print memory block, stdio to UART
	and FILE, add Pico and PicoinoMini devices, some USB repairs
08/14/2023 version 1.02: added video player, added DemoVGA board, added
	compilation scripts for Linux
08/19/2023 version 1.03: SPI SD card speed fix, timer-alarm treatment for
	short times
08/26/2023 version 1.04: Replace qvga/vga/qdraw/drawtft libraries with
	more versatile minivga/draw libraries with 4/8/15/16 bit output
	and 320x240 to 800x600 pixel resolution. Slide-show player added.
	Added Picotron device, with 4-bit VGA output. Added BOOTSEL to
	boot3 loader (in Battery menu).
09/09/2023 version 1.05: CSYNC in VGA driver, added PicoVGA8 library,
	added VREG library, setup volume and backlight, calibrate crystal.
10/03/2023 version 1.06: added PicoPadVGA device and some drawing functions.
10/06/2023 version 1.07: support for RAM programs, support for saving and
	loading Flash boot loader
10/18/2023 version 1.08: simulate keypad using USB keyboard (USE_USBPAD,
	A->Ctrl, B->Alt, X->Shift, Y->Esc).
12/05/2023 version 1.09: ADPCM sound compression, original-SDK interface.
12/30/2023 version 1.10: Intel 4004/4040 CPU emulator, DVI (HDMI) display,
	DVIVGA (HDMI with VGA) display, file selector
01/27/2024 version 1.11: loader passes home path to the application; CPU
	emulators: I4004, I4040, I8008, I8048, I8052, I8080, I8085, I8086,
	I8088, I80186, I80188, M6502, M65C02, X80, Z80.
05/01/2024 version 1.12: Screen saver in loader to turn off when charging.
	Simple PC DOS emulator.
06/14/2024 version 1.13: Game Boy Emulator
10/08/2024 version 2.00: RP2350 Pico 2 support
10/27/2024 version 2.01: Fast float library for RISC-V Hazard3 core.
11/05/2024 version 2.02: NES Emulator
12/03/2024 version 2.03: Fast double library for RISC-V Hazard3 core.
02/24/2025 version 2.04: DispHSTX library v1.00, DrawCan library, faster
	compilation, configurable compilation paths
03/02/2025 version 2.05: Build of DispHSTX v1.01 library for Raspberry PicoSDK
03/05/2025 version 2.06: PWMSnd update - higher quality audio output, less
	noise, higher PWM output sample rate, 12-bit output, stereo and 16-bit
	format support
03/14/2025 version 2.07: MP3 decoder and player
05/01/2025 version 2.08: Added PicopadHSTX device and DispHSTXMini driver

Missing and @TODO
-----------------
SDK supports that are missing in the library and are needed @TODO:

- bluetooth and wifi
- more USB drivers (audio, bluetooth, dfu, midi, msc, net, tmc, video)
- DDS compression image format, JPG compression
- 3D graphics support
- encapsulation of text strings into C++ objects
