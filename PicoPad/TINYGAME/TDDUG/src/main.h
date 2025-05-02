//       >>>>> T-I-N-Y D-D-U-G for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-DDug is free software: you can redistribute it and/or modify
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


// The Tiny-DDug source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tddug.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the tddug.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#ifndef _MAIN_H
#define _MAIN_H


void RESET_ALL_GD_TDDUG(void);
void RESET_WEAPON_TDDUG(void);
void LOAD_ENEMY_POS_TDDUG(void);
void LOAD_LEVEL_TDDUG(uint8_t Lev_);
void NEWGAME_TDDUG(void);
void NEXTLEVEL_TDDUG(void);
void Trigger_adj_TDDUG(void);
void SND_TDDUG(uint8_t Val_);
void SND_DDUG(void);
void SCORES_ADD_TDDUG(void);
void COMPILING_TDDUG(void);
void DLAY(void);
uint8_t GamePlay_TDDUG(void);
void HowManyEnemy_TDDUG(void);
uint8_t Universal_TDDUG(int8_t ax,int8_t ay,int8_t aw,int8_t ah,int8_t bx,int8_t by,int8_t bw,int8_t bh);
void Check_Collision_TDDUG(void);
void ADJUST_FRAME_MAIN_DDUG(void);
void TL_ENEMY_TDDUG(void);
uint8_t Check_Balistic_Colid_TDDUG(void);
int8_t TrackX_TDDUG(int8_t t_);
int8_t TrackY_TDDUG(int8_t t_);
void UPDATE_ENEMY_TDDOG(void);
void Colapse_Enemy_Anim_TDDUG(void);
uint8_t RECUPE_ENEMY_BIG_TDDUG(uint8_t SPR_);
uint8_t RECUPE_ENEMY_FRAME_TDDUG(uint8_t SPR_);
void Adjust_main_speed_TDDUG(uint8_t RT_);
void UPDATE_PANNEL_TDDUG(uint8_t t_);
void COMPIL_SCO_TDDUG(void);
uint8_t READ_GRID_TDDUG(int8_t X_,int8_t Y_);
uint8_t WRITE_GRID_TDDUG(int8_t X_,int8_t Y_);
int8_t RND_TDDUG(void);
void ANIM_UPDATE_MAIN_TDDUG(uint8_t Direct_);
uint8_t blitzSprite_TDDUG(int8_t xPos,int8_t yPos,uint8_t xPASS,uint8_t yPASS,uint8_t FRAME,const uint8_t *SPRITES);
uint8_t BACK_TDDUG(uint8_t xPASS,uint8_t yPASS);
void ADJUST_LIVE_COMP_TDDUG(void);
uint8_t RECUPE_LIVE_TDDUG(uint8_t xPASS,uint8_t yPASS);
uint8_t BACKGROUND_TDDUG(uint8_t xPASS,uint8_t yPASS);
uint8_t MAIN_TDDUG(uint8_t xPASS,uint8_t yPASS);
uint8_t RECUPE_ENEMY_TDDUG(uint8_t xPASS,uint8_t yPASS);
uint8_t RECUPE_WEAPON_TDDUG(uint8_t xPASS,uint8_t yPASS);
uint8_t Recupe_TDDUG(uint8_t xPASS,uint8_t yPASS);
uint8_t SplitSpriteDecalageY_TDDUG(uint8_t decalage,uint8_t Input,uint8_t UPorDOWN);
int8_t RecupeLineY_TDDUG(int8_t Valeur);
uint8_t RecupeDecalageY_TDDUG(uint8_t Valeur);
void Tiny_Flip_TDDUG(uint8_t Select_,uint8_t Blink_);
uint8_t RECUPE_START_TDDUG(uint8_t xPASS,uint8_t yPASS,uint8_t Blink_);
uint8_t recupe_SCORES_TDDUG(uint8_t xPASS,uint8_t yPASS,int8_t Pos_);
uint8_t SPEED_BLITZ_TDDUG(int8_t xPos,int8_t yPos,uint8_t xPASS,uint8_t yPASS,uint8_t FRAME,const uint8_t *SPRITES);
void Sound_TDDUG(uint8_t freq_,uint8_t dur);
#endif // _MAIN_H
