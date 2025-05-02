@echo off
rem Compilation...

set TARGET=141PF
set GRPDIR=EMU
set MEMMAP=

if "%1"=="" goto default
if "%1"=="picopadvga" goto stop
..\..\..\_c1.bat %1

:default
rem ..\..\..\_c1.bat picopad08
rem ..\..\..\_c1.bat picopad10
..\..\..\_c1.bat picopad20
rem ..\..\..\_c1.bat picopad20riscv
rem ..\..\..\_c1.bat picopadhstx
rem ..\..\..\_c1.bat picopadhstxriscv

:stop
