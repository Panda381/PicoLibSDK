@echo off
rem Compilation...

set TARGET=tiles_and_sprites
set GRPDIR=PicoDVI
set MEMMAP=

if exist ..\_libdvi\dvi_serialiser.pio.h del ..\_libdvi\dvi_serialiser.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk ..\_libdvi\dvi_serialiser.pio ..\_libdvi\dvi_serialiser.pio.h
if not exist ..\_libdvi\dvi_serialiser.pio.h goto err

if exist ..\_libdvi\tmds_encode_1bpp.pio.h del ..\_libdvi\tmds_encode_1bpp.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk ..\_libdvi\tmds_encode_1bpp.pio ..\_libdvi\tmds_encode_1bpp.pio.h
if exist ..\_libdvi\tmds_encode_1bpp.pio.h goto ok

:err
pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
rem ..\..\..\_c1.bat pico2
rem ..\..\..\_c1.bat pico2riscv
