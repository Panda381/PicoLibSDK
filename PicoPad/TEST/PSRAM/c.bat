@echo off
rem Compilation...

set TARGET=PSRAM
set GRPDIR=TEST
set MEMMAP=

if "%1"=="" goto default
if "%1"=="picopad08" goto stop
if "%1"=="picopadvga" goto stop
if "%1"=="picopad10" goto stop
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat picopad20
rem ..\..\..\_c1.bat picopad20riscv
rem ..\..\..\_c1.bat picopadhstx
rem ..\..\..\_c1.bat picopadhstxriscv
:stop
