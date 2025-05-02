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

extern PICOCOMPATIBILITY PICOKIT;


extern uint8_t JumpCycle;
extern uint8_t JumpCycleMem;
extern uint8_t RopeMode;
extern uint8_t UPDOWN;
extern uint8_t DDead;
extern uint8_t RoomChange;
extern uint8_t Frm;

  void TIMERGITD::Init(uint8_t Interval_) {
    startTime = 0;
    interval = Interval_;
    Activ = 0;
  }

  uint8_t TIMERGITD::Get_A(void) {
    return Activ;
  }

  void TIMERGITD::Activate(void) {
    Activ = 1;
  }

  void TIMERGITD::DeActivate(void) {
    Activ = 0;
  }

  uint8_t TIMERGITD::Trigger(void) {
    if (Activ == 0) return 0;
    if (startTime < interval) {
      startTime++;
      return 0;
    } else {
      startTime = 0;
      return 1;
    }
  }


  void StaticSprite_GITD::StaticInit(void) {
    Actif = 0;
    X = 0;
    Y = 0;
  }

  uint8_t StaticSprite_GITD::Get_A(void) {
    return Actif;
  }
  int8_t StaticSprite_GITD::Get_X(void) {
    return X;
  }
  int8_t StaticSprite_GITD::Get_Y(void) {
    return Y;
  }

  void StaticSprite_GITD::Put_A(uint8_t A_) {
    Actif = A_;
  }

  void StaticSprite_GITD::Put_X(int8_t X_) {
    X = X_;
  }

  void StaticSprite_GITD::Put_Y(int8_t Y_) {
    Y = Y_;
  }


  uint8_t AcidDrop_GITD::Get_Dl(void) {
    return Delays;
  }

  void AcidDrop_GITD::InitAcidDrop(void) {
    Put_A(0);
  }

  void AcidDrop_GITD::SetAcidDrop(int8_t x_, int8_t y_) {
    Put_A(1);
    Put_X(x_ + x_offset);
    Put_Y(y_ + y_offset);
    Delays = 35;
  }

  void AcidDrop_GITD::UpdateAcideDrop(void) {
    if (Delays > 0) {
      Delays--;
    } else {
      Put_Y(Get_Y() + 1);
    }
  }



  void Sprite_GITD::InitDeadAnim(void) {
    if (DDead == 4) {
      SubLive();
      DeadFrameStep = 110;
      Put_X(Get_X() + StretchADD);
      StretchADD = 0;
      Stretch = 0;
      DDead = 0;
    }
  }

  void Sprite_GITD::ForcedJumpTrig(void) {
    GravitySpeed = -1.2;
    FloorStat = 0;
    OneClick = 0;
  }

  void Sprite_GITD::SpriteStretchSelect(void) {
    switch (Stretch) {
      case -29 ... 29:
        RopeJump();
        StretchADD = 0;
        break;
      case -50 ... - 30: StretchADD = -2; break;
      case 30 ... 50: StretchADD = 2; break;
      case -68 ... - 51:
        {
          Put_X(Get_X() - 4);
          StretchADD = 0;
          Stretch = 0;
        }
        break;
      case 51 ... 68:
        {
          Put_X(Get_X() + 4);
          StretchADD = 0;
          Stretch = 0;
        }
        break;
      default: break;
    }
  }

  uint8_t Sprite_GITD::ColWallP(void) {
    return ColWall(Get_X(), Get_Y());
  }

  uint8_t Sprite_GITD::MainAnimSelect(void) {
    return ((GET_FLS()) ? Get_ANIMFRM() : (Get_Direction() == 6) ? Get_ANIMFRM():1)+ Get_Direction();
  }


  void Sprite_GITD::Init(void) {
    Direction = 0;
    StaticInit();
    Type = 0;
    Put_A(0);
    TrigSpeedRestriction = 0;
    SpeedRestriction = 4;
    AnimFrame = 0;
    GravitySpeed = 0;
    FloorStat = 1;
    OneClick = 1;
    OneClick2 = 1;
    Stretch = 0;
    StretchADD = 0;
    DeadFrameStep = 0;
    DDead = 0;
  }

  int8_t Sprite_GITD::Get_Stretch(void) {
    return StretchADD;
  }

  uint8_t Sprite_GITD::DeadAnim(void) {
    DeadFrameStep = (DeadFrameStep > 0) ? DeadFrameStep - 1 : DeadFrameStep;
    if (DeadFrameStep > 106) {
      return 9;
    } else {
      if ((DeadFrameStep > 98)) { Sound(1, 4); }
      if (DeadFrameStep == 0) DDead = 3;
      return 10;
    }
  }

  uint8_t Sprite_GITD::NoiseMain(void) {
    if (DeadFrameStep != 0) {
      DeadFrameStep--;
    } else {
      DeadFrameStep = 3;
    }
    return 11 + DeadFrameStep;
  }

  uint8_t Sprite_GITD::GetMainFrame(void) {
    switch (DDead) {
      case 0: return MainAnimSelect();
      case 1:
        if (GravitySpeed < 0) { GravitySpeed = 0; }
        return MainAnimSelect();
      case 2: return DeadAnim();
      case 3:
      case 4: return NoiseMain();
      default: return 0; break;
    }
  }

  void Sprite_GITD::Put_1Click2(void) {
    OneClick2 = 0;
    OneClick = 0;
  }

  void Sprite_GITD::Put_Direction(uint8_t D_) {
    Direction = D_;
  }

  uint8_t Sprite_GITD::GET_FLS(void) {
    return FloorStat;
  }

  int8_t Sprite_GITD::Get_AddStretch(void) {
    return StretchADD;
  }

  void Sprite_GITD::JumpTrig(void) {
    if (DDead) return;
    if ((FloorStat) && (OneClick)) {
      GravitySpeed = -1.2;
      FloorStat = 0;
      OneClick = 0;
    }
  }

  int8_t Sprite_GITD::Get_GS(void) {
    return GravitySpeed;
  }
  uint8_t Sprite_GITD::Get_1Click(void) {
    return OneClick;
  }

  void Sprite_GITD::GravityReset(void) {
    GravitySpeed = 0;
  }

  void Sprite_GITD::GravityUpdate(void) {
    if ((GravitySpeed + GravityQ) < 3) {
      GravitySpeed = GravitySpeed + GravityQ;
    } else {
      DDead = 1;
    }
    Put_Y(Get_Y() + GravitySpeed);
  }

  void Sprite_GITD::FloorDirectCheck(void) {
    if (ColWall(Get_X(), Get_Y() + 1)) { DDead = 2; }
  }

  void Sprite_GITD::Gravitycalcule(void) {
    if (DDead == 1) {
      FloorDirectCheck();
    }
    if ((GravitySpeed) >= 0) {
Recheck:;
      if (ColWall(Get_X(), Get_Y())) {
        GravityReset();
        Stretch = 0;
        FloorStat = 1;
        if (BUTTON_UP) OneClick = 1;
        Put_Y(Get_Y() - 1);
        if (DDead == 1) { DDead = 2; }
        goto Recheck;
      }
    } else {
Recheck2:;
      if (ColWall(Get_X(), Get_Y())) {
        GravityReset();
        Put_Y(Get_Y() + 1);
        JumpCycleMem = 2;
        goto Recheck2;
      }
    }

    if (GravitySpeed > 1) {
      FloorStat = 0;
      OneClick = 0;
      OneClick2 = 0;
    }
  }

  void Sprite_GITD::SetSpritePos(int8_t x_, int8_t y_) {
    Put_X(x_offset + x_);
    Put_Y(y_offset + y_);
  }

  int8_t Sprite_GITD::Get_Direction(void) {
    return Direction;
  }

  void Sprite_GITD::Neutral(void) {
    TrigSpeedRestriction = SpeedRestriction;
    AnimFrame = 1;
  }

  int8_t Sprite_GITD::Get_ANIMFRM(void) {
    return AnimFrame;
  }

  void Sprite_GITD::Go2Left(void) {
    uint8_t Trig_ = 0;
    if ((DDead == 1) || (DDead == 2) || (DDead == 3)) return;
    InitDeadAnim();
    if (TrigSpeedRestriction < SpeedRestriction) {
      TrigSpeedRestriction++;
    } else {
      if (Get_X() > 0) {
        Put_X(Get_X() - 1);
        if ((FloorStat) && (AnimFrame == 2)) Trig_ = 1;
      }
      if (ColWallP()) {
        Trig_ = 0;
        Put_X(Get_X() + 1);
        if (GravitySpeed < 0) GravitySpeed = 0;
      } else {
        TrigSpeedRestriction = 0;
        AnimFrame = (AnimFrame < 2) ? AnimFrame + 1 : 0;
        Direction = 0;
        if (Trig_) SoundSystem(1);
      }
    }
  }

  void Sprite_GITD::Go2Right(void) {
    uint8_t Trig_ = 0;
    if ((DDead == 1) || (DDead == 2) || (DDead == 3)) return;
    InitDeadAnim();
    if (TrigSpeedRestriction < SpeedRestriction) {
      TrigSpeedRestriction++;
    } else {
      if (Get_X() < 127) {
        Put_X(Get_X() + 1);
        if ((FloorStat) && (AnimFrame == 2)) Trig_ = 1;
      }
      if (ColWallP()) {
        Put_X(Get_X() - 1);
        if (GravitySpeed < 0) GravitySpeed = 0;
      } else {
        TrigSpeedRestriction = 0;
        AnimFrame = (AnimFrame < 2) ? AnimFrame + 1 : 0;
        Direction = 3;
        if (Trig_) SoundSystem(1);
      }
    }
  }

  void Sprite_GITD::GoClimb(void) {
    uint8_t Trig_ = 0;
    if (DDead) return;
    if (TrigSpeedRestriction < SpeedRestriction) {
      TrigSpeedRestriction++;
    } else {
      Put_Y(Get_Y() - 1);
      Trig_ = 1;
      if (ColWallP()) {
        AnimFrame = (AnimFrame > 0) ? AnimFrame - 1 : AnimFrame;
        Put_Y(Get_Y() + 1);
        Trig_ = 0;
      }

      GravitySpeed = 0;
      TrigSpeedRestriction = 0;
      AnimFrame = (AnimFrame < 2) ? AnimFrame + 1 : 0;
      if (Trig_) SoundSystem(3);
    }
  }

  void Sprite_GITD::GoFall(void) {
    if (DDead) return;
    if (TrigSpeedRestriction < SpeedRestriction) {
      TrigSpeedRestriction++;
    } else {
      Put_Y(Get_Y() + 1);
      GravitySpeed = 2;
      TrigSpeedRestriction = 0;
      AnimFrame = (AnimFrame > 0) ? AnimFrame - 1 : 2;
      SoundSystem(4);
    }
  }

  void Sprite_GITD::RopeJump(void) {
    if (DDead) return;
    if (BUTTON_UP) { OneClick2 = 1; }
    if ((BUTTON_DOWN) && (OneClick2)) {
      UPDOWN = 2;
      RopeMode = 0;
      if (TINYJOYPAD_LEFT ) {
        JumpCycle = 0;
        JumpCycle = 0;
        JumpCycleMem = 0;
        ForcedJumpTrig();
      } else if (TINYJOYPAD_RIGHT ) {
        JumpCycle = 1;
        JumpCycle = 1;
        JumpCycleMem = 1;
        ForcedJumpTrig();
      }
    }
  }

  void Sprite_GITD::leftstretch(void) {
    if (DDead) return;
    if (Stretch > -127) Stretch--;
    SpriteStretchSelect();
    if (!StretchADD) {
    } else {
      Direction = 0;
    }
  }

  void Sprite_GITD::rightstretch(void) {
    if (DDead) return;
    if (Stretch < 127) Stretch++;
    SpriteStretchSelect();

    if (!StretchADD) {
    } else {
      Direction = 3;
    }
  }

  void Sprite_GITD::NeutralStretch(void) {
    if ((Stretch > 29) || (Stretch < -29)) return;
    Stretch = 0;
    SpriteStretchSelect();
  }



  uint8_t Ballon_GITD::collXBallon(void) {
    if (PICOKIT.getPixelColor(Get_X() + 1, Get_Y() + 5) || PICOKIT.getPixelColor(Get_X() + 2, Get_Y() + 5)) {
      return 1;
    } else {
      return 0;
    }
  }

  uint8_t Ballon_GITD::collYBallon(void) {
    if ((PICOKIT.getPixelColor(Get_X() + 2, Get_Y() + 6)) || (PICOKIT.getPixelColor(Get_X() + 2, Get_Y() + 7))) {
      return 1;
    } else {
      return 0;
    }
  }

  void Ballon_GITD::UpdateGravity(void) {
    if (Step < 2) Step += .2;
    Put_Y(Get_Y() + Step);
    Anim = 0;
Recheck3:;
    if (collYBallon()) {
      Anim = 1;
      Step = -.8;
      Put_Y(Get_Y() - 1);
      goto Recheck3;
    }
  }

  void Ballon_GITD::leftmove(void) {
    if (!collXBallon()) {
      Put_X(Get_X() - 1);
    } else {
      Put_A(0);
      Step = 0;
      Rset_Ballon();
    }
  }


  void Ballon_GITD::Init_Ballon(void) {
    Put_A(0);
    Put_X(0);
    Put_Y(0);
  }

  void Ballon_GITD::Set_Ballon(int8_t x_, int8_t y_) {
    Put_A(1);
    Put_X(x_);
    Put_Y(y_);
    Step = 0;
    Anim = 0;
    XMEM = x_;
    YMEM = y_;
  }

  void Ballon_GITD::Rset_Ballon(void) {
    Put_A(1);
    Put_X(XMEM);
    Put_Y(YMEM);
  }

  uint8_t Ballon_GITD::Get_Anim(void) {
    return Anim;
  }

  void Ballon_GITD::Update_Ballon(void) {
    if (!Get_A()) return;
    leftmove();
    UpdateGravity();
  }



  uint8_t TIMEOUT::FlipFrm(void) {
    Flip = (Flip < 10) ? Flip + 1 : 0;
    if (Flip < 6) {
      return 0;
    } else {
      return 1;
    }
  }



  uint16_t TIMEOUT::Get_Time(void) {
    return Time;
  }

  void TIMEOUT::Set_Time(void) {
    Time = 4000;
  }

  void TIMEOUT::ResetTime(void) {
    if ((Actif) && (Time == 0)) { Time = 2048; }
  }

  void TIMEOUT::deactivate(void) {
    Actif = 0;
    Flip = 0;
  }

  void TIMEOUT::Activate(void) {
    if (Actif) return;
    Actif = 1;
    x = 0;
    y = 0;
    RL = 1;
    UD = 1;
  }

  void TIMEOUT::TimeOutFunction(int8_t x_, int8_t y_) {
    if (DDead > 2) return;
    if (Time > 0) {
      Time--;
    } else {
      Activate();
    }
    if (!Actif) return;
    if (colid_univ(x_ + 1, 4, y_ + 2, 5, x, 5, y, 3)) {
      if (!DDead) DDead = 1;
    }
    if (UD) {
      Go2Down();
    } else {
      Go2UP();
    }
    if (RL) {
      Go2Right();
    } else {
      Go2Left();
    }
    PICOKIT.drawSelfMaskedColor(x, y, TimeOut_GITD, FlipFrm(),color(27,55,27));
  }

  void TIMEOUT::Go2Right(void) {
    if (x < (x_offset + 96)) {
      x++;
    } else {
      RL = 0;
    }
  }

  void TIMEOUT::Go2Left(void) {
    if (x > (x_offset)) {
      x--;
    } else {
      RL = 1;
    }
  }

  void TIMEOUT::Go2UP(void) {
    if (y < (y_offset + 60)) {
      y++;
    } else {
      UD = 1;
    }
  }

  void TIMEOUT::Go2Down(void) {
    if (y > (y_offset)) {
      y--;
    } else {
      UD = 0;
    }
  }

