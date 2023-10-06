@echo off
rem Compilation... Compile one project (one output UF2 file)

rem First command-line parameter (%1) of this batch contains device name (e.g. picopad10).
rem This batch is called from 2nd-level subdirectory, base directory of the project
rem (e.g. from the DEMO\HELLOW\c.bat).

rem ------------------------------------------------
rem Edit this line to setup path to GCC-ARM compiler
rem ------------------------------------------------
rem set PATH=..\..\_tools;C:\ARM10\bin;%PATH%

rem ----------------------------------------------------------------------
rem Use following commands before running this batch, to setup compilation
rem ----------------------------------------------------------------------

rem Command to setup target project name (i.e. name of output file).
rem This command is located in c.bat file. Name "LOADER" is reserved for boot loader.
rem    set TARGET=HELLOW

rem Command to setup target group sub-directory (directory to which the program
rem will be copied on the target SD card). This command is located in c.bat file.
rem    set GRPDIR=DEMO

rem Command to select target device (use names from the _setup.bat file).
rem This command is located in _setup.bat file. Parameter for the command is
rem passed as command-line parameter of c.bat, c_all.bat, _setup.bat and _c1.bat.
rem    set DEVICE=picopad10

rem Command to select class of target device (to use common compilation setup).
rem This command is located in _setup.bat file.
rem    set DEVCLASS=picopad

rem Command to setup target device directory (directory containing image of SD card).
rem This command is located in _setup.bat file.
rem    set DEVDIR=!PicoPad

rem Setup parameters DEVICE, DEVCLASS and DEVDIR (%1 = device name)
call ..\..\..\_setup.bat %1

rem Check if target name "LOADER" is used in other directory than root.
rem We need it to detect compilation of boot loader in makefile.
if not "%TARGET%"=="LOADER" goto noloader
if "%GRPDIR%"=="." goto noloader
echo ---- Application cannot have name LOADER ----
goto err

:noloader
rem Delete all target files
if exist %TARGET%.uf2 del %TARGET%.uf2
if exist %TARGET%.bin del %TARGET%.bin

rem Compile
echo Device: %DEVICE%
..\..\..\_tools\make.exe all
rem If you want to see all error messages, compile using this command:
rem ..\..\..\_tools\make.exe all 2> err.txt

rem Check compilation result
if errorlevel 1 goto err
if not exist %TARGET%.uf2 goto err
if not exist %TARGET%.bin goto err

rem Compilation OK, list memory size
echo.
type %TARGET%.siz

rem Calculate CRC to check by boot loader (skip if compiling boot loader)
if "%TARGET%"=="LOADER" goto skipcrc
if "%DEVCLASS%"=="pico" goto skipcrc
if "%MEMMAP%"=="noflash" goto skipcrc
..\..\..\_tools\PicoPadLoaderCrc\LoaderCrc.exe %TARGET%.bin %TARGET%.uf2
if errorlevel 1 goto err

:skipcrc
rem Copy UF2 file to destination folder with image of SD card
if not exist ..\..\..\%DEVDIR%\%GRPDIR%\*.UF2 md ..\..\..\%DEVDIR%\%GRPDIR%
copy /b %TARGET%.uf2 ..\..\..\%DEVDIR%\%GRPDIR%\%TARGET%.UF2 > nul
goto end

:err
rem Compilation error, stop and wait for key press
pause ERROR!

:end
