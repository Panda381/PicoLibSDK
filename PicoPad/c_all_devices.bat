@echo off
rem Compilation... Compile all projects of all devices

rem Compilation... Compile all projects of device "PicoPad beta version 0.8"
call d.bat
call c.bat picopad08

rem Compilation... Compile all projects of device "PicoPad version 1.0"
call d.bat
call c.bat picopad10

rem Compilation... Compile all projects of device "PicoPadVGA"
call d.bat
call c.bat picopadvga

call d.bat
