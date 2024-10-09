@echo off
rem Compilation...

set TARGET=Led
set GRPDIR=Base
set MEMMAP=

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
rem ..\..\..\_c1.bat pico
rem ..\..\..\_c1.bat pico2
 ..\..\..\_c1.bat pico2riscv
