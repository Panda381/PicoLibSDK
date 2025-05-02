@echo off
rem Compilation...

set TARGET=OVERCLCK
set GRPDIR=TEST

rem Normally it is possible to run the program from the Flash.
rem For the DispHSTX console, it is better to run from RAM
rem (parameter "noflash") to avoid signal dropout during loading
rem code from Flash when changing the sys_clock.
set MEMMAP=noflash
if "%1"=="picopadhstx" goto useram
if "%1"=="picopadhstxriscv" goto useram
set MEMMAP=
:useram

if "%1"=="picopadvga" goto stop
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
rem ..\..\..\_c1.bat picopad08
rem ..\..\..\_c1.bat picopad10
..\..\..\_c1.bat picopad20
rem ..\..\..\_c1.bat picopad20riscv
rem ..\..\..\_c1.bat picopadhstx
rem ..\..\..\_c1.bat picopadhstxriscv
:stop
