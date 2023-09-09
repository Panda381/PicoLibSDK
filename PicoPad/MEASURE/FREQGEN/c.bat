@echo off
rem Compilation...

set TARGET=FREQGEN
set GRPDIR=MEASURE

if exist freqgen.pio.h del freqgen.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\freqgen.pio freqgen.pio.h
if exist freqgen.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
rem ..\..\..\_c1.bat picopad08
..\..\..\_c1.bat picopad10

:stop
