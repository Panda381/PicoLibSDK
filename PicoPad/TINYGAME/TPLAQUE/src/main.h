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

#ifndef _MAIN_H
#define _MAIN_H

void INIT_NEW_GAME_TPLAQUE(void);
void ADJUST_TUBE_TIMER_TPLAQUE(void);
void ADJUST_TUBE_TPLAQUE(void);
void SCORE_ADD_TPLAQUE(uint8_t SC_);
void UPDATE_PANNEL_TPLAQUE(void);
void COMPIL_SCO_TPLAQUE(void);
uint8_t GAME_PLAY_TPLAQUE(void);
uint8_t  END_OF_LEVEL_TPLAQUE(void);
void DSOUND_TPLAQUE(uint8_t T_AD);
void DSOUND2_TPLAQUE(void);
void DECOUNT_TPLAQUE(void);
void RESTORE_TEETH_TPLAQUE(void);
void Sound_ADDTEETH_TPLAQUE(void);
void ADD_TEETH_TPLAQUE(void);
uint8_t PUT_TEETH_TPLAQUE(Sprite_TPLAQUE &TEETH_);
void NEXT_LEVEL_TPLAQUE(void);
void LOAD_LEVEL_TPLAQUE(void);
void DELETE_TEETH_TPLAQUE(void);
void HIT_BOX_TPLAQUE(void);
void CHECK_NUMBER_OF_TEETH_TPLAQUE(void);
void Check_Collision_WBALISTIC_TPLAQUE(uint8_t T_V);
uint8_t Check_Collision_WTEETH_TPLAQUE(int8_t FX,int8_t FW,int8_t FY,int8_t FH,Sprite_TPLAQUE &SP_);
uint8_t Collision_WTEETH_AGAIN_TPLAQUE(uint8_t &Count,Sprite_TPLAQUE &TEETH_);
void INVERT_FOOD_DIRECTION_TPLAQUE(void);
void TEETH_RESET_TPLAQUE(void);
void FOOD_RESET_TPLAQUE(void);
void FOOD_MOVE_UPDATE_TPLAQUE(void);
void ADD_FOOD_TPLAQUE(void);
void COLAPS_FOOD_TPLAQUE(uint8_t T_V);
void UPDATE_FOOD_TRIGER_TPLAQUE(void);
void UPDATE_CHANGEX_TPLAQUE(void);
uint8_t blitzSprite_TPLAQUE(int8_t xPos,int8_t yPos,uint8_t xPASS,uint8_t yPASS,uint8_t FRAME,const uint8_t *SPRITES);
uint8_t BACK_TPLAQUE(uint8_t xPASS,uint8_t yPASS);
uint8_t TEETH_TPLAQUE(uint8_t xPASS);
uint8_t TEETH_DOWN_TPLAQUE(uint8_t xPASS,uint8_t MULT);
uint8_t TUBE_TPLAQUE(uint8_t xPASS,uint8_t yPASS);
uint8_t Food_Recupe_TPLAQUE(uint8_t xPASS,uint8_t yPASS);
void Recupe_TPLAQUE(uint8_t xPASS,uint8_t yPASS);
uint8_t SplitSpriteDecalageY_TPLAQUE(uint8_t decalage,uint8_t Input,uint8_t UPorDOWN);
int8_t RecupeLineY_TPLAQUE(int8_t Valeur);
uint8_t RecupeDecalageY_TPLAQUE(uint8_t Valeur);
void Tiny_Flip_TPLAQUE(uint8_t T_FLIP);
void INTRO_TPLAQUE(uint8_t xPASS,uint8_t yPASS);
uint8_t TRACE_TPLAQUE(uint8_t xPASS,uint8_t yPASS);
uint8_t recupe_SCORES_TPLAQUE(uint8_t xPASS,uint8_t yPASS);
void Recupe_ExtraTeeth_TPLAQUE(uint8_t xPASS,uint8_t yPASS);
uint8_t SPEED_BLITZ_TPLAQUE(int8_t xPos,int8_t yPos,uint8_t xPASS,uint8_t yPASS,uint8_t FRAME,const uint8_t *SPRITES);


void Draw_Waepon(uint8_t xPASS,uint8_t yPASS);
void Draw_Food(uint8_t xPASS,uint8_t yPASS);
void Draw_Tube(uint8_t xPASS,uint8_t yPASS);
void Draw_Extra_Teeth(uint8_t xPASS,uint8_t yPASS);
void Draw_BACKGROUND(uint8_t xPASS,uint8_t yPASS);
void Draw_Scores(uint8_t xPASS,uint8_t yPASS);


#endif // _MAIN_H
