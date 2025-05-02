@echo off
rem Compilation...

set TARGET=DVI640
set GRPDIR=DispHSTX
set MEMMAP=

if "%1"=="" goto default
if "%1"=="pico" goto stop
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico2
rem ..\..\..\_c1.bat pico2riscv
:stop
