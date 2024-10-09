@echo off
rem Compilation...

set TARGET=pio_manchester
set GRPDIR=Orig_UART
set MEMMAP=

if exist manchester_encoding.pio.h del manchester_encoding.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\manchester_encoding.pio manchester_encoding.pio.h
if exist manchester_encoding.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
rem ..\..\..\_c1.bat pico2
rem ..\..\..\_c1.bat pico2riscv
