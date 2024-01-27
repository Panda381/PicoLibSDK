@echo off
AS4040.exe ALARM.ASM
if errorlevel 1 goto err
echo.

AS4040.exe BLINK.ASM
if errorlevel 1 goto err
echo.

AS4040.exe COUNTER.ASM
if errorlevel 1 goto err
echo.

AS4040.exe KEYLED.ASM
if errorlevel 1 goto err
echo.

AS4040.exe MESSAGE.ASM
if errorlevel 1 goto err
echo.

AS4040.exe RUNLED.ASM
if errorlevel 1 goto err
echo.

AS4040.exe TESTALL.ASM
if errorlevel 1 goto err
echo.

goto ok
:err
pause
:ok
