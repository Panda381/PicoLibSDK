@echo off
rem Compilation...

set TARGET=pio_st7789
set GRPDIR=Orig_Base
set MEMMAP=

if exist st7789_lcd.pio.h del st7789_lcd.pio.h
..\..\..\_tools\pioasm.exe -o c-sdk src\st7789_lcd.pio st7789_lcd.pio.h
if exist st7789_lcd.pio.h goto ok

pause
goto stop

:ok
if "%1"=="" goto default
..\..\..\_c1.bat %1

:default
..\..\..\_c1.bat pico
