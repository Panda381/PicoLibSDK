@echo off
rem Compilation...

set TARGET=pio_uartrx
set GRPDIR=Orig_USB
set MEMMAP=

if exist uart_rx.pio.h del uart_rx.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\uart_rx.pio uart_rx.pio.h
if exist uart_rx.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
