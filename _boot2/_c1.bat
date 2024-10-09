rem @echo off
rem Compilation...

rem ------------------------------------------------
rem Edit this line to setup path to GCC-ARM compiler
rem ------------------------------------------------
rem set PATH=..\_tools;C:\ARM10\bin;%PATH%

rem Delete all target files
if exist %TARGET%.bin del %TARGET%.bin

rem Compile
..\_tools\make.exe all

rem Check compilation result
if errorlevel 1 goto err
if not exist %TARGET%.bin goto err

rem Export bin to assembler data file, including CRC.
boot2crc\boot2crc.exe %TARGET%.bin %1 > %TARGET%_bin.S
if errorlevel 1 goto err

rem Compilation OK, list memory size
echo.
type %TARGET%.siz
goto end

:err
rem Compilation error, stop and wait for key press
pause ERROR!

:end
