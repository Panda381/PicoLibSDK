//       >>>>> T-I-N-Y P-L-A-Q-U-E for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-Plaque is free software: you can redistribute it and/or modify
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


// The Tiny-Plaque source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tplaque.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the tplaque.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#ifndef _CLASSTPLAQUE_H
#define _CLASSTPLAQUE_H

#include "PictureTPLAQUE.h"

#define MAX_LEVEL_TPLAQUE 16
#define MAIN_ACCEL_SPEED_TPLAQUE 32
#define FOOD_ACCEL_SPEED_TPLAQUE 60
#define RENEW_NUMBER_TPLAQUE 20

#define MAX_LEVEL_TPLAQUE 16
#define MAIN_ACCEL_SPEED_TPLAQUE 32
#define FOOD_ACCEL_SPEED_TPLAQUE 60
#define RENEW_NUMBER_TPLAQUE 20

//PUBLIC:VAR

//PUBLIC: STRUCT VAR
struct GAME_DATA_TPLAQUE {
uint16_t Scores_TPLAQUE;
uint8_t Level;
uint8_t EXTRA_TEETH;
uint8_t EXTRA_TEETH_COMP;
int8_t up_down;
int8_t sp;
uint8_t renew;
uint8_t REGEN_NO;
uint8_t NOT_MOVE;
uint8_t SCAN_CHANGE_DIRECTION;
uint8_t DELAY_DIRECTION_CHANGE;
uint8_t Delay_Direction_Change;
uint8_t END_OF_GAME;
uint8_t RENEW_FOOD;
uint8_t x_move_active;
uint8_t Food_Type;
uint8_t TIMER_TEETH;
uint8_t Skip_Frame;
uint8_t TEETH_COUNT_UP;
uint8_t TEETH_COUNT_DOWN;
uint8_t TUBE_FUEL;
uint16_t TUBE_FUEL_TIMER;
uint16_t TUBE_FUEL_TIMER_REF;
uint8_t Tube_Refresh;
};

uint8_t RND_POS_TPLAQUE(void);

int8_t RND_TPLAQUE(void);

//start Sprite
class Sprite_TPLAQUE {
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
//end sprite

//start Moving sprite
class Moving_Sprite_TPLAQUE:public Sprite_TPLAQUE {
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
};
//end moving sprite

//start Main sprite
class Main_Sprite_TPLAQUE : public Moving_Sprite_TPLAQUE{
public:
void MHAUT(void);
void MDROITE(void);
void MBAS(void);
void MGAUCHE(void);
void DECEL_X(void);
void DECEL_Y(void);
void LIMIT_CHECK(void);
void INIT(void);
private:
};

//start Food_sprite
class Food_Sprite_TPLAQUE:public Moving_Sprite_TPLAQUE {
private:
uint8_t colapsed;
uint8_t Start_Pos;
public:
uint8_t COLAPSED();
uint8_t START_POS();
void COPY_OBJ(Food_Sprite_TPLAQUE &T_OBJ);
void PUT_COLAPSED(uint8_t COL_);
void PUT_START_POS(uint8_t x_t);
void CREAT_FOOD();
void MOVE_UPDATE();
void ADJUST_X();
void RESET_OBJ();
void DISABLE_RESET(void);
};

//START WAPON_SPRITE
class Weapon_Sprite_TPLAQUE:public Sprite_TPLAQUE {
private:
public:
void Start(Main_Sprite_TPLAQUE Main_S_);
void Update(void);
uint8_t COLLISION_DETECT(void);
};

#endif


