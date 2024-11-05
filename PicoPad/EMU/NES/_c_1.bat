@echo off
echo.
if "%1" == "" goto stop

if exist src\program.cpp del src\program.cpp
if exist src\setup.h del src\setup.h

if exist build\program.o del build\program.o
if exist build\main.o del build\main.o
if exist build\InfoNES.o del build\InfoNES.o
if exist build\K6502_rw.o del build\K6502_rw.o


rem Uncomment to compile with rarely used switches:
if exist build\InfoNES_Mapper.o del build\InfoNES_Mapper.o
if exist build\NES_APU.o del build\NES_APU.o
if exist build\emu_nes_menu.o del build\emu_nes_menu.o


if not exist samples\%1.H copy SETUP.H samples\%1.H > nul

echo %1.NES
NESprep\NESprep samples\%1.nes src\program.cpp %1
copy samples\%1.H src\setup.h > nul

call _c.bat
copy /b NES.uf2 samples\%1.UF2 > nul

if not exist samples\%1.BMP copy NES.BMP samples\%1.BMP > nul
if not exist samples\%1.PNG copy NES.PNG samples\%1.PNG > nul
if not exist samples\%1.TXT copy NES.TXT samples\%1.TXT > nul

rem find /C "MBC=" samples\%1.TXT > nul
find /C "CRC=" samples\%1.TXT > nul
if not errorlevel 1 goto stop
NESprep\MapperNo\MapperNo samples\%1.nes >> samples\%1.TXT
if not errorlevel 1 goto stop

echo %1.NES error!
pause

:stop
