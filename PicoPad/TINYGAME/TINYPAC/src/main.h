//      >>>>> T-I-N-Y  P-A-C-M-A-N for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2019-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  tiny-pacman is free software: you can redistribute it and/or modify
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


// The Tiny-Pacman source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tinypac.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "ReadMe.txt".

#ifndef _MAIN_H
#define _MAIN_H

void StartGame(struct PERSONAGE *Spk_);
void TinyOLED_Send(uint8_t x_,uint8_t y_,uint8_t Byte_);
void ResetVar(void);
uint8_t CollisionPac2Caracter(struct PERSONAGE *Spk_);
void RefreshCaracter(struct PERSONAGE *Spk_);
uint8_t CheckCollisionWithBack(uint8_t SpriteCheck,uint8_t HorVcheck,struct PERSONAGE *Spk_);
uint8_t RecupeBacktoCompV(uint8_t SpriteCheck,struct PERSONAGE *Spk_);
uint8_t Trim(uint8_t Y1orY2,uint8_t TrimValue,uint8_t Decalage);
uint8_t ScanHRecupe(uint8_t UporDown,uint8_t Decalage);
uint8_t RecupeBacktoCompH(uint8_t SpriteCheck,struct PERSONAGE *Spk_);
void Tiny_Flip(uint8_t render0_picture1,struct PERSONAGE *Spk_);
uint8_t FruitWrite(uint8_t x,uint8_t y);
uint8_t LiveWrite(uint8_t x,uint8_t y);
uint8_t DotsWrite(uint8_t x,uint8_t y,struct PERSONAGE *Spk_);
uint8_t checkDotPresent(uint8_t  DotsNumber);
void DotsDestroy(uint8_t DotsNumber);
uint8_t SplitSpriteDecalageY(uint8_t decalage,uint8_t Input,uint8_t UPorDOWN);
uint8_t SpriteWrite(uint8_t x,uint8_t y,uint8_t var1,struct PERSONAGE  *Spk_);
uint8_t SpriteWrite2(uint8_t x,uint8_t y,PERSONAGE  *Spk_);
uint8_t return_if_sprite_present(uint8_t x,struct PERSONAGE  *Spk_,uint8_t SpriteNumber);
uint8_t background(uint8_t x,uint8_t y);
void Sound(uint8_t freq,uint8_t dur);

#endif
