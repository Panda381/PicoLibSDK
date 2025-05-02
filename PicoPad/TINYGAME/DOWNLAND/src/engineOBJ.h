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


#ifndef _ENGINEOBJ_H
#define _ENGINEOBJ_H

//#include "../include.h"

#define x_offset 7
#define y_offset 0


extern uint8_t JumpCycle;
extern uint8_t JumpCycleMem;
extern uint8_t RopeMode;
extern uint8_t UPDOWN;
extern uint8_t DDead;
extern uint8_t RoomChange;
extern uint8_t Frm;
extern uint8_t Flip;


extern uint8_t VisibleKeyAndDoors[22];  //Total of 21 keys -  First Byte= 1!
extern uint8_t VisibleSprite[30];

void SoundSystem(uint8_t Snd_);

void SubLive(void);
uint8_t colid_univ(int8_t x1_, int8_t w1_, int8_t y1_, int8_t h1_, int8_t x2_, int8_t w2_, int8_t y2_, int8_t h2_);


typedef struct NUMGITD {
  uint8_t M10000;
  uint8_t M1000;
  uint8_t M100;
  uint8_t M10;
  uint8_t M1;
  uint16_t Scores;
  uint16_t HIScores;
  uint8_t Lives;
} NUMGITD;

class TIMERGITD {
private:
  uint8_t Activ;
  uint8_t startTime;
  uint8_t interval;
public:
  void Init(uint8_t Interval_);

  uint8_t Get_A(void) ;

  void Activate(void) ;

  void DeActivate(void) ;

  uint8_t Trigger(void) ;
};

class StaticSprite_GITD {
private:
  uint8_t Actif;
  int8_t X;
  int8_t Y;
public:
  void StaticInit(void) ;
  uint8_t Get_A(void) ;
  int8_t Get_X(void) ;
  int8_t Get_Y(void) ;
  void Put_A(uint8_t A_);
  void Put_X(int8_t X_) ;
  void Put_Y(int8_t Y_) ;
};

class AcidDrop_GITD : public StaticSprite_GITD {
private:
  uint8_t Delays;
public:
  uint8_t Get_Dl(void) ;
  void InitAcidDrop(void);
  void SetAcidDrop(int8_t x_, int8_t y_) ;
  void UpdateAcideDrop(void) ;
};

class Sprite_GITD : public StaticSprite_GITD {
private:
  uint8_t Type;
  uint8_t Direction;

  uint8_t AnimFrame;
  uint8_t DeadFrameStep;
  int8_t StretchADD;
  uint8_t TrigSpeedRestriction;
  uint8_t SpeedRestriction;

  const float GravityQ = .2;
  float GravitySpeed;
  uint8_t FloorStat;
  uint8_t OneClick;
  uint8_t OneClick2;
  int8_t Stretch;

  void InitDeadAnim(void);
  void ForcedJumpTrig(void);
  void SpriteStretchSelect(void);
  uint8_t ColWallP(void);
  uint8_t MainAnimSelect(void);
public:
  void Init(void);
  int8_t Get_Stretch(void);
  uint8_t DeadAnim(void);
  uint8_t NoiseMain(void);
  uint8_t GetMainFrame(void);
  void Put_1Click2(void) ;
  void Put_Direction(uint8_t D_) ;
  uint8_t GET_FLS(void) ;
  int8_t Get_AddStretch(void) ;
  void JumpTrig(void);
  int8_t Get_GS(void);
  uint8_t Get_1Click(void);
  void GravityReset(void) ;
  void GravityUpdate(void) ;
  void FloorDirectCheck(void);
  void Gravitycalcule(void) ;
  void SetSpritePos(int8_t x_, int8_t y_);
  int8_t Get_Direction(void);
  void Neutral(void) ;
  int8_t Get_ANIMFRM(void) ;
  void Go2Left(void);
  void Go2Right(void);
  void GoClimb(void);
  void GoFall(void) ;
  void RopeJump(void);
  void leftstretch(void) ;
  void rightstretch(void) ;
  void NeutralStretch(void);
};

class Ballon_GITD : public StaticSprite_GITD {
private:
  float Step;
  uint8_t Anim;
  int8_t XMEM;
  int8_t YMEM;
  uint8_t collXBallon(void);
  uint8_t collYBallon(void) ;
  void UpdateGravity(void) ;
  void leftmove(void);

public:
  void Init_Ballon(void);
  void Set_Ballon(int8_t x_, int8_t y_) ;
  void Rset_Ballon(void) ;
  uint8_t Get_Anim(void);
  void Update_Ballon(void) ;
};

class TIMEOUT {

private:
  uint16_t Time;
  uint8_t Flip;
  uint8_t Actif;
  int8_t x;
  int8_t y;
  uint8_t UD;
  uint8_t RL;
  uint8_t FlipFrm(void);

public:
  uint16_t Get_Time(void);
  void Set_Time(void);
  void ResetTime(void) ;
  void deactivate(void);
  void Activate(void);
  void TimeOutFunction(int8_t x_, int8_t y_);
  void Go2Right(void) ;
  void Go2Left(void) ;
  void Go2UP(void) ;
  void Go2Down(void) ;
};

#endif
