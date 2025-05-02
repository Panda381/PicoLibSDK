//   >>>>> T-I-N-Y  I-N-V-A-D-E-R-S for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  tiny-Invaders is free software: you can redistribute it and/or modify
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


// The tiny-Invaders source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "invaders.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the invaders.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK


#ifndef _MAIN_H
#define _MAIN_H
 //#include "../include.h" 
void LoadMonstersLevels(int8_t Levels,struct SPACE *space);
int main(void) ;
void SnD(int8_t Sp_,uint8_t SN);
void SpeedControle(struct SPACE *space);
void GRIDMonsterFloorY(struct SPACE *space);
uint8_t LivePrint(uint8_t x,uint8_t y);
void Tiny_Flip(uint8_t render0_picture1,struct SPACE *space);
uint8_t UFOWrite(uint8_t x,uint8_t y,struct SPACE *space);
void UFOUpdate(struct SPACE *space);
void ShipDestroyByMonster(struct SPACE *space);
void MonsterShootupdate(struct SPACE *space);
void MonsterShootGenerate(struct SPACE *space);
uint8_t MonsterShoot(uint8_t x,uint8_t y,struct SPACE *space);
uint8_t ShieldDestroy(uint8_t Origine,uint8_t VarX,uint8_t VarY,struct SPACE *space);
void ShieldDestroyWrite(uint8_t BOOLWRITE,uint8_t line,struct SPACE *space,uint8_t Origine);
uint8_t MyShield(uint8_t x,uint8_t y,struct SPACE *space);
uint8_t ShieldBlitz(uint8_t Part,uint8_t LineSH );
uint8_t BOOLREAD(uint8_t SHnum,uint8_t LineSH,struct SPACE *space );
void RemoveExplodOnMonsterGrid(struct SPACE *space);
uint8_t background(uint8_t x,uint8_t y,struct SPACE *space);
uint8_t Vesso(uint8_t x,uint8_t y,struct SPACE *space);
void UFO_Attack_Check(uint8_t x,struct SPACE *space);
uint8_t MyShoot(uint8_t x,uint8_t y,struct SPACE *space);
void Monster_Attack_Check(struct SPACE *space);
int8_t OuDansLaGrilleMonster(uint8_t x,uint8_t y,struct SPACE *space);
uint8_t SplitSpriteDecalageY(uint8_t Input,uint8_t UPorDOWN,struct SPACE *space);
uint8_t Murge_Split_UP_DOWN(uint8_t x,struct SPACE *space);
uint8_t WriteMonster14(uint8_t x);
uint8_t Monster(uint8_t x,uint8_t y,struct SPACE *space);
uint8_t MonsterRefreshMove(struct SPACE *space);
void VarResetNewLevel(struct SPACE *space);

#endif // _MAIN_H
