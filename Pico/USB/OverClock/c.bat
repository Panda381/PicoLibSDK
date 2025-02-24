@echo off
rem Compilation...

set TARGET=OverClock
set GRPDIR=USB
set MEMMAP=

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
rem ..\..\..\_c1.bat pico
..\..\..\_c1.bat pico2
rem ..\..\..\_c1.bat pico2riscv
