@echo off
rem Compilation...

set TARGET=PULSGRAM
set GRPDIR=MEASURE

if exist pulsgram.pio.h del pulsgram.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\pulsgram.pio pulsgram.pio.h
if exist pulsgram.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
rem ..\..\..\_c1.bat picopad08
..\..\..\_c1.bat picopad10

:stop
