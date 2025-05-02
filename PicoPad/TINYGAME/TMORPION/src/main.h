//     >>>>> T-I-N-Y M-O-R-P-I-O-N for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-Morpion is free software: you can redistribute it and/or modify
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


// The Tiny-Morpion source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tmorpion.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the tmorpion.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#ifndef _MAIN_H
#define _MAIN_H

void INIT_M_DATA_TMORPION(void);
void Assigner_x_Main_TMORPION(uint8_t CPU0_PLY1);
void SND_BOX_TMORPION(uint8_t SND_);
void CPU_PLAY_TMORPION(void);
uint8_t CPU_REPLICATE_TMORPION(void);
uint8_t CPU_DOUBLE_TMORPION(void);
uint8_t  CPU_CORNER_TMORPION(void);
uint8_t COMPACT_MAP_TMORPION(uint8_t PLAYER_);
uint8_t CPU_MIDDLE_TMORPION(void);
void Check_WIN_TMORPION(uint8_t O_X);
void NULL_GAME_TMORPION(void);
void BLINK_WINNER_TMORPION(void);
uint8_t COMPLETED_PROCEDURE_TMORPION(void);
uint8_t RECUPE_POS_GRID_TMORPION(uint8_t Pos_);
uint8_t CPU_TERMINATION_TMORPION(uint8_t PLYs_);
void CPU_RND_TMORPION(void);
void REFRESH_SCREEN_TMORPION(void);
uint8_t RECUPE_UP_X_BOARD_TMORPION(uint8_t xPASS);
uint8_t RECUPE_MIDDLE_X_BOARD_TMORPION(uint8_t xPASS);
uint8_t RECUPE_DOWN_X_BOARD_TMORPION(uint8_t xPASS);
uint8_t RECUPE_UP_TMORPION(uint8_t xPASS,uint8_t yPASS);
uint8_t RECUPE_MIDDLE_TMORPION(uint8_t xPASS,uint8_t yPASS);
uint8_t RECUPE_DOWN_TMORPION(uint8_t xPASS,uint8_t yPASS);
uint8_t RECUPE_POINTER_TMORPION(uint8_t xPASS,uint8_t yPASS);
uint8_t Display_TMORLPION(uint8_t xPASS,uint8_t yPASS);
uint8_t RECUPE_BACK_TMORPION(uint8_t xPASS,uint8_t yPASS);
uint8_t Recupe_TMORPION(uint8_t xPASS,uint8_t yPASS);
void Tiny_Flip_TMORPION(uint8_t POINTER_,uint8_t START_,uint8_t END_);
uint8_t Recupe_MENU_TMORPION(uint8_t xPASS,uint8_t yPASS);
void Tiny_MENU_TMORPION(uint8_t POINTER_);

#endif // _MAIN_H
