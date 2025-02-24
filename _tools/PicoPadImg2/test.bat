@echo off
md out
md test

PicoPadImg in\img1bit.bmp  out\img1bit.cpp  Img1Bit 1
if errorlevel 1 goto err

PicoPadImg in\img2bit.bmp  out\img2bit.cpp  Img2Bit 2
if errorlevel 1 goto err

PicoPadImg in\img3bit.bmp  out\img3bit.cpp  Img3Bit 3
if errorlevel 1 goto err

PicoPadImg in\img4bit.bmp  out\img4bit.cpp  Img4Bit 4
if errorlevel 1 goto err

PicoPadImg in\img6bit.bmp  out\img6bit.cpp  Img6Bit 6
if errorlevel 1 goto err

PicoPadImg in\img8bit.bmp  out\img8bit.cpp  Img8Bit 8
if errorlevel 1 goto err

PicoPadImg in\img15bit.bmp out\img12bit.cpp Img12Bit 12
if errorlevel 1 goto err

PicoPadImg in\img15bit.bmp out\img12bitd.cpp Img12BitD 12d
if errorlevel 1 goto err

PicoPadImg in\img15bit.bmp out\img15bit.cpp Img15Bit 15
if errorlevel 1 goto err

PicoPadImg in\img24bit.bmp out\img16bit.cpp Img16Bit 16
if errorlevel 1 goto err

PicoPadImg in\img4bit.bmp  out\imgrle4.cpp  ImgRle4 r4
if errorlevel 1 goto err

PicoPadImg in\img8bit.bmp  out\imgrle8.cpp  ImgRle8 r8
if errorlevel 1 goto err

PicoPadImg in\img15bit.bmp out\imghstx15.cpp  ImgHstx15 x15
if errorlevel 1 goto err

PicoPadImg in\img24bit.bmp out\imghstx16.cpp  ImgHstx16 x16
if errorlevel 1 goto err

PicoPadImg in\img4bit.bmp  out\imgattr1.cpp  ImgAttr1 a1 test\imgattr1_test.bmp 
if errorlevel 1 goto err

PicoPadImg in\img4bit.bmp  out\imgattr2.cpp  ImgAttr2 a2 test\imgattr2_test.bmp 
if errorlevel 1 goto err

PicoPadImg in\img4bit.bmp  out\imgattr3.cpp  ImgAttr3 a3 test\imgattr3_test.bmp 
if errorlevel 1 goto err

PicoPadImg in\img4bit.bmp  out\imgattr4.cpp  ImgAttr4 a4 test\imgattr4_test.bmp 
if errorlevel 1 goto err

PicoPadImg in\img8bit.bmp  out\imgattr5.cpp  ImgAttr5 a5 test\imgattr5_test.bmp 
if errorlevel 1 goto err

PicoPadImg in\img8bit.bmp  out\imgattr6.cpp  ImgAttr6 a6 test\imgattr6_test.bmp 
if errorlevel 1 goto err

PicoPadImg in\img8bit.bmp  out\imgattr7.cpp  ImgAttr7 a7 test\imgattr7_test.bmp 
if errorlevel 1 goto err

PicoPadImg in\img8bit.bmp  out\imgattr8.cpp  ImgAttr8 a8 test\imgattr8_test.bmp 
if errorlevel 1 goto err

goto ok
:err
pause
:ok
