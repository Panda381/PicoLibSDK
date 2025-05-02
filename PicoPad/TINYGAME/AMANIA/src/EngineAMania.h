//   >>>>> A-R-D-U-M-A-N-I-A for PicoPad RP2040 & RP2350+  GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Ardumania is free software: you can redistribute it and/or modify
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


// The Ardumania source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "AMania.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the AMania.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

// If recompiling for RP2040, uncomment this line: //#define OnlyRP2040 in the PICOKIT.cpp file!

#ifndef ENGINEAMANIA_H
#define ENGINEAMANIA_H

//#include "../include.h"

// Déclarations externes
extern ALVL Level;

// Prototypes de fonction
void InitCamGrid(void);
void loop_ARDUMANIA(void);
uint16_t ArduMap(float x, float in_min, float in_max, float out_min, float out_max);
void InitFirstBoot(void);
void InitFrameRepeat(void);
void InitNewGame(void);
void InitResetPos(void);
void InitHSVar(void);
void InitGamePlay(uint8_t Lvl_);
void FILLSCREENSHADE(void);
void GreenOn(void);
void GreenOff(void);
void RedOn(void);
void RedOff(void);
void LEDOFF(void);
void Snd_Drive(void);
void Delay_SND(uint16_t Tm_);
void FakeLoad(void);
void BootIntro(void);
void Fail(void);
void DrawBar(uint8_t Scr);
void DrawArduMania(uint8_t Scr);
void Menu(void);
void ForcedSaveEEPROM(void);
void CheckNewHighScore(void);
void ClassementHighScore(void);
uint8_t READEEPROM(uint8_t Adress_);
uint8_t CheckMarqueurEEPROM(void);
void SaveMarqueurEEPROM(void);
void LoadSlotEEPROM(uint8_t Slot_);
void SaveSlotEEPROM(uint8_t Slot_, uint8_t Avatar_, uint16_t Valeur);
void LoadHighScoreData(void);
void RestHighScore(void);
void ScoreMenu(void);
void AnimLvlChange(void);
void ScorePannel(uint8_t x_, uint8_t y_);
void Pannel(void);
void CalculateNum(void);
void CheckGob(uint8_t a, uint8_t b);
void ActivateHighSpeed(void);
void ProgramExitMode(uint8_t EXITMODE, uint8_t FADE_INOUT);
uint8_t ExitTime(void);
void FrameRedundancy(void);
void Fade_SCR(void);
void GobFruit(void);
void ONEUP(void);
void UpdateFruitDelivery(void);
void LiberateFruit(void);
void Snd(uint8_t f_, uint8_t D_);
void SoundSystem(uint8_t Val_);
void deadSound(void);
void GhostJumpCalculate(void);
uint8_t AnimateDeadMania(void);
void RCupDeadAnim(void);
void RefreshMovingArdu(void);
void CheckPadFunction(void);
uint8_t MaskUse(void);
void LoopScreen(int8_t* x_);
uint8_t GhostTime(uint8_t t_);
void RefreshTimers(void);
void RemoveDot(void);
void SwitchModeGhost(uint8_t Tested_, uint8_t Set_);
void GhostUturn(uint8_t t);
uint8_t collision(void);
void GhostRespawn(uint8_t val_);
void GhostDirectionChoser(void);
uint8_t RNDGhost(uint8_t t_);
uint8_t TrackObjectifX(uint8_t Val_);
uint8_t TrackObjectifY(uint8_t Val_);
uint8_t GateExit(uint8_t Val_);
void GhostRight(uint8_t t);
void GhostLeft(uint8_t t);
void GhostUp(uint8_t t);
void GhostDown(uint8_t t);
void RefreshMovingGhost(void);
uint8_t RandVar(void);
uint8_t FruitActif(int8_t x_, int8_t y_);
void InitLevelBuffer(void);
uint16_t Total_Dot_Count_test(void);
void BigDotAssign(void);
void Initialize2Bits(int8_t x_, int8_t y_);
uint8_t Read2Bits(int8_t x_, int8_t y_);
void Define2Bits(int8_t x_, int8_t y_, uint8_t initValue);
void CopyLevelToMem_test(void);
void Reset_SpritesPos(void);
void RemoveDotNotUse(void);
void Load_Selected_Level(uint8_t level_to_Load);
void DeleteSerialDot(void);
void SetGhost(void);
void Center_Screen(void);
uint8_t RCupAnimMask(uint8_t d_);
int8_t RCupAnim(uint8_t d_);
uint8_t InvertBlock(uint8_t Lvl_);
void Out_Check(uint8_t* val, int8_t y);
uint8_t ExploreMapChose(int8_t x_, int8_t y_, uint8_t SriteType_);
uint8_t ExploreMap(int8_t x_, int8_t y_);
uint8_t Select_Byte(uint8_t y_);
uint8_t Select_Byte_2Bits(uint8_t y_);

void UP_SET(void);
void DOWN_SET(void);
void RIGHT_SET(void);
void LEFT_SET(void);

// Déclaration de la classe TIMER
class TIMER {
private:
    uint8_t Activ;
    uint8_t startTime;
    uint8_t interval;
public:
    void Init(uint8_t Interval_);
    uint8_t Get_A(void);
    void Activate(void);
    void DeActivate(void);
    uint8_t Trigger(void);
};

// Déclaration de la classe SpriteAmania
class SpriteAmania {
private:
    uint8_t SpriteType;
    int8_t SpriteDirection;
    uint8_t DirectionX;
    uint8_t DirectionY;
    int8_t GridX;
    int8_t GridY;
    int8_t DecX;
    int8_t DecY;
    uint8_t Priority;
    uint8_t PadX;
    uint8_t PadY;

    uint8_t JmpPos;
    uint8_t JmpTrig;
    uint8_t JmpSeq;
    uint8_t SpriteMode;

public:
    void Init(int8_t x_, int8_t y_, uint8_t SpriteType_);
    void Put_SpriteMode(uint8_t Val_);
    void Put_JmpPos(uint8_t Val_);
    void Put_JmpTrig(uint8_t Val_);
    void Put_JmpSeq(uint8_t Val_);
    void Put_Priority(uint8_t Val_);
    void Put_DirectionX(uint8_t val_);
    void Put_DirectionY(uint8_t val_);
    void Put_GridX(int8_t val_);
    void Put_GridY(int8_t val_);
    void Put_DecX(int8_t val_);
    void Put_DecY(int8_t val_);
    void Put_PadX(int8_t val_);
    void Put_PadY(int8_t val_);

    uint8_t Get_SpriteMode(void);
    uint8_t Get_JmpPos(void);
    uint8_t Get_JmpTrig(void);
    uint8_t Get_JmpSeq(void);
    uint8_t Get_SpriteDirection(void);
    uint8_t Get_DirectionX(void);
    uint8_t Get_DirectionY(void);
    int8_t Get_GridX(void);
    int8_t Get_GridY(void);
    int8_t Get_DecX(void);
    int8_t Get_DecY(void);

    void AdjustControl(void);
    uint8_t CheckPriorityX(void);
    uint8_t CheckPriorityY(void);
    void RefreshJump(void);
    void RefreshMove(void);
    void GoUp(void);
    void GoDown(void);
    void GoRight(void);
    void GoLeft(void);
};

#endif
