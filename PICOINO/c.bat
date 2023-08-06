@echo off
rem Compilation... Compile all projects in all sub-directories

rem First command-line parameter (%1) of this batch can contain device name (e.g. picoino10).
rem If no device name is specified, default device will be used,
rem marked with ":default" label in the _setup.bat file.

rem Only Picoino 1.0 and PicoinoMini
if "%1"=="picoino10" goto start
if "%1"=="picoinomini" goto start
if not "%1"=="" goto stop

:start
rem Compile projects from all sub-directories
for /D %%d in (*) do call :comp1 %%d %%1
exit /b

rem Sub-batch to compile one project in %1 subdirectory, as %2 device.
:comp1
if not exist %1\c.bat goto stop
cd %1
echo.
echo ======== Compiling %1 ========
call c.bat %2
cd ..
:stop
