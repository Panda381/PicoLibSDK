@echo off
rem Compilation...

set TARGET=SDCARD
set GRPDIR=TEST

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat demovga
