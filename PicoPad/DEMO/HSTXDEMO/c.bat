@echo off
rem Compilation...

set TARGET=HSTXDEMO
set GRPDIR=DEMO
set MEMMAP=

if "%1"=="" goto default
if "%1"=="picopad08" goto stop
if "%1"=="picopadvga" goto stop
if "%1"=="picopad10" goto stop
if "%1"=="picopad20" goto stop
if "%1"=="picopad20riscv" goto stop
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat picopadhstx
rem ..\..\..\_c1.bat picopadhstxriscv
:stop
