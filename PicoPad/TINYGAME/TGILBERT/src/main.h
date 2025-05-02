//       >>>>> T-I-N-Y G-I-L-B-E-R-T for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-Gilbert is free software: you can redistribute it and/or modify
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


// The Tiny-Gilbert source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tgilbert.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the tgilbert.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#ifndef _MAIN_H
#define _MAIN_H

uint8_t delKey(uint8_t Xin,uint8_t Yin);
void sound(uint8_t SND);
uint8_t SPEED_BLITZ(int8_t xPos,int8_t yPos,uint8_t xPASS,uint8_t yPASS,uint8_t FRAME,const uint8_t *SPRITES);
void INTRO_TGILBERT(void);
void ScrollUpdate(DriftSprite* DSprite);
void UpdateVerticalSlide(DriftSprite* DSprite);
void GravityUpdate(DriftSprite* DSprite);
void JumpProcedure(DriftSprite* DSprite);
int8_t CollisionCheck(DriftSprite* DSprite);
void ResetVar(void);
void ResetVarNextLevel(void);
void NextLevel(void);
void TinyMainShift(DriftSprite* DSprite);
void SpriteShiftInitialise(DriftSprite* DSprite);
void Tiny_Flip(DriftSprite* DSprite);

#endif // _MAIN_H
