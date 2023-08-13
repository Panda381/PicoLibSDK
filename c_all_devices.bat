@echo off
rem Compilation... Compile all projects of all devices

rem Compilation... Compile all projects of device "Raspberry Pico"
call d_all.bat
call c_all.bat pico

rem Compilation... Compile all projects of device "Picoino version 1.0"
call d_all.bat
call c_all.bat picoino10

rem Compilation... Compile all projects of device "Picoino Mini"
call d_all.bat
call c_all.bat picoinomini

rem Compilation... Compile all projects of device "DemoVGA"
call d_all.bat
call c_all.bat demovga

rem Compilation... Compile all projects of device "PicoPad beta version 0.8"
call d_all.bat
call c_all.bat picopad08

rem Compilation... Compile all projects of device "PicoPad version 1.0"
call d_all.bat
call c_all.bat picopad10

call d_all.bat
