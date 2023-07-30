@echo off
rem Setup device to compile

rem First command-line parameter (%1) of this batch contains device name (e.g. picopad10).
rem This batch is called from the _c1.bat file.

rem Use configuration name as parameter of the batch, to select target device:
rem   picoino10 ... Picoino version 1.0 with QVGA RGB332 output
rem   picopad08 ... PicoPad beta version 0.8
rem   picopad10 ... PicoPad version 1.0 with TFT RGB565 output
rem   (empty) ... default compilation

rem Move ":default" label before configuration, which you want to use as default.

if "%1" == "picoino10" goto picoino10
if "%1" == "picopad08" goto picopad08
if "%1" == "picopad10" goto picopad10
goto default

:picoino10
set DEVICE=picoino10
set DEVCLASS=picoino
set DEVDIR=!Picoino10
exit /b

:picopad08
set DEVICE=picopad08
set DEVCLASS=picopad
set DEVDIR=!PicoPad08
exit /b

:default
:picopad10
set DEVICE=picopad10
set DEVCLASS=picopad
set DEVDIR=!PicoPad10
exit /b
