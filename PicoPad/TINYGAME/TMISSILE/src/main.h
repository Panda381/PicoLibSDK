//  >>>>> T-I-N-Y M-I-S-S-I-L-E for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-Missile is free software: you can redistribute it and/or modify
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


// Tiny-Missile  source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tmissile.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the tmissile.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#ifndef _MAIN_H
#define _MAIN_H


uint8_t ALL_ANIM_END_TMISSILE(void);
void RESTORE_DOME_TMISSILE(void);
uint8_t Score_Dome_Munition_TMISSILE(void);
void ADJ_LEVEL_TMISSILE(uint8_t Level_);
void NEW_GAME_TMISSILE(void);
void NEXTLEVEL_TMISSILE(void);
void INC_SCORES_TMISSILE(void);
uint8_t UPDATE_ENGINE_TMISSILE(void);
void UPDATE_DOME_TMISSILE(void);
void UPDATE_Intercept_TMISSILE(void);
void Destroy_TMISSILE(uint8_t t_);
void Create_New_Intercept_TMISSILE(int8_t X_,int8_t Y_);
void NEW_POS_TMISSILE(int8_t *ST_,int8_t *END_);
void Start_RDLP_MIX(void);
void RDLP_MIX(void);
void UPDATE_DFENCE_TMISSILE(void);
void Renew_Shield_TMISSILE(int8_t x_END,int8_t y_END);
void REFRESH_TOPBAR_TMISSILE(void);
void RESET_ALL_MISSILE_TMISSILE(void);
void RENEW_MISSILE(void);
uint8_t RECUP_LINE_TMISSILE(uint8_t xPASS,uint8_t yPASS);
uint8_t RECUP_DOME_TMISSILE(uint8_t xPASS,uint8_t yPASS);
uint8_t RECUP_CROSS_TMISSILE(uint8_t xPASS,uint8_t yPASS);
uint8_t RECUP_SHIELD_TMISSILE(uint8_t xPASS,uint8_t yPASS);
uint8_t RECUPE_TOPBAR_TMISSILE(uint8_t xPASS,uint8_t yPASS);
uint8_t RECUP_INTERCEPT_TMISSILE(uint8_t xPASS,uint8_t yPASS);
uint8_t Recupe_TMISSILE(uint8_t xPASS,uint8_t yPASS);
uint8_t BACKGROUND_TMISSILE(uint8_t xPASS,uint8_t yPASS);
void Tiny_Flip_TMISSILE(void);
void INTRO_TMISSILE(uint8_t FL_);
uint8_t RECUPE_INTRO_TMISSILE(uint8_t xPASS,uint8_t yPASS,uint8_t Fl_);
uint8_t RECUP_PANEL_TMISSILE(uint8_t xPASS,uint8_t yPASS);
uint8_t INVENTORY_TMISSILE(uint8_t xPASS);
uint8_t MUNITION_TMISSILE(uint8_t xPASS);
uint8_t NUMERIC_TMISSILE(uint8_t xPASS);
uint8_t recupe_SCORES_TMISSILE(uint8_t xPASS);
void COMPIL_SCO_TMISSILE(void);
void SNDBOX_TMISSILE(uint8_t Snd_);

#endif // _MAIN_H
