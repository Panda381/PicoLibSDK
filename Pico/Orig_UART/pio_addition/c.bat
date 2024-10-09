@echo off
rem Compilation...

set TARGET=pio_addition
set GRPDIR=Orig_UART
set MEMMAP=

if exist addition.pio.h del addition.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\addition.pio addition.pio.h
if exist addition.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
rem ..\..\..\_c1.bat pico2
rem ..\..\..\_c1.bat pico2riscv
