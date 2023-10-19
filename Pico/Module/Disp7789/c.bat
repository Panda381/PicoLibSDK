@echo off
rem Compilation...

set TARGET=Disp7789
set GRPDIR=Module
set MEMMAP=

if exist grabber.pio.h del grabber.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\grabber.pio grabber.pio.h
if exist grabber.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
if not "%1"=="extdisp" goto stop

:default
..\..\..\_c1.bat extdisp

:stop
