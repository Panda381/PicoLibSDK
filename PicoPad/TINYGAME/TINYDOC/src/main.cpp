//       >>>>> T-I-N-Y  D-O-C for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  tiny-Doc is free software: you can redistribute it and/or modify
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


// The tiny-Doc source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "TinyDoc.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the TinyDoc.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#include "../include.h"

extern PICOCOMPATIBILITY PICOKIT;

//PUBLIC VARs
uint8_t FullRefresh_TD;
uint8_t GameOver_TD;
uint8_t TimerDrop_TD;
uint8_t STEPRIGHT_TD;
uint8_t STEPLEFT_TD;
uint8_t AnimPos_TD;
uint8_t PILLMODE_TD;
uint8_t FRMVIRUS_TD;
uint8_t ANIMSPEEDVIRUS_TD;
uint8_t ButtonTrig_TD;
uint8_t AutoTrigL_TD;
uint8_t AutoTrigR_TD;
uint8_t VirusTypeActif_TD[3];
unsigned long currentMillis_TD;
unsigned long MemMillis_TD;
uint8_t TAB_TD[8][10];
uint8_t BACKCHECK_TD[8][10];

uint8_t LAST_Total_Virus_left_TD;
uint8_t Total_Virus_left_TD;

uint8_t LvlDisplay1_TD;
uint8_t LvlDisplay10_TD;
uint8_t VDisplay1_TD;
uint8_t VDisplay10_TD;

uint16_t Score_TD;
uint8_t M10000_TD;
uint8_t M1000_TD;
uint8_t M100_TD;
uint8_t M10_TD;
uint8_t M1_TD;

static const uint8_t ConnectionCheck[5] = { 0, 2, 1, 4, 3 };  // 0:default, 1:left, 2:right, 3:up, 4:down

#define Xpos_TP 46
#define Ypos_TP 13

void FPS_Count_TD(uint8_t Ms_) {
  while ((currentMillis_TD - MemMillis_TD) < Ms_) {
    currentMillis_TD = millis();
  }
  MemMillis_TD = currentMillis_TD;
}

const size_t Size_TD = sizeof(RBBRVV) / sizeof(RBBRVV[0]);

typedef struct GAME_PLAY_VAR {
  uint8_t Speed_TD;
  uint8_t Tab_range;
  uint8_t Level_TD;
  uint8_t Total_Virus_In_level;
} GAME_PLAY_VAR;

typedef struct SPLITPILL_TD {
  uint8_t Stat;
} SPLITPILL_TD;

typedef struct PILL_TD {
  uint8_t Active;
  uint8_t InTab;
  int8_t GraphPosX;
  int8_t GraphPosY;
  uint8_t Gridx;
  uint8_t Gridy;
  uint8_t Vertical;
  SPLITPILL_TD PillPart1Preview;
  SPLITPILL_TD PillPart2Preview;
  SPLITPILL_TD PillPart1;
  SPLITPILL_TD PillPart2;
} PILL_TD;

typedef struct ELEMENT_TD {
  uint8_t CLR;
  uint8_t ScanHV;
  uint8_t ElementCount;
  uint8_t ElementStart;
  uint8_t ElementStop;
  uint8_t ElementType;
} ELEMENT_TD;

ELEMENT_TD Element_TD;
PILL_TD Pill_TD;
GAME_PLAY_VAR GameP_TD;

int main() {
	
  PICOKIT.Init();
  PICOKIT.SetColorBuffer(16,41,31,16,41,31,0,0,10);
  PICOKIT.SetFPS(33);
  
  uint8_t blk = 0;
  while (1) {  //New Game
    while (1) {
      rand();
      IntroFlip_TD((blk < 16) ? 1 : 0);
      if (BUTTON_DOWN) break;
      blk = (blk < 33) ? blk + 1 : 0;
      FPS_Count_TD(33);
    }
    SND_TDOC_TD(6);
    init_public_Var_For_New_game_TD();
NextLevel_TD:;
    init_public_Var_For_New_Level_TD();
    AdjustGamePlay();
    init_RND_TD();
    Virus_Loupe_TD();
    FirstCalculeDisplay_TD();
    InitNewPill_TD(1);
    TinyFlip_TD();
    _delay_ms(1000);
    while (1) {  //In game
      if (PILLMODE_TD == 4) { TINYJOYPAD_UPDATE_TD(); }
      PillProcess_TD();
      Virus_Loupe_TD();
      COMPIL_SCORE_TD();
      TinyFlip_TD();
      FPS_Count_TD(33);
      if (ANIMSPEEDVIRUS_TD < 2) {
        ANIMSPEEDVIRUS_TD++;
      } else {
        FRMVIRUS_TD = (FRMVIRUS_TD < 2) ? FRMVIRUS_TD + 1 : 0;
        ANIMSPEEDVIRUS_TD = 0;
      }
      if (GameOver_TD == 1) break;
      if (Total_Virus_left_TD == 0) {
        LevelClearedFlip_TD();
        SND_TDOC_TD(1);
        while (!BUTTON_DOWN) {
        }
        GameP_TD.Level_TD++;
        goto NextLevel_TD;
      }
    }
    SND_TDOC_TD(5);
    InitNewPill_TD(0);
    _delay_ms(2000);
  }
}

void Compile_LV_Display_TD(uint8_t *A10_, uint8_t *B1_, uint8_t NUMBER_) {
  *A10_ = NUMBER_ / 10;  // Dizaines
  *B1_ = NUMBER_ % 10;   // Unités
}

void AdjustGamePlay(void) {
  if (GameP_TD.Level_TD > 20) { GameP_TD.Level_TD = 20; }
  Total_Virus_left_TD = LAST_Total_Virus_left_TD = GameP_TD.Total_Virus_In_level = ((GameP_TD.Level_TD * 2) + 4);
  GameP_TD.Speed_TD = Mymap(GameP_TD.Level_TD, 0, 20, 25, 4);
  GameP_TD.Tab_range = Mymap(GameP_TD.Level_TD, 0, 20, 3, 8);
}

void FirstCalculeDisplay_TD(void) {
  Score_TD = Score_TD + ((LAST_Total_Virus_left_TD - Total_Virus_left_TD) * Mymap(GameP_TD.Level_TD,0,20,20,60));
  LAST_Total_Virus_left_TD = Total_Virus_left_TD;
  FullRefresh_TD = 1;

  M10000_TD = Score_TD / 10000;       // Dizaines de milliers
  M1000_TD = (Score_TD / 1000) % 10;  // Milliers
  M100_TD = (Score_TD / 100) % 10;    // Centaines
  M10_TD = (Score_TD / 10) % 10;      // Dizaines
  M1_TD = Score_TD % 10;              // Unités

  Compile_LV_Display_TD(&LvlDisplay10_TD, &LvlDisplay1_TD, GameP_TD.Level_TD);
  Compile_LV_Display_TD(&VDisplay10_TD, &VDisplay1_TD, Total_Virus_left_TD);
}

void COMPIL_SCORE_TD(void) {
  if (LAST_Total_Virus_left_TD != Total_Virus_left_TD) {
    FirstCalculeDisplay_TD();
  }
}

void Virus_Loupe_TD(void) {
  VirusTypeActif_TD[0] = 0;
  VirusTypeActif_TD[1] = 0;
  VirusTypeActif_TD[2] = 0;
  for (uint8_t y = 0; y < 10; y++) {
    for (uint8_t x = 0; x < 8; x++) {
      switch (TAB_TD[x][y]) {
        case 1: VirusTypeActif_TD[0]++; break;
        case 2: VirusTypeActif_TD[1]++; break;
        case 3: VirusTypeActif_TD[2]++; break;
        default: break;
      }
    }
  }
  Total_Virus_left_TD = VirusTypeActif_TD[0] + VirusTypeActif_TD[1] + VirusTypeActif_TD[2];
}


void SND_TDOC_TD(uint8_t Snd_) {
  uint8_t q;
  switch (Snd_) {
    case 0: Sound(200, 1); break;
    case 1:
      for (q = 1; q < 14; q++) {
        Sound(4, 80);
        Sound(100, 80);
      }
      _delay_ms(1000);
      break;
    case 2:
      Sound(240, 4);
      Sound(100, 4);
      break;
    case 3: Sound(2, 1); break;
    case 4: Sound(10, 12); break;
    case 5:
      for (q = 200; q > 10; q--) {
        Sound(200 - q, 3);
        Sound(q, 12);
      }
      break;
    case 6:
      Sound(100, 255);
      Sound(60, 255);
      break;
    default: break;
  }
}

void init_public_Var_For_New_game_TD(void) {
  GameP_TD.Speed_TD = 0;
  GameP_TD.Tab_range = 0;
  GameP_TD.Level_TD = 0;
  Score_TD = 0;
  GameOver_TD = 0;
  M10000_TD = 0;
  M1000_TD = 0;
  M100_TD = 0;
  M10_TD = 0;
  M1_TD = 0;
}

void init_public_Var_For_New_Level_TD(void) {
  FullRefresh_TD = 1;
  TimerDrop_TD = 0;
  STEPRIGHT_TD = 0;
  STEPLEFT_TD = 0;
  AnimPos_TD = 0;
  PILLMODE_TD = 0;
  FRMVIRUS_TD = 1;
  ANIMSPEEDVIRUS_TD = 0;
  ButtonTrig_TD = 0;
  AutoTrigL_TD = 0;
  AutoTrigR_TD = 0;
  memset(BACKCHECK_TD, 0, sizeof(BACKCHECK_TD));
  memset(TAB_TD, 0, sizeof(TAB_TD));
  memset(VirusTypeActif_TD, 0, sizeof(VirusTypeActif_TD));
  LAST_Total_Virus_left_TD = 0;
  Total_Virus_left_TD = 0;
  GameP_TD.Total_Virus_In_level = 0;
  Pill_TD.PillPart1Preview.Stat = 0;
  Pill_TD.PillPart2Preview.Stat = 0;
}

void TINYJOYPAD_UPDATE_TD(void) {
  if (BUTTON_DOWN) {
    _delay_ms(4);
    if (BUTTON_DOWN) {
      if (ButtonTrig_TD == 1) {
        RotatePill_TD();
        ButtonTrig_TD = 0;
      }
    }
  } else {
    ButtonTrig_TD = 1;
  }

  if (TINYJOYPAD_RIGHT) {
    _delay_ms(12);
    if (TINYJOYPAD_RIGHT) {
      if (AutoTrigR_TD > 0) {
        AutoTrigR_TD--;
      } else {
        AutoTrigR_TD = 1;
        STEPRIGHT_TD = 0;
      }
      if (STEPRIGHT_TD == 0) {
        if (!CheckColision_TD(Pill_TD.Gridx + 1, Pill_TD.Gridy)) {
          STEPRIGHT_TD = 1;
          Pill_TD.Gridx++;
          SND_TDOC_TD(0);
        }
      }
    }
  } else {
    STEPRIGHT_TD = 0;
    AutoTrigR_TD = 5;
  }

  if (TINYJOYPAD_LEFT) {
    _delay_ms(12);
    if (TINYJOYPAD_LEFT) {
      if (AutoTrigL_TD > 0) {
        AutoTrigL_TD--;
      } else {
        AutoTrigL_TD = 1;
        STEPLEFT_TD = 0;
      }
      if (STEPLEFT_TD == 0) {
        if (!CheckColision_TD(Pill_TD.Gridx - 1, Pill_TD.Gridy)) {
          STEPLEFT_TD = 1;
          Pill_TD.Gridx--;
          SND_TDOC_TD(0);
        }
      }
    }
  } else {
    AutoTrigL_TD = 5;
    STEPLEFT_TD = 0;
  }
  // if (TINYJOYPAD_DOWN == 0) FreezeDrop_TD = 0;
  if ((TimerDrop_TD < ((TINYJOYPAD_DOWN) ? /*((FreezeDrop_TD) ? 25 :*/ 0 /*)*/ : GameP_TD.Speed_TD))) {
    TimerDrop_TD++;
  } else {
    if (!CheckColision_TD(Pill_TD.Gridx, Pill_TD.Gridy + 1)) {
      Pill_TD.Gridy++;
      SND_TDOC_TD(3);
    } else {
      FixPill_TD();
      //  FreezeDrop_TD = 1;
      do {
        CheckCompletedLine_TD();
      } while (DropPills_TD());
      PILLMODE_TD = 1;
    }
    TimerDrop_TD = 0;
  }
}

void InitNewBackTab_TD(void) {
  Element_TD.CLR = 0;
  Element_TD.ScanHV = 0;
  Element_TD.ElementCount = 0;
  Element_TD.ElementStart = 0;
  Element_TD.ElementStop = 0;
  Element_TD.ElementType = 0xFF;
  memset(BACKCHECK_TD, 0, sizeof(BACKCHECK_TD));
}

void CopyItem2Delete_TD(uint8_t x_, uint8_t y_) {
  Element_TD.CLR = 1;
  uint8_t start = Element_TD.ElementStart;
  uint8_t stop = Element_TD.ElementStop + 1;
  if (Element_TD.ScanHV == HORIZONTAL_TD) {
    while (start < stop) BACKCHECK_TD[start++][y_] = 20;
  } else {
    while (start < stop) BACKCHECK_TD[x_][start++] = 20;
  }
}

void NewStepLine_TD(void) {
  Element_TD.ElementCount = 0;
  Element_TD.ElementStart = 0;
  Element_TD.ElementStop = 0;
  Element_TD.ElementType = 0xFF;
}

void ElementCounter_TD(uint8_t x_, uint8_t y_, uint8_t Element_) {
  uint8_t Pos_ = (Element_TD.ScanHV == HORIZONTAL_TD) ? x_ : y_;
  uint8_t EndLine = (Element_TD.ScanHV == HORIZONTAL_TD) ? 7 : 9;

  if ((Element_TD.ElementType != Element_) || (Element_ == 0)) {
    if (Element_TD.ElementCount > 3) CopyItem2Delete_TD(x_, y_);
    Element_TD.ElementType = Element_;
    Element_TD.ElementCount = 1;
    Element_TD.ElementStart = Pos_;
  } else {
    Element_TD.ElementCount++;
    Element_TD.ElementStop = Pos_;
    if (Pos_ == EndLine && Element_TD.ElementCount > 3) {
      CopyItem2Delete_TD(x_, y_);
      NewStepLine_TD();
    }
  }
}

void CheckCompletedLine_TD(void) {
  InitNewBackTab_TD();
  
  Element_TD.ScanHV = HORIZONTAL_TD;
  for (uint8_t y = 0; y < 10; y++) {
    NewStepLine_TD();
    for (uint8_t x = 0; x < 8; x++) {
      ElementCounter_TD(x, y, ReturnType_TD(TAB_TD[x][y]));
    }
  }

  Element_TD.ScanHV = VERTICAL_TD;
  for (uint8_t x = 0; x < 8; x++) {
    NewStepLine_TD();
    for (uint8_t y = 0; y < 10; y++) {
      ElementCounter_TD(x, y, ReturnType_TD(TAB_TD[x][y]));
    }
  }
  if (Element_TD.CLR) ClearLine_TD();
}

void ClearLine_TD(void) {
  uint8_t *back_ptr = (uint8_t *)BACKCHECK_TD;
  uint8_t *tab_ptr = (uint8_t *)TAB_TD;
  for (uint8_t Anim = 4; Anim < 10; Anim++) {
    for (uint8_t i = 0; i < 80; i++) {
      if (back_ptr[i] == 20) tab_ptr[i] = Anim;
    }
    TinyFlip_TD();
    FPS_Count_TD(40);
    SND_TDOC_TD(2);
  }
  for (uint8_t i = 0; i < 80; i++) {
    if (back_ptr[i] == 20) tab_ptr[i] = 0;
  }

  TinyFlip_TD();
  FPS_Count_TD(40);
  Element_TD.CLR = 0;
  SetSinglePill_TD();
}

uint8_t ReturnConectionType_TD(uint8_t Type_) {
  return Type_ >> 4;
}

uint8_t ReturnType_TD(uint8_t Val_) {
  uint8_t Skin = Val_ & 0x0F;
  return (Skin < 10) ? Skin : Skin - 10;
}

uint8_t CheckMatch_TD(int8_t x_, int8_t y_) {
  return (x_ >= 0 && x_ <= 7 && y_ >= 0 && y_ <= 9) ? TAB_TD[x_][y_] : 0;
}

uint8_t ReturnCombinedPillDirection_TD(int8_t x_, int8_t y_, uint8_t Type_) {
  uint8_t dir = Type_ >> 4;
  if (dir == 0) return 0;
  int8_t dx = (dir == 1) ? -1 : (dir == 2) ? 1
                                           : 0;
  int8_t dy = (dir == 3) ? -1 : (dir == 4) ? 1
                                           : 0;
  return (ReturnConectionType_TD(CheckMatch_TD(x_ + dx, y_ + dy)) != ConnectionCheck[dir]) ? 1 : 0;
}

void SetSinglePill_TD(void) {
  for (uint8_t y = 0; y < 10; y++) {
    for (uint8_t x = 0; x < 8; x++) {
      if (ReturnCombinedPillDirection_TD(x, y, TAB_TD[x][y])) TAB_TD[x][y] &= 0x0F;
    }
  }
}

void AnimPillShoot_TD(void) {
  Pill_TD.GraphPosX = Anim_TD[AnimPos_TD * 2];
  Pill_TD.GraphPosY = Anim_TD[(AnimPos_TD * 2) + 1];
  if (AnimPos_TD < 10) {
    AnimPos_TD++; /*Pill.Vertical=!Pill.Vertical;*/
  } else {
    if ((TAB_TD[Pill_TD.Gridx][Pill_TD.Gridy] != 0) || (TAB_TD[Pill_TD.Gridx + 1][Pill_TD.Gridy] != 0)) { GameOver_TD = 1; }
    GeneratenewPreviewPill_TD();
    Pill_TD.InTab = 1;
    AnimPos_TD = 0;
    FullRefresh_TD = 1;
    PILLMODE_TD++;
  }
}

void DropMode_TD(void) {
  if (Pill_TD.Active == 0) { PILLMODE_TD = 1; }
}

void PillProcess_TD(void) {
  switch (PILLMODE_TD) {
    case 0:
      GeneratenewPreviewPill_TD();
      InitNewPill_TD(1);
      AnimPillShoot_TD();
      PILLMODE_TD = 3;
      break;
    case 1:
      InitNewPill_TD(1);
      AnimPillShoot_TD();
      PILLMODE_TD = 3;
      break;
    case 2: PILLMODE_TD++; break;
    case 3: AnimPillShoot_TD(); break;
    case 4: DropMode_TD(); break;
  }
}

uint8_t DropPills_TD(void) {
  uint8_t Repeat_ = 0;
  for (uint8_t y = 8; y < 255; y--) {
    for (uint8_t x = 0; x < 8; x++) {
      if ((TAB_TD[x][y] & 0x0F) > 4) {
        if (ReturnConectionType_TD(TAB_TD[x][y]) == 0) {
          if (TAB_TD[x][y + 1] == 0) {
            TAB_TD[x][y + 1] = TAB_TD[x][y];
            TAB_TD[x][y] = 0;
            Repeat_ = 1;
          }
        } else {
          switch (ReturnConectionType_TD(TAB_TD[x][y])) {
            case (2):
              if ((TAB_TD[x][y + 1] == 0) && (TAB_TD[x + 1][y + 1] == 0)) {
                TAB_TD[x][y + 1] = TAB_TD[x][y];
                TAB_TD[x + 1][y + 1] = TAB_TD[x + 1][y];
                TAB_TD[x][y] = 0;
                TAB_TD[x + 1][y] = 0;
                Repeat_ = 1;
              }
              break;
            case (3):
              if (TAB_TD[x][y + 1] == 0) {
                TAB_TD[x][y + 1] = TAB_TD[x][y];
                TAB_TD[x][y] = TAB_TD[x][y - 1];
                TAB_TD[x][y - 1] = 0;
                Repeat_ = 1;
              }
              break;
            default: break;
          }
        }
      }
    }
  }
  TinyFlip_TD();
  FPS_Count_TD(66);
  return Repeat_;
}

uint8_t GenerateSidePill_TD(uint8_t Side) {
  uint8_t ColPill = ((rand() % 3) + 1) + 10;
  uint8_t SidePill = Side << 4;
  return ColPill | SidePill;
}

void GeneratenewPreviewPill_TD(void) {
  Pill_TD.PillPart1Preview.Stat = GenerateSidePill_TD(2);
  Pill_TD.PillPart2Preview.Stat = GenerateSidePill_TD(1);
}

void InitNewPill_TD(uint8_t Act_) {
  Pill_TD.Active = Act_;
  Pill_TD.Vertical = 0;
  Pill_TD.InTab = 0;
  Pill_TD.GraphPosX = -100;
  Pill_TD.GraphPosY = -100;
  Pill_TD.Gridx = 3;
  Pill_TD.Gridy = 0;
  Pill_TD.PillPart1.Stat = Pill_TD.PillPart1Preview.Stat;
  Pill_TD.PillPart2.Stat = Pill_TD.PillPart2Preview.Stat;
}

uint8_t Select_TD(void) {
  uint8_t Rnd = rand() % 3;
  switch (Rnd) {
    case 0: return 1;
    case 1: return 2;
    case 2: return 3;
    default: return 0;
  }
}

uint8_t OrderSelect_TD(uint8_t Select_, uint8_t Chiffre_) {
  return SpriteOrder_TD[(Select_ * 3) + Chiffre_];
}

uint8_t GetElement_TD(size_t index_) {
  return RBBRVV[index_ % Size_TD];
}

void init_RND_TD(void) {
  uint8_t VirusCount = 0;
  uint8_t SetSelect = rand() % 3;
  uint8_t x;
  uint8_t y;
  while (1) {
    x = rand() % 8;
    y = 10 - ((rand() % GameP_TD.Tab_range) + 1);
    if (TAB_TD[x][y] == 0) {
      TAB_TD[x][y] = OrderSelect_TD(SetSelect, GetElement_TD(x + (y * 8)));
      VirusCount++;
    }
    if (VirusCount == GameP_TD.Total_Virus_In_level) break;
  }
}

uint8_t CheckRotateIsPosible_TD(void) {
  if (Pill_TD.Vertical) {
    Pill_TD.Vertical = 0;
    if (CheckColision_TD(Pill_TD.Gridx, Pill_TD.Gridy)) {
      if (!CheckColision_TD(Pill_TD.Gridx - 1, Pill_TD.Gridy)) {
        Pill_TD.Gridx -= 1;
        Pill_TD.Vertical = 1;
        return 1;
      }
      Pill_TD.Vertical = 1;
      return 0;
    }
    Pill_TD.Vertical = 1;
    return 1;
  } else {
    Pill_TD.Vertical = 1;
    if (CheckColision_TD(Pill_TD.Gridx, Pill_TD.Gridy)) {
      Pill_TD.Vertical = 0;
      return 0;
    }
    Pill_TD.Vertical = 0;
    return 1;
  }
}

void RotatePill_TD(void) {
  if (!CheckRotateIsPosible_TD()) return;
  uint8_t Mem = Pill_TD.PillPart1.Stat;
  if (Pill_TD.Vertical) {
    Pill_TD.Vertical = 0;
  } else {
    Pill_TD.Vertical = 1;
    Pill_TD.PillPart1.Stat = Pill_TD.PillPart2.Stat;
    Pill_TD.PillPart2.Stat = Mem;
  }
  SND_TDOC_TD(0);
  SetPicDirection_TD(Pill_TD.Vertical);
}

void SetPicDirection_TD(uint8_t Vertical_) {
  if (Vertical_) {
    Pill_TD.PillPart1.Stat = ((Pill_TD.PillPart1.Stat & 0x0f) | (3 << 4));
    Pill_TD.PillPart2.Stat = (Pill_TD.PillPart2.Stat & 0x0f) | (4 << 4);
  } else {
    Pill_TD.PillPart1.Stat = (Pill_TD.PillPart1.Stat & 0x0f) | (2 << 4);
    Pill_TD.PillPart2.Stat = (Pill_TD.PillPart2.Stat & 0x0f) | (1 << 4);
  }
}

void FixPill_TD(void) {
  if (Pill_TD.Vertical) {
    if ((Pill_TD.Gridy - 1) >= 0) {
      TAB_TD[Pill_TD.Gridx][Pill_TD.Gridy] = Pill_TD.PillPart1.Stat;
      TAB_TD[Pill_TD.Gridx][Pill_TD.Gridy - 1] = Pill_TD.PillPart2.Stat;
    } else {
      TAB_TD[Pill_TD.Gridx][Pill_TD.Gridy] = Pill_TD.PillPart1.Stat & 0x0f;
    }
  } else {
    TAB_TD[Pill_TD.Gridx][Pill_TD.Gridy] = Pill_TD.PillPart1.Stat;
    TAB_TD[Pill_TD.Gridx + 1][Pill_TD.Gridy] = Pill_TD.PillPart2.Stat;
  }
  Pill_TD.Active = 0;
  SND_TDOC_TD(4);
}

uint8_t CheckColision_TD(int8_t X_, int8_t Y_) {
  if (X_ < 0) return 1;
  if (X_ > 7) return 1;
  if (Y_ > 9) return 1;
  if (Pill_TD.Vertical) {
    if (TAB_TD[X_][Y_]) return 1;
    if ((Y_ - 1) < 0) {
      return 0;
    } else if ((TAB_TD[X_][Y_ - 1])) {
      return 1;
    }
  } else {
    if (TAB_TD[X_][Y_]) return 1;
    if ((TAB_TD[X_ + 1][Y_]) || ((X_) == 7)) return 1;
  }
  return 0;
}

void ReturnScanLineY_TD(uint8_t *a, uint8_t *b, uint8_t y) {
  switch (y) {
    case 1:
      *a = 0;
      *b = 1;
      return;
    case 2:
      *a = 0;
      *b = 2;
      return;
    case 3:
      *a = 2;
      *b = 3;
      return;
    case 4:
      *a = 4;
      *b = 5;
      return;
    case 5:
      *a = 5;
      *b = 6;
      return;
    case 6:
      *a = 7;
      *b = 8;
      return;
    case 7:
      *a = 8;
      *b = 9;
      return;
    default: return;
  }
}

void ReturnScanLineX_TD(uint8_t *a, uint8_t x) {
  switch (x) {
    case 46 ... 50: *a = 0; return;
    case 51 ... 54: *a = 1; return;
    case 56 ... 59: *a = 2; return;
    case 61 ... 64: *a = 3; return;
    case 66 ... 69: *a = 4; return;
    case 71 ... 74: *a = 5; return;
    case 76 ... 79: *a = 6; return;
    case 81 ... 84: *a = 7; return;
    default: return;
  }
}

uint8_t ReturncorectPills_TD(uint8_t Val_) {
  uint8_t Val_Low = Val_ & 0x0f;
  uint8_t Val_High = (Val_ >> 4);
  if (Val_High == 0) {
    switch (Val_Low) {
      case 1: 
      case 2:
      case 3: return 1;
      case 11: 
      case 12: 
      case 13: return 2;
      case 4: return 15;
      case 5: return 16;
      case 6: return 15;
      case 7: return 16;
      case 8: return 0;
      case 9: return 17;
      case 16: return 17;
      default: break;
    }
  } else {
    switch (Val_Low) {
      case 11: return 11 + (Val_High - 1);
      case 12: return 11 + (Val_High - 1);
      case 13: return 11 + (Val_High - 1);
      default: break;
    }
  }
  return 0;
}

uint8_t DrawNewPill_TD(uint8_t x, uint8_t y) {
  uint8_t Byteout = 0;
  if ((x < 46) || (x > 108)) return 0;
  if (Pill_TD.Active) {
    if (Pill_TD.InTab) {
      uint8_t XX = Xpos_TP + (Pill_TD.Gridx * 5);
      uint8_t YY = Ypos_TP + (Pill_TD.Gridy * 5);
      PICOKIT.DirectDraw(x,y,blitzSprite(XX, YY, x, y, ReturncorectPills_TD(Pill_TD.PillPart1.Stat), pills_TD),ReturnCorectColorPill(Pill_TD.PillPart1.Stat));
      if (Pill_TD.Vertical) {
		  PICOKIT.DirectDraw(x,y,blitzSprite(XX, YY - 5, x, y, ReturncorectPills_TD(Pill_TD.PillPart2.Stat), pills_TD),ReturnCorectColorPill(Pill_TD.PillPart2.Stat));
      } else {
		   PICOKIT.DirectDraw(x,y,blitzSprite(XX + 5, YY, x, y, ReturncorectPills_TD(Pill_TD.PillPart2.Stat), pills_TD),ReturnCorectColorPill(Pill_TD.PillPart2.Stat));
      }
    } else {
		PICOKIT.DirectDraw(x,y,blitzSprite(Pill_TD.GraphPosX, Pill_TD.GraphPosY, x, y, ReturncorectPills_TD(Pill_TD.PillPart1.Stat), pills_TD),ReturnCorectColorPill(Pill_TD.PillPart1.Stat));
      if (Pill_TD.Vertical) {
		  PICOKIT.DirectDraw(x,y,blitzSprite(Pill_TD.GraphPosX, Pill_TD.GraphPosY - 5, x, y, ReturncorectPills_TD(Pill_TD.PillPart2.Stat), pills_TD),ReturnCorectColorPill(Pill_TD.PillPart2.Stat));
      } else {
		  PICOKIT.DirectDraw(x,y,blitzSprite(Pill_TD.GraphPosX + 5, Pill_TD.GraphPosY, x, y, ReturncorectPills_TD(Pill_TD.PillPart2.Stat), pills_TD),ReturnCorectColorPill(Pill_TD.PillPart2.Stat));
      }
    }
  }
  return Byteout;
}

uint8_t SwitchRecupVirus_TD(uint8_t Type_) {
  switch (Type_) {
    case 1 ... 3: return FRMVIRUS_TD;
    default: break;
  }
  return 1;
}


uint16_t ReturnCorectColorPill(uint8_t Val_){
	uint8_t TP_VAL=Val_&0x0f;
	switch(TP_VAL){
		case 1:
		case 11:return color(31,0,0);break;
		case 2:
		case 12:return color(31,63,0);break;
		case 3:
		case 13:return color(0,63,31);break;
		default:break;
	
	}	return color(31,63,31);;
}

uint8_t DrawTAB_TP(uint8_t x, uint8_t y) {
  if (y < 1) return 0;
  if ((x < 46) || (x > 84)) return 0;
  uint8_t Byteout = 0;
  uint8_t aa = 0;
  uint8_t bb = 0;
  uint8_t xx = 0;
  ReturnScanLineY_TD(&aa, &bb, y);
  ReturnScanLineX_TD(&xx, x);
  for (uint8_t yy = aa; yy != (bb + 1); yy++) {
    if (TAB_TD[xx][yy] != 0) {
		PICOKIT.DirectDraw(x,y,blitzSprite(Xpos_TP + (xx * 5), Ypos_TP + (yy * 5), x, y, ReturncorectPills_TD(TAB_TD[xx][yy]), pills_TD),ReturnCorectColorPill(TAB_TD[xx][yy]));
		PICOKIT.DirectDraw(x,y,blitzSprite(Xpos_TP + (xx * 5), Ypos_TP + (yy * 5), x, y, SwitchRecupVirus_TD(TAB_TD[xx][yy]), virus_TD),ReturnCorectColorPill(TAB_TD[xx][yy]));
    }
  }
  return Byteout;
}

uint8_t DrawPreviewPill_TD(uint8_t x, uint8_t y) {
  if (PILLMODE_TD != 4) return 0;
  if (y != 2) return 0;
  if (x < 95) return 0;
  if (x > 108) return 0;
  uint8_t Byteout = 0x00;
 PICOKIT.DirectDraw(x,y,blitzSprite(97, 18, x, y, ReturncorectPills_TD(Pill_TD.PillPart1Preview.Stat), pills_TD),ReturnCorectColorPill(Pill_TD.PillPart1Preview.Stat));
 PICOKIT.DirectDraw(x,y,blitzSprite(102, 18, x, y, ReturncorectPills_TD(Pill_TD.PillPart2Preview.Stat), pills_TD),ReturnCorectColorPill(Pill_TD.PillPart2Preview.Stat));
  return Byteout;
}

uint8_t DrawShadowPreviewPill_TD(uint8_t x, uint8_t y) {
  uint8_t Byteout = 0x00;
  if (y > 2) return Byteout;
  if (PILLMODE_TD == 3) {
    Byteout = blitzSprite(Pill_TD.GraphPosX - 1, Pill_TD.GraphPosY - 1, x, y, 0, SHADOW_TD);
  }
  return Byteout;
}

uint8_t DrawLoupe_TD(uint8_t x, uint8_t y) {
  uint8_t Byteout = 0x00;
  if (x < 12) return 0;
  if (x > 29) return 0;
  if (y < 4) return 0;
  if (y > 6) return 0;
  if (VirusTypeActif_TD[0])  PICOKIT.DirectDraw(x,y, blitzSprite(15, 35, x, y, 3 + FRMVIRUS_TD, VirusLoupe_TD),ReturnCorectColorPill(1));
  if (VirusTypeActif_TD[1])  PICOKIT.DirectDraw(x,y, blitzSprite(23, 42, x, y, 3 + FRMVIRUS_TD, VirusLoupe_TD),ReturnCorectColorPill(2));
  if (VirusTypeActif_TD[2])  PICOKIT.DirectDraw(x,y, blitzSprite(12, 45, x, y, 3 + FRMVIRUS_TD, VirusLoupe_TD),ReturnCorectColorPill(3));
  return Byteout;
}

uint8_t Draw_Score_TD(uint8_t x, uint8_t y) {
  uint8_t Byteout = 0x00;
  if (x < 10) return 0;
  if (x > 28) return 0;
  if (y != 1) return 0;
  Byteout |= blitzSprite(10, 11, x, y, M10000_TD, police_TD);
  Byteout |= blitzSprite(14, 11, x, y, M1000_TD, police_TD);
  Byteout |= blitzSprite(18, 11, x, y, M100_TD, police_TD);
  Byteout |= blitzSprite(22, 11, x, y, M10_TD, police_TD);
  Byteout |= blitzSprite(26, 11, x, y, M1_TD, police_TD);
  return Byteout;
}

void IntroFlip_TD(uint8_t Show_) {
  for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 0; x < 128; x++) {
PICOKIT.TinyOLED_Send(x,y,((Show_)?~SPEED_BLITZ(41, 6, x, y, 0, shad_TD):0xff)&Introgame_TD[x + (y * 128)]);
    }
  }
PICOKIT.display();
}

uint8_t DrawDisplayLV_TD(uint8_t x, uint8_t y) {
  uint8_t Byteout = 0x00;
  if (x < 109) return 0;
  if (x > 115) return 0;
  if ((y < 5) || (y > 6)) return 0;
  Byteout |= blitzSprite(109, 41, x, y, LvlDisplay10_TD, police_TD);
  Byteout |= blitzSprite(113, 41, x, y, LvlDisplay1_TD, police_TD);
  Byteout |= blitzSprite(109, 50, x, y, VDisplay10_TD, police_TD);
  Byteout |= blitzSprite(113, 50, x, y, VDisplay1_TD, police_TD);
  return Byteout;
}

void TinyFlip_TD(void) {
  for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 0; x < 128; x++) {
    PICOKIT.TinyOLED_Send(x,y, ((~DrawShadowPreviewPill_TD(x, y) & background_TD[x + (y * 128)])));
    }
  }
  PICOKIT.DrawSSDBuffer();
    for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 0; x < 128; x++) {
PICOKIT.DirectDraw(x,y,DrawDisplayLV_TD(x, y),color(31,63,31));
 PICOKIT.DirectDraw(x,y,Draw_Score_TD(x, y),color(31,63,31));
   DrawLoupe_TD(x, y);
   DrawPreviewPill_TD(x, y);
   DrawTAB_TP(x, y);
   DrawNewPill_TD(x, y);
    }
  }
  
PICOKIT.DisplayColor();
  FullRefresh_TD = 0;
}

void LevelClearedFlip_TD(void) {
  for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 0; x < 128; x++) {	
PICOKIT.TinyOLED_Send(x,y, (background_TD[x + (y * 128)]));
    }
  }
    PICOKIT.DrawSSDBuffer();
    for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 0; x < 128; x++) {	
  PICOKIT.DirectDraw(x,y,DrawDisplayLV_TD(x, y),color(31,63,31));
 PICOKIT.DirectDraw(x,y,Draw_Score_TD(x, y),color(31,63,31));
  PICOKIT.DirectDraw(x,y,SPEED_BLITZ(51, 2, x, y, 0, LevelCleared_TD),color(31,63,31));
 DrawLoupe_TD(x, y);
}}
PICOKIT.DisplayColor();
}
