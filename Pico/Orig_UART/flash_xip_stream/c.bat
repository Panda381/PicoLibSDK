@echo off
rem Compilation...

set TARGET=flash_xip_stream
set GRPDIR=Orig_UART
set MEMMAP=

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
rem ..\..\..\_c1.bat pico2
rem ..\..\..\_c1.bat pico2riscv
