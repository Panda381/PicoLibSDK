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

#include "../include.h"


// Ajout des nouvelles déclarations externes
const uint8_t AnimJawND[4]={0,1,0,1};
const uint8_t AnimenemyND[4]={0,0,1,1};
const uint8_t AnimClimND[4]={0,1,2,3};
const uint8_t AnimGlobND[4]={0,1,2,1};

  void TUNES::tone(uint16_t Snd_,uint16_t Dur_) {
Sound(Snd_,Dur_);
}
  void TIMERND::Init(uint8_t Interval_) {
    startTime = 0;
    interval = Interval_;
    Activ = 0;
  }
  uint8_t TIMERND::Get_A(void) {
    return Activ;
  }

  void TIMERND::Activate(void) {
    Activ = 1;
  }

  void TIMERND::DeActivate(void) {
    Activ = 0;
  }

  uint8_t TIMERND::Trigger(void) {
    if (Activ == 0) return 0;
    if (startTime < interval) {
      startTime++;
      return 0;
    } else {
      startTime = 0;
      return 1;
    }
  }


  void Sprite_ND::CheckPic(uint8_t Spk) {
    AnimFrame = 0;
    switch (Spk) {
      case (MAINND):
        PIC = &MainPND[0];
        ANIM = &AnimenemyND[0];
        break;
      case (GLOBND):
        PIC = &GlobND[0];
        ANIM = &AnimGlobND[0];
        break;
      case (JAWND):
        PIC = &enemyND[0];
        ANIM = &AnimJawND[0];
        break;
      case (CLIMND):
        PIC = (X > 64) ? &ClimRND[0] : &ClimLND[0];
        ANIM = &AnimClimND[0];
        break;

      case (WINDOWND):
        PIC = &WindowND[0];
        ANIM = &AnimFrame;
        break;
      case (PLANTND):
        PIC = (X > 64) ? &FlowerRND[0] : &FlowerLND[0];
        ANIM = &AnimFrame;
        break;
      case (SLIDEND):
        PIC = &SlideMainPND[0];
        ANIM = &AnimFrame;
        break;
      case (EXPLODND):
        PIC = &ExplodND[0];
        ANIM = &AnimFrame;
        break;
      case (LINEND):
        PIC = &LineND[0];
        ANIM = &AnimFrame;
        break;
      default: PIC = nullptr; break;
    }
  }

  void Sprite_ND::Update_move(void) {
    if (Direction) {
      X++;
    } else {
      X--;
    }
  }


  void Sprite_ND::Init(void) {
    Type = 0;
    Actif = 0;
    X = 0;
    Y = 0;
    W = 0;
    H = 0;
    AnimFrame = 0;
    PIC = nullptr;
    ANIM = nullptr;
  }

  void Sprite_ND::ActivateSprite(int8_t x, int8_t y, uint8_t Type_) {
    X = x;
    Y = y;
    Actif = 1;
    Type = Type_;
    CheckPic(Type);
    Direction = rand() % 2;
  }

  const uint8_t* Sprite_ND::Get_PIC(void) {
    return PIC;
  }
  const uint8_t* Sprite_ND::Get_ANIM(void) {
    return ANIM;
  }
  uint8_t Sprite_ND::Get_AnimFrame(void) {
    return AnimFrame;
  }
  void Sprite_ND::Put_AnimFrame(uint8_t Fr_) {
    AnimFrame = Fr_;
  }
  uint8_t Sprite_ND::Get_A(void) {
    return Actif;
  }
  uint8_t Sprite_ND::Get_Type(void) {
    return Type;
  }
  int8_t Sprite_ND::Get_X(void) {
    return X;
  }
  int8_t Sprite_ND::Get_Y(void) {
    return Y;
  }
  uint8_t Sprite_ND::Get_D(void) {
    return Direction;
  }
  void Sprite_ND::Put_D(uint8_t D_) {
    Direction = D_;
  }
  void Sprite_ND::Put_Y(int8_t y_) {
    Y = y_;
  }
  void Sprite_ND::Put_X(int8_t x_) {
    X = x_;
  }
  void Sprite_ND::Desactive(void) {
    Init();
  }
  void Sprite_ND::Destroy(void) {
    AnimFrame = 0;
    Type = EXPLODND;
    X = X - 3;
    Y = Y + 1;
    PIC = &ExplodND[0];
    ANIM = &AnimFrame;
  }
  void Sprite_ND::SPY(void) {
    Y--;
  }
  void Sprite_ND::MoveXND(void) {
    if ((X > 10) && (X < 102)) {
      Update_move();
    } else {
      Direction = (Direction == 1) ? 0 : 1;
      Update_move();
    }
  }


