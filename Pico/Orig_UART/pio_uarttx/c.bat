@echo off
rem Compilation...

set TARGET=pio_uarttx
set GRPDIR=Orig_UART
set MEMMAP=

if exist uart_tx.pio.h del uart_tx.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\uart_tx.pio uart_tx.pio.h
if exist uart_tx.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
