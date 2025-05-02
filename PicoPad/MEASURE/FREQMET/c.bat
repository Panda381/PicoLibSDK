@echo off
rem Compilation...

if "%1"=="picopadvga" goto stop

set TARGET=FREQMET
set GRPDIR=MEASURE
set MEMMAP=

if exist freqmet.pio.h del freqmet.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\freqmet.pio freqmet.pio.h
if exist freqmet.pio.h goto ok

pause
goto stop

:ok
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
