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


#ifndef _MAIN_H
#define _MAIN_H

#include "../include.h"


#define NBoC 11 //Number of chamber
#define Start_Room 16 //Start chamber 16

typedef struct LastPos {
  int8_t x;
  int8_t y;
} LastPos;

extern LastPos Lastpos;



void DrawRecBW(uint8_t x, uint8_t y, uint8_t x2, uint8_t y2, uint8_t Col_) ;
void InvertPix(uint8_t Flip_, uint8_t x, uint8_t y, uint8_t x2, uint8_t y2) ;
void SplashScreen(void) ;
void InitPan(void) ;
void InitnextRoom(void) ;
void ResetPickupSprite(void) ;
void InitAllItems(void) ;
void ResetVarForNewGame(void) ;
void DrawMainScreen(void) ;
void SoundSystem(uint8_t Snd_) ;
void SubLive(void) ;
void DrawLives(int8_t x_, int8_t y_) ;
void DrawCH(int8_t x_, int8_t y_) ;
void DisplayNum(void) ;
void DrawTime(uint8_t xx_, uint8_t yy_) ;
void DrawScore(uint8_t xx_, uint8_t yy_, uint16_t Val_) ;
void NumCalculate(uint16_t Nub_) ;
void DrawAllSprites(void) ;
void DrawRoomInProgress(void) ;
void TransitReDraw(void) ;
void RoomTransition(void) ;
void CheckIfBallExist(const int8_t *SPK_) ;
void Sprite_Colid(void) ;
uint8_t colid_univ(int8_t x1_, int8_t w1_, int8_t y1_, int8_t h1_, int8_t x2_, int8_t w2_, int8_t y2_, int8_t h2_) ;
void update_Ballon(void) ;
void Draw_Ballon(void) ;
void ReleaseNewDrop(const int8_t *DropeSelect) ;
void CalculateAcidDrop(void) ;
void DrawAcidDrop(void) ;
uint8_t ColAcidDrop(int8_t x_, int8_t y_, uint8_t t_) ;
bool checkMainColision(int8_t T_, int8_t bx, int8_t by) ;
uint8_t PickupSprites(int8_t T_, int8_t X_, int8_t Y_, int8_t I_) ;
void GotoNexRoom(int8_t DoorNumber_) ;
void DrawSprites(const int8_t *SPK_) ;
void MovingMode(void) ;
void RopeProcess(void) ;
void RopeModeGITD(void) ;
void WalkingModeGITD(void) ;
uint8_t RopeDetectionGITD(int8_t x_, int8_t y_, const int8_t *Rope_) ;
int STDHcollisionGITD(int8_t x1, int8_t y1, int8_t L_, int8_t x2, int8_t y2) ;
int STDVcollisionGITD(int8_t x1, int8_t y1, int8_t L_, int8_t x2, int8_t y2) ;
void FrmCall(void) ;
void Draw_Gilbert_ITD(void) ;
void DrawVector(int8_t x_, int8_t y_, const int8_t *VectorSelect) ;
void SetVectorStartPos(int8_t x_, int8_t y_) ;
void DrawWalls(int8_t y0, int8_t y1, int8_t x_) ;
void DrawStalagtite(int8_t x0, int8_t x1, int8_t y_) ;
void drawRopeVector(int8_t x_, int8_t y_, const int8_t *VectorSelect) ;
void drawRope(int8_t x0, int8_t y0, int8_t x1, int8_t y1) ;
void drawLine(int8_t x0, int8_t y0, int8_t x1, int8_t y1) ;
uint8_t ColWall(int8_t CX_, int8_t CY_) ;

#endif // _MAIN_H
