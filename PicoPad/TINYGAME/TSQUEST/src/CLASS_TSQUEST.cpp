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

#include "../include.h"

//public:
  void PASIVE_SPRITE_TSQUEST::INIT(uint8_t Active_, int8_t X_, int8_t Y_) {
    x = X_;
    y = Y_;
    Direction = 0;
    active = Active_;
    killed = 0;
    width = 7;
    height = 7;
  }

  uint8_t PASIVE_SPRITE_TSQUEST::GET_D(void) {
    return Direction;
  }

  void PASIVE_SPRITE_TSQUEST::PUT_D(uint8_t D_) {
    Direction = D_;
  }

  uint8_t PASIVE_SPRITE_TSQUEST::GET_K(void) {
    return killed;
  }

  void PASIVE_SPRITE_TSQUEST::PUT_K(uint8_t K_) {
    killed = K_;
  }

  int8_t PASIVE_SPRITE_TSQUEST::GET_X(void) {
    return x;
  }

  int8_t PASIVE_SPRITE_TSQUEST::GET_Y(void) {
    return y;
  }

  uint8_t PASIVE_SPRITE_TSQUEST::GET_W(void) {
    return width;
  }

  uint8_t PASIVE_SPRITE_TSQUEST::GET_H(void) {
    return height;
  }

  void PASIVE_SPRITE_TSQUEST::PUT_A(uint8_t F_) {
    active = F_;
  }

  uint8_t PASIVE_SPRITE_TSQUEST::GET_A(void) {
    return active;
  }

  void PASIVE_SPRITE_TSQUEST::PUT_W(uint8_t Width_) {
    width = Width_;
  }

  void PASIVE_SPRITE_TSQUEST::PUT_H(uint8_t height_) {
    height = height_;
  }

  void PASIVE_SPRITE_TSQUEST::PUT_X(int8_t X_) {
    x = X_;
  }

  void PASIVE_SPRITE_TSQUEST::PUT_Y(int8_t Y_) {
    y = Y_;
  }



//public:

  void ACTIVE_SPRITE_TSQUEST ::ACTIVE_SPRITE_INIT(uint8_t Active_, int8_t X_, int8_t Y_, int8_t Speed_) {
    INIT(Active_, X_, Y_);
    if ((X_ < 63) && (X_ > -100)) {
      PUT_D(3);
    } else {
      PUT_D(0);
    }
    Speed = Speed_;
    Ballistic = 0;
    BallisticPositionX = 0;
    BallisticPositionY = 0;
    BallisticSpeed = 0;
  }

  void ACTIVE_SPRITE_TSQUEST ::PUT_SPEED(uint8_t Speed_) {
    Speed = Speed_;
  }

  uint8_t ACTIVE_SPRITE_TSQUEST ::GET_SPEED(void) {
    return Speed;
  }

  uint8_t ACTIVE_SPRITE_TSQUEST ::GET_Ballistic(void) {
    return Ballistic;
  }

  void ACTIVE_SPRITE_TSQUEST ::DESTROY_Ballistic(void) {
    Ballistic = 0;
  }

  int8_t ACTIVE_SPRITE_TSQUEST ::GET_BallisticX(void) {
    return BallisticPositionX;
  }

  int8_t ACTIVE_SPRITE_TSQUEST ::GET_BallisticY(void) {
    return BallisticPositionY;
  }

  void ACTIVE_SPRITE_TSQUEST ::MOVE_X_R(uint8_t Width_) {
    if ((Width_ + GET_X()) < MAX_RIGHT) {
      PUT_X(GET_X() + Speed);
      PUT_D(3);
    }
  }

  void ACTIVE_SPRITE_TSQUEST ::MOVE_X_L(void) {
    if ((GET_X()) > MAX_LEFT) {
      PUT_X(GET_X() - Speed);
      PUT_D(0);
    }
  }

  void ACTIVE_SPRITE_TSQUEST ::MOVE_Y_D(uint8_t Width_) {
    if ((Width_ + GET_Y()) < MAX_DOWN) { PUT_Y(GET_Y() + Speed); }
  }
  void ACTIVE_SPRITE_TSQUEST ::MOVE_Y_U(void) {
    if ((GET_Y()) > MAX_UP) { PUT_Y(GET_Y() - Speed); }
  }

  void ACTIVE_SPRITE_TSQUEST ::SPEEDCALC_POS(uint8_t Speed_Frame) {
    uint8_t Speed_ = (Speed == 0) ? 1 : Speed;
    if ((Speed == 0) && (Speed_Frame == 0)) { return; }
    if ((GET_X() + Speed_) <= 127) {
      PUT_X(GET_X() + Speed_);
    } else {
      PUT_A(0);
    }
  }

  void ACTIVE_SPRITE_TSQUEST ::SPEEDCALC_NEG(uint8_t Width_, uint8_t Speed_Frame) {
    uint8_t Speed_ = (Speed == 0) ? 1 : Speed;
    if ((Speed == 0) && (Speed_Frame == 0)) { return; }
    if ((GET_X() + Width_) > 0) {
      PUT_X(GET_X() - Speed_);
    } else {
      if ((GET_X()) < -100) {
        PUT_X(GET_X() - Speed_);
      } else {
        PUT_A(0);
      }
    }
  }

  void ACTIVE_SPRITE_TSQUEST ::AUTOMOVE(uint8_t Width_, uint8_t Speed_Frame) {
    if (GET_A() == 0) return;
    if (GET_D() > 0) {
      SPEEDCALC_POS(Speed_Frame);
    } else {
      SPEEDCALC_NEG(Width_, Speed_Frame);
    }
  }

  void ACTIVE_SPRITE_TSQUEST ::BallisticDeploy(uint8_t Width_, uint8_t Speed_Balistic_) {
    if (Ballistic == 0) {
      Ballistic = 1;
      BallisticPositionX = GET_X() + uint8_t(Width_ >> 1);
      BallisticPositionY = GET_Y() + 3;
      BallisticSpeed = (GET_D() == 0) ? -Speed_Balistic_ : Speed_Balistic_;
    }
  }

  void ACTIVE_SPRITE_TSQUEST ::BallisticUpdate(void) {
    if (Ballistic != 0) {
      BallisticPositionX = BallisticPositionX + BallisticSpeed;
      if (BallisticPositionX < -6) { Ballistic = 0; }
    }
  }

