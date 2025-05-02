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

#ifndef _MAIN_H
#define _MAIN_H

void NOHZDYVE_Setup() ;
void ExitWindowND() ;
void PlayGameND() ;
void GamePlayAdj() ;
void SpriteSetCFGND() ;
void TimerConfigND() ;
void InitPan1ND(void) ;
void CalculatePanND(uint16_t Scores_) ;
void ScorePannelND(uint8_t x_, uint8_t y_,uint16_t Col_) ;
void ResetStepSNND() ;
void PlayINGameND() ;
void PlaySongND() ;
void StartToneND() ;
void DeadToneND() ;
void GlobToneND() ;
void SplashScreenND() ;
void SelectND() ;
uint8_t RecupeFadePosND(uint8_t *scan_, uint8_t Frame_) ;
void Merge2PicND(const uint8_t *pic1_, const uint8_t *pic2_) ;
void ResetVarND(void) ;
void ChangePicND(uint8_t *Exit_) ;
void Explod_MainND(void) ;
void Explod_GlobND(void) ;
void AdjustScoreND() ;
void DrawLivesND(void) ;
void DrawScoreND(void) ;
void CollisionCheckND(void) ;
void Main2SolidND() ;
void Main2GlobND() ;
uint8_t ColidCheckND(int8_t x, int8_t y, int8_t x2, int8_t y2, uint8_t MargingX, uint8_t MargingY) ;
void WallCollisionCheckMainND(void) ;
void RefreshMoveJAWND() ;
void LibarateSpriteND() ;
void LiberateLineND() ;
int8_t YLINENDSETND(void) ;
void LiberateJawND() ;
int8_t JAWNDSTARTPOSND() ;
void LiberateGlobND() ;
int8_t GLOBDSTARTPOSND() ;
void LiberateWallSpriteND() ;
int8_t XPLANTNDSETND() ;
int8_t YPLANTNDSETND(void) ;
int8_t XCLIMNDSETND() ;
int8_t YCLIMNDSETND(void) ;
void CleanOverScanND() ;
void ExplodLogisticND() ;
void SinWavRunND(void) ;
void UpDateMainND(void) ;
void JoyPadRefreshND(void) ;
void FrameRateND() ;
void FlipScreenND() ;
void DrawMainScreenND() ;
void StartPageND() ;
uint8_t FadeMainScreenND(uint8_t *FD_, uint8_t TR3_) ;
uint8_t SelectFrameND(uint8_t t_) ;
int8_t WavAnimND(uint8_t t) ;
int8_t WavXAnimND(uint8_t t) ;
void DrawSpritesND() ;
void EraseScreenND() ;
void DrawSideWallsND() ;
void ScrollDownND(uint8_t Spd_) ;
void MovingSpriteND() ;

#endif // _MAIN_H
