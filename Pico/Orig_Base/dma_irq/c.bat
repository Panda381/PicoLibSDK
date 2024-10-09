@echo off
rem Compilation...

set TARGET=dma_irq
set GRPDIR=Orig_Base
set MEMMAP=

if exist pio_serialiser.pio.h del pio_serialiser.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\pio_serialiser.pio pio_serialiser.pio.h
if exist pio_serialiser.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
rem ..\..\..\_c1.bat pico2
rem ..\..\..\_c1.bat pico2riscv
