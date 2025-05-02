//       >>>>> T-I-N-Y B-E-R-T for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-Bert is free software: you can redistribute it and/or modify
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


// The Tiny-Bert source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tinybert.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the tinybert.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#ifndef _MAIN_H
#define _MAIN_H


void DIFICULTY_CHANGE(void);
void ResetDataVar(Sprite* DATA);
void RESET_GAME(void);
void SEED_RND(void);
uint8_t RANDOM(uint8_t STEP);
void Recupe_HighScore(void);
void Rst_Score(void);
void recupe_Score(void);
uint16_t HighScore(uint8_t D_3,uint8_t D_2,uint8_t D_1,uint8_t D_0);
void DEAD_SOUND(void);
void DEADIFY(uint8_t SPr,Sprite *DATA);
void RenewSprite(uint8_t SPr,Sprite *DATA);
uint8_t Renew_if_Posible(Sprite *DATA);
uint8_t Renew_Check_if_Posible(uint8_t SPr,Sprite *DATA);
uint8_t COLLISION_CHECK(Sprite *DATA);
void FLIP_PLATE(void);
void GamePlayUpdate(Sprite *DATA);
void renew_if_possible_add(uint8_t Sprite );
void ResetPlateGrid(void);
void ScoreChange(void);
void refreshJump(uint8_t SpUSE,Sprite *DATA);
void TELEPORT_MAIN(Sprite *DATA);
uint8_t UPDATE_LIFT_MAIN(uint8_t SpUSE,Sprite *DATA);
void UPDATE_MAIN_MOVE(uint8_t SpUSE,Sprite *DATA);
void UPDATE_MAIN_DEAD(uint8_t SpUSE,Sprite *DATA);
void LimitCheck(uint8_t SpUSE,uint8_t JOYKEY,Sprite *DATA);
uint8_t GridLimitTest(uint8_t SpUSE,int8_t x,int8_t y,Sprite *DATA);
uint8_t Level_Completed(void);
void Ball_move_update(uint8_t SpUSE,Sprite *sprite);
uint8_t GridPlate(uint8_t xPASS,uint8_t yPASS);
uint8_t Recupe(uint8_t xPASS,uint8_t yPASS,Sprite *DATA);
uint8_t blitzSprite(int8_t xPos,int8_t yPos,uint8_t xPASS,uint8_t yPASS,uint8_t FRAME,const uint8_t *SPRITES);
uint8_t SplitSpriteDecalageY(uint8_t decalage,uint8_t Input,uint8_t UPorDOWN);
int8_t RecupeLineY(int8_t Valeur);
uint8_t RecupeDecalageY(uint8_t Valeur);
uint8_t Tiny_Bert_Live_Print(uint8_t xPASS,uint8_t yPASS);
uint8_t Police_Print(uint8_t xPASS,uint8_t yPASS);
void Tiny_Flip(Sprite *DATA);

#endif // _MAIN_H
