@echo off
AS4040.exe busicom.s
if errorlevel 1 goto err

goto ok
:err
pause
:ok
