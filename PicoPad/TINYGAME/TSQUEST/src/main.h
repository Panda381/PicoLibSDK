//       >>>>> T-I-N-Y S-Q-U-E-S-T for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-SQuest is free software: you can redistribute it and/or modify
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


// The Tiny-SQuest source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tsquest.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the tsquest.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#ifndef _MAIN_H
#define _MAIN_H

int main() ;
void RENDER_INTRO_TSQUEST(uint8_t BL_);
void RESET_VAR_TSQUEST(void) ;
void REST_LEVEL_TSQUEST(void) ;
void Init_TSQUEST(void) ;
uint8_t Amount_Sprite_SET_TSQUEST(uint8_t Level_) ;
void Level_Adjust_TSQUEST(uint8_t Level_) ;
uint8_t Blink_Start_TSQUEST(uint8_t X_, uint8_t Y_, uint8_t BL_) ;
void Intro_TSQUEST(void) ;
void CheckCollisionSolo_TSQUEST(void) ;
void NEW_LEVEL_TSQUEST(void) ;
void Respond_TSQUEST(void) ;
void Game_Over_TSQUEST(void) ;
void Kiling_Main_TSQUEST(void) ;
void Refresh_Anim_TSQUEST(void) ;
void Refresh_Main_Anim_TSQUEST(void) ;
void Sound_1_TSQUEST(void);
void Start_Proccess_Destroy_Main_TSQUEST(void) ;
void Start_Proccess_Destroy_Main_Step3_TSQUEST(void) ;
void Start_Proccess_Destroy_Main_Step4_TSQUEST(void) ;
void Step2_counter_TSQUEST(uint8_t Skip_) ;
void Step3_counter_TSQUEST(void) ;
void JoyPad_Refresh_TSQUEST(void) ;
void OX_REDUCE_TSQUEST(void) ;
void REMOVE_DIVER_TSQUEST(void) ;
void Refill_TSQUEST(uint8_t Y_) ;
void NEXT_LEVEL_TRIGGER_TSQUEST(void) ;
void NEXT_LEVEL_TSQUEST(void) ;
void UPDATE_GAMEPLAY_TSQUEST(void) ;
int8_t P_TSQUEST(uint8_t Direction_, uint8_t position_) ;
uint8_t LINE_TSQUEST(uint8_t position_) ;
void Check_Endingline_TSQUEST(uint8_t Line_) ;
void LIBERATE_LINE_TSQUEST(uint8_t Line_) ;
void MAKE_LINE_TSQUEST(uint8_t *A_, uint8_t *B_, uint8_t *C_) ;
void FLIPFLOP2_TSQUEST(void) ;
void FLIPFLOP_TSQUEST(void) ;
void OSD_Diver_TSQUEST(uint8_t STATE_) ;
void OSD_OX_TSQUEST(uint8_t STATE_) ;
void OSD_MANAGER_SQUEST(void) ;
void RND3X_tsquest(void) ;
uint8_t RECUP_RND3_tsquest(void) ;
void RND2X_tsquest(void) ;
uint8_t RECUP_RND2_tsquest(void) ;
int8_t Recup_sin_Add_TSQUEST(uint8_t Activ_) ;
void Sin_Move_TSQUEST(void) ;
void CHECK_COLLISION_BALLISTIC_TSQUEST(void) ;
void CHECK_OTHER_COLLISION_BALLISTIC_TSQUEST(void) ;
void Check_Collision_Main2Other_TSQUEST(void) ;
void SCORE_ADD_TSQUEST(void) ;
int8_t COLLISION_TSQUEST(int8_t x, int8_t y, int8_t w, int8_t h, int8_t x2, int8_t y2, int8_t w2, int8_t h2) ;
uint8_t Balistic_Line_TSQUEST(uint8_t Sprite_, uint8_t State_) ;
uint8_t Check_if_Deployed_TSQUEST(uint8_t T_) ;
void UPDATE_BALLISTIC_TSQUEST(void) ;
uint8_t Recupe_SPRBANK_TSQUEST(void);
uint8_t RECUPE_MAIN_TSQUEST(uint8_t xPASS, uint8_t yPASS) ;
uint8_t RECUPE_BALLISTIC_MAIN_TSQUEST(uint8_t xPASS, uint8_t yPASS) ;
uint8_t RECUPE_BALLISTIC_OTHER_TSQUEST(uint8_t xPASS, uint8_t yPASS) ;
void RECUP_RANGE_TSQUEST(uint8_t yPASS) ;
uint8_t RECUPE_OTHER_TSQUEST(uint8_t xPASS, uint8_t yPASS) ;
uint8_t RecupeRanged_TSQUEST(uint8_t xPASS, uint8_t yPASS) ;
uint8_t Recupe_SUBsolo_TSQUEST(uint8_t xPASS, uint8_t yPASS) ;
void RENDER_TSQUEST(void) ;
void RENDER_DISPLAY_TSQUEST(uint8_t FlipFlop1_, uint8_t FlipFlop2_) ;
uint8_t Recupe_OX_TSQUEST(uint8_t Flip_, uint8_t xPASS) ;
void Config_Display_Diver_TSQUEST(uint8_t *A_, uint8_t *B_) ;
uint8_t Recupe_Diver_TSQUEST(uint8_t xPASS) ;
uint8_t Recupe_LIVE_TSQUEST(uint8_t xPASS) ;
uint8_t FullDisplayRefresh_TSQUEST(uint8_t FlipFlop1_, uint8_t FlipFlop2_, uint8_t xPASS, uint8_t yPASS) ;
void COMPIL_SCO_TSQUEST(void) ;
uint8_t recupe_SCORES_TSQUEST(uint8_t xPASS, uint8_t yPASS) ;
uint16_t Return_Spk_Col(uint8_t Spk_);


#endif // _MAIN_H
