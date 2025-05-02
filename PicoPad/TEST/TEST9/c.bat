@echo off
rem Compilation...

rem Skip RISC-V, compilation freezes at linking stage (really no idea why?)
if "%1"=="picopadhstxriscv" goto stop
if "%1"=="picopad20riscv" goto stop

echo --- Wait, this compilation is very long! ---

set TARGET=TEST9
set GRPDIR=TEST
set MEMMAP=

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
rem ..\..\..\_c1.bat picopad08
rem ..\..\..\_c1.bat picopadvga
rem ..\..\..\_c1.bat picopad10
..\..\..\_c1.bat picopad20
rem ..\..\..\_c1.bat picopad20riscv
rem ..\..\..\_c1.bat picopadhstx
rem ..\..\..\_c1.bat picopadhstxriscv
:stop
