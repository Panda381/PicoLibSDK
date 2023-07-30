@echo off
HidComp in1.txt out1.txt
if errorlevel 1 goto err
HidComp in2.txt out2.txt
if errorlevel 1 goto err
HidComp in3.txt out3.txt
if errorlevel 1 goto err
HidComp in4.txt out4.txt
if errorlevel 1 goto err
HidComp in5.txt out5.txt
if errorlevel 1 goto err
HidComp in6.txt out6.txt
if errorlevel 1 goto err
HidComp in7.txt out7.txt
if errorlevel 1 goto err
HidComp in8.txt out8.txt
if errorlevel 1 goto err
HidComp in9.txt out9.txt
if errorlevel 1 goto err


HidComp out1.txt outB1.txt 
if errorlevel 1 goto err
HidComp out2.txt outB2.txt
if errorlevel 1 goto err
HidComp out3.txt outB3.txt
if errorlevel 1 goto err
HidComp out4.txt outB4.txt
if errorlevel 1 goto err
HidComp out5.txt outB5.txt
if errorlevel 1 goto err
HidComp out6.txt outB6.txt
if errorlevel 1 goto err
HidComp out7.txt outB7.txt
if errorlevel 1 goto err
HidComp out8.txt outB8.txt
if errorlevel 1 goto err
HidComp out9.txt outB9.txt
if errorlevel 1 goto err


goto ok

:err
pause

:ok
