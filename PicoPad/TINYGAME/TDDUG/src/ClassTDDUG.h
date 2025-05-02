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

#ifndef _CLASSTDDUG_H
#define _CLASSTDDUG_H

#define MAIN_ACCEL_SPEED_TDDUG 50
#define SPRITE_ACCEL_SPEED_TDDUG 50
#define ENEMY_SPEED_STEP_TDDUG 25

//PUBLIC: STRUCT VAR
struct GAME_DATA_TDDUG {
uint8_t TimeLaps;
uint8_t  FirsT_Time;
uint8_t One_Time_TDDUG;
uint8_t Counter; 
uint8_t Trigger_Counter;
uint8_t MAIN_FRAME;
uint8_t MAIN_ANIM_FRAME;
uint8_t MAIN_ANIM;
uint8_t DIRECTION_ANIM;
uint16_t Scores;
uint8_t Skip_Frame;
uint8_t LEVEL;
uint8_t GO_OUT;
uint8_t NOSPRITE_GO_OUT;
uint8_t DEAD;
uint8_t LIVE;
uint8_t LIVE_COMP;
};



//start Sprite
class Sprite_TDDUG {
private:
int8_t x;
int8_t y;
int8_t direction_x;
int8_t direction_y;
int8_t active;
public:
int8_t X(void);
int8_t Y(void);
int8_t ACTIVE(void);
void PUT_X(int8_t XX);
void PUT_Y(int8_t YY);
void PUT_ACTIVE(int8_t ACT);
int8_t DIRECTION_X(void);
int8_t DIRECTION_Y(void);
void PUT_DIRECTION_X(int8_t D_X);
void PUT_DIRECTION_Y(int8_t D_Y);
};


//start Moving sprite
class Moving_Sprite_TDDUG:public Sprite_TDDUG {
private:
int8_t Som_x;
int8_t Som_y;
int8_t sx;
int8_t sy;
public:
int8_t SOMX(void);
int8_t SOMY(void);
int8_t SX(void);
int8_t SY(void);
void PUT_SX(int8_t SX_);
void PUT_SY(int8_t SY_);
void PUT_SOMX(int8_t PUT_SOMX_);
void PUT_SOMY(int8_t PUT_SOMX_);
void HAUT(void);
void DROITE(void);
void BAS(void);
void GAUCHE(void);
void Ou_suis_je(uint8_t &x_,uint8_t &y_);
};

class Enemy_Sprite_TDDUG : public Moving_Sprite_TDDUG{
public:
uint8_t ANIM_DIRECT(void);
void PUT_ANIM_DIRECT(uint8_t H_Dir_);
void PUT_TYPE(uint8_t TY_);
uint8_t TYPE(void);
void PUT_TRACKING(int8_t TR_);
int8_t TRACKING(void);
void INIT(int8_t X_,int8_t Y_,int8_t Type_);
void HAUT(void);
void DROITE(void);
void BAS(void);
void GAUCHE(void);
void NEW_DIRECTION(int8_t dir_);
void NEW_LIMITE_DIRECTION(int8_t dir_);
void PUT_ANIM(uint8_t Ani_);
uint8_t ANIM(void);
uint8_t E_GRID_UPDATE_UP(void);
uint8_t E_GRID_UPDATE_DOWN(void);
uint8_t E_GRID_UPDATE_RIGHT(void);
uint8_t E_GRID_UPDATE_LEFT(void);
uint8_t FIRST(void);
void PUT_FIRST(uint8_t Fir_);
uint8_t BIG_ZIP(void);
void PUT_BIG_ZIP(uint8_t Fir_);
private:
uint8_t First;
uint8_t Type;
int8_t Tracking;
uint8_t Anim_Direct;
uint8_t Anim;
uint8_t Big_zip;
};


class Main_Sprite_TDDUG : public Moving_Sprite_TDDUG{
public:
void MHAUT(void);
void MDROITE(void);
void MBAS(void);
void MGAUCHE(void);
void INIT(void);
void WALK_RIGHT(void);
void WALK_LEFT(void);
void WALK_UP(void);
void WALK_DOWN(void);
void GRID_UPDATE_RIGHT(void);
void GRID_UPDATE_LEFT(void);
void GRID_UPDATE_UP(void);
void GRID_UPDATE_DOWN(void);
private:
};

class WEAPON_TDDUG : public Sprite_TDDUG {
public:
void INIT_WEAPON(void);
void ADJUST_WEAPON(Main_Sprite_TDDUG &MS_);
void ADJUST_WEAPON2(WEAPON_TDDUG &W_,Main_Sprite_TDDUG &MS_);
void WEAPON_COLISION_TDDUG(WEAPON_TDDUG W_,uint8_t Nu_);
uint8_t ANIM_OR(void);
void PUT_ANIM_OR(uint8_t WA_);
void Ou_suis_je(uint8_t &x_,uint8_t &y_);
private:
uint8_t anim_or;
};

#endif
