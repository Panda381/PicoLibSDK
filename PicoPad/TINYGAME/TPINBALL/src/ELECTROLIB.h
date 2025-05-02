//     >>>>> T-I-N-Y P-I-N-B-A-L-L for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-Pinball is free software: you can redistribute it and/or modify
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


// The Tiny-Pinball source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tpinball.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the tpinball.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#ifndef _ELECTROLIB_H
#define _ELECTROLIB_H

//prototype
uint8_t blitzSprite(int8_t xPos,int8_t yPos,uint8_t xPASS,uint8_t yPASS,uint8_t FRAME,const uint8_t *SPRITES);
uint8_t SPEED_BLITZ(int8_t xPos,int8_t yPos,uint8_t xPASS,uint8_t yPASS,uint8_t FRAME,const uint8_t *SPRITES);
uint8_t SplitSpriteDecalageY(uint8_t decalage,uint8_t Input,uint8_t UPorDOWN);
int8_t RecupeLineY(int8_t Valeur);
uint8_t RecupeDecalageY(uint8_t Valeur);
uint8_t Mymap(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max);
uint8_t Trace_LINE(uint8_t DESACTIVE_,uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t xPASS,uint8_t yPASS);
uint8_t DIRECTION_LINE(uint8_t DESACTIVE_,uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t xPASS,uint8_t yPASS);
uint8_t Return_Full_Byte(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t xPASS,uint8_t yPASS);
void Universal_Swap(uint8_t *A_,uint8_t *B_);
uint8_t RECONSTRUCT_BYTE(uint8_t Min_,uint8_t Max_);
void FADE_TPIPE(const uint8_t *PIC,uint8_t FADE);
void PLAY_MUSIC(const uint8_t *Music_);
void TINYJOYPAD_INIT(void);

#endif
