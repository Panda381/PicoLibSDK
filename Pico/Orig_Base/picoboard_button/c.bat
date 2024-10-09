@echo off
rem Compilation...

set TARGET=picoboard_button
set GRPDIR=Orig_Base
set MEMMAP=

if "%1"=="" goto default
if "%1"=="pico2" goto stop
if "%1"=="pico2riscv" goto stop
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
