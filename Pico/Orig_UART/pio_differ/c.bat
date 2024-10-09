@echo off
rem Compilation...

set TARGET=pio_differ
set GRPDIR=Orig_UART
set MEMMAP=

if exist differential_manchester.pio.h del differential_manchester.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\differential_manchester.pio differential_manchester.pio.h
if exist differential_manchester.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
rem ..\..\..\_c1.bat pico2
rem ..\..\..\_c1.bat pico2riscv
