//  >>>>> GILBERT IN THE DOWNLAND for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Gilbert in the Downland is free software: you can redistribute it and/or modify
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


// The Gilbert in the Downland source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "downland.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the downland.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK


#include "../include.h"

#define Gilbert_Col color(23,53,31)
#define Gilbert_SH_Col color(0,33,23)
#define Display_col color(15,31,15)
extern uint8_t SnD_;
typedef struct GITD_LvL {
  uint8_t Room;
  uint8_t MemRoom;
  const int8_t *Vec[NBoC] = { Vec0, Vec1, Vec2, Vec3, Vec4, Vec5, Vec6, Vec7, Vec8, Vec9, Vec10 };
  const int8_t *RopeVev[NBoC] = { RopeVec0, RopeVec1, RopeVec2, RopeVec3, RopeVec4, RopeVec5, RopeVec6, RopeVec7, RopeVec8, RopeVec9, RopeVec10 };
  const int8_t *SPK[NBoC] = { SPK0, SPK1, SPK2, SPK3, SPK4, SPK5, SPK6, SPK7, SPK8, SPK9, SPK10 };
  const int8_t *AcidDrop[NBoC] = { AcidDrop0, AcidDrop1, AcidDrop2, AcidDrop3, AcidDrop4, AcidDrop5, AcidDrop6, AcidDrop7, AcidDrop8, AcidDrop9, AcidDrop10 };
} GITD_LvL;

extern PICOCOMPATIBILITY PICOKIT;

uint8_t JumpCycle = 0;
uint8_t JumpCycleMem = 0;
uint8_t RopeMode = 0;
uint8_t UPDOWN = 2;
uint8_t DDead = 0;
uint8_t RoomChange = 0;
uint8_t Frm = 0;

uint8_t VisibleKeyAndDoors[22];  //Total of 21 keys -  First Byte= 1!
uint8_t VisibleSprite[30];
uint8_t Flip = 0;
LastPos Lastpos;

Sprite_GITD GITD;
AcidDrop_GITD AcidDrop[6];
TIMERGITD TM1, TM2, TM3;
Ballon_GITD Ballon;
GITD_LvL GITDLvL;
NUMGITD NumPan;
TIMEOUT TMout;


void DrawRecBW(uint8_t x, uint8_t y, uint8_t x2, uint8_t y2, uint8_t Col_) {
  for (uint8_t y_ = y; y_ <= y2; y_++) {
    for (uint8_t x_ = x; x_ <= x2; x_++) {
      PICOKIT.drawPixel(x_, y_, Col_);
    }
  }
}

void InvertPix(uint8_t Flip_, uint8_t x, uint8_t y, uint8_t x2, uint8_t y2) {
  for (uint8_t y_ = y; y_ <= y2; y_++) {
    for (uint8_t x_ = x; x_ <= x2; x_++) {
      if (Flip_) {
        PICOKIT.drawPixel(x_, y_, !PICOKIT.getPixel(x_, y_));
      }
    }
  }
}

void SplashScreen(void) {
PICOKIT.SetColorBuffer(0,40,0,0,40,0,0,8,0);
  uint8_t Flip_ = 0;
  for (uint8_t Loop_ = 0; Loop_ < 9; Loop_++) {
    Flip_ = !Flip_;
    PICOKIT.clear();
    PICOKIT.drawSelfMasked(0, 0, intro_GITD, 0);
    InvertPix(Flip_, 28, 9, 32, 15);
    DrawRecBW(28, 19, 36, 33, WHITE);
    PICOKIT.display();
    _delay_ms(400);
  }
  PICOKIT.clear();
  PICOKIT.drawSelfMasked(0, 0, intro_GITD, 0);
  InvertPix(Flip_, 28, 9, 32, 15);
  DrawRecBW(28, 25, 36, 33, WHITE);
  PICOKIT.display();
  _delay_ms(250);

while(1) {
    Flip_ =( Flip_<30)?Flip_+1:0;
    PICOKIT.clear();
    PICOKIT.drawSelfMasked(0, 0, intro_GITD, 0);
    InvertPix(1, 28, 9, 32, 15);
    if (Flip_<15) {DrawRecBW(28, 25, 36, 33, WHITE);}
    PICOKIT.display();
    if (BUTTON_DOWN) {
      PICOKIT.clear();
      PICOKIT.display();
      Sound(50,100);
      Sound(10,100);
      while(1){
    if (BUTTON_UP) {
      break;}
      }
      _delay_ms(250);
      break;
      }
    while (!PICOKIT.nextFrame()) {}
  }
  PICOKIT.SetColorBuffer(0,40,0,0,40,0,0,0,0);
}

int main() {
  PICOKIT.setFrameRate(50);
  PICOKIT.Init();
  SplashScreen();
  TM1.Init(3);
  TM1.Activate();
  TM2.Init(10);
  TM2.Activate();
  TM3.Init(2);
  TM3.Activate();
  NumPan.HIScores = 0;
NewGame:;
  DrawMainScreen();
  TMout.deactivate();
  GITD.Init();
  InitAllItems();
  VisibleKeyAndDoors[Start_Room] = 1;  //unlock Room
  GotoNexRoom(Start_Room);             //=Start Chamber;
  VisibleKeyAndDoors[Start_Room] = 0;  //Relock Room
NewLive:;
  TMout.ResetTime();
  TMout.deactivate();
  DDead = 4;
  FrmCall();
  while (1) {
    DrawRoomInProgress();
    MovingMode();
    RopeProcess();
    CalculateAcidDrop();
    DrawAllSprites();
    if (!RoomChange) drawRopeVector(x_offset, y_offset, GITDLvL.RopeVev[GITDLvL.Room]);
    Draw_Gilbert_ITD();
    Draw_Ballon();
    if (!RoomChange) Sprite_Colid();
    RoomTransition();
    DisplayNum();
    TMout.TimeOutFunction(GITD.Get_X(), GITD.Get_Y());
    PICOKIT.DisplayColor();
    if (DDead == 3) {
      if (NumPan.Lives == 0) {
        _delay_ms(100);
        NumPan.HIScores = (NumPan.HIScores < NumPan.Scores) ? NumPan.Scores : NumPan.HIScores;
        PICOKIT.clearPicoBuffer();
        PICOKIT.DisplayColor();
        _delay_ms(100);
        goto NewGame;
      }
      goto NewLive;
    }
    while (!PICOKIT.nextFrame()) {}
  }
}

void InitPan(void) {
  NumPan.Scores = 0;
  NumPan.Lives = 4;
  NumPan.M10000 = 0;
  NumPan.M1000 = 0;
  NumPan.M100 = 0;
  NumPan.M10 = 0;
  NumPan.M1 = 0;
}

void InitnextRoom(void) {
  for (uint8_t t_ = 0; t_ < 6; t_++) {
    AcidDrop[t_].InitAcidDrop();
  }
  CheckIfBallExist(GITDLvL.SPK[GITDLvL.Room]);
}

void ResetPickupSprite(void) {
  for (uint8_t t = 0; t < 22; t++) {
    VisibleKeyAndDoors[t] = 0;
  }
  VisibleKeyAndDoors[0] = 1;
  for (uint8_t t = 0; t < 30; t++) {
    VisibleSprite[t] = 1;
  }
}

void InitAllItems(void) {
  ResetPickupSprite();
  InitnextRoom();
  InitPan();
  ResetVarForNewGame();
}

void ResetVarForNewGame(void) {
  JumpCycle = 0;
  JumpCycleMem = 0;
  RopeMode = 0;
  UPDOWN = 2;
  DDead = 0;
  RoomChange = 0;
}

void DrawMainScreen(void) {
  GITDLvL.Room = 10;
  uint8_t Alt = 0;
  while (1) {
    PICOKIT.clearPicoBuffer();
    DrawVector(x_offset, y_offset, GITDLvL.Vec[GITDLvL.Room]);
for (uint8_t t=0;t<2;t++){
    CalculateAcidDrop();}
    PICOKIT.drawSelfMaskedColor(6 + x_offset, 6 + y_offset, MainTitle_GITD, 0,color(0,0,31));
    if (Alt > 25) {
      for (uint8_t t = 25; t < 84; t++) {
        PICOKIT.drawEraseColor(t, 45, byte__GITD, 0);
      }
    }
    Alt = (Alt < 50) ? Alt + 1 : 0;
    DrawScore(64, 56, NumPan.HIScores);
    DrawAcidDrop();
   PICOKIT.DisplayColor();
    if (BUTTON_DOWN) { goto Exit_1; }
    while (!PICOKIT.nextFrame()) {}
  }
Exit_1:;
  RoomChange = 1;
  GITDLvL.MemRoom = 10;
  GITDLvL.Room = 0;
  GITD.SetSpritePos(88, 56);
  RoomTransition();
}

void SoundSystem(uint8_t Snd_) {
  switch (Snd_) {
    case 0:
      Sound(100, 100);
      Sound(140, 100);
      break;
    case 1:
      Sound(1, 2);
      Sound(200, 1);
      break;
    case 2:
      Sound(1, 2);
      Sound(200, 1);
      break;
    case 3:
      Sound(150, 2);
      Sound(175, 2);
      break;
    case 4: Sound(150, 1); break;
    default: break;
  }
}

void SubLive(void) {
  if (NumPan.Lives > 0) {
    NumPan.Lives--;
  }
}

void DrawLives(int8_t x_, int8_t y_) {
  for (uint8_t t = 0; t < NumPan.Lives; t++) {
    PICOKIT.drawSelfMaskedColor(DigL_GITD[t * 2] + x_, DigL_GITD[(t * 2) + 1] + y_, gilbert_GITD, Frm,Gilbert_SH_Col);
  }
}

void DrawCH(int8_t x_, int8_t y_) {
  PICOKIT.drawSelfMaskedColor(0 + x_, y_, CH_GITD, 0,color(0,0,31));
  PICOKIT.drawSelfMaskedColor(11 + x_, y_, NUMERICGITD, GITDLvL.Room,Display_col);
}

void DisplayNum(void) {
  DrawScore(102, 0, NumPan.Scores);
  DrawTime(106, 59);
  DrawLives(114, 32);
  DrawCH(114, 16);
}

void DrawTime(uint8_t xx_, uint8_t yy_) {
  uint8_t x_ = xx_ + x_offset;
  uint8_t y_ = yy_ + y_offset;
  NumCalculate(TMout.Get_Time());
  PICOKIT.drawSelfMaskedColor(x_, y_, NUMERICGITD, NumPan.M1000,Display_col);
  PICOKIT.drawSelfMaskedColor(x_ + 4, y_, NUMERICGITD, NumPan.M100,Display_col);
  PICOKIT.drawSelfMaskedColor(x_ + 8, y_, NUMERICGITD, NumPan.M10,Display_col);
  PICOKIT.drawSelfMaskedColor(x_ + 12, y_, NUMERICGITD, NumPan.M1,Display_col);
}

void DrawScore(uint8_t xx_, uint8_t yy_, uint16_t Val_) {
  uint8_t x_ = xx_ + x_offset;
  uint8_t y_ = yy_ + y_offset;
  NumCalculate(Val_);
  PICOKIT.drawSelfMaskedColor(x_, y_, NUMERICGITD, NumPan.M10000,Display_col);
  PICOKIT.drawSelfMaskedColor(x_ + 4, y_, NUMERICGITD, NumPan.M1000,Display_col);
  PICOKIT.drawSelfMaskedColor(x_ + 8, y_, NUMERICGITD, NumPan.M100,Display_col);
  PICOKIT.drawSelfMaskedColor(x_ + 12, y_, NUMERICGITD, NumPan.M10,Display_col);
  PICOKIT.drawSelfMaskedColor(x_ + 16, y_, NUMERICGITD, NumPan.M1,Display_col);
}

void NumCalculate(uint16_t Nub_) {
  NumPan.M10000 = (Nub_ / 10000);
  NumPan.M1000 = (((Nub_) - (NumPan.M10000 * 10000)) / 1000);
  NumPan.M100 = (((Nub_) - (NumPan.M1000 * 1000) - (NumPan.M10000 * 10000)) / 100);
  NumPan.M10 = (((Nub_) - (NumPan.M100 * 100) - (NumPan.M1000 * 1000) - (NumPan.M10000 * 10000)) / 10);
  NumPan.M1 = ((Nub_) - (NumPan.M10 * 10) - (NumPan.M100 * 100) - (NumPan.M1000 * 1000) - (NumPan.M10000 * 10000));
}

void DrawAllSprites(void) {
//	Draw_Gilbert_ITD();
  DrawSprites(GITDLvL.SPK[GITDLvL.Room]);
  DrawAcidDrop();
}

void DrawRoomInProgress(void) {
  PICOKIT.clearPicoBuffer();
  DrawVector(x_offset, y_offset, GITDLvL.Vec[GITDLvL.Room]);
  update_Ballon();
}

void TransitReDraw(void) {
  PICOKIT.clearPicoBuffer();
  DrawRoomInProgress();
  DrawAllSprites();
  drawRopeVector(x_offset, y_offset, GITDLvL.RopeVev[GITDLvL.Room]);
  DisplayNum();
}

void RoomTransition(void) {

	//copy Old level in buffer

  if (RoomChange) {
	    uint8_t Tmp;
  uint16_t OldPic[111 * 64] = { 0 };
  uint16_t NewPic[111 * 64] = { 0 };
    Tmp = GITDLvL.Room;
    GITDLvL.Room = GITDLvL.MemRoom;
    drawRopeVector(x_offset, y_offset, GITDLvL.RopeVev[GITDLvL.Room]);  
    GITDLvL.Room = Tmp;
    DisplayNum();
   // uint8_t *buffer = PICOKIT.getBuffer();
    for (uint8_t y = 0; y < 64; y++) {
      for (int8_t x = 0; x < 111; x++) {
          uint16_t B_ = (x) + (y * 111);
          uint16_t Bn_ = (((x*2)+PICO_OffsetX) + (((y*2)+PICO_OffsetY) * WIDTH));
       OldPic[B_] = FrameBuf[Bn_];
      }
    }
    //copy Old level in buffer end
    
    //copy new level in buffer
      PICOKIT.clearPicoBuffer();
      TransitReDraw();
      for (uint8_t y = 0; y < 64; y++) {
      for (int8_t x = 0; x < 111; x++) {
          uint16_t B_ = (x) + (y * 111);
          uint16_t Bn_ = (((x*2)+PICO_OffsetX) + (((y*2)+PICO_OffsetY) * WIDTH));
       NewPic[B_] = FrameBuf[Bn_];
      }
    }
       //copy new level in buffer end
       
       //Redraw Old pic on PicoBuffer
             PICOKIT.clearPicoBuffer();
      for (uint8_t y = 0; y < 64; y++) {
      for (int8_t x = 0; x < 111; x++) {
          uint16_t B_ = (x) + (y * 111);
          uint16_t Bn_ = (((x*2)+PICO_OffsetX) + (((y*2)+PICO_OffsetY) * WIDTH));
        FrameBuf[Bn_]=OldPic[B_];
            #ifdef FULL_PIXEL
            FrameBuf[Bn_+1] =OldPic[B_];
            FrameBuf[Bn_+WIDTH] =OldPic[B_];
            FrameBuf[Bn_+WIDTH+1] =OldPic[B_];
            #endif
       
      }
    }
     DisplayNum();
       //Redraw Old pic on PicoBuffer end
    
    //start transition
    if ((GITDLvL.Room == 0) && (GITDLvL.MemRoom == 9)) ResetPickupSprite();
    
    for (uint8_t t = 15; t != 0; t--) {
      
      for (uint8_t y = 15-t; y-t < 64-t; y=y+16) {
          Sound(220,1);
        for (int8_t x = 0; x < 111; x++) {

      // gpio_put(15,SnD_);
         //_delay_us(1);
     //  gpio_put(15, false);
         // _delay_us(1);
          
          uint16_t B_ = (x) + (y * 111);
          uint16_t Bn_ = (((x*2)+PICO_OffsetX) + (((y*2)+PICO_OffsetY) * WIDTH));
      
       if (t!=0) FrameBuf[Bn_+ (WIDTH*2)] =  color(31,63,31);
       FrameBuf[Bn_] =  NewPic[B_];
       
            #ifdef FULL_PIXEL
            FrameBuf[Bn_+1] =NewPic[B_];
            FrameBuf[Bn_+WIDTH] =NewPic[B_];
            FrameBuf[Bn_+WIDTH+1] =NewPic[B_];
            #endif
          
        }
      }
      _delay_ms(30);
      PICOKIT.DisplayColor();
    }
    RoomChange = 0;
  }
}

void CheckIfBallExist(const int8_t *SPK_) {
  int8_t Type_ = 0;
  int8_t X_pos = 0;
  int8_t Y_pos = 0;
  uint8_t PS_ = 0;
  while (1) {
    Type_ = SPK_[PS_];
    X_pos = SPK_[PS_ + 1] + x_offset;
    Y_pos = SPK_[PS_ + 2] + y_offset;

    if (Type_ == BALLONSPK) {
      Ballon.Set_Ballon(X_pos, Y_pos);
      return;
    }
    PS_ += 4;
    if (Type_ == EXIT) { break; }
  }
  Ballon.Init_Ballon();
}

void Sprite_Colid(void) {
  if (!DDead == 0) { return; }
#define px GITD.Get_X()
#define py GITD.Get_Y()
  for (uint8_t t = 0; t < 6; t++) {
    if ((AcidDrop[t].Get_A()) && (AcidDrop[t].Get_Dl() == 0)) {
      if (colid_univ(px + 2 + GITD.Get_Stretch(), 2 + GITD.Get_Stretch(), py, 5, AcidDrop[t].Get_X(), 0, AcidDrop[t].Get_Y(), 0)) {
        DDead = 1;
        break;
      }
    }
  }
  if (Ballon.Get_A()) {
    if (colid_univ(px + 1 + GITD.Get_Stretch(), 5 + GITD.Get_Stretch(), py, 5, Ballon.Get_X() + 1, 2, Ballon.Get_Y() + 5, 2)) {
      DDead = 1;
    }
  }
}

uint8_t colid_univ(int8_t x1_, int8_t w1_, int8_t y1_, int8_t h1_, int8_t x2_, int8_t w2_, int8_t y2_, int8_t h2_) {
  if ((x1_ > (x2_ + w2_)) || ((x1_ + w1_) < x2_) || (y1_ > (y2_ + h2_/*w2*/)) || ((y1_ + h1_) < y2_)) return 0;
  return 1;
}

void update_Ballon(void) {
  if (TM3.Trigger()) {
    Ballon.Update_Ballon();
  }
}

void Draw_Ballon(void) {
  if (Ballon.Get_A()) {
	 PICOKIT.drawSelfMaskedColor(Ballon.Get_X(), Ballon.Get_Y(), Ballon_SPK_GITD, Ballon.Get_Anim(),color(0,63,31));
  }
}

void ReleaseNewDrop(const int8_t *DropeSelect){
  int8_t NB_ = DropeSelect[0];
  
  for (uint8_t t_ = 0; t_ < 6; t_++) {
    if (!AcidDrop[t_].Get_A()) {
      uint8_t Rnd_ = ((rand() % NB_) * 2) + 1;
      AcidDrop[t_].SetAcidDrop(DropeSelect[Rnd_], (DropeSelect[Rnd_ + 1]));
      return;
    }
  }
}

void CalculateAcidDrop(void) {
  if (Flip == 0) {
    for (uint8_t t_ = 0; t_ < 6; t_++) {
      if (AcidDrop[t_].Get_A()) {
        ColAcidDrop(AcidDrop[t_].Get_X(), AcidDrop[t_].Get_Y(), t_);
        AcidDrop[t_].UpdateAcideDrop();
      }
    }
  }
}

void DrawAcidDrop(void) {
  if (Flip == 0) {
    for (uint8_t t_ = 0; t_ < 6; t_++) {
      if (AcidDrop[t_].Get_A()) {
        PICOKIT.drawPixelColor(AcidDrop[t_].Get_X(), AcidDrop[t_].Get_Y(),color(0,63,0));
      }
    }
    Flip = 1;
  } else {
    Flip = 0;
    if (TM2.Trigger()) ReleaseNewDrop(GITDLvL.AcidDrop[GITDLvL.Room]);
  }
}

uint8_t ColAcidDrop(int8_t x_, int8_t y_, uint8_t t_) {
  if (PICOKIT.getPixelColor(x_, y_)) {
    AcidDrop[t_].Put_A(0);
    return 0;
  } else {
    return 1;
  }
}

bool checkMainColision(int8_t T_, int8_t bx, int8_t by) {

  int8_t ADD_, SOUS_;

  if (T_ == DOOR) {
    ADD_ = 3;
    SOUS_ = 7;
  } else {
    ADD_ = 0;
    SOUS_ = 0;
  }

  if (!colid_univ(GITD.Get_X() + 1 + GITD.Get_Stretch(), 4 + GITD.Get_Stretch(), GITD.Get_Y() + 1, 4, bx + 1 - ADD_, 3 + SOUS_, by + 3, 3)) {
    return 0;
  }
  return 1;
}

uint8_t PickupSprites(int8_t T_, int8_t X_, int8_t Y_, int8_t I_) {
  if (checkMainColision(T_, X_, Y_)) {
    switch (T_) {
      case (GOLD):
      case (GOLD2):
        if (VisibleSprite[I_] == 1) {
          NumPan.Scores += 282;
          VisibleSprite[I_] = 0;
          SoundSystem(0);
        }
        return 0;
        break;
      case (KEY):
        if (VisibleKeyAndDoors[I_] == 0) {
          NumPan.Scores += 496;
          VisibleKeyAndDoors[I_] = 1;
          if (I_ == 9) { VisibleKeyAndDoors[21] = 1; }
          SoundSystem(0);
        }
        return 0;
        break;
      case (DOOR):
        GotoNexRoom(I_);
        return 1;
        break;
      default: return 0;
    }
  }
  return 0;
}

void GotoNexRoom(int8_t DoorNumber_) {

  int8_t Type_ = 0;
  int8_t Room_ = 0;
  int8_t Pos_ = 0;
  uint8_t PS_ = 0;
  while (1) {
    Type_ = InAndOut_GITD[PS_];
    Room_ = InAndOut_GITD[PS_ + 1];
    Pos_ = InAndOut_GITD[PS_ + 2];
    if (Type_ == DoorNumber_) {
      if (VisibleKeyAndDoors[((Type_ > 99) ? 0 : Type_)] == 0) return;
      GITDLvL.MemRoom = GITDLvL.Room;
      GITDLvL.Room = Room_;
      TMout.Set_Time();
      GITD.SetSpritePos(Position_GITD[(Pos_ * 2)], Position_GITD[(Pos_ * 2) + 1]);
      InitnextRoom();
      TMout.deactivate();
      return;
    }
    PS_ += 3;
  }
}

void DrawSprites(const int8_t *SPK_) {
  int8_t Type_ = 0;
  int8_t X_pos = 0;
  int8_t Y_pos = 0;
  int8_t Item_Pos = 0;
  uint8_t PS_ = 0;
  while (1) {
    Type_ = SPK_[PS_];
    X_pos = SPK_[PS_ + 1] + x_offset;
    Y_pos = SPK_[PS_ + 2] + y_offset;
    Item_Pos = SPK_[PS_ + 3];
    if (PickupSprites(Type_, X_pos, Y_pos, Item_Pos)) {
      if ((Type_ == DOOR) && (VisibleKeyAndDoors[((Item_Pos > 99) ? 0 : Item_Pos)] == 1)) { RoomChange = 1; }
    }
    if (Item_Pos > 99) Item_Pos = 0;
    switch (Type_) {
      case (GOLD):
        if (VisibleSprite[Item_Pos]) PICOKIT.drawSelfMaskedColor(X_pos, Y_pos, Sprites_World_GITD, 0,color(31,40,19));
        break;
      case (GOLD2):
        if (VisibleSprite[Item_Pos]) PICOKIT.drawSelfMaskedColor(X_pos, Y_pos, Sprites_World_GITD, 1,color(31,53,14));
        break;
      case (KEY):
        if (!VisibleKeyAndDoors[Item_Pos]) PICOKIT.drawSelfMaskedColor(X_pos, Y_pos, Sprites_World_GITD, 2,color(31,56,0));
        break;
      case (DOOR):
        if (VisibleKeyAndDoors[Item_Pos]) PICOKIT.drawSelfMaskedColor(X_pos, Y_pos, Sprites_World_GITD, 3,color(31,25,0));
        break;
      case (EXIT): return; break;
      default: break;
    }
    PS_ += 4;
  }
}

void MovingMode(void) {
  if (TINYJOYPAD_LEFT) {
    JumpCycle = 0;
  } else if (TINYJOYPAD_RIGHT) {
    JumpCycle = 1;
  } else {
    JumpCycle = 2;
  }
  if (BUTTON_DOWN) { GITD.JumpTrig(); }
}

void RopeProcess(void) {
  uint8_t Tmp_;
  JumpCycleMem = (GITD.Get_1Click() || GITD.GET_FLS()) ? JumpCycle : JumpCycleMem;
  if (GITD.Get_GS() > 1.4) {
    JumpCycle = JumpCycleMem = 2;
  }
  Tmp_ = RopeDetectionGITD(x_offset, y_offset, GITDLvL.RopeVev[GITDLvL.Room]);
  if (DDead) Tmp_ = 0;
  if (Tmp_) {
    JumpCycle = JumpCycleMem = 2;
    RopeMode = Tmp_;
    if (!GITD.Get_AddStretch()) GITD.Put_Direction(6);
    GITD.GravityReset();
  }
  if (RopeMode) {
    RopeModeGITD();
    RopeMode = 0;
  } else {
    WalkingModeGITD();
    if (TM1.Trigger()) {
      GITD.GravityUpdate();
    }
    GITD.Gravitycalcule();
  }
}

void RopeModeGITD(void) {
  if (TINYJOYPAD_UP) {
    if (!GITD.Get_AddStretch()) UPDOWN = (RopeMode == 2) ? 2 : 0;
  } else if (TINYJOYPAD_DOWN) {
    if (!GITD.Get_AddStretch()) UPDOWN = (RopeMode == 2) ? 2 : 1;
  } else if (TINYJOYPAD_RIGHT) {
    UPDOWN = 3;
  } else if (TINYJOYPAD_LEFT) {
    UPDOWN = 4;
  } else {
    UPDOWN = 2;
  }
  switch (UPDOWN) {
    case 0: GITD.GoClimb(); break;
    case 1: GITD.GoFall(); break;
    case 2: GITD.NeutralStretch(); break;
    case 3: GITD.rightstretch(); break;
    case 4: GITD.leftstretch(); break;
    default: break;
  }
}

void WalkingModeGITD(void) {
  switch (JumpCycleMem) {
    case 0: GITD.Go2Left(); break;
    case 1: GITD.Go2Right(); break;
    case 2: GITD.Neutral(); break;
    default: break;
  }
}

uint8_t RopeDetectionGITD(int8_t x_, int8_t y_, const int8_t *Rope_) {
  if (GITD.Get_GS() < 0) return 0;
  uint8_t SCAN_ = 0;
  while (1) {
    int8_t HV_ = Rope_[SCAN_];
    int8_t X_ = Rope_[SCAN_ + 1] + x_;
    int8_t Y_ = Rope_[SCAN_ + 2] + y_;
    int8_t L_ = Rope_[SCAN_ + 3];
    switch (HV_) {
      case EXIT: return 0; break;
      case VERTICAL:
        if (STDVcollisionGITD(X_, Y_, L_, GITD.Get_X() + 3, GITD.Get_Y() + 3)) { return 1; };
        break;
      case HORIZONTAL:
        if (STDHcollisionGITD(X_, Y_, L_, GITD.Get_X() + 3, GITD.Get_Y() + 3)) { return 2; };
        break;
      default: break;
    }
    SCAN_ += 4;
  }
}

int STDHcollisionGITD(int8_t x1, int8_t y1, int8_t L_, int8_t x2, int8_t y2) {
  if (y1 != y2 || x1 + L_ < x2 || x2 < x1) {
    return 0;
  }
  return 1;
}

int STDVcollisionGITD(int8_t x1, int8_t y1, int8_t L_, int8_t x2, int8_t y2) {
  if (x1 != x2 || y1 + L_ < y2 || y2 + 3 < y1) {
    return 0;
  }
  return 1;
}

void FrmCall(void) {
  Frm = GITD.GetMainFrame();
}

void Draw_Gilbert_ITD(void) {

  PICOKIT.drawSelfMaskedColor(GITD.Get_X() + GITD.Get_AddStretch(), GITD.Get_Y(), gilbert_GITD, Frm = GITD.GetMainFrame(),Gilbert_Col);
}

void DrawVector(int8_t x_, int8_t y_, const int8_t *VectorSelect) {
  Lastpos.x = x_;
  Lastpos.y = y_;
  int8_t Mx = x_;
  int8_t My = y_;
  int8_t Command_ = 0;
  int8_t X_pos = 0;
  int8_t Y_pos = 0;
  uint8_t PS_ = 0;
  while (1) {
    Command_ = VectorSelect[PS_];
    X_pos = VectorSelect[PS_ + 1] + Mx;
    Y_pos = VectorSelect[PS_ + 2] + My;
    switch (Command_) {
      case (-1): return; break;
      case (0): SetVectorStartPos(X_pos, Y_pos); break;
      case (1): DrawStalagtite(Lastpos.x, X_pos, Y_pos); break;
      case (2): DrawWalls(Lastpos.y, Y_pos, X_pos); break;
      case (3):
     // drawLine(Lastpos.x, Lastpos.y, X_pos, Y_pos);
        PICOKIT.drawLineColor(Lastpos.x, Lastpos.y, X_pos, Y_pos,color(0,0,31));
        Lastpos.x = X_pos;
        Lastpos.y = Y_pos;
        break;
      case (4): DrawVector(X_pos, Y_pos, Small_Ilot); break;
      case (5): DrawVector(X_pos, Y_pos, Big_Ilot); break;
      default: break;
    }
    PS_ += 3;
  }
}

void SetVectorStartPos(int8_t x_, int8_t y_) {
  Lastpos.x = x_;
  Lastpos.y = y_;
}

void DrawWalls(int8_t y0, int8_t y1, int8_t x_) {
  int8_t y0_;
  int8_t y1_;
  if (y0 < y1) {
    y0_ = y0;
    y1_ = y1;
  } else {
    y0_ = y1;
    y1_ = y0;
  }
  uint8_t step = 3;
  int8_t Ys = y0_;
  uint8_t alt = x_;
  int8_t fmx = x_, fmy = y0_;
  int8_t smx = x_, smy = y0_ + 2;

  while (1) {
	//DrawLine(PICO_OffsetX+(fmx*2), PICO_OffsetY+(fmy*2), PICO_OffsetX+(smx*2), PICO_OffsetY+(smy*2),color(31,63,0));
    PICOKIT.drawLineColor(fmx, fmy, smx, smy,color(0,0,31));
  //  drawLine(fmx, fmy, smx, smy);
    Ys += step;
    if (Ys <= y1_) {
      fmx = smx;
      fmy = smy;
      smy = Ys;
      alt = (alt == x_) ? x_ - 1 : x_;
      smx = alt;
    } else {
      Lastpos.x = smx;
      Lastpos.y = y1;
      break;
    }
  }
}

void DrawStalagtite(int8_t x0, int8_t x1, int8_t y_) {
  uint8_t step = 3;
  y_ -= 3;
  uint8_t alt = y_ + 3;
  uint8_t fmx = x0, fmy = y_ + 3;
  uint8_t smx = x0 + 3, smy = y_;
  while (1) {
//    drawLine(fmx, fmy, smx, smy); 
    PICOKIT.drawLineColor(fmx, fmy, smx, smy,color(0,0,31));
    x0 += step;
    if (x0 <= x1) {
      fmx = smx;
      fmy = smy;
      smx = x0;
      alt = (alt == y_) ? y_ + 3 : y_;
      smy = alt;
    } else {
      Lastpos.x = x0 - step;
      Lastpos.y = smy;
      break;
    }
  }
}

void drawRopeVector(int8_t x_, int8_t y_, const int8_t *VectorSelect) {
  uint8_t Addrs_ = 0;
  while (1) {
    int8_t H_V = VectorSelect[Addrs_++];
    int8_t x = (VectorSelect[Addrs_++]) + x_;
    int8_t y = (VectorSelect[Addrs_++]) + y_;
    uint8_t length = VectorSelect[Addrs_++];
    switch (H_V) {
      case EXIT: return; break;
      case HORIZONTAL: drawRope(x, y, x + length, y); break;
      case VERTICAL: drawRope(x, y, x, y + length); break;
      default: return; break;
    }
  }
}

void drawRope(int8_t x0, int8_t y0, int8_t x1, int8_t y1) {
  int8_t dx = abs(x1 - x0);
  int8_t dy = abs(y1 - y0);
  int8_t sx = (x0 < x1) ? 1 : -1;
  int8_t sy = (y0 < y1) ? 1 : -1;
  int16_t err = dx - dy;
  bool drawPixelFlag = true;
  while (true) {
    if (drawPixelFlag) {
      PICOKIT.drawPixelColor(x0, y0, color(31,63,31));
    }
    drawPixelFlag = !drawPixelFlag;
    if (x0 == x1 && y0 == y1) break;
    int err2 = err * 2;
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

void drawLine(int8_t x0, int8_t y0, int8_t x1, int8_t y1) {
  int8_t dx = abs(x1 - x0);
  int8_t dy = abs(y1 - y0);
  int8_t sx = (x0 < x1) ? 1 : -1;
  int8_t sy = (y0 < y1) ? 1 : -1;
  int16_t err = dx - dy;
  while (true) {
    PICOKIT.drawPixel(x0, y0, WHITE);

    if (x0 == x1 && y0 == y1) break;
    int err2 = err * 2;
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

uint8_t ColWall(int8_t CX_, int8_t CY_) {
  int8_t x_ = CX_;
  int8_t y_ = CY_;
  for (y_ = CY_ + 5; y_ < CY_ + 7; y_++) {
    for (x_ = CX_ + 2; x_ < CX_ + 5; x_++) {
      if (PICOKIT.getPixelColor(x_, y_)) {
        JumpCycle = 2;
        return 1;
      }
    }
  }

  if (PICOKIT.getPixelColor(CX_ + 3, CY_ + 4)) {
    switch (JumpCycleMem) {
      case 0:
        JumpCycle = 1;
        JumpCycleMem = 1;
        break;
      case 1:
        JumpCycle = 0;
        JumpCycleMem = 0;
        break;
      default: break;
    }
    return 2;
  }
  return 0;
}

