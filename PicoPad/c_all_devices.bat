@echo off
rem Compilation... Compile all projects of all devices

rem Compilation... Compile all projects of device "PicoPad version 1.0"
call d.bat
call c.bat picopad10

rem Compilation... Compile all projects of device "PicoPad version 2.0"
call d.bat
call c.bat picopad20

rem Compilation... Compile all projects of device "PicoPad version 2.0 RISCV"
call d.bat
call c.bat picopad20riscv

rem Compilation... Compile all projects of device "PicoPadVGA"
call d.bat
call c.bat picopadvga

rem Compilation... Compile all projects of device "PicoPadHSTX"
call d.bat
call c.bat picopadhstx

rem Compilation... Compile all projects of device "PicoPadHSTX RISCV"
call d.bat
call c.bat picopadhstxriscv

call d.bat
