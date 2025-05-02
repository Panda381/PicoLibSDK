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

#ifndef _MAIN_H
#define _MAIN_H

void FPS_Count_TD(uint8_t Ms_) ;
int main() ;
void Compile_LV_Display_TD(uint8_t *A10_, uint8_t *B1_, uint8_t NUMBER_) ;
void AdjustGamePlay(void) ;
void FirstCalculeDisplay_TD(void) ;
void COMPIL_SCORE_TD(void) ;
void Virus_Loupe_TD(void) ;
void SND_TDOC_TD(uint8_t Snd_) ;
void init_public_Var_For_New_game_TD(void) ;
void init_public_Var_For_New_Level_TD(void) ;
void TINYJOYPAD_UPDATE_TD(void) ;
void InitNewBackTab_TD(void) ;
void CopyItem2Delete_TD(uint8_t x_, uint8_t y_) ;
void NewStepLine_TD(void) ;
void ElementCounter_TD(uint8_t x_, uint8_t y_, uint8_t Element_) ;
void CheckCompletedLine_TD(void) ;
void ClearLine_TD(void) ;
uint8_t ReturnConectionType_TD(uint8_t Type_) ;
uint8_t ReturnType_TD(uint8_t Val_) ;
uint8_t CheckMatch_TD(int8_t x_, int8_t y_) ;
uint8_t ReturnCombinedPillDirection_TD(int8_t x_, int8_t y_, uint8_t Type_) ;
void SetSinglePill_TD(void) ;
void AnimPillShoot_TD(void) ;
void DropMode_TD(void) ;
void PillProcess_TD(void) ;
uint8_t DropPills_TD(void) ;
uint8_t GenerateSidePill_TD(uint8_t Side) ;
void GeneratenewPreviewPill_TD(void) ;
void InitNewPill_TD(uint8_t Act_) ;
uint8_t Select_TD(void) ;
uint8_t OrderSelect_TD(uint8_t Select_, uint8_t Chiffre_) ;
uint8_t GetElement_TD(size_t index_) ;
void init_RND_TD(void) ;
uint8_t CheckRotateIsPosible_TD(void) ;
void RotatePill_TD(void) ;
void SetPicDirection_TD(uint8_t Vertical_) ;
void FixPill_TD(void) ;
uint8_t CheckColision_TD(int8_t X_, int8_t Y_) ;
void ReturnScanLineY_TD(uint8_t *a, uint8_t *b, uint8_t y) ;
void ReturnScanLineX_TD(uint8_t *a, uint8_t x) ;
uint8_t ReturncorectPills_TD(uint8_t Val_) ;
uint16_t ReturnCorectColorPill(uint8_t Val_);
uint8_t DrawNewPill_TD(uint8_t x, uint8_t y) ;
uint8_t SwitchRecupVirus_TD(uint8_t Type_) ;
uint8_t DrawTAB_TP(uint8_t x, uint8_t y) ;
uint8_t DrawPreviewPill_TD(uint8_t x, uint8_t y) ;
uint8_t DrawShadowPreviewPill_TD(uint8_t x, uint8_t y) ;
uint8_t DrawLoupe_TD(uint8_t x, uint8_t y) ;
uint8_t Draw_Score_TD(uint8_t x, uint8_t y) ;
void IntroFlip_TD(uint8_t Show_) ;
uint8_t DrawDisplayLV_TD(uint8_t x, uint8_t y) ;
void TinyFlip_TD(void) ;
void LevelClearedFlip_TD(void) ;

#endif // _MAIN_H
