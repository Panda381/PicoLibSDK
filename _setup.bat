@echo off
rem Setup device to compile

rem First command-line parameter (%1) of this batch contains device name (e.g. picopad10).
rem This batch is called from the _c1.bat file.

rem Use configuration name as parameter of the batch, to select target device:
rem   pico .............. base Raspberry Pico module
rem   pico2 ............. base Raspberry Pico 2 module in ARM mode
rem   pico2riscv ........ base Raspberry Pico 2 module in RISC-V mode
rem   picotron .......... Picotron with VGA YRGB1111 output
rem   picoinomini ....... Picoino mini version with VGA RGB332 output
rem   picoino10 ......... Picoino version 1.0 with VGA RGB332 output
rem   demovga ........... DemoVGA board with VGA RGB565 output
rem   picopad08 ......... PicoPad beta version 0.8
rem   picopad10 ......... PicoPad version 1.0 with TFT RGB565 output
rem   picopad20 ......... PicoPad version 2.0 (ARM) with TFT RGB565 output
rem   picopad20riscv .... PicoPad version 2.0 (RISC-V) with TFT RGB565 output
rem   picopadvga ........ PicoPad with VGA RGB565 output
rem   picopadhstx ....... PicoPad RP2350 (ARM) and HSTX HDMI+VGA output
rem   picopadhstxriscv .. PicoPad RP2350 (RISC-V) and HSTX HDMI+VGA output
rem   (empty) ........... default compilation

rem Move ":default" label before configuration, which you want to use as default.

if "%1"=="pico" goto pico
if "%1"=="pico2" goto pico2
if "%1"=="pico2riscv" goto pico2riscv
if "%1"=="picotron" goto picotron
if "%1"=="picoinomini" goto picoinomini
if "%1"=="picoino10" goto picoino10
if "%1"=="demovga" goto demovga
if "%1"=="picopad08" goto picopad08
if "%1"=="picopad10" goto picopad10
if "%1"=="picopad20" goto picopad20
if "%1"=="picopad20riscv" goto picopad20riscv
if "%1"=="picopadvga" goto picopadvga
if "%1"=="picopadhstx" goto picopadhstx
if "%1"=="picopadhstxriscv" goto picopadhstxriscv
if "%1"=="" goto default

echo.
echo Incorrect configuration "%1"!
echo Press Ctrl+C to break compilation.
pause
goto default

:pico
set DEVICE=pico
set DEVCLASS=pico
set DEVDIR=!Pico
exit /b

:pico2
set DEVICE=pico2
set DEVCLASS=pico
set DEVDIR=!Pico2
exit /b

:pico2riscv
set DEVICE=pico2riscv
set DEVCLASS=pico
set DEVDIR=!Pico2riscv
exit /b

:picotron
set DEVICE=picotron
set DEVCLASS=picotron
set DEVDIR=!Picotron
exit /b

:picoinomini
set DEVICE=picoinomini
set DEVCLASS=picoino
set DEVDIR=!PicoinoMini
exit /b

:picoino10
set DEVICE=picoino10
set DEVCLASS=picoino
set DEVDIR=!Picoino10
exit /b

:demovga
set DEVICE=demovga
set DEVCLASS=demovga
set DEVDIR=!DemoVGA
exit /b

:picopad08
set DEVICE=picopad08
set DEVCLASS=picopad
set DEVDIR=!PicoPad08
exit /b

:picopad10
set DEVICE=picopad10
set DEVCLASS=picopad
set DEVDIR=!PicoPad10
exit /b

:default
:picopad20
set DEVICE=picopad20
set DEVCLASS=picopad
set DEVDIR=!PicoPad20
exit /b

:picopad20riscv
set DEVICE=picopad20riscv
set DEVCLASS=picopad
set DEVDIR=!PicoPad20riscv
exit /b

:picopadvga
set DEVICE=picopadvga
set DEVCLASS=picopad
set DEVDIR=!PicoPadVGA
exit /b

:picopadhstx
set DEVICE=picopadhstx
set DEVCLASS=picopad
set DEVDIR=!PicoPadHSTX
exit /b

:picopadhstxriscv
set DEVICE=picopadhstxriscv
set DEVCLASS=picopad
set DEVDIR=!PicoPadHSTXriscv
exit /b
