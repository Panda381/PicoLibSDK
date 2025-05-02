//      >>>>> T-I-N-Y  B-O-M-B-E-R for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  tiny-Bomber is free software: you can redistribute it and/or modify
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


// The Tiny-Bomber source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tbomber.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the tbomber.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK
 
#ifndef _MAIN_H
#define _MAIN_H

      #include "../include.h" 
                                                                                                                                
void LoadLevel(uint8_t LeveL);
void ResetVar(struct PERSONAGE * Sprite);
void resetMonster(struct PERSONAGE *Sprite);
void resetBomb(void);
void StartGame(void);
uint8_t CheckLevelEnded(struct PERSONAGE *Sprite);
void DeadSong(void);
void DestroyBloc(void);
void DestroyEnemy(struct PERSONAGE *Sprite);
uint8_t COLLISION11(uint8_t x1,uint8_t y1,uint8_t w1,uint8_t h1,uint8_t x2,uint8_t y2,uint8_t w2,uint8_t h2);
uint8_t CollisionTiny2Caracter(struct PERSONAGE *Sprite);
void RefreshCaracter(struct PERSONAGE *Sprite);
uint8_t CheckCollisionWithBack(uint8_t SpriteCheck,uint8_t HorVcheck,struct PERSONAGE *Sprite);
uint8_t CheckCollisionWithBomb(uint8_t SpriteCheck,struct PERSONAGE *Sprite);
uint8_t CheckCollisionWithBlock(uint8_t SpriteCheck,struct PERSONAGE *Sprite);
uint8_t RecupeBacktoCompV(uint8_t SpriteCheck,struct PERSONAGE *Sprite);
uint8_t Trim(uint8_t Y1orY2,uint8_t TrimValue,uint8_t Decalage);
uint8_t ScanHRecupe(uint8_t UporDown,uint8_t Decalage);
uint8_t RecupeBacktoCompH(uint8_t SpriteCheck,struct PERSONAGE *Sprite);
void Tiny_Flip(uint8_t render0_picture1,struct PERSONAGE *Sprite);
uint8_t PrintLive(uint8_t x,uint8_t y);
uint8_t Explose(uint8_t x,uint8_t y);
uint8_t BombBlitz(uint8_t x,uint8_t y);
uint8_t BlockBomb(uint8_t x,uint8_t y,uint8_t Divx);
void BOOLWRITE0(uint8_t BoolNumber);
uint8_t BOOLREAD(uint8_t Numero);
uint8_t SplitSpriteDecalageY(uint8_t decalage,uint8_t Input,uint8_t UPorDOWN);
uint8_t SpriteMain(uint8_t x,uint8_t y,struct PERSONAGE  *Sprite);
uint8_t SpriteWrite(uint8_t x,uint8_t y,struct PERSONAGE  *Sprite);
uint8_t return_if_sprite_present(uint8_t x,struct PERSONAGE  *Sprite,uint8_t SpriteNumber);
uint8_t background(uint8_t x,uint8_t y);

#endif
