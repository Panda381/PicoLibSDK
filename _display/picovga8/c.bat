@echo off
rem Compilation...

if exist picovga8.pio.h del picovga8.pio.h
..\..\_tools\pioasm.exe -o c-sdk picovga8.pio picovga8.pio.h
if not exist picovga8.pio.h goto err
goto stop

:err
pause
:stop
