@echo off
rem Compilation... Compile all projects in all sub-directories

rem First command-line parameter (%1) of this batch can contain device name (e.g. picopad10).
rem If no device name is specified, default device will be used,
rem marked with ":default" label in the _setup.bat file.

rem ------------------------------------------------
rem Edit this line to setup path to GCC-ARM compiler
rem ------------------------------------------------
rem set PATH=_tools;C:\ARM10\bin;%PATH%

rem skip boot2 and boot3 loader, it should be already compiled
goto start

rem Compile boot2 stage - it needs to be compiled first
cd _boot2
echo.
echo ======== Compiling _boot2 ========
call c.bat %1
cd ..

rem Compile boot3 loader - it needs to be compiled second
cd _loader\LOADER
echo.
echo ======== Compiling _loader ========
call c.bat %1
cd ..\..


:start
rem Compile all projects from all sub-directories
for /D %%d in (*) do call :comp1 %%d %%1
exit /b


rem Sub-batch to compile one group of projects in %1 subdirectory, as %2 device.
rem Skipping sub-directories with boot2 stage and with boot loader.
:comp1
if "%1" == "_boot2" goto stop
if "%1" == "_loader" goto stop
if not exist %1\c.bat goto stop
cd %1
echo.
echo ======== Compiling group %1 ========
call c.bat %2
cd ..
:stop
