//       >>>>> T-I-N-Y S-Q-U-E-S-T for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-SQuest is free software: you can redistribute it and/or modify
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


// The Tiny-SQuest source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tsquest.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the tsquest.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#ifndef _CLASS_TSQUEST_H
#define _CLASS_TSQUEST_H

//DEFINE
#define MAX_LEFT 2
#define MAX_RIGHT 125
#define MAX_DOWN 51
#define MAX_UP 6
//Structure

typedef struct GamePlay_TSQUEST {
  uint8_t RETURN_START;
  uint8_t Level;
  uint8_t OX;
  uint16_t Scores;
  uint8_t Diver;
  uint8_t Speed;
  uint8_t Main_SPK_Bank;
  uint8_t Amount_Sprite;
  uint8_t Lib_Sprite_Timer;
  uint8_t LIBTRIG;
  uint8_t Speed_Balistic;
  uint8_t Frame;
  uint8_t Frame_Main;
  uint8_t Main_Live;
  uint8_t Frame_Cycle_STEP2;
  int8_t SUBSOLO_X;
  uint8_t DIVER_OSD_STATE;
  uint8_t OX_OSD_STATE;
  uint8_t Refresh_OSD;
  uint8_t FlipFlop;
  uint8_t Flipflop_Counter;
  uint8_t FlipFlop2_OX;
  uint8_t FlipFlop2_OX_Counter_;
  uint8_t Limit_move_Other_Sprite;
  uint8_t Limit_Refresh;
  uint8_t latch_0_FIRSTFULLING;
  uint8_t latch_1_REFILL;
  uint8_t latch_2_AFTERDEAD;
  uint8_t NO_JOY;
  uint8_t NEXTLEVEL;
  uint8_t Sin_Anim;
  uint8_t sa, eb;
  uint8_t RND3Pos;
  uint8_t RND2Pos;
} GamePlay_TSQUEST;

//PROTOTYPE

class PASIVE_SPRITE_TSQUEST {
private:
  int8_t x;
  int8_t y;
  uint8_t Direction;
  uint8_t killed;
  uint8_t active;
  uint8_t width;
  uint8_t height;

public:
  void INIT(uint8_t Active_, int8_t X_, int8_t Y_);

  uint8_t GET_D(void);

  void PUT_D(uint8_t D_);

  uint8_t GET_K(void);

  void PUT_K(uint8_t K_);

  int8_t GET_X(void);

  int8_t GET_Y(void);

  uint8_t GET_W(void);

  uint8_t GET_H(void) ;

  void PUT_A(uint8_t F_);

  uint8_t GET_A(void);

  void PUT_W(uint8_t Width_);

  void PUT_H(uint8_t height_);

  void PUT_X(int8_t X_);

  void PUT_Y(int8_t Y_);

};

class ACTIVE_SPRITE_TSQUEST : public PASIVE_SPRITE_TSQUEST {
private:
  uint8_t Speed;
  uint8_t SpeedAdd;
  uint8_t Ballistic;
  int8_t BallisticPositionX;
  int8_t BallisticPositionY;
  int8_t BallisticSpeed;
public:

  void ACTIVE_SPRITE_INIT(uint8_t Active_, int8_t X_, int8_t Y_, int8_t Speed_) ;

  void PUT_SPEED(uint8_t Speed_);

  uint8_t GET_SPEED(void);

  uint8_t GET_Ballistic(void);
  void DESTROY_Ballistic(void) ;

  int8_t GET_BallisticX(void);
  int8_t GET_BallisticY(void);

  void MOVE_X_R(uint8_t Width_) ;

  void MOVE_X_L(void);

  void MOVE_Y_D(uint8_t Width_) ;
  void MOVE_Y_U(void) ;

  void SPEEDCALC_POS(uint8_t Speed_Frame) ;

  void SPEEDCALC_NEG(uint8_t Width_, uint8_t Speed_Frame) ;

  void AUTOMOVE(uint8_t Width_, uint8_t Speed_Frame) ;

  void BallisticDeploy(uint8_t Width_, uint8_t Speed_Balistic_) ;

  void BallisticUpdate(void) ;
};

#endif
