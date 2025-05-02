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

#include "../include.h"


extern PICOCOMPATIBILITY PICOKIT;

uint8_t blitzSprite(int8_t xPos,int8_t yPos,uint8_t xPASS,uint8_t yPASS,uint8_t FRAME,const uint8_t *SPRITES){
uint8_t OUTBYTE;
uint8_t WSPRITE=(SPRITES[0]);
uint8_t HSPRITE=(SPRITES[1]);
uint16_t Wmax=((HSPRITE*WSPRITE)+1);
uint16_t PICBYTE=FRAME*(Wmax-1);
int8_t RECUPELINEY=RecupeLineY(yPos);
if ((xPASS>((xPos+(WSPRITE-1))))||(xPASS<xPos)||((RECUPELINEY>yPASS)||((RECUPELINEY+(HSPRITE))<yPASS))) {return 0x00;}
int8_t SPRITEyLINE=(yPASS-(RECUPELINEY));
uint8_t SPRITEyDECALAGE=(RecupeDecalageY(yPos));
uint16_t ScanA=(((xPASS-xPos)+(SPRITEyLINE*WSPRITE))+2);
uint16_t ScanB=(((xPASS-xPos)+((SPRITEyLINE-1)*WSPRITE))+2);
if (ScanA>Wmax) {OUTBYTE=0x00;}else{OUTBYTE=SplitSpriteDecalageY(SPRITEyDECALAGE,SPRITES[ScanA+(PICBYTE)],1);}
if ((SPRITEyLINE>0)) {
uint8_t OUTBYTE2=SplitSpriteDecalageY(SPRITEyDECALAGE,SPRITES[ScanB+(PICBYTE)],0);
if (ScanB>Wmax) {return OUTBYTE;}else{return OUTBYTE|OUTBYTE2;}
}else{return OUTBYTE;}
}

uint8_t BACKGROUND(uint8_t xPASS,uint8_t yPASS,const uint8_t *BITMAP){
return BITMAP[xPASS+(yPASS*128)];
}

uint8_t SPEED_BLITZ(int8_t xPos,int8_t yPos,uint8_t xPASS,uint8_t yPASS,uint8_t FRAME,const uint8_t *SPRITES){
uint8_t WSPRITE=(SPRITES[0]);
uint8_t HSPRITE=(SPRITES[1]);
if ((xPASS>((xPos+(WSPRITE-1))))||(xPASS<xPos)||((yPASS<yPos)||(yPASS>(yPos+(HSPRITE-1))))) {return 0x00;}
return SPRITES[2+(((xPASS-xPos)+((yPASS-yPos)*(WSPRITE)))+(FRAME*(HSPRITE*WSPRITE)))];
}

uint8_t SplitSpriteDecalageY(uint8_t decalage,uint8_t Input,uint8_t UPorDOWN){
if (UPorDOWN) {return Input<<decalage;}
return Input>>(8-decalage);
}

int8_t RecupeLineY(int8_t Valeur){
return (Valeur>>3); 
}

uint8_t RecupeDecalageY(uint8_t Valeur){
return (Valeur-((Valeur>>3)<<3));
}
/*
int8_t Mymap(int8_t x, int8_t in_min, int8_t in_max, int8_t out_min, int8_t out_max){
return ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}
*/
uint8_t Trace_LINE(uint8_t DESACTIVE_,int8_t x1,int8_t y1,int8_t x2,int8_t y2,uint8_t xPASS,uint8_t yPASS){
uint8_t Y1_=RecupeLineY(y1);
uint8_t Y2_=RecupeLineY(y2);
if ((xPASS<x1)&&(xPASS<x2)) return 0;
if ((xPASS>x1)&&(xPASS>x2)) return 0;
if (Y1_<Y2_) {
if (yPASS<Y1_){return 0;}
if (yPASS>Y2_){return 0;}
}else{
if (yPASS>Y1_){return 0;}
if (yPASS<Y2_){return 0;} 
}
return DIRECTION_LINE(DESACTIVE_,x1,y1,x2,y2,xPASS,yPASS);
}

uint8_t DIRECTION_LINE(uint8_t DESACTIVE_,int8_t x1,int8_t y1,int8_t x2,int8_t y2,uint8_t xPASS,uint8_t yPASS){
 uint8_t XL_=0;
 uint8_t YL_=0;
 uint8_t RESULTANT=0;
 uint8_t Yref_=0;
XL_=abs(x1-x2);
YL_=abs(y1-y2);
if ((XL_<YL_)&&(DESACTIVE_==0)) {
return Return_Full_Byte(x1,y1,x2,y2,xPASS,yPASS);
 }else{
RESULTANT=Mymap(xPASS,x1,x2,y1,y2);
Yref_=RecupeLineY(RESULTANT);
if (Yref_==yPASS) {return (1<<(RecupeDecalageY(RESULTANT)));}else{return 0;}
  }
}

uint8_t Return_Full_Byte(int8_t x1,int8_t y1,int8_t x2,int8_t y2,uint8_t xPASS,uint8_t yPASS){
uint8_t Byte_add_=0;
uint8_t t=0,tout=0;
uint8_t A_=0,B_=0;
A_=(yPASS<<3);
B_=(A_+7);
if (y1>y2){
Universal_Swap(&y1,&y2);
Universal_Swap(&x1,&x2);}
A_=(A_<y1)?y1:A_;
B_=(B_>y2)?y2:B_;
for (t=A_;t<B_+1;t++){
if (yPASS==RecupeLineY(t)) {
tout=Mymap(t,y1,y2,x1,x2);
Byte_add_|=(tout==xPASS)?(0b00000001<<(RecupeDecalageY(t))):0x00;
}}
return Byte_add_;
}

void Universal_Swap(int8_t *A_,int8_t *B_){ //for int8_t only
int8_t T_=*A_;
*A_=*B_;
*B_=T_;
}

uint8_t RECONSTRUCT_BYTE(uint8_t Min_,uint8_t Max_){
uint8_t A_=(0xff<<Min_);
uint8_t B_=0xff-(0xff<<Max_);
return (A_&B_);
}

void TinySound(uint8_t freq_,uint8_t dur){
Sound(freq_,dur);
}

void TINYJOYPAD_INIT(void){
}
