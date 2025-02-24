@echo off

PicoPadImg DispHSTX_logo.bmp DispHSTX_logo.cpp ImgLogo 8
if errorlevel 1 goto err

PicoPadImg Img532x400_24b.bmp Img532x400_16b.cpp Img532x400_16b 16
if errorlevel 1 goto err

PicoPadImg pattern.bmp pattern.cpp ImgPattern 8
if errorlevel 1 goto err

PicoPadImg Img1b.bmp Img1b.cpp Img1b 1
if errorlevel 1 goto err

PicoPadImg Img2b.bmp Img2b.cpp Img2b 2
if errorlevel 1 goto err

PicoPadImg Img3b.bmp Img3b.cpp Img3b 3
if errorlevel 1 goto err

PicoPadImg Img4b.bmp Img4b.cpp Img4b 4
if errorlevel 1 goto err

PicoPadImg Img6b.bmp Img6b.cpp Img6b 6
if errorlevel 1 goto err

PicoPadImg Img8b.bmp Img8b.cpp Img8b 8
if errorlevel 1 goto err

PicoPadImg Img12b.bmp Img12b.cpp Img12b 12d
if errorlevel 1 goto err

PicoPadImg Img15b.bmp Img15b.cpp Img15b 15
if errorlevel 1 goto err

PicoPadImg Img24b.bmp Img16b.cpp Img16b 16
if errorlevel 1 goto err

goto ok
:err
pause
:ok
