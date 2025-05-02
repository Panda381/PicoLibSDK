//  >>>>> T-I-N-Y M-I-S-S-I-L-E for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-Missile is free software: you can redistribute it and/or modify
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


// Tiny-Missile  source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tmissile.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the tmissile.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#ifndef _CLASS_TMISSILE_H
#define _CLASS_TMISSILE_H

//DEFINE CLASS CROSS
#define CROSS_ACCEL_SPEEDX_TMISSILE 6
#define CROSS_ACCEL_SPEEDY_TMISSILE 4 
#define CROSS_ACCEL_STEPX_TMISSILE 3
#define CROSS_ACCEL_STEPY_TMISSILE 2
#define SPEED_SHOOT_TMISSILE 3

//CLASS ARMY 
class ARMY_TMISSILE{
public:
void RESET_WEAPON(uint8_t LEVEL_);
uint8_t USE_WEAPON(void);
void ATTACK_WEAPON(void);
uint8_t GET_SPARE(void);
uint8_t GET_ROCKET(void);
private:
uint8_t SPARE;
uint8_t ROCKET; 
};

class STATIC_SPRITE_TMISSILE {
  public:
  void INIT(int8_t X_,int8_t Y_,uint8_t ACTIVE_);
  void PUT_X(int8_t X_);
  void PUT_Y(int8_t Y_);
  void PUT_ACTIVE(uint8_t ACTIVE_);
  int8_t GET_X(void);
  int8_t GET_Y(void);
  uint8_t GET_ACTIVE(void);
  private:
  int8_t X;
  int8_t Y;
  uint8_t ACTIVE;
};

class STATIC_SPRITE_ANIM_TMISSILE:public STATIC_SPRITE_TMISSILE {
  public:
void INIT_DOME(int8_t X_,int8_t Y_,uint8_t ACTIVE_);
void PUT_FRAME(uint8_t FRAME_);
uint8_t PROGRESS_ANIM(void);
uint8_t GET_FRAME(void);
  private:
uint8_t FRAME;
};

class CROSS:public STATIC_SPRITE_TMISSILE{
public:
void INIT(void);
void RIGHT(void);
void LEFT(void);
void DOWN(void);
void UP(void);

void R(void);
void D(void);
void U(void);
void L(void);

void DECELX(void);
void DECELY(void);

void LIMITR(void);
void LIMITL(void);
void LIMITU(void);
void LIMITD(void);
private:
};

class LINE_TMISSILE {
public:
void RESET(void);
void INIT(int8_t START_X_,int8_t END_X_,uint8_t ACTIVE_);
void DOME_COLLISION(STATIC_SPRITE_ANIM_TMISSILE *DOME_,ARMY_TMISSILE *ARM_);
uint8_t PROGRESS(STATIC_SPRITE_ANIM_TMISSILE *DOME_,ARMY_TMISSILE *ARM_);
int8_t GET_START_X(void);
int8_t GET_POS_X(void);
int8_t GET_POS_Y(void);
int8_t GET_END_X(void);
uint8_t GET_yPass(void);
uint8_t GET_yDeca(void);
uint8_t GET_ACTIVE(void);  
uint8_t GET_TRACK(void);  
void PUT_START_X(int8_t START_X_);
void PUT_POS_X(int8_t POS_X_);
void PUT_POS_Y(int8_t POS_Y_);
void PUT_END_X(int8_t END_X_); 
void PUT_ACTIVE(uint8_t ACTIVE_);  
void PUT_TRACK(uint8_t TRACK_);  
  private:
int8_t START_X;
int8_t POS_X;
int8_t POS_Y;
uint8_t yPass;
uint8_t yDeca;
uint8_t Track;
int8_t END_X;
uint8_t ACTIVE;
};

class DEFENCE:public STATIC_SPRITE_TMISSILE{
public:
void UPDATE_DEFENCE(void);
void INIT(void);
void NEW(int8_t xEND_,int8_t yEND_);
private: 
float XF=63;
float YF=51;
float XCF;
float YCF;
uint8_t Count;
int8_t SWAP_XY;
};

class CLK {
public:
void INIT(uint8_t Start_,uint8_t End_,uint8_t Loop_);
uint8_t PROGRESS(void);
void RESET(void);
private:
uint8_t Start;
uint8_t End;
uint8_t Pos;
uint8_t Trig;
uint8_t Loop;
};

#endif
