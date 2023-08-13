@echo off
rem Compilation...

set TARGET=USBDEV
set GRPDIR=USB

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat demovga
