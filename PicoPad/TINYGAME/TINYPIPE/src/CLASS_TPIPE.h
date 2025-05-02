//       >>>>> T-I-N-Y P-I-P-E for PicoPad RP2040 & RP2350+ GPLv3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-Pipe is free software: you can redistribute it and/or modify
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


// Tiny-Pipe source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tinypipe.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the tinypipe.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#ifndef _CLASS_TPIPE_H
#define _CLASS_TPIPE_H
//DEFINE
#define NO_SPR 4
//Structure

typedef struct GamePlay_TPIPE{
uint8_t LIVES;
uint8_t FIRSTTIME;
uint8_t STATE;
uint8_t Level;
uint8_t TIMER_RENEW;
uint8_t LevelXspeed;
uint8_t TOTAL_TURTLE_LEVEL;
uint8_t SPEED_POP_TURTLE;
int8_t POWER;
uint8_t EARTQUAKE;
uint8_t EARTQUAKE_INVERT;
uint8_t NOMOVE_TIME;
uint8_t DIGIT1;
uint8_t DIGIT2;
}GamePlay_TPIPE;

//PROTOTYPE
uint8_t COLLISION_1VS1(int8_t x1,int8_t x2,int8_t y1,int8_t y2,int8_t sx1,int8_t sx2,int8_t sy1,int8_t sy2);
uint8_t FLOORS_VS_SPRITE(uint8_t Killed_,int8_t x1,int8_t x2,int8_t y1,int8_t y2);

class PASIVE_SPRITE_TPIPE{
private:
int8_t x;
int8_t y;
uint8_t frame;
uint8_t killed;
uint8_t active;
uint8_t width;
uint8_t height;
public:

void INIT(uint8_t Active_,int8_t X_,int8_t Y_);

uint8_t GET_K(void);
void PUT_K(uint8_t K_);
int8_t GET_X(void);
int8_t GET_Y(void);
uint8_t GET_W(void);
uint8_t GET_H(void);
uint8_t GET_F(void);
void PUT_A(uint8_t F_);
uint8_t GET_A (void);
void PUT_F(uint8_t F_);
void PUT_W(uint8_t Width_);
void PUT_H(uint8_t height_);
void PUT_X(int8_t X_);
void PUT_Y(int8_t Y_);
void ANIM(void);
};


class SPRITE_TPIPE:public PASIVE_SPRITE_TPIPE {
private:
int8_t xspeed;
int8_t xadd;
int8_t yspeed;
int8_t yadd;
uint8_t Anim_Direction;
uint8_t Cancel_Jump;
uint8_t NoMoveTimer;
public:
uint8_t GET_NOMOVETIMER(void);
void PUT_NOMOVETIMER(uint8_t NoMoveTimer_);
int8_t GET_XSPEED(void);
void PUT_XSPEED(int8_t XSPEED_);
int8_t GET_YSPEED(void);
void PUT_YSPEED(int8_t YSPEED_);
uint8_t GET_AD(void);
void PUT_AD(uint8_t F_);

void DECEL(void);

uint8_t GET_CJ(void);
void PUT_CJ(uint8_t CJ_);
void Reset_X_Speed(void);

void RunR(void);
  
void RunL(void);

void GRAVITY_REFRESH(void);

void GRAVITY(uint8_t Main_);

void RESET_GRAVITY(void);

void JUMP(void);

void RIGHTMOVE(void);
  
void LEFTMOVE(void);

void DIRECT_R_MOVE(void);
  
void DIRECT_L_MOVE(void);

void AUTO_MOVE(void);
};

uint8_t COLLISION_SIMPLIFIED(SPRITE_TPIPE *Main_,SPRITE_TPIPE *Other_);

uint8_t COLLISION_1VS1(int8_t x1,int8_t x2,int8_t y1,int8_t y2,int8_t sx1,int8_t sx2,int8_t sy1,int8_t sy2);

uint8_t FLOORS_VS_SPRITE(uint8_t Killed_,int8_t x1,int8_t x2,int8_t y1,int8_t y2);

void RND_MIXER_TPIPE(void);

uint8_t PSEUDO_RND_TPIPE(void);

#endif
