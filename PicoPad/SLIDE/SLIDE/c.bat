@echo off
rem Compilation...

set TARGET=SLIDE
set GRPDIR=SLIDE

if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
rem ..\..\..\_c1.bat picopad08
rem ..\..\..\_c1.bat picopadvga
..\..\..\_c1.bat picopad10
