@echo off
rem Compilation...

set TARGET=GBC
set GRPDIR=EMU
set MEMMAP=

if "%1"=="" goto default
if "%1"=="picopadvga" goto stop
if "%1"=="picopad08" goto stop
..\..\..\_c1.bat %1

:default
rem ..\..\..\_c1.bat picopad08
..\..\..\_c1.bat picopad10
rem ..\..\..\_c1.bat picopad20
rem ..\..\..\_c1.bat picopad20riscv

:stop
