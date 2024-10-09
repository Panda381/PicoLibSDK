@echo off
rem Compilation...

set TARGET=pio_spiflash
set GRPDIR=Orig_USB
set MEMMAP=

if exist spi.pio.h del spi.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\spi.pio spi.pio.h
if exist spi.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
rem ..\..\..\_c1.bat pico2
rem ..\..\..\_c1.bat pico2riscv
