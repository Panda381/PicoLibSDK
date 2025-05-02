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

//PUBLIC VAR
uint8_t lvlcol;
uint8_t J1Flip;
uint8_t M10000_TSQUEST = 0;
uint8_t M1000_TSQUEST = 0;
uint8_t M100_TSQUEST = 0;
uint8_t M10_TSQUEST = 0;
uint8_t M1_TSQUEST = 0;
uint8_t FlipFlop1=0;//Arduboy only
uint8_t FlipFlop2=0;//Arduboy only

unsigned long currentMillis=0;
unsigned long MemMillis=0;
//33 26
#define FRAME_CONTROL_TSQUEST while((currentMillis-MemMillis)<33){currentMillis=millis();}MemMillis=currentMillis
#define FRAME_CONTROL_2_TSQUEST while((currentMillis-MemMillis)<26){currentMillis=millis();}MemMillis=currentMillis


//DEFINE
#define DIVERDISPLAY 7
#define LIVEDISPLAY 0
#define COUNT_RESET 5
#define LEVELMAX 16

const uint8_t *SpR[3] = { &PLONGEUR_TSQUEST[0], &FISH_TSQUEST[0], &SUB2_TSQUEST[0] };
const uint8_t *SpR_SUB_Main[3] = { &T_SUBMAIN_TSQUEST[0], &BLINK_MAINSUB_TSQUEST[0], &DESTROYED_MAINSUB_TSQUEST[0] };

extern PICOCOMPATIBILITY PICOKIT;
GamePlay_TSQUEST SQUEST;
ACTIVE_SPRITE_TSQUEST Sub_Main;
ACTIVE_SPRITE_TSQUEST Other_Sprite[9];

/////////////////////////////////////////////////////////////////////////////
////////////////////////////////// main  ////////////////////////////////////
int main() {
	PICOKIT.Init();
	PICOKIT.SetColorBuffer(21,53,31,21,43,21,0,0,19);
START_:;
lvlcol=3;
J1Flip=1;
  Intro_TSQUEST();
  
  RESET_VAR_TSQUEST();
  Sound(100, 255);
  Sound(10, 255);
  Sub_Main.ACTIVE_SPRITE_INIT(1, 54, 6, 2);
  uint8_t Timer_1 = 0;
NEXT_LEVEL:;
lvlcol=(lvlcol<3)?lvlcol+1:0;
  Level_Adjust_TSQUEST(SQUEST.Level);
RESTART_LEVEL:;
 PICOKIT.FillScreenColor(color(0,0,19));
  REST_LEVEL_TSQUEST();
  SQUEST.DIVER_OSD_STATE = 1;
  SQUEST.OX_OSD_STATE = 1;
  OSD_MANAGER_SQUEST();
  RENDER_DISPLAY_TSQUEST(SQUEST.DIVER_OSD_STATE, SQUEST.OX_OSD_STATE);
  RENDER_TSQUEST();
    _delay_ms(400);
  while (1) {
    PICOKIT.FillScreenColor(color(0,0,19));
    if (SQUEST.Main_SPK_Bank == 0) {
      CheckCollisionSolo_TSQUEST();
      UPDATE_GAMEPLAY_TSQUEST();
    }
    if (Timer_1 < 1) {
      Timer_1++;
      if (Sub_Main.GET_Y() < 12) {
      }
    } else {
      Sin_Move_TSQUEST();
      Timer_1 = 0;
      SQUEST.SUBSOLO_X = (SQUEST.Level > 3) ? SQUEST.SUBSOLO_X - 1 : -60;
      switch (SQUEST.Main_SPK_Bank) {
        case 0:Refresh_Anim_TSQUEST();
        case 1:Refresh_Main_Anim_TSQUEST();Step2_counter_TSQUEST(SQUEST.Main_SPK_Bank);break;
        case 2: Step3_counter_TSQUEST(); break;
        default: break;
      }
      if (SQUEST.Main_SPK_Bank == 254) goto NEXT_LEVEL;
      if (SQUEST.Main_SPK_Bank == 255) goto RESTART_LEVEL;
    }
    if (SQUEST.RETURN_START == 1) { goto START_; }
 	if (J1Flip){  
    RENDER_TSQUEST();
     FRAME_CONTROL_TSQUEST;
}
J1Flip=1;
  }
}
////////////////////////////////// main end /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void RESET_VAR_TSQUEST(void) {
  SQUEST.RETURN_START = 0;
  SQUEST.RND3Pos = 0;
  SQUEST.RND2Pos = 0;
  SQUEST.Diver = 0;
  SQUEST.Level = 1;
  SQUEST.Scores = 0;
  SQUEST.Frame = 0;
  SQUEST.Frame_Main = 0;
  SQUEST.Main_Live = 3;
  SQUEST.Frame_Cycle_STEP2 = 0;
  SQUEST.DIVER_OSD_STATE = 1;
  SQUEST.OX_OSD_STATE = 1;
  SQUEST.Refresh_OSD = 1;
  SQUEST.FlipFlop = 0;
  SQUEST.FlipFlop2_OX = 0;
  SQUEST.Limit_move_Other_Sprite = 0;
  SQUEST.Limit_Refresh = 0;
  SQUEST.latch_1_REFILL = 0;
  SQUEST.latch_2_AFTERDEAD = 0;
  SQUEST.Sin_Anim = 0;
}

void REST_LEVEL_TSQUEST(void) {
  SQUEST.SUBSOLO_X = -60;
  SQUEST.Lib_Sprite_Timer = 0;
  SQUEST.Flipflop_Counter = COUNT_RESET;
  SQUEST.FlipFlop2_OX_Counter_ = COUNT_RESET;
  SQUEST.NEXTLEVEL = 0;
  SQUEST.NO_JOY = 0;
  SQUEST.OX = 62;
  SQUEST.latch_2_AFTERDEAD = 0;
  SQUEST.latch_1_REFILL = 0;
  SQUEST.latch_0_FIRSTFULLING = 0;
  Sub_Main.ACTIVE_SPRITE_INIT(1, 55, 6, 2);
  SQUEST.Main_SPK_Bank = 0;
  uint16_t MAX_ =pico_millis()/* analogRead(A3) + analogRead(A2)*/;
  for (uint16_t t = 0; t < MAX_; t++) {
    RND3X_tsquest();
    RND2X_tsquest();
  }
  Init_TSQUEST();
}

void Init_TSQUEST(void) {
  for (uint8_t t_ = 0; t_ < 9; t_++) {
    Other_Sprite[t_].INIT(0, 90, 32);
    Other_Sprite[t_].DESTROY_Ballistic();
  }
}

uint8_t Amount_Sprite_SET_TSQUEST(uint8_t Level_) {
  switch (Level_) {
    case 1 ... 2: return 1; break;
    case 3 ... 6: return 2; break;
    default: return 3; break;
  }
}

void Level_Adjust_TSQUEST(uint8_t Level_) {
  SQUEST.LIBTRIG = Mymap(Level_, 1, LEVELMAX, 25, 1);
  SQUEST.Speed = Mymap(Level_, 1, LEVELMAX, 0, 3);
  SQUEST.Amount_Sprite = Amount_Sprite_SET_TSQUEST(Level_);
  SQUEST.Speed_Balistic = Mymap(Level_, 1, LEVELMAX, 2, 5);
}

void Intro_TSQUEST(void) {
  uint8_t BL_ = 0;
  while (1) {
    BL_ = (BL_ < 12) ? BL_ + 1 : 0;
  RENDER_INTRO_TSQUEST(BL_);
    if (BUTTON_DOWN) { break; }
    _delay_ms(33);
  }
}

void CheckCollisionSolo_TSQUEST(void) {
  if (COLLISION_TSQUEST(Sub_Main.GET_X(), Sub_Main.GET_Y(), 16, 7, SQUEST.SUBSOLO_X, 8, 6, 4)) {
    SQUEST.SUBSOLO_X = -10;
    Start_Proccess_Destroy_Main_TSQUEST();
  }
}

void NEW_LEVEL_TSQUEST(void) {
  SQUEST.Level = (SQUEST.Level < LEVELMAX) ? SQUEST.Level + 1 : LEVELMAX;
  SQUEST.Main_SPK_Bank = 254;
}

void Respond_TSQUEST(void) {

  SQUEST.Main_SPK_Bank = 255;
}

void Game_Over_TSQUEST(void) {
  SQUEST.RETURN_START = 1;
}

void Kiling_Main_TSQUEST(void) {
  if (SQUEST.latch_1_REFILL == 0) { Start_Proccess_Destroy_Main_TSQUEST(); }
}

void Refresh_Anim_TSQUEST(void) {
  SQUEST.Frame = (SQUEST.Frame < 2) ? SQUEST.Frame + 1 : 0;
}
void Refresh_Main_Anim_TSQUEST(void) {
  SQUEST.Frame_Main = (SQUEST.Frame_Main < 2) ? SQUEST.Frame_Main + 1 : 0;
}
void Sound_1_TSQUEST(void){
    for (uint8_t t=0;t<5;t++){
  Sound(10,25);
  Sound(40,25);
  Sound(80,25);
  }
  }

void Start_Proccess_Destroy_Main_TSQUEST(void) {
Sound_1_TSQUEST();
  SQUEST.Main_SPK_Bank = 1;
  SQUEST.Frame_Main = 0;
  SQUEST.Frame_Cycle_STEP2 = 0;
}
void Start_Proccess_Destroy_Main_Step3_TSQUEST(void) {
  SQUEST.Main_SPK_Bank = 2;
  SQUEST.Frame_Main = 0;
}

void Start_Proccess_Destroy_Main_Step4_TSQUEST(void) {
  if (SQUEST.Diver > 0) SQUEST.Diver--;
  SQUEST.Main_SPK_Bank = 1;
  SQUEST.Frame_Main = 2;
  Sub_Main.PUT_A(0);
  if (SQUEST.Main_Live != 0) {
    SQUEST.Main_Live--;
  } else {
    Game_Over_TSQUEST();
  }
  RENDER_DISPLAY_TSQUEST(SQUEST.DIVER_OSD_STATE, SQUEST.OX_OSD_STATE);
J1Flip=1;
  PLAY_MUSIC(Music);
  _delay_ms(1000);
  Respond_TSQUEST();
}

void Step2_counter_TSQUEST(uint8_t Skip_) {
  if (Skip_ != 1) return;
  if (SQUEST.Frame_Cycle_STEP2 == 16) {
    Start_Proccess_Destroy_Main_Step3_TSQUEST();
  } else {
    SQUEST.Frame_Cycle_STEP2++;
  }
}

void Step3_counter_TSQUEST(void) {
  if (SQUEST.Frame_Main == 2) {
    Start_Proccess_Destroy_Main_Step4_TSQUEST();
  } else {
    SQUEST.Frame_Main = (SQUEST.Frame_Main < 2) ? SQUEST.Frame_Main + 1 : SQUEST.Frame_Main;
  }
}

void JoyPad_Refresh_TSQUEST(void) {
  if (SQUEST.NO_JOY) return;
  if (TINYJOYPAD_LEFT) { Sub_Main.MOVE_X_L(); }
  if (TINYJOYPAD_RIGHT) { Sub_Main.MOVE_X_R(T_SUBMAIN_TSQUEST[0]); }
  if (TINYJOYPAD_UP) { Sub_Main.MOVE_Y_U(); }
  if (TINYJOYPAD_DOWN) { Sub_Main.MOVE_Y_D(6); }
  if (BUTTON_DOWN) {
    if ((Sub_Main.GET_Ballistic() == 0) && (Sub_Main.GET_Y() > 12)) {
      Sound(200, 1);
      Sub_Main.BallisticDeploy(T_SUBMAIN_TSQUEST[0], 8);
    }
  }
}

void OX_REDUCE_TSQUEST(void) {
  if (SQUEST.Limit_Refresh < 15) {
    SQUEST.Limit_Refresh++;
  } else {
    SQUEST.Limit_Refresh = 0;
    if (SQUEST.OX > 61) {
      SQUEST.OX--;
      if (SQUEST.OX > 70) { OSD_OX_TSQUEST(1); }
    } else {
      Kiling_Main_TSQUEST();
    }
  }
}

void REMOVE_DIVER_TSQUEST(void) {
  if (SQUEST.Diver > 0) {
    SQUEST.Diver--;
  } else {
    Kiling_Main_TSQUEST();
  }
}

void Refill_TSQUEST(uint8_t Y_) {
	    PICOKIT.FillScreenColor(color(0,0,19));
  if (Y_ == 6) {
    if (SQUEST.latch_1_REFILL == 0) {
      if (SQUEST.latch_0_FIRSTFULLING == 0) {
        SQUEST.latch_0_FIRSTFULLING = 1;
      } else {
        if (SQUEST.Diver == 6) {
          SQUEST.NO_JOY = 1;
          SQUEST.DIVER_OSD_STATE=1;
  //        RENDER_DISPLAY_TSQUEST(SQUEST.DIVER_OSD_STATE, 1);
          for (uint8_t t2 = 0; t2 < 2; t2++) {
            for (uint8_t t = 200; t > 10; t--) {
              Sound(t, 5);
            }
            _delay_ms(10);
          }
          _delay_ms(200);
          NEXT_LEVEL_TRIGGER_TSQUEST();
        } else {
          REMOVE_DIVER_TSQUEST();
        }
      }
      SQUEST.latch_1_REFILL = 1;
    } else {
      if (SQUEST.OX < 90) {
        SQUEST.NO_JOY = 1;
        SQUEST.OX++;
        Sound(SQUEST.OX << 1, 10);
        OSD_Diver_TSQUEST(1);
        OSD_OX_TSQUEST(1);
   //     RENDER_DISPLAY_TSQUEST(SQUEST.DIVER_OSD_STATE, 1);
      } else {
        SQUEST.NO_JOY = 0;
        SQUEST.latch_2_AFTERDEAD = 1;
      }
    }

  } else {
    SQUEST.latch_1_REFILL = 0;
  }
}
void NEXT_LEVEL_TRIGGER_TSQUEST(void) {
  SQUEST.NEXTLEVEL = 1;
}

void NEXT_LEVEL_TSQUEST(void) {
  while (SQUEST.Diver > 0) {
    SQUEST.Diver--;
    SCORE_ADD_TSQUEST();
    OSD_Diver_TSQUEST(1);
    OSD_OX_TSQUEST(1);
    RENDER_DISPLAY_TSQUEST(SQUEST.DIVER_OSD_STATE, SQUEST.DIVER_OSD_STATE);
    OSD_MANAGER_SQUEST();
    Sound(100, 255);
    _delay_ms(300);
  }
  NEW_LEVEL_TSQUEST();
}

void UPDATE_GAMEPLAY_TSQUEST(void) {
  RND3X_tsquest();
  RND2X_tsquest();
  FLIPFLOP_TSQUEST();
  FLIPFLOP2_TSQUEST();
  if (SQUEST.NEXTLEVEL == 0) {
    Refill_TSQUEST(Sub_Main.GET_Y());
    Check_Endingline_TSQUEST((RECUP_RND3_tsquest()) - 1);
    if (SQUEST.latch_1_REFILL == 0) {
      Check_Collision_Main2Other_TSQUEST();
      CHECK_COLLISION_BALLISTIC_TSQUEST();
      CHECK_OTHER_COLLISION_BALLISTIC_TSQUEST();
      OX_REDUCE_TSQUEST();
    }
    if (SQUEST.latch_2_AFTERDEAD == 1) {
      JoyPad_Refresh_TSQUEST();
      UPDATE_BALLISTIC_TSQUEST();
      for (uint8_t t = 0; t < 9; t++) {
        if (Other_Sprite[t].GET_A()) {
          Other_Sprite[t].AUTOMOVE(SUB2_TSQUEST[0], SQUEST.Limit_move_Other_Sprite);
        }
      }
      SQUEST.Limit_move_Other_Sprite = (SQUEST.Limit_move_Other_Sprite == 0) ? 1 : 0;
    }
  } else {
    SQUEST.latch_1_REFILL = 1;
    SQUEST.latch_2_AFTERDEAD = 0;
    NEXT_LEVEL_TSQUEST();
  }
  OSD_MANAGER_SQUEST();
}

int8_t P_TSQUEST(uint8_t Direction_, uint8_t position_) {
  return POSITION_TSQUEST[(Direction_ * 3) + position_];
}

uint8_t LINE_TSQUEST(uint8_t position_) {
  return Line_TSQUEST[(position_)];
}

void Check_Endingline_TSQUEST(uint8_t Line_) {
  uint8_t Line_Temp = Line_ * 3;
  for (uint8_t t = 0; t < 3; t++) {
    if ((Other_Sprite[Line_Temp + t].GET_A()) || (Other_Sprite[Line_Temp + t].GET_Ballistic())) return;
  }
  if (SQUEST.Lib_Sprite_Timer == SQUEST.LIBTRIG) {
    SQUEST.Lib_Sprite_Timer = 0;
    LIBERATE_LINE_TSQUEST(Line_);
  } else {
    SQUEST.Lib_Sprite_Timer++;
  }
}

void LIBERATE_LINE_TSQUEST(uint8_t Line_) {
#define ActualSpeed SQUEST.Speed
  uint8_t A_ = 0, B_ = 0, C_ = 0;
  uint8_t Line_Temp = (Line_ * 3);
  uint8_t Direction_ = RECUP_RND2_tsquest();
  uint8_t Line_use = LINE_TSQUEST(Line_);
  MAKE_LINE_TSQUEST(&A_, &B_, &C_);
  switch (SQUEST.Amount_Sprite) {
    case (3): Other_Sprite[(Line_Temp + 2)].ACTIVE_SPRITE_INIT(C_, P_TSQUEST(Direction_, 2), Line_use, ActualSpeed);
    case (2): Other_Sprite[(Line_Temp + 1)].ACTIVE_SPRITE_INIT(B_, P_TSQUEST(Direction_, 1), Line_use, ActualSpeed);
    case (1): Other_Sprite[Line_Temp].ACTIVE_SPRITE_INIT(A_, P_TSQUEST(Direction_, 0), Line_use, ActualSpeed); break;
    default: break;
  }
}

void MAKE_LINE_TSQUEST(uint8_t *A_, uint8_t *B_, uint8_t *C_) {
  *A_ = RECUP_RND3_tsquest();
  switch (*A_) {
    case 1:
    case 2:
      *B_ = 2;
      *C_ = 2;
      break;
    case 3:
      *B_ = 3;
      *C_ = 3;
      break;
    default: break;
  }
}

void FLIPFLOP2_TSQUEST(void) {
  if (SQUEST.OX > 70) {
    SQUEST.OX_OSD_STATE = 1;
    return;
  }
  if (SQUEST.FlipFlop2_OX_Counter_ == 0) {
    SQUEST.FlipFlop2_OX = (SQUEST.FlipFlop2_OX == 0) ? 1 : 0;
    Sound(((SQUEST.FlipFlop2_OX == 0) ? 100 : 140), 10);
    SQUEST.FlipFlop2_OX_Counter_ = 5;
  } else {
    SQUEST.FlipFlop2_OX_Counter_--;
  }
  OSD_OX_TSQUEST(SQUEST.FlipFlop2_OX);
}

void FLIPFLOP_TSQUEST(void) {
  if (SQUEST.Diver != 6) {
    SQUEST.DIVER_OSD_STATE = 1;
    return;
  }
  if (SQUEST.Flipflop_Counter == 0) {
    SQUEST.FlipFlop = (SQUEST.FlipFlop == 0) ? 1 : 0;
    SQUEST.Flipflop_Counter = 5;
  } else {
    SQUEST.Flipflop_Counter--;
  }
  OSD_Diver_TSQUEST(SQUEST.FlipFlop);
}

void OSD_Diver_TSQUEST(uint8_t STATE_) {
  SQUEST.Refresh_OSD = 1;
  SQUEST.DIVER_OSD_STATE = STATE_;
}

void OSD_OX_TSQUEST(uint8_t STATE_) {
  SQUEST.Refresh_OSD = 1;
  SQUEST.OX_OSD_STATE = STATE_;
}

void OSD_MANAGER_SQUEST(void) {
  if (SQUEST.Refresh_OSD != 0) {
    SQUEST.Refresh_OSD = 0;
 //   RENDER_DISPLAY_TSQUEST(SQUEST.DIVER_OSD_STATE, SQUEST.OX_OSD_STATE);
  }
}

void RND3X_tsquest(void) {
  SQUEST.RND3Pos = (SQUEST.RND3Pos < 14) ? SQUEST.RND3Pos + 1 : 0;
}

uint8_t RECUP_RND3_tsquest(void) {
  RND3X_tsquest();
  return RND3_TSQUEST[SQUEST.RND3Pos];
}

void RND2X_tsquest(void) {
  SQUEST.RND2Pos = (SQUEST.RND2Pos < 10) ? SQUEST.RND2Pos + 1 : 0;
}

uint8_t RECUP_RND2_tsquest(void) {
  RND2X_tsquest();
  return RND2_TSQUEST[SQUEST.RND2Pos];
}

int8_t Recup_sin_Add_TSQUEST(uint8_t Activ_) {
  if (Activ_ != 2) return 0;
  return SIN_TSQUEST[SQUEST.Sin_Anim];
}

void Sin_Move_TSQUEST(void) {
  SQUEST.Sin_Anim = (SQUEST.Sin_Anim < 11) ? SQUEST.Sin_Anim + 1 : 0;
}

void CHECK_COLLISION_BALLISTIC_TSQUEST(void) {
  RECUP_RANGE_TSQUEST(RecupeLineY(Sub_Main.GET_BallisticY()));
  for (uint8_t t = SQUEST.sa; t < SQUEST.eb; t++) {
#define PX Sub_Main.GET_BallisticX()
#define PY Sub_Main.GET_BallisticY()
#define SQX Other_Sprite[t].GET_X()
#define SQW 7
#define SQY Other_Sprite[t].GET_Y()
#define SQH 6
    if (Other_Sprite[t].GET_A() > 1) {
      if (COLLISION_TSQUEST(PX, PY, 2, 1, SQX, SQY, SQW, SQH)) {
        if (Sub_Main.GET_Ballistic()) {
          Other_Sprite[t].PUT_A(0);
          SCORE_ADD_TSQUEST();
          Sub_Main.DESTROY_Ballistic();
        }
      }
    }
  }
};

void CHECK_OTHER_COLLISION_BALLISTIC_TSQUEST(void) {
  RECUP_RANGE_TSQUEST(RecupeLineY(Sub_Main.GET_Y()));
  for (uint8_t t = SQUEST.sa; t < SQUEST.eb; t++) {
#define PX2 Other_Sprite[t].GET_BallisticX()
#define PY2 Other_Sprite[t].GET_BallisticY()
#define SQX2 Sub_Main.GET_X()
#define SQW2 16
#define SQY2 Sub_Main.GET_Y()
#define SQH2 6
    if (COLLISION_TSQUEST(PX2, PY2, 2, 1, SQX2, SQY2, SQW2, SQH2)) {
      if (Other_Sprite[t].GET_Ballistic()) {
        Kiling_Main_TSQUEST();
        Other_Sprite[t].DESTROY_Ballistic();
        return;
      }
    }
  }
};

void Check_Collision_Main2Other_TSQUEST(void) {
  for (uint8_t t = 0; t < 9; t++) {
    if (Other_Sprite[t].GET_A()) {
      if (COLLISION_TSQUEST(SQX2, SQY2, SQW2, SQH2, SQX, SQY, SQW, SQH)) {
        switch (Other_Sprite[t].GET_A()) {
          case 1:
            if (SQUEST.Diver < 6) {
              SQUEST.Diver++;
              Sound(100, 10);
              Sound(200, 10);
              OSD_Diver_TSQUEST(1);
              OSD_MANAGER_SQUEST();
              Other_Sprite[t].PUT_A(0);
            }
            break;
          case 2:
          case 3:
            Kiling_Main_TSQUEST();
            break;
          default: break;
        }
      }
    }
  }
}

void SCORE_ADD_TSQUEST(void) {
  uint8_t TMP_=M100_TSQUEST;
  SQUEST.Scores += 3;
  COMPIL_SCO_TSQUEST();
  if (M100_TSQUEST!=TMP_)  {SQUEST.Main_Live=(SQUEST.Main_Live<5)?SQUEST.Main_Live+1:SQUEST.Main_Live;}
//  RENDER_DISPLAY_TSQUEST(SQUEST.DIVER_OSD_STATE, SQUEST.OX_OSD_STATE);
}

int8_t COLLISION_TSQUEST(int8_t x, int8_t y, int8_t w, int8_t h, int8_t x2, int8_t y2, int8_t w2, int8_t h2) {
  return ((x + w < x2 || x > x2 + w2 || y + h < y2 || y > y2 + h2) ? 0 : 1);
}

uint8_t Balistic_Line_TSQUEST(uint8_t Sprite_, uint8_t State_) {
  switch (Sprite_) {
    case 2:
    case 5: return 0; break;
    default: return (State_ == 0) ? 0 : State_; break;
  }
}

uint8_t Check_if_Deployed_TSQUEST(uint8_t T_) {
  uint8_t A_ = 0, B_ = 0;
  switch (T_) {
    case 0 ... 2:
      A_ = 0;
      B_ = 2;
      break;
    case 3 ... 5:
      A_ = 3;
      B_ = 5;
      break;
    case 6 ... 8:
      A_ = 6;
      B_ = 8;
      break;
    default: break;
  }
  for (uint8_t t_ = A_; t_ < B_; t_++) {
    if (Other_Sprite[t_].GET_Ballistic() != 0) return 1;
  }
  return 0;
}

void UPDATE_BALLISTIC_TSQUEST(void) {
  uint8_t One_Shoot = 0;
  Sub_Main.BallisticUpdate();
  for (uint8_t t = 0; t < 9; t++) {
    Other_Sprite[t].BallisticUpdate();
    if (Other_Sprite[t].GET_A() == 3) {
      if ((Other_Sprite[t].GET_Ballistic() == 0) && (One_Shoot == 0) && (Check_if_Deployed_TSQUEST(t) == 0)) { Other_Sprite[t].BallisticDeploy(SUB2_TSQUEST[0], SQUEST.Speed_Balistic); }
    }
    One_Shoot = Balistic_Line_TSQUEST(t, One_Shoot);
  }
}

uint8_t Recupe_SPRBANK_TSQUEST(void){
switch(SQUEST.Main_SPK_Bank){
  case 254:return 0;break;
  case 255:return 3;break;
  default:return SQUEST.Main_SPK_Bank;break;
}
}

uint8_t RECUPE_MAIN_TSQUEST(uint8_t xPASS, uint8_t yPASS) {
  switch (yPASS) {
    case 0:
    case 7:
      return 0;
    default:
      break;
  }
  if (Sub_Main.GET_X() > xPASS) return 0x00;
  if ((Sub_Main.GET_X() + 16) < xPASS) return 0x00;
  return blitzSprite(Sub_Main.GET_X(), Sub_Main.GET_Y(), xPASS, yPASS, (Sub_Main.GET_D() + SQUEST.Frame_Main), SpR_SUB_Main[Recupe_SPRBANK_TSQUEST()]);
}

uint8_t RECUPE_BALLISTIC_MAIN_TSQUEST(uint8_t xPASS, uint8_t yPASS) {
  if (Sub_Main.GET_Ballistic() == 0) return 0;
  if (Sub_Main.GET_BallisticX() > xPASS) return 0x00;
  if ((Sub_Main.GET_BallisticX() + 2) < xPASS) return 0x00;
  return blitzSprite(Sub_Main.GET_BallisticX(), Sub_Main.GET_BallisticY(), xPASS, yPASS, 0, BALLISTIC_TSQUEST);
}

uint8_t RECUPE_BALLISTIC_OTHER_TSQUEST(uint8_t xPASS, uint8_t yPASS) {
  uint8_t Byte_ = 0x00;
  for (uint8_t t = SQUEST.sa; t < SQUEST.eb; t++) {
    if (Other_Sprite[t].GET_Ballistic() != 0) {
      if (Other_Sprite[t].GET_BallisticX() > xPASS) goto NeXt;
      if ((Other_Sprite[t].GET_BallisticX() + 2) < xPASS) goto NeXt;
      Byte_ |= blitzSprite(Other_Sprite[t].GET_BallisticX(), Other_Sprite[t].GET_BallisticY(), xPASS, yPASS, 0, BALLISTIC2_TSQUEST);
NeXt:;
    }
  }
  return Byte_;
}

void RECUP_RANGE_TSQUEST(uint8_t yPASS) {
  switch (yPASS) {
    case 2:
      SQUEST.sa = 0;
      SQUEST.eb = 3;
      return;
      break;
    case 3:
    case 4:
      SQUEST.sa = 3;
      SQUEST.eb = 6;
      return;
      break;
    case 5:
    case 6:
      SQUEST.sa = 6;
      SQUEST.eb = 9;
      return;
      break;
    default:
      SQUEST.sa = 0;
      SQUEST.eb = 9;
      return;
      break;
  }
}


const uint16_t Col_TSquest[4]={0,color(0,43,31),color(0,63,0),color(24,48,24)};

const uint16_t Col_Fish[4]={color(0,63,0),color(31,63,0),color(31,0,31),color(31,17,31)};


uint16_t Return_Spk_Col(uint8_t Spk_){
	switch(Spk_){
		case 1: 
		case 3: return Col_TSquest[Spk_];break;
		default:return Col_Fish[lvlcol];break;
	}
}


uint8_t RECUPE_OTHER_TSQUEST(uint8_t xPASS, uint8_t yPASS) {
  uint8_t BYTE_SUM = 0;
  for (uint8_t t = SQUEST.sa; t < SQUEST.eb; t++) {
    if (Other_Sprite[t].GET_A() != 0) {
      if (Other_Sprite[t].GET_X() > xPASS) goto NeXt;
      if ((Other_Sprite[t].GET_X() + 7) < xPASS) goto NeXt;
      
      

      PICOKIT.DirectDraw(xPASS,yPASS,
      blitzSprite(Other_Sprite[t].GET_X(),Other_Sprite[t].GET_Y() + Recup_sin_Add_TSQUEST(Other_Sprite[t].GET_A()), xPASS, yPASS, (Other_Sprite[t].GET_D() + SQUEST.Frame), SpR[(Other_Sprite[t].GET_A()) - 1])
        ,Return_Spk_Col(Other_Sprite[t].GET_A()));
NeXt:;
    }
  }
  return BYTE_SUM;
}

uint8_t RecupeRanged_TSQUEST(uint8_t xPASS, uint8_t yPASS) {
  switch (yPASS) {
    case 0:
    case 1:
    case 7:
      return 0;
    default:
      break;
  }
    RECUPE_OTHER_TSQUEST(xPASS, yPASS);
  //  PICOKIT.DirectDraw(xPASS,yPASS, RECUPE_OTHER_TSQUEST(xPASS, yPASS),color(0,61,0));
    PICOKIT.DirectDraw(xPASS,yPASS,  RECUPE_BALLISTIC_MAIN_TSQUEST(xPASS, yPASS),color(31,63,0));
    PICOKIT.DirectDraw(xPASS,yPASS,  RECUPE_BALLISTIC_OTHER_TSQUEST(xPASS, yPASS),color(20,40,20));
//  return (RECUPE_OTHER_TSQUEST(xPASS, yPASS) | RECUPE_BALLISTIC_MAIN_TSQUEST(xPASS, yPASS) | RECUPE_BALLISTIC_OTHER_TSQUEST(xPASS, yPASS));
return 0;
}

uint8_t Recupe_SUBsolo_TSQUEST(uint8_t xPASS, uint8_t yPASS) {
  if (yPASS == 1) return SPEED_BLITZ(SQUEST.SUBSOLO_X, 1, xPASS, yPASS, SQUEST.Frame, SUB2_TSQUEST);
  else { return 0x00; }
}

uint8_t Blink_Start_TSQUEST(uint8_t X_, uint8_t Y_, uint8_t BL_) {
  if (BL_ > 6) {
    return SPEED_BLITZ(38, 4, X_, Y_, 0, START_TSQUEST);
  } else {
    return 0x00;
  }
}
void RENDER_INTRO_TSQUEST( uint8_t BL_){
PICOKIT.FillScreenColor(color(0,0,19));
  uint8_t y, x;
  for (x = 0; x < 128; x++) {
   
  }
  for (y = 0; y < 8; y++) {

    for (x = 0; x < 128; x++) {
	
	   PICOKIT.DirectDraw(x,y,BACKGROUND_TSQUEST[x + ((y)*128)],color(16,48,26));
       PICOKIT.DirectDraw(x,y,SPEED_BLITZ(24, 2, x, y, 0, INTRO_TSQUEST),color(31,63,0));
       PICOKIT.DirectDraw(x,y,Blink_Start_TSQUEST(x,y,BL_),color(31,63,31));
    }
 }
PICOKIT.DisplayColor();
}


void RENDER_TSQUEST(void) {
  uint8_t y, x;
  for (y = 0; y < 8; y++) {
    RECUP_RANGE_TSQUEST(y);
    for (x = 0; x < 128; x++) {
		RecupeRanged_TSQUEST(x, y);
	   PICOKIT.DirectDraw(x,y, BACKGROUND_TSQUEST[x + ((y)*128)],color(16,48,26));
	   PICOKIT.DirectDraw(x,y, Recupe_SUBsolo_TSQUEST(x, y) ,color(24,48,24));
	   PICOKIT.DirectDraw(x,y,RECUPE_MAIN_TSQUEST(x, y),color(31,63,0));
       FullDisplayRefresh_TSQUEST(1,1,x,y);
    }
 }
PICOKIT.DisplayColor();
}

void RENDER_DISPLAY_TSQUEST(uint8_t FlipFlop1_, uint8_t FlipFlop2_) {
FlipFlop1=FlipFlop1_;
FlipFlop2=FlipFlop2_;
RENDER_TSQUEST();
J1Flip=0;
}

uint8_t Recupe_OX_TSQUEST(uint8_t Flip_, uint8_t xPASS) {
  uint8_t Tmp_ = (Flip_ == 0) ? 61 : SQUEST.OX;
  if ((xPASS > Tmp_) && (xPASS < 90)) {
    return 0b11001111;
  }
  return 0xff;
}

void Config_Display_Diver_TSQUEST(uint8_t *A_, uint8_t *B_) {
  if (SQUEST.Diver > 3) {
    *A_ = (3 * 11) + 1;
    *B_ = (((SQUEST.Diver - 3) * 11) + 91);
  } else {
    *A_ = ((SQUEST.Diver) * 11) - 1;
    *B_ = 70;
  }
}

uint8_t Counter_Display_Diver = 0;

uint8_t Recupe_Diver_TSQUEST(uint8_t xPASS) {
  if (SQUEST.Diver == 0) return 0xff;
  uint8_t ByTe_ = 0xff;
  uint8_t Dive_A;
  uint8_t Dive_B;

  Config_Display_Diver_TSQUEST(&Dive_A, &Dive_B);
  switch (xPASS) {
    case  0 ... 32 :
      if (xPASS > Dive_A) { return 0xff; }
    case 92 ... 124:
      if (xPASS > Dive_B) { return 0xff; }
      ByTe_ = Display_DIVER_TSQUEST[Counter_Display_Diver];
      Counter_Display_Diver = (Counter_Display_Diver < 10) ? Counter_Display_Diver + 1 : 0;
      return ByTe_;
      break;
    default: return 0xff;
  }
  return 0xff;
}

uint8_t Counter_Display_Live = 0;

uint8_t Recupe_LIVE_TSQUEST(uint8_t xPASS) {
  uint8_t ByTe_ = 0xff;
  if (SQUEST.Main_Live == 0) return 0xff;
  if (xPASS > (((SQUEST.Main_Live * 9) - 1))) return 0xff;
  ByTe_ = LIVE_TSQUEST[Counter_Display_Live];
  Counter_Display_Live = (Counter_Display_Live < 8) ? Counter_Display_Live + 1 : 0;
  return ByTe_;
}

uint8_t FullDisplayRefresh_TSQUEST(uint8_t FlipFlop1_, uint8_t FlipFlop2_, uint8_t xPASS, uint8_t yPASS) {
	//test
	if (yPASS==0) {
		PICOKIT.DirectDraw(xPASS,yPASS,~Recupe_LIVE_TSQUEST(xPASS),color(0,0,0));
	    PICOKIT.DirectDraw(xPASS,yPASS,recupe_SCORES_TSQUEST(xPASS, yPASS),color(31,63,31));
	}
	if (yPASS==7){
       if (SQUEST.OX_OSD_STATE) PICOKIT.DirectDraw(xPASS,yPASS,~Recupe_OX_TSQUEST(FlipFlop2_, xPASS),color(31,0,0));
	   if (SQUEST.DIVER_OSD_STATE) PICOKIT.DirectDraw(xPASS,yPASS,~Recupe_Diver_TSQUEST(xPASS),color(0,0,0));
	}
	return 0;
	//test
	
	
	/*
  switch (yPASS) {
    case 0: return (Recupe_LIVE_TSQUEST(xPASS)) & (0xff - recupe_SCORES_TSQUEST(xPASS, yPASS)); break;
    case 7: return (((FlipFlop1_ == 1) ? Recupe_Diver_TSQUEST(xPASS) : 0xff) & (Recupe_OX_TSQUEST(FlipFlop2_, xPASS))); break;
    default: return 0xFF;
  }*/
}

void COMPIL_SCO_TSQUEST(void) {
  M10000_TSQUEST = (SQUEST.Scores / 10000);
  M1000_TSQUEST = (((SQUEST.Scores) - (M10000_TSQUEST * 10000)) / 1000);
  M100_TSQUEST = (((SQUEST.Scores) - (M1000_TSQUEST * 1000) - (M10000_TSQUEST * 10000)) / 100);
  M10_TSQUEST = (((SQUEST.Scores) - (M100_TSQUEST * 100) - (M1000_TSQUEST * 1000) - (M10000_TSQUEST * 10000)) / 10);
  M1_TSQUEST = ((SQUEST.Scores) - (M10_TSQUEST * 10) - (M100_TSQUEST * 100) - (M1000_TSQUEST * 1000) - (M10000_TSQUEST * 10000));
}

uint8_t recupe_SCORES_TSQUEST(uint8_t xPASS, uint8_t yPASS) {

  return (SPEED_BLITZ(52, 0, xPASS, yPASS, M10000_TSQUEST, police_TSQUEST) | SPEED_BLITZ(56, 0, xPASS, yPASS, M1000_TSQUEST, police_TSQUEST) | SPEED_BLITZ(60, 0, xPASS, yPASS, M100_TSQUEST, police_TSQUEST) | SPEED_BLITZ(64, 0, xPASS, yPASS, M10_TSQUEST, police_TSQUEST) | SPEED_BLITZ(68, 0, xPASS, yPASS, M1_TSQUEST, police_TSQUEST) | SPEED_BLITZ(72, 0, xPASS, yPASS, 0, police_TSQUEST));
}
