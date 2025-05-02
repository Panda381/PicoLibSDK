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

#ifndef _MAIN_H
#define _MAIN_H

void falseBall(void);
void BallupDate(struct BALL *B);
uint8_t SelectByte(uint8_t ByteSelect,uint8_t FFx0);
void TrimXY(struct BALL  *B);
void SimulateRebounce(uint8_t Sim,struct BALL  *B);
uint8_t CheckColisionType(struct BALL  *B);
void TrimBallOnSpring(struct BALL  *B);
void WriteMove(struct BALL  *B);
void WriteMoveBounce(struct BALL  *B);
void SimulateMove(struct BALL  *B);
uint8_t ColisionCheck(float x,float y,struct BALL  *B);
uint8_t RecupeScreen(uint8_t nn,uint8_t mm);
uint8_t Recup_Intro_PIC(uint8_t xPASS,uint8_t yPASS);
void PIC_DRAW(uint8_t PIC_);
void Tiny_Flip(struct BALL  *B);
uint8_t addBin(uint8_t a,uint8_t b,uint8_t c,uint8_t d);
uint8_t RecupeByte(uint8_t x,uint8_t y);
uint8_t RecupeFlip(uint8_t x,uint8_t y);
uint8_t RecupeSpring(uint8_t x,uint8_t y);
uint8_t PixelAsign(uint8_t Value);
uint8_t PixelConvert(uint8_t horiz,uint8_t verti,struct BALL  *B);
uint8_t SliceByte(uint8_t Vertical,uint8_t Byte);
void Tiny_Flip2(uint8_t select,struct BALL *B);
#endif // _MAIN_H
