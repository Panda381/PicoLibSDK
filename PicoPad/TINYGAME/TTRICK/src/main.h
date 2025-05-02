//   >>>>> T-I-N-Y  T-R-I-C-K for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  tiny-Trick is free software: you can redistribute it and/or modify
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


// The tiny-Trick source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "ttrick.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the ttrick.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#ifndef _MAIN_H
#define _MAIN_H

#define Cpu color(31,0,0)
#define Ply color(0,0,31)

void Reset_new_game(void);
void ResetAllSprites(void);
void INIT_ALL_VAR(void);
void Goal_Adj(uint8_t Y_POS,uint8_t SPR_Type);
void SND(uint8_t Snd);
uint8_t check_Puck2Gard(uint8_t Gard);
uint8_t GOAL(uint8_t Goal_0,uint8_t Goal_1);
void Reverse_x_puck_2_pos(uint8_t SpriteTested);
void Reverse_x_puck_2_neg(uint8_t SpriteTested);
uint8_t collision_check_LimiteScreen(uint8_t SpriteTested);
void KICK_PUCK(uint8_t Player_Rotate);
void Drag_Puck_Update(void);
void Drag_Puck_test(void);
void Speed_Puck_Decel(uint8_t SPR_TYPE);
void BOUNCE_SPRITE(void);
void Reverse_y_pla(uint8_t SpriteTested);
void Reverse_x_pla(uint8_t SpriteTested);
uint8_t collision_check_Sprite2Sprite(uint8_t Sp_1,uint8_t Sp_2);
uint8_t Computer_rotation_filter_update(void);
uint8_t Direction_Recupe(uint8_t Con);
uint8_t Update_Computer_Direction(void);
void Speed_H_Decel(uint8_t SPR_TYPE);
void Speed_V_Decel(uint8_t SPR_TYPE);
uint8_t Adjust_Speed_Computer(uint8_t Puck,uint8_t Val_Computer_2);
void Adjust_Cumule_Speed(uint8_t SPRITE_NO);
void Speed_adj_H_P(uint8_t SPRITE_NO);
void Speed_adj_H_N(uint8_t SPRITE_NO);
void Speed_adj_V_P(uint8_t SPRITE_NO);
void Speed_adj_V_N(uint8_t SPRITE_NO);
uint8_t blitzSprite(int8_t xPos,int8_t yPos,uint8_t xPASS,uint8_t yPASS,uint8_t FRAME,const uint8_t *SPRITES);
uint8_t Recupe(uint8_t xPASS,uint8_t yPASS);
uint8_t Patinoire1_2(uint8_t xPASS,uint8_t yPASS);
uint8_t SplitSpriteDecalageY(uint8_t decalage,uint8_t Input,uint8_t UPorDOWN);
uint8_t RecupeLineY(uint8_t Valeur);
uint8_t RecupeDecalageY(uint8_t Valeur);
void Tiny_Flip(void);
void intro(uint8_t GOAL_SCREEN,uint8_t SLIDE);
uint8_t Lost_win(uint8_t xPASS,uint8_t yPASS,const uint8_t *SPRITES );
void SCREEN_GOAL(void);

#endif // _MAIN_H
