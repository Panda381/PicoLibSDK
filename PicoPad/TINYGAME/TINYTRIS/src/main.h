//     >>>>>  T-I-N-Y  T-R-I-S v2.1 for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                     Programmer: Daniel C 2019-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-tris is free software: you can redistribute it and/or modify
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


// The Tiny-tris source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "TinyTris.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "ReadMe.txt".

#ifndef _MAIN_H
#define _MAIN_H

void reset_Score_TTRIS(void);
uint8_t PSEUDO_RND_TTRIS(void);
void SND_TTRIS(uint8_t Snd_TTRIS);
void INTRO_MANIFEST_TTRIS(void);
void END_DROP_TTRIS(void);
void SETUP_NEW_PREVIEW_PIECE_TTRIS(uint8_t *Rot_TTRIS);
void CONTROLE_TTRIS(uint8_t *Rot_TTRIS);
void Game_Play_TTRIS(void);
uint8_t End_Play_TTRIS(void);
void DELETE_LINE_TTRIS(void);
uint8_t Calcul_of_Score_TTRIS(uint8_t Tmp_TTRIS);
void FLASH_LINE_TTRIS(uint8_t *PASS_LINE);
void PAINT_LINE_TTRIS(uint8_t VISIBLE,uint8_t *PASS_LINE);
void Clean_Grid_TTRIS(uint8_t *PASS_LINE);
uint8_t CHECK_if_Rot_Ok_TTRIS(uint8_t *Rot_TTRIS);
uint8_t Check_collision_x_TTRIS(int8_t x_Axe);
uint8_t Check_collision_y_TTRIS(int8_t y_Axe);
void Move_Piece_TTRIS(void);
void Ou_suis_Je_TTRIS(int8_t xx_,int8_t yy_);
void Select_Piece_TTRIS(uint8_t Piece_);
void rotate_Matrix_TTRIS(uint8_t ROT);
uint8_t Scan_Piece_Matrix_TTRIS(int8_t x_Mat,int8_t y_Mat);
uint8_t GRID_STAT_TTRIS(int8_t X_SCAN,int8_t Y_SCAN);
uint8_t CHANGE_GRID_STAT_TTRIS(int8_t X_SCAN,int8_t Y_SCAN,uint8_t VALUE);
uint8_t blitzSprite_TTRIS(int8_t xPos,int8_t yPos,uint8_t xPASS,uint8_t yPASS,uint8_t FRAME,const uint8_t *SPRITES);
uint8_t H_grid_Scan_TTRIS(uint8_t xPASS);
uint8_t Recupe_TTRIS(uint8_t xPASS,uint8_t yPASS);
uint8_t NEXT_BLOCK_TTRIS(uint8_t xPASS,uint8_t yPASS);
uint8_t RECUPE_BACKGROUND_TTRIS(uint8_t xPASS,uint8_t yPASS);
uint8_t DropPiece_TTRIS(uint8_t xPASS,uint8_t yPASS);
uint8_t SplitSpriteDecalageY_TTRIS(uint8_t decalage,uint8_t Input,uint8_t UPorDOWN);
uint8_t RecupeLineY_TTRIS(uint8_t Valeur);
uint8_t RecupeDecalageY_TTRIS(uint8_t Valeur);
void Tiny_Flip_TTRIS(uint8_t HR_TTRIS);
void Flip_intro_TTRIS(uint8_t *TIMER1);
uint8_t intro_TTRIS(uint8_t xPASS,uint8_t yPASS,uint8_t *TIMER1);
uint8_t Recupe_Start_TTRIS(uint8_t xPASS,uint8_t yPASS,uint8_t *TIMER1);
uint8_t recupe_Chateau_TTRIS(uint8_t xPASS,uint8_t yPASS);
uint8_t recupe_SCORES_TTRIS(uint8_t xPASS,uint8_t yPASS);
void Convert_Nb_of_line_TTRIS(void);
uint8_t recupe_Nb_of_line_TTRIS(uint8_t xPASS,uint8_t yPASS);
uint8_t recupe_LEVEL_TTRIS(uint8_t xPASS,uint8_t yPASS);
void INIT_ALL_VAR_TTRIS(void);
void recupe_HIGHSCORE_TTRIS(void);
void Reset_Value_TTRIS(void);
void save_HIGHSCORE_TTRIS(void);
void Check_NEW_RECORD(void);
uint8_t checksum(uint8_t Byte_);
void Sound_TTRIS(uint8_t freq_,uint8_t dur);
void TinyOLED_Send(uint8_t x_,uint8_t y_,uint8_t Byte_);

#endif // _MAIN_H

