//   >>>>> T-I-N-Y A-R-K-A-N-O-I-D for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-Arkanoid is free software: you can redistribute it and/or modify
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


// The Tiny-Arkanoid source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "arkanoid.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the arkanoid.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#ifndef _MAIN_H
#define _MAIN_H

void RsVarNewGame(GROUPE *VAR);
void PLAYMUSIC(void);
uint8_t BallMissing(GROUPE *VAR);
uint8_t CheckLevelEnded(GROUPE *VAR);
void UpdateBall(GROUPE *VAR);
void RecupeBALLPosForSIM(GROUPE *VAR);
void TestMoveBALL(GROUPE *VAR);
void SimulMove(uint8_t Sim,GROUPE *VAR);
uint8_t CheckCollisionBall(GROUPE *VAR);
uint8_t CheckCollisionWithBLOCK(GROUPE *VAR);
void RecupePositionOnGrid(GROUPE *VAR);
uint8_t RecupeXPositionOnGrid(GROUPE *VAR);
uint8_t RecupeYPositionOnGrid(GROUPE *VAR);
uint8_t CheckCollisionWithTRACKBAR(GROUPE *VAR);
void WriteBallMove(GROUPE *VAR);
void Tiny_Flip(uint8_t render0_picture1,GROUPE *VAR);
uint8_t PannelLevel(uint8_t X,uint8_t Y,GROUPE *VAR);
uint8_t Block(uint8_t X,uint8_t Y,GROUPE *VAR);
uint8_t RecupeDecalageY(uint8_t Valeur);
uint8_t Ball(uint8_t X,uint8_t Y,GROUPE *VAR);
uint8_t SplitSpriteDecalageY(uint8_t decalage,uint8_t Input,uint8_t UPorDOWN);
uint8_t TrackBar(uint8_t X,uint8_t Y,GROUPE *VAR);
uint8_t PannelLive(uint8_t X,uint8_t Y,GROUPE *VAR);
uint8_t background(uint8_t X,uint8_t Y);
void LoadLevel(uint8_t Level,GROUPE *VAR);
void ResetVar(GROUPE *VAR);
void ResetBall(GROUPE *VAR);

#endif // _MAIN_H
