@echo off
rem Compilation...

set TARGET=rtc_hello
set GRPDIR=Orig_USB
set MEMMAP=

if "%1"=="" goto default
if "%1"=="pico2" goto stop
if "%1"=="pico2riscv" goto stop
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
:stop
