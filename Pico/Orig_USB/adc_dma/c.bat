@echo off
rem Compilation...

set TARGET=adc_dma
set GRPDIR=Orig_USB
set MEMMAP=

if exist resistor_dac.pio.h del resistor_dac.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\resistor_dac.pio resistor_dac.pio.h
if exist resistor_dac.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
rem ..\..\..\_c1.bat pico2
rem ..\..\..\_c1.bat pico2riscv

:stop
