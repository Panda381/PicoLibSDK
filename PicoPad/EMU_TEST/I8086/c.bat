@echo off
rem Compilation...

set TARGET=I8086
set GRPDIR=EMU_TEST
set MEMMAP=

if "%1"=="" goto default
if "%1"=="picopadvga" goto stop
if "%1"=="picopad08" goto stop
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat picopad10
:stop
