@echo off
rem Compilation...

set TARGET=Led
set GRPDIR=Base

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
