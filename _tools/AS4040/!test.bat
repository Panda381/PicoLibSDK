@echo off
if exist busicom.bin del busicom.bin
AS4040.exe busicom.s
echo.
fc /b busicom.bin busicom_orig.bin
echo.
pause
