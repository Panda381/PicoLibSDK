//       >>>>> N-O-H-Z-D-Y-V-E for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2024-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Nohzdyve is free software: you can redistribute it and/or modify
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


// The Nohzdyve source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "nohzdyve.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the nohzdyve.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK


#ifndef _ENGINEOBJ_H
#define _ENGINEOBJ_H

enum {
  MAINND = 0,
  GLOBND = 1,
  JAWND = 2,
  CLIMND = 3,
  WINDOWND = 4,
  PLANTND = 5,
  SLIDEND = 6,
  EXPLODND = 7,
  LINEND = 8
};

typedef struct NUMND {
  uint8_t ND10000;
  uint8_t ND1000;
  uint8_t ND100;
  uint8_t ND10;
  uint8_t ND1;
  uint16_t Scores;
  uint16_t HIScores;
  uint16_t Lives;
} NUMND;

class TUNES {
private:
public:
  void tone(uint16_t Snd_,uint16_t Dur_);
};

class TIMERND {
private:
  uint8_t Activ;
  uint8_t startTime;
  uint8_t interval;
public:
  void Init(uint8_t Interval_) ;
  uint8_t Get_A(void) ;

  void Activate(void);

  void DeActivate(void) ;

  uint8_t Trigger(void);
};

class Sprite_ND {
private:
  uint8_t Actif;
  uint8_t Type;
  uint8_t Direction;
  int8_t X;
  int8_t Y;
  uint8_t W;
  uint8_t H;
  uint8_t AnimFrame;
  const uint8_t* PIC;
  const uint8_t* ANIM;
  void CheckPic(uint8_t Spk) ;

  void Update_move(void);

public:

  void Init(void) ;

  void ActivateSprite(int8_t x, int8_t y, uint8_t Type_);

  const uint8_t* Get_PIC(void) ;
  const uint8_t* Get_ANIM(void);
  uint8_t Get_AnimFrame(void);
  void Put_AnimFrame(uint8_t Fr_) ;
  uint8_t Get_A(void) ;
  uint8_t Get_Type(void);
  int8_t Get_X(void);
  int8_t Get_Y(void);
  uint8_t Get_D(void) ;
  void Put_D(uint8_t D_) ;
  void Put_Y(int8_t y_);
  void Put_X(int8_t x_) ;
  void Desactive(void) ;
  void Destroy(void);
  void SPY(void) ;
  void MoveXND(void) ;
};

#endif
