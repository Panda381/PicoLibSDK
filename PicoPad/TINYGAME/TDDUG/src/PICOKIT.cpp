//       >>>>> T-I-N-Y D-D-U-G for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-DDug is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// Reference in file "COPYING.txt".
// -__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-


// The Tiny-DDug source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tddug.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the tddug.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#include "../include.h"

uint8_t SnD_;
PICOCOMPATIBILITY PICOKIT;
  //private Methode
uint8_t PICOCOMPATIBILITY::Select_Byte(uint8_t y_) {
  return (y_ >> 3);
}
  
  int8_t PICOCOMPATIBILITY::ESP_RecupeLineY(int8_t Valeur) {
    return (Valeur >> 3);
  }

  int8_t PICOCOMPATIBILITY::ESP_RecupeDecalageY(uint8_t Valeur) {
    return (Valeur - ((Valeur >> 3) << 3));
  }
  
  uint8_t PICOCOMPATIBILITY::ESP_SplitSpriteDecalageY(uint8_t decalage, uint8_t Input, uint8_t UPorDOWN) {
    if (UPorDOWN) { return Input << decalage; }
    return Input >> (8 - decalage);
  }

   uint8_t PICOCOMPATIBILITY::ESP_blitzSprite(int8_t xPos, int8_t yPos, int8_t HSPRITE_, uint8_t xPASS, uint8_t yPASS, uint8_t FRAME, const uint8_t *SPRITES) {
    uint8_t OUTBYTE;
    uint8_t WSPRITE=SPRITES[0];
    int8_t HSPRITE = HSPRITE_;
    int16_t Wmax = ((HSPRITE * WSPRITE) + 1);
    int16_t PICBYTE = FRAME * (Wmax - 1);
    int8_t RECUPELINEY = ESP_RecupeLineY(yPos);
    if (
      (xPASS > (xPos + (WSPRITE - 1))) || (xPASS < xPos) || (RECUPELINEY > yPASS) || ((RECUPELINEY + (HSPRITE)) < yPASS)) { return 0x00; }
    int8_t SPRITEyLINE = (yPASS - (RECUPELINEY));
    int8_t SPRITEyDECALAGE = (ESP_RecupeDecalageY(yPos));
    int16_t ScanA = (((xPASS - xPos) + (SPRITEyLINE * WSPRITE)) + 2);
    int16_t ScanB = (((xPASS - xPos) + ((SPRITEyLINE - 1) * WSPRITE)) + 2);
    if (ScanA > Wmax) {
      OUTBYTE = 0x00;
    } else {
      OUTBYTE = ESP_SplitSpriteDecalageY(SPRITEyDECALAGE, SPRITES[ScanA + (PICBYTE)], 1);
    }
    if (SPRITEyLINE > 0) {
      uint8_t OUTBYTE2 = ESP_SplitSpriteDecalageY(SPRITEyDECALAGE, SPRITES[ScanB + (PICBYTE)], 0);
      if (ScanB > Wmax) {
        return OUTBYTE;
      } else {
        return OUTBYTE | OUTBYTE2;
      }
    } else {
      return OUTBYTE;
    }
  }

  //private methode end
  

uint16_t PICOCOMPATIBILITY::RecupBackColor(void){
return Back_Color;
}

void PICOCOMPATIBILITY::TinyOLED_Send(uint8_t x_,uint8_t y_,uint8_t Byte_){
Buffer[x_+(y_*128)]=Byte_;
}

void PICOCOMPATIBILITY::drawPixel(int16_t x_, int16_t y_){
	  uint8_t MaxX = 128;
  uint8_t ByteNo = Select_Byte(y_);
  uint16_t Byte_=(x_ + (ByteNo * MaxX));
    Buffer[Byte_] |= (1 << (y_ - (ByteNo << 3)));  // Mettre le bit à 1

}

void PICOCOMPATIBILITY::drawPixel(int16_t x_, int16_t y_,uint8_t Col_){
  uint8_t MaxX = 128;
  uint8_t ByteNo = Select_Byte(y_);
  uint16_t Byte_=(x_ + (ByteNo * MaxX));
  if (Col_) {
    Buffer[Byte_] |= (1 << (y_ - (ByteNo << 3)));  // Mettre le bit à 1
  } else {
     Buffer[Byte_] &= ~(1 << (y_ - (ByteNo << 3))); // Mettre le bit à 0
  }
}

void PICOCOMPATIBILITY::drawPixelColor(int16_t x_, int16_t y_,uint16_t Col_){
    const uint16_t WIDTH_MOD = WIDTH;
   x_=(x_*2)+PICO_OffsetX;
   y_=(y_*2)+PICO_OffsetY;
   int16_t Xx=(x_ + 1);
   int16_t Yy=(y_ + 1);
            FrameBuf[x_ + (y_ * WIDTH_MOD)] = Col_;
            #ifdef FULL_PIXEL
            FrameBuf[Xx + (y_ * WIDTH_MOD)] = Col_;
            FrameBuf[x_ + (Yy * WIDTH_MOD)] = Col_;
            FrameBuf[Xx + (Yy * WIDTH_MOD)] = Col_;
            #endif
}

uint16_t PICOCOMPATIBILITY::getPixelColor(int16_t x_, int16_t y_) {
    const uint16_t WIDTH_MOD = WIDTH;
   x_=(x_*2)+PICO_OffsetX;
   y_=(y_*2)+PICO_OffsetY;
           return FrameBuf[x_ + (y_ * WIDTH_MOD)] ;
             
}

uint8_t PICOCOMPATIBILITY::getPixel(int16_t x_, int16_t y_) {
  uint8_t MaxX = 128;
  uint8_t ByteNo = Select_Byte(y_);
  uint8_t ByTe = (Buffer[(x_ + (ByteNo * MaxX))]);
  return ((ByTe & (1 << (y_ - (ByteNo << 3)))) == 0) ? 0 : 1;
}

uint8_t *PICOCOMPATIBILITY::getBuffer(void)
{
    return Buffer;
}

  uint8_t PICOCOMPATIBILITY::RBuffer(uint16_t pos_) {
    return Buffer[pos_];
  }

  void PICOCOMPATIBILITY::sBuffer(uint16_t pos_, uint8_t Val_) {
    Buffer[pos_] = Val_;
  }

  void PICOCOMPATIBILITY::SetFPS(uint8_t Val_) {
    FPS_Break = (1000 / Val_);
  }
  
      void PICOCOMPATIBILITY::setFrameRate(uint8_t Frm){
		      FPS_Break = (1000 / Frm);
  }
  

  void PICOCOMPATIBILITY::FPS_Temper(void) {
  while ((curtMil - MemMil) < FPS_Break) { curtMil = pico_millis(); }
    MemMil = curtMil;
  }

  void PICOCOMPATIBILITY::Init(void) {
SoundSet();
SetColorBuffer(31,63,31,31,63,31,0,0,0);
DigitalControl(PWMSND_GPIO,false);
  }
  void PICOCOMPATIBILITY::clear(void) {
memset(Buffer, 0, sizeof(Buffer));
}

void PICOCOMPATIBILITY::FillScreenColor(uint16_t Col_){
		   // Définition des dimensions de la section à effacer
    const int width = 256;
    const int height = 128;

    // Définition des offsets pour centrer la section
    const int offsetx = 32;
    const int offsety = 48;

    // Calcule de l'index de départ
    int startIdx = (offsety * 320) + offsetx;

    // Effacement de la section de buffer
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            FrameBuf[startIdx + y * 320 + x] = Col_;
        }
    }  
	  
}

  void PICOCOMPATIBILITY::clearPicoBuffer(void) {
   memset(FrameBuf,0, 320 * 240 * sizeof(uint16_t)); 
}

  void PICOCOMPATIBILITY::Begin(void) {}
  
  void PICOCOMPATIBILITY::DisplayColor(void) {
//DrawSSDBuffer();
//transferBufferToColorDisplay_320x240();
DispDirtyRect(32,48,256,128);
DispUpdate();
        #if USE_SCREENSHOT
		switch (KeyGet())
		{
		case KEY_Y: SmallScreenShot();
		}
		#else
		if (KeyPressed(KEY_Y)) ResetToBootLoader();
		#endif
  }

  void PICOCOMPATIBILITY::display(void) {
DrawSSDBuffer();
//transferBufferToColorDisplay_320x240();
DispDirtyRect(32,48,256,128);
DispUpdate();
        #if USE_SCREENSHOT
		switch (KeyGet())
		{
		case KEY_Y: SmallScreenShot();
		}
		#else
		if (KeyPressed(KEY_Y)) ResetToBootLoader();
		#endif
  }

  void PICOCOMPATIBILITY::fillScreen(uint8_t t) {
memset(Buffer, t, sizeof(Buffer));
  }

  void PICOCOMPATIBILITY::ClearBuffer(void) {
memset(Buffer, 0, sizeof(Buffer));
  }

  uint8_t PICOCOMPATIBILITY::nextFrame(void) {
if ((curtMil - MemMil) < FPS_Break) { curtMil = pico_millis();return 0; }else{
    MemMil = curtMil;return 1;}
  }
  uint8_t PICOCOMPATIBILITY::pressed(uint8_t) {
    return 1;
  }

  void PICOCOMPATIBILITY::drawSelfMasked(int8_t x_, int8_t y_, uint8_t const *PRGM_, uint8_t F_) {
int16_t PositionByte;
    if ((y_ > 63) || (x_ < -16) || (y_ < -16)) return;

    uint8_t WSPRITE_ = PRGM_[0];
    int8_t HSPRITE_ = ESP_RecupeLineY(PRGM_[1]);
    int8_t STARTY = ESP_RecupeLineY(y_);
    int8_t BL_STARTY = STARTY;

  //  if (STARTY < 0) STARTY = 0;

    if (((PRGM_[1]) - (HSPRITE_ << 3)) != 0) {
      HSPRITE_++;
    }

    uint8_t endY = BL_STARTY + HSPRITE_ + 1;

    for (int16_t y = STARTY; y < endY; y++) {

      if (y > 63) break;

      for (int16_t x = x_; x < (WSPRITE_ + x_); x++) {

        if (x > 127) break;
       PositionByte= x + (y << 7);
if ((PositionByte>=0)&& (PositionByte<=1023)) {
       Buffer[PositionByte] |=ESP_blitzSprite(x_, y_, HSPRITE_, x, y, F_, PRGM_);
      }}
    }
  }
  
   void PICOCOMPATIBILITY::drawSelfMaskedColor(int16_t x_, int16_t y_, uint8_t const *PRGM_, uint8_t F_,uint16_t col) {
int16_t PositionByte;
    if ((y_ > 63) || (x_ < -16) || (y_ < -16)) return;

    uint8_t WSPRITE_ = PRGM_[0];
    int8_t HSPRITE_ = ESP_RecupeLineY(PRGM_[1]);
    int8_t STARTY = ESP_RecupeLineY(y_);
    int8_t BL_STARTY = STARTY;

  //  if (STARTY < 0) STARTY = 0;

  /*  if (((PRGM_[1]) - (HSPRITE_ << 3)) != 0) {
      HSPRITE_++;
    }*/

    uint8_t endY = BL_STARTY + HSPRITE_ + 1;

    for (int16_t y = STARTY; y < endY; y++) {

      if (y > 63) break;

      for (int16_t x = x_; x < (WSPRITE_ + x_); x++) {

        if (x > 127) break;
       PositionByte= x + (y << 7);
if ((PositionByte>=0)&& (PositionByte<=1023)) {
	DirectDraw(x,y,ESP_blitzSprite(x_, y_, HSPRITE_, x, y, F_, &PRGM_[0]),col);
   //    Buffer[PositionByte] |=ESP_blitzSprite(x_, y_, HSPRITE_, x, y, F_, PRGM_);
      }}
    }
  }

  void PICOCOMPATIBILITY::drawSelfMasked_OverScan(int8_t x_, int8_t y_, uint8_t const *PRGM_, uint8_t F_) {
int16_t PositionByte;
    if ((y_ > 63) || (x_ < 0) || (y_ < -64)) return;


    uint8_t WSPRITE_ = PRGM_[0];
    int8_t HSPRITE_ = ESP_RecupeLineY(PRGM_[1]);
    int8_t STARTY = ESP_RecupeLineY(y_);
    int8_t BL_STARTY = STARTY;

    if (STARTY < 0) STARTY = 0;

    if (((PRGM_[1]) - (HSPRITE_ << 3)) != 0) {
      HSPRITE_++;
    }

    uint8_t endY = BL_STARTY + HSPRITE_ + 1;

    for (int16_t y = STARTY; y < endY; y++) {

      if (y > 63) break;


      for (int16_t x = x_; x < (WSPRITE_ + x_); x++) {

        if (x > 127) break;
       PositionByte= x + (y << 7);
if ((PositionByte>=0)&& (PositionByte<=1023)) {
       Buffer[PositionByte] |= ESP_blitzSprite(x_, y_, HSPRITE_, x, y, F_, PRGM_);
      }}
    }
  }

  void PICOCOMPATIBILITY::drawErase(int8_t x_, int8_t y_, uint8_t const *PRGM_, uint8_t F_) {
int16_t PositionByte;
    if ((y_ > 63) || (x_ < -16) || (y_ < -16)) return;


    uint8_t WSPRITE_ = PRGM_[0];
    int8_t HSPRITE_ = ESP_RecupeLineY(PRGM_[1]);
    int8_t STARTY = ESP_RecupeLineY(y_);
    int8_t BL_STARTY = STARTY;

 //   if (STARTY < 0) STARTY = 0;

    if (((PRGM_[1]) - (HSPRITE_ << 3)) != 0) {
      HSPRITE_++;
    }


    uint8_t endY = BL_STARTY + HSPRITE_ + 1;

    for (int16_t y = STARTY; y < endY; y++) {

      if (y > 63) break;


      for (int16_t x = x_; x < (WSPRITE_ + x_); x++) {

        if (x > 127) break;
       PositionByte= x + (y << 7);
if ((PositionByte>=0)&& (PositionByte<=1023)) {
        Buffer[PositionByte] &= ~(ESP_blitzSprite(x_, y_, HSPRITE_, x, y, F_, PRGM_));
	}
      }
    }
  }

 void PICOCOMPATIBILITY::drawEraseColor(int16_t x_, int16_t y_, uint8_t const *PRGM_, uint8_t F_) {
drawSelfMaskedColor(x_, y_, PRGM_, F_,color(Back_Red,Back_Green,Back_Blue));
 }

  void PICOCOMPATIBILITY::drawInvertPixel(int8_t x_, int8_t y_, uint8_t const *PRGM_, uint8_t F_) {
int16_t PositionByte;
    if ((y_ > 63) || (x_ < -16) || (y_ < -16)) return;


    uint8_t WSPRITE_ = PRGM_[0];
    int8_t HSPRITE_ = ESP_RecupeLineY(PRGM_[1]);
    int8_t STARTY = ESP_RecupeLineY(y_);
    int8_t BL_STARTY = STARTY;

    if (STARTY < 0) STARTY = 0;


    if (((PRGM_[1]) - (HSPRITE_ << 3)) != 0) {
      HSPRITE_++;
    }


    uint8_t endY = BL_STARTY + HSPRITE_ + 1;

    for (int16_t y = STARTY; y < endY; y++) {

      if (y > 63) break;


      for (int16_t x = x_; x < (WSPRITE_ + x_); x++) {

        if (x > 127) break;
       PositionByte= x + (y << 7);
if ((PositionByte>=0)&& (PositionByte<=1023)) {

        Buffer[PositionByte] ^= ESP_blitzSprite(x_, y_, HSPRITE_, x, y, F_, PRGM_);
      }}
    }
  }

  void PICOCOMPATIBILITY::drawOverwrite(int8_t x_, int8_t y_, uint8_t const *PRGM_, uint8_t F_) {
int16_t PositionByte;
    if ((y_ > 63) || (x_ < -16) || (y_ < -16)) return;


    uint8_t WSPRITE_ = PRGM_[0];
    int8_t HSPRITE_ = ESP_RecupeLineY(PRGM_[1]);
    int8_t STARTY = ESP_RecupeLineY(y_);
    int8_t BL_STARTY = STARTY;

    if (STARTY < 0) STARTY = 0;


    if (((PRGM_[1]) - (HSPRITE_ << 3)) != 0) {
      HSPRITE_++;
    }


    int16_t endY = BL_STARTY + HSPRITE_ + 1;

    for (int16_t y = STARTY; y < endY; y++) {

      if (y > 63) break;


      for (int16_t x = x_; x < (WSPRITE_ + x_); x++) {

        if (x > 127) break;
       PositionByte= x + (y << 7);
if ((PositionByte>=0)&& (PositionByte<=1023)) {

       Buffer[PositionByte] &= BlackSquare(y_, y, PRGM_);
       Buffer[PositionByte] |= ESP_blitzSprite(x_, y_, HSPRITE_, x, y, F_, PRGM_);
   }
      }
    }
  }

  void PICOCOMPATIBILITY::drawOverwriteColor(int16_t x_, int16_t y_, uint8_t const *PRGM_, uint8_t F_,uint16_t col_) {
int16_t PositionByte;
    if ((y_ > 63) || (x_ < -16) || (y_ < -16)) return;


    uint8_t WSPRITE_ = PRGM_[0];
    int8_t HSPRITE_ = ESP_RecupeLineY(PRGM_[1]);
    int8_t STARTY = ESP_RecupeLineY(y_);
    int8_t BL_STARTY = STARTY;

    if (STARTY < 0) STARTY = 0;

/*
    if (((PRGM_[1]) - (HSPRITE_ << 3)) != 0) {
      HSPRITE_++;
    }
*/

    int16_t endY = BL_STARTY + HSPRITE_ + 1;

int16_t sx=32+(x_*2),sy=48+(y_*2),wc=(WSPRITE_*2),hc=(PRGM_[1]*2);




for (int16_t y=sy;y<(sy+hc);y++){
for (int16_t x=sx;x<(sx+wc);x++){
	 FrameBuf[x + (y * WIDTH)] =0;// Back_Color;
}	
}


    for (int16_t y = STARTY; y < endY; y++) {

      if (y > 63) break;


      for (int16_t x = x_; x < (WSPRITE_ + x_); x++) {

        if (x > 127) break;
       PositionByte= x + (y << 7);
if ((PositionByte>=0)&& (PositionByte<=1023)) {

DirectDraw(ESP_blitzSprite(x_, y_, HSPRITE_, x, y, F_, PRGM_),x,y,col_);
   }
      }
    }
  }

void PICOCOMPATIBILITY::drawLineColor(int8_t x0, int8_t y0, int8_t x1, int8_t y1, uint16_t Pix_) {
    const uint16_t WIDTH_MOD = WIDTH; // Assuming WIDTH is constant
    const int8_t dx = abs(x1 - x0);
    const int8_t dy = abs(y1 - y0);
    const int8_t sx = (x0 < x1) ? 1 : -1;
    const int8_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;
    const int16_t OffsetX = PICO_OffsetX;
    const int16_t OffsetY = PICO_OffsetY;
    
    uint16_t *pFrameBuf = FrameBuf; // Direct pointer to FrameBuf

    while (1) {
        int16_t xx = (x0 << 1) + OffsetX; // x0 * 2 via bit shift
        int16_t yy = (y0 << 1) + OffsetY; // y0 * 2 via bit shift
        uint16_t *baseAddr = pFrameBuf + xx + (yy * WIDTH_MOD);
        
        *baseAddr = Pix_;
        #ifdef FULL_PIXEL
        *(baseAddr + 1) = Pix_;
        *(baseAddr + WIDTH_MOD) = Pix_;
        *(baseAddr + WIDTH_MOD + 1) = Pix_;
        #endif
        
        if (x0 == x1 && y0 == y1) break;

        int16_t err2 = err << 1; // err * 2 via bit shift
        if (err2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (err2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

  uint8_t PICOCOMPATIBILITY::BlackSquare(int8_t y, uint8_t yPASS, const uint8_t *PRGM_) {
    uint8_t Result = 0xff;

    uint8_t Line = ESP_RecupeLineY(y);
    uint8_t Dec = ESP_RecupeDecalageY(y);

    uint8_t MH = PRGM_[1];
    uint8_t MH_Line = ESP_RecupeLineY(MH + y);
    uint8_t DecH = ESP_RecupeDecalageY(MH + y);

    if (((yPASS * 8) > y) && (((yPASS * 8) + 7) < (MH + y))) {
      return 0x00;
    }

    if (Line == yPASS) {
      Result >>= (8 - Dec);
    }

    if (MH_Line == yPASS) {
      Result <<= DecH;
    }

    return Result;
  }

uint8_t PICOCOMPATIBILITY::ExploreMap(uint8_t x_, uint8_t y_){
  uint8_t MaxX = 128;
  uint8_t ByteNo = Select_Byte(y_);
  uint8_t ByTe = (Buffer[(x_ + (ByteNo * MaxX))]);
  return ((ByTe & (1 << (y_ - (ByteNo << 3)))) == 0) ? 0 : 1;
}

void PICOCOMPATIBILITY::DirectDraw(int16_t XPASS, int16_t YPASS, uint8_t Byte_, uint16_t Col_) {
    if (XPASS < 0 || XPASS > 127 || YPASS < 0 || YPASS > 127) return;

    const uint16_t WIDTH_MOD = WIDTH;
    const int16_t Off_setX = PICO_OffsetX;
    const int16_t Off_setY = PICO_OffsetY;
    const int16_t YPassMul = YPASS << 3; // YPASS * 8 via décalage
    
    uint16_t *pFrameBuf = FrameBuf; // Pointer to FrameBuf for direct memory access
    
    for (uint8_t B_ = 0; B_ < 8; B_++) {
        if (Byte_ & (1 << B_)) {
            int16_t x2 = (XPASS << 1) + Off_setX; // XPASS * 2 via décalage
            int16_t y2 = (YPassMul << 1) + Off_setY + (B_ << 1); // YPassMul * 2 + B_*2 via décalage
            
            uint16_t *baseAddr = pFrameBuf + x2 + (y2 * WIDTH_MOD);
            
            *baseAddr = Col_;
            #ifdef FULL_PIXEL
            *(baseAddr + 1) = Col_;
            *(baseAddr + WIDTH_MOD) = Col_;
            *(baseAddr + WIDTH_MOD + 1) = Col_;
            #endif
        }
    }
}


void PICOCOMPATIBILITY::SetColorBuffer(uint8_t TR,uint8_t TG,uint8_t TB,uint8_t DR,uint8_t DG,uint8_t DB,uint8_t BR,uint8_t BG,uint8_t BB){
Top_Red=TR;
Top_Green=TG;
Top_Blue=TB;

Down_Red=DR;
Down_Green=DG;
Down_Blue=DB;

Back_Red=BR;
Back_Green=BG;
Back_Blue=BB;

Back_Color=color(Back_Red,Back_Green,Back_Blue);	
}

uint16_t PICOCOMPATIBILITY::PrivateColorRecup(uint8_t y_){
return color(Mymap(y_,0,63,Top_Red,Down_Red),Mymap(y_,0,63,Top_Green,Down_Green),Mymap(y_,0,63,Top_Blue,Down_Blue));
}

uint16_t PICOCOMPATIBILITY::PrivateColorRecup2(uint8_t y_){
switch(y_){
	case 0 ... 7 : return color(0,0,31); break;
	case 8 ... 19 : return color(31,53,0); break;
	case 20 ... 31 : return color(31,35,0); break;
	case 32 ... 42 : return color(31,20,0); break;
	case 43 ... 56 : return color(31,5,0); break;
}
return color(31,63,31);
}

void PICOCOMPATIBILITY::DrawSSDBuffer(void) {
    #define Off_setX 32
    #define Off_setY 48
    // Dimension des buffers
    const uint16_t WIDTH_MOD = WIDTH; // Eviter la répétition
    uint8_t y, x;
    for (y = 0; y < 64; y++) {
        for (x = 0; x < 128; x++) {
			uint16_t Pix_ = (ExploreMap(x, y)) ? PrivateColorRecup2(y) : color(Back_Red,Back_Green,Back_Blue);
            uint16_t x2 = (x * 2) + Off_setX;
            uint16_t y2 = (y * 2) + Off_setY;

            // Utilisé un seul accès à FrameBuf pour mettre à jour les pixels
             
            FrameBuf[x2 + (y2 * WIDTH_MOD)] = Pix_;
            #ifdef FULL_PIXEL
            FrameBuf[(x2 + 1) + (y2 * WIDTH_MOD)] = Pix_;
            FrameBuf[x2 + ((y2 + 1) * WIDTH_MOD)] = Pix_;
            FrameBuf[(x2 + 1) + ((y2 + 1) * WIDTH_MOD)] = Pix_; 
            #endif
        }
    } 

}

void PICOCOMPATIBILITY::transferBufferToColorDisplay_320x240() {

    for (int yOrig = 0; yOrig < 64; ++yOrig) {
        for (int xOrig = 0; xOrig < 128; ++xOrig) {
            // Trouver l'index du byte correspondant dans le buffer monochrome
            int byteIndex = (yOrig / 8) * 128 + xOrig;
            int bitIndex = yOrig % 8; // Le bit dans le byte représente la position verticale

            // Extraire le bit (pixel) du byte
            bool pixel = (Buffer[byteIndex] >> bitIndex) & 1;

            // Calculer la position sur l'écran couleur avec les offsets
            int yScreen = 48 + (yOrig*2);
            int xScreen = 32 + (xOrig*2);

            // S'assurer que les coordonnées sont dans les limites de l'écran
            if (yScreen < 240 && xScreen < 320) {
                FrameBuf[(yScreen) * 320 + (xScreen)] = pixel ? 0xFFFF : 0x0000; // Blanc ou Noir en RGB565
            }
        }
    }
    }

void DigitalControl(uint8_t gpio_pin, bool state) {
    // Initialiser le GPIO
    gpio_init(gpio_pin);
    // Définir le GPIO comme sortie
    gpio_set_dir(gpio_pin, GPIO_OUT);
    // Mettre le GPIO à l'état spécifié (true pour ON, false pour OFF)
    gpio_put(gpio_pin, state);
}

int16_t Mymap(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint16_t color(uint8_t R, uint8_t G, uint8_t B) {
    // Décaler R de 11 bits (puisque R occupe les bits 11 à 15)
    // Décaler G de 5 bits (puisque G occupe les bits 5 à 10)
    // B occupe les bits 0 à 4, donc pas de décalage nécessaire
    return ((R & 0x1F) << 11) | ((G & 0x3F) << 5) | (B & 0x1F);
}

void SoundSet(void){
	if (Config.volume>0) {SnD_=true;}else{SnD_=false;}
}

void Sound(uint8_t freq_, uint8_t dur){
uint8_t Invert_=(255-freq_);
uint8_t H_L=(freq_!=0)?SnD_:0;
uint8_t HiDur=Mymap(Config.volume,0,254,0,Invert_);
uint16_t LoDur=Invert_+(Invert_-HiDur);

for (uint8_t t=0;t<dur;t++){
  gpio_put(PWMSND_GPIO, H_L);
for (uint8_t t=0;t<HiDur;t++){
_delay_us(1);}
  gpio_put(PWMSND_GPIO, false);
for (uint16_t t=0;t<LoDur;t++){
_delay_us(1); }}
  gpio_put(PWMSND_GPIO, false);
}

uint32_t pico_millis(void) {
    // Obtenez le nombre de millisecondes depuis le démarrage de la Pico
    return to_ms_since_boot(get_absolute_time());
}

void MEGA_PLAY_MUSIC(const uint8_t *Music_) {
  uint8_t t, to_ = Music_[0];
  for (t = 1; t < to_; t = t + 2) {
    Sound(Music_[t], Music_[t + 1]);
  }
}

