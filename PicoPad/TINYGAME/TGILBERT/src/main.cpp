//       >>>>> T-I-N-Y G-I-L-B-E-R-T for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-Gilbert is free software: you can redistribute it and/or modify
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


// The Tiny-Gilbert source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tgilbert.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the tgilbert.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#include "../include.h"

extern PICOCOMPATIBILITY PICOKIT;

uint8_t xPicoScan;
uint8_t yPicoScan;

uint8_t Map[8][34]={{0}};
uint8_t timer=0;
int8_t scrool=0;
int8_t step4=0;
uint8_t MainAnim=0,LorR=1;
int8_t Jump=0;
int8_t  jumpcancel=0;
const float VSlide[9]={1,2,4,8,16,32,64,128,256};
uint8_t key[20][2]={{0}};
uint8_t keyS=0;
float VSlideOut=0;
uint8_t LevelMult=0,levelType=0;
uint8_t ByteMem=0;
uint8_t visible=1;
uint8_t injur=0;
uint8_t LIVE=0;

unsigned long currentMillis_TG;
unsigned long MemMillis_TG;

void FPS_Control_TA(uint8_t Ms_) {
  while ((currentMillis_TG - MemMillis_TG) < Ms_) {
    currentMillis_TG = millis();
  }
  MemMillis_TG = currentMillis_TG;
}

uint8_t delKey(uint8_t Xin,uint8_t Yin){
int8_t x=0;  
for (x=0;x<23;x++){
if   ((key[x][0]==0) && (key[x][1]==0)) {return 11;}
if ((key[x][0]==Xin) && (key[x][1]==Yin)) {return 0;}  }
return 11;}

void sound(uint8_t SND){
if (SND==1) {Sound(210,10);Sound(240,2);Sound(180,5);}
if (SND==2){for (uint8_t x=255;x>2;x--){ Sound(x,1);}
}
}

int main() {
PICOKIT.Init();
PICOKIT.SetColorBuffer(0,63,31,17,57,17,0,0,0);
//digitalWrite(3,LOW);
RESTARTGAME:
sound(1);sound(2);sound(2);sound(1);sound(2);sound(2);
_delay_ms(200);
ResetVar();
INTRO_TGILBERT();
_delay_ms(400);
 while(1){
INTRO_TGILBERT();
if (BUTTON_DOWN) {sound(1);break;}}
RESTARTLEVEL:
ResetVarNextLevel();
NEXTLEVEL:
if (levelType>9) {goto RESTARTGAME;}
_delay_ms(400);
DriftSprite MainSprite;
SpriteShiftInitialise(&MainSprite);
#define RBACKUP  if  ((CollisionCheck(&MainSprite)==1)){MainSprite.x4decalage--;}
#define LBACKUP  if  ((CollisionCheck(&MainSprite)==1)) {MainSprite.x4decalage++;}   
while(1){
if (TINYJOYPAD_RIGHT)  {
if (timer%4==0) {
MainAnim++;if (MainAnim>2) {MainAnim=0;}
}
LorR=0;
MainSprite.x4decalage++;RBACKUP;if (MainSprite.x4decalage>3) {MainSprite.x4decalage=0;MainSprite.MainPositionOnGridH++;}
}
if (TINYJOYPAD_LEFT) {
if (timer%4==0) {
MainAnim++;if (MainAnim>2) {MainAnim=0;}
}
LorR=1;
MainSprite.x4decalage--;LBACKUP;if (MainSprite.x4decalage<0) {MainSprite.x4decalage=3;MainSprite.MainPositionOnGridH--;}
} 
ScrollUpdate(&MainSprite); 
#define exclude(Spick) (ByteMem==Spick)
#define SpritePickup (exclude(11))
if (MainSprite.MainPositionOnGridV>=7) {sound(2);LIVE--;if (LIVE==0) {goto RESTARTGAME;}goto RESTARTLEVEL;}
for (uint8_t x = 0; x < 33; x++)
{
#define LevelS (x+scrool)/4 //(x+scrool)/4
  switch (levelType){
  case(0):LevelMult=Level0[LevelS];break;
  case(1):LevelMult=Level1[LevelS];break;
  case(2):LevelMult=Level2[LevelS];break;
  case(3):LevelMult=Level3[LevelS];break;
  case(4):LevelMult=Level4[LevelS];break;
  case(5):LevelMult=Level5[LevelS];break;
  case(6):LevelMult=Level6[LevelS];break;
  case(7):LevelMult=Level7[LevelS];break;
  case(8):LevelMult=Level8[LevelS];break;
  case(9):LevelMult=Level9[LevelS];break;
  default:goto RESTARTGAME;}
#define LevelShift (((x+scrool)%4)+(LevelMult*4))
  ByteMem=map1couche2[LevelShift];
  if ((SpritePickup)) {Map[1][x]=delKey(x+scrool,1);}else{Map[1][x]=ByteMem;}
  ByteMem=map1couche3[LevelShift];
  if ((SpritePickup)) {Map[2][x]=delKey(x+scrool,2);}else{Map[2][x]=ByteMem;}
  ByteMem=map1couche4[LevelShift];
  if ((SpritePickup)) {Map[3][x]=delKey(x+scrool,3);}else{Map[3][x]=ByteMem;}
  ByteMem=map1couche5[LevelShift];
  if ((SpritePickup)) {Map[4][x]=delKey(x+scrool,4);}else{Map[4][x]=ByteMem;}
  ByteMem=map1couche6[LevelShift];
  if ((SpritePickup)) {Map[5][x]=delKey(x+scrool,5);}else{Map[5][x]=ByteMem;}
  ByteMem=map1couche7[LevelShift];
  if ((SpritePickup)) {Map[6][x]=delKey(x+scrool,6);}else{Map[6][x]=ByteMem;}
  ByteMem=map1couche8[LevelShift];
  if ((SpritePickup)) {Map[7][x]=delKey(x+scrool,7);}else{Map[7][x]=ByteMem;}
}
if (Jump==0) {GravityUpdate(&MainSprite);}
if ((BUTTON_DOWN)&&(Jump==0)&&(jumpcancel==0)&&(CollisionCheck(&MainSprite)==0)){
if (MainSprite.y8decalage==0) {Jump=3;}
}
if (BUTTON_UP) {jumpcancel=0;}
if (Jump>0) {JumpProcedure(&MainSprite);}
#define pickup(Vadd,Hadd,SPRITE) (Map[MainSprite.MainPositionOnGridV+Vadd][MainSprite.MainPositionOnGridH+Hadd]==SPRITE)
#define Pictup2(SP) ((pickup(0,0,SP))||(pickup(0,1,SP))||(pickup(0,2,SP)))
#define Pictup4(SP2) (Pictup2(SP2)||(pickup(1,0,SP2))||(pickup(1,1,SP2))||(pickup(1,2,SP2)))
if ((MainSprite.y8decalage==0)) {
if( pickup(0,0,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH;keyS++;sound(1);}
if( pickup(0,1,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH+1;keyS++;sound(1);}
if( pickup(0,2,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH+2;keyS++;sound(1);}
if ((Pictup2(8))&&(injur==0)) {LIVE--;if (LIVE==0) {goto RESTARTGAME;}Jump=2;injur=30;sound(2);}
if (Pictup2(13)||Pictup2(14)) {if ((KeyinLevel[levelType]==keyS)&&(Jump>0)) {NextLevel();goto NEXTLEVEL;}}
Map[MainSprite.MainPositionOnGridV][MainSprite.MainPositionOnGridH]=MainSprite.DriftGrid[0][0];
Map[MainSprite.MainPositionOnGridV][MainSprite.MainPositionOnGridH+1]=MainSprite.DriftGrid[0][1];}else{
if( pickup(0,0,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH;keyS++;sound(1);}
if( pickup(0,1,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH+1;keyS++;sound(1);}
if( pickup(0,2,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH+2;keyS++;sound(1);}
if( pickup(1,0,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV+1;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH;keyS++;sound(1);}
if( pickup(1,1,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV+1;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH+1;keyS++;sound(1);}
if( pickup(1,2,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV+1;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH+2;keyS++;sound(1);}   
if ((Pictup2(8))&&(injur==0)) {LIVE--;if (LIVE==0) {goto RESTARTGAME;}Jump=2;injur=30;sound(2);}
if (Pictup4(13)||Pictup4(14)) {if ((KeyinLevel[levelType]==keyS)&&(Jump>0)) {NextLevel();goto NEXTLEVEL;}}
Map[MainSprite.MainPositionOnGridV][MainSprite.MainPositionOnGridH]=MainSprite.DriftGrid[0][0];
Map[MainSprite.MainPositionOnGridV][MainSprite.MainPositionOnGridH+1]=MainSprite.DriftGrid[0][1];
Map[MainSprite.MainPositionOnGridV+1][MainSprite.MainPositionOnGridH]=MainSprite.DriftGrid[1][0];
Map[MainSprite.MainPositionOnGridV+1][MainSprite.MainPositionOnGridH+1]=MainSprite.DriftGrid[1][1];
}
if (timer%2==0)  {if (injur>0) {if (visible==1) {visible=0;}else{visible=1;}injur--;}}
UpdateVerticalSlide(&MainSprite);
Tiny_Flip(&MainSprite);
FPS_Control_TA(28);
timer++;
if (timer>60) {timer=0;}
}}
////////////////////////////////// main end /////////////////////////////////

uint8_t SPEED_BLITZ(int8_t xPos,int8_t yPos,uint8_t xPASS,uint8_t yPASS,uint8_t FRAME,const uint8_t *SPRITES){
uint16_t WSPRITE=(SPRITES[0]);
uint16_t HSPRITE=(SPRITES[1]);
if ((xPASS>((xPos+(WSPRITE-1))))||(xPASS<xPos)||((yPASS<yPos)||(yPASS>(yPos+(HSPRITE-1))))) {return 0xFF;}
return SPRITES[2+(((xPASS-xPos)+((yPASS-yPos)*(WSPRITE)))+(FRAME*(HSPRITE*WSPRITE)))];
}

void INTRO_TGILBERT(void){
uint8_t y,x;
for (y = 0; y < 8; y++){ 
for (x = 0; x < 128; x++){ 
PICOKIT.TinyOLED_Send(x,y,SPEED_BLITZ(32,4,x,y,0,start));
}
}
PICOKIT.display();
}


void ScrollUpdate(DriftSprite* DSprite){
if ((DSprite->MainPositionOnGridH)<10) {if ((scrool>0)&&(step4<=3)) {step4=step4+1;}if ((step4>3)&&(scrool>0)) {step4=0;scrool--;DSprite->MainPositionOnGridH=DSprite->MainPositionOnGridH+1;}}
if ((DSprite->MainPositionOnGridH)>18) {step4=step4-1;if (step4<0) {step4=3;scrool++;DSprite->MainPositionOnGridH=DSprite->MainPositionOnGridH-1;}}
}

void UpdateVerticalSlide(DriftSprite* DSprite){
if (DSprite->y8decalage==0) {
if (visible==1){
DSprite->DriftGrid[0][0]=5;
DSprite->DriftGrid[0][1]=6;}else{
DSprite->DriftGrid[0][0]=0;
DSprite->DriftGrid[0][1]=0;
}}else{
if (visible==1){
DSprite->DriftGrid[0][0]=5;
DSprite->DriftGrid[0][1]=6;
DSprite->DriftGrid[1][0]=55;
DSprite->DriftGrid[1][1]=66;}else{
DSprite->DriftGrid[0][0]=0;
DSprite->DriftGrid[0][1]=0;
DSprite->DriftGrid[1][0]=0;
DSprite->DriftGrid[1][1]=0;   
}}}

void GravityUpdate(DriftSprite* DSprite){
int8_t memy8decalage=DSprite->y8decalage;
int8_t memMainPositionOnGridV=DSprite->MainPositionOnGridV;
DSprite->y8decalage=DSprite->y8decalage+2;
if (DSprite->y8decalage>7) {
DSprite->y8decalage=0;
DSprite->MainPositionOnGridV++;
}
if (CollisionCheck(DSprite)>=1){
DSprite->MainPositionOnGridV=memMainPositionOnGridV;
DSprite->y8decalage=memy8decalage;
}}

void JumpProcedure(DriftSprite* DSprite){
int8_t memo2=0;
if ((Jump>0)){
memo2=DSprite->MainPositionOnGridV;
DSprite->y8decalage=DSprite->y8decalage-(Jump*2);
if (DSprite->y8decalage<0)   {DSprite->y8decalage=7; DSprite->MainPositionOnGridV--;Jump--;
if ((DSprite->MainPositionOnGridV<=0)||(jumpcancel==1)||(CollisionCheck(DSprite)>=1)) {
Jump=0;
jumpcancel=1; 
DSprite->y8decalage=0;
DSprite->MainPositionOnGridV=memo2;
Jump=0;jumpcancel=1;}
if (Jump==0) {jumpcancel=1;}
}}}  

int8_t CollisionCheck(DriftSprite* DSprite){
int8_t xscan=0,yscan=0;
 //varable de la grille
#define MX DSprite->MainPositionOnGridH
#define MY DSprite->MainPositionOnGridV
#define MXDRIFT DSprite->x4decalage
#define MYDRIFT DSprite->y8decalage
#define x1 (MX*4)+MXDRIFT
#define y1 (MY*8)+MYDRIFT
#define x2 (MX*4)+MXDRIFT+7
#define y2 (MY*8)+MYDRIFT
#define x3 (MX*4)+MXDRIFT
#define y3 (MY*8)+MYDRIFT+7
#define x4 (MX*4)+MXDRIFT+7
#define y4 (MY*8)+MYDRIFT+7
#define bx1 (MX+xscan)*4
#define by1 (MY+yscan)*8
#define bx2 ((MX+xscan)*4)+3
#define by2 (MY+yscan)*8
#define bx3 (MX+xscan)*4
#define by3 ((MY+yscan)*8)+7
#define bx4 ((MX+xscan)*4)+3
#define by4 ((MY+yscan)*8)+7
#define NoTested ((Map[MY+yscan][MX+xscan]!=8)&&(Map[MY+yscan][MX+xscan]!=0)&&(Map[MY+yscan][MX+xscan]!=13)&&(Map[MY+yscan][MX+xscan]!=14)&&(Map[MY+yscan][MX+xscan]!=11)&&(Map[MY+yscan][MX+xscan]!=5)&&(Map[MY+yscan][MX+xscan]!=6)&&(Map[MY+yscan][MX+xscan]!=55)&&(Map[MY+yscan][MX+xscan]!=66))
for (yscan=-1;yscan<3;yscan++){
for (xscan=-1;xscan<3;xscan++){
if (NoTested) { 
if ((x1>bx2)||(x2<bx1)||(y1>by3)||(y3<by1)) {}else{return 1;}
}}}
return 0;
}

void ResetVar(void){
ResetVarNextLevel();
levelType=0;
LIVE=7;
}

void ResetVarNextLevel(void){
scrool=0;
step4=0;
MainAnim=0;
LorR=1;
Jump=0;
jumpcancel=0;
VSlideOut=0;
for (uint8_t x=0;x<20;x++){
key[x][0]=0;
key[x][1]=0;}
keyS=0;
LevelMult=0;}

void NextLevel(void){
ResetVarNextLevel();
levelType++; 
sound(2);sound(2);sound(2);sound(2);}

void TinyMainShift(DriftSprite* DSprite){
DSprite->DriftGrid[0][0]=5;
DSprite->DriftGrid[0][1]=6;
DSprite->DriftGrid[1][0]=5;
DSprite->DriftGrid[1][1]=6;}

void SpriteShiftInitialise(DriftSprite* DSprite){
DSprite->DriftGrid[0][0]=0; //bas gauche
DSprite->DriftGrid[0][1]=0; //bas droite
DSprite->DriftGrid[1][0]=0; //haut gauche
DSprite->DriftGrid[1][1]=0; //haut droite
DSprite->x4decalage=0;
DSprite->y8decalage=0;
DSprite->MainPositionOnGridH=11;
DSprite->MainPositionOnGridV=3;
}



void ResetPicoScan(void){
xPicoScan=0;
yPicoScan=xPicoScan;
PICOKIT.clearPicoBuffer();
}

void SendColorByte(uint8_t Byte_,uint16_t Col_){
PICOKIT.DirectDraw(xPicoScan,yPicoScan,Byte_,Col_);
if (xPicoScan<127) {xPicoScan++;}else{
if (yPicoScan<7) {yPicoScan++;xPicoScan=0;}	
	}
}

#define ColBrick color(18,56,31)//pic
#define ColBrick2 color(20,42,20)
#define ColMain color(0,52,21)
#define ColDoor color(31,0,0)
#define Colkey color(31,63,0)
#define ColFloor color(12,46,12)


/////////////////////////////////////////////////////
void Tiny_Flip(DriftSprite* DSprite){
ResetPicoScan();
uint8_t *buffer=PICOKIT.getBuffer();
uint8_t nn,x,m,n,t,Start,decal;
uint8_t while1=1;
#define PrecessQuit nn++;if (nn>127) {while1=0;break;}
for (x=0;x<LIVE-1;x++){
for(t=0;t<4;t++){SendColorByte(sprite26[t],ColMain);}
for(t=0;t<4;t++){SendColorByte(sprite27[t],ColMain);}
}
for (x=0;x<11+(3-(LIVE-1));x++){
for(t=0;t<8;t++){SendColorByte(0x00,color(0,0,0));}}
if ((KeyinLevel[levelType]==keyS)&&(timer<=30)) {
for (x=0;x<4;x++){
SendColorByte(sprite12[x],Colkey);}
for (x=0;x<3;x++){
for (t=0;t<4;t++){SendColorByte(0x00,color(0,0,0));}
}
}else{
for (x=0;x<4;x++){
for (t=0;t<4;t++){SendColorByte(0x00,color(0,0,0));}
}}  
for (m = 1; m < 8; m++)
{

n=0; 
nn=0;  
while1=1;
Start=3-step4; 
#define decalIN for(decal=0;decal<DSprite->x4decalage;decal++){ SendColorByte(0X00,color(0,0,0));PrecessQuit}// for main sprite scr pix2pix
#define decalOUT for(decal=0;decal<4-DSprite->x4decalage;decal++){SendColorByte(0X00,color(0,0,0));PrecessQuit}// for main sprite scr pix2pix
while(while1){
if ((Map[m][n]==7)&&(while1!=0)) {
for(t=Start;t<4;t++){ SendColorByte(sprite7[t],color(0,0,31));PrecessQuit}Start=0;
// main sprite 
}else if (((Map[m][n]==5)||(Map[m][n]==55))&&(while1!=0)) {
if (Map[m][n]==55) {VSlideOut=((100/VSlide[8-DSprite->y8decalage])/100);}else{VSlideOut=VSlide[DSprite->y8decalage];}
decalIN;
if (LorR==1){
if (MainAnim==0) {
for(t=0;t<4;t++){ SendColorByte(sprite20[t]*VSlideOut,ColMain);PrecessQuit}Start=0;
}
if (MainAnim==1) { 
for(t=0;t<4;t++){SendColorByte(sprite22[t]*VSlideOut,ColMain);PrecessQuit}Start=0;
}
if (MainAnim==2) {
for(t=0;t<4;t++){SendColorByte(sprite24[t]*VSlideOut,ColMain);PrecessQuit}Start=0;
} 
}else{  
if (MainAnim==0) {
for(t=0;t<4;t++){SendColorByte(sprite26[t]*VSlideOut,ColMain);PrecessQuit}Start=0;
}
if (MainAnim==1) {
for(t=0;t<4;t++){SendColorByte(sprite28[t]*VSlideOut,ColMain);PrecessQuit}Start=0; 
}
if (MainAnim==2) {
for(t=0;t<4;t++){SendColorByte(sprite30[t]*VSlideOut,ColMain);PrecessQuit}Start=0;
}}
}else if (((Map[m][n]==6)||(Map[m][n]==66))&&(while1!=0)) {
if (Map[m][n]==66) {VSlideOut=((100/VSlide[8-DSprite->y8decalage])/100);}else{VSlideOut=VSlide[DSprite->y8decalage];}
if (LorR==1){
if (MainAnim==0) {
for(t=0;t<4;t++){ SendColorByte(sprite21[t]*VSlideOut,ColMain);PrecessQuit Start=DSprite->x4decalage;}
}
if (MainAnim==1) {
for(t=0;t<4;t++){ SendColorByte(sprite23[t]*VSlideOut,ColMain);PrecessQuit Start=DSprite->x4decalage;}
}
if (MainAnim==2) {
for(t=0;t<4;t++){ SendColorByte(sprite25[t]*VSlideOut,ColMain);PrecessQuit Start=DSprite->x4decalage;}
} }else{  
if (MainAnim==0) {
for(t=0;t<4;t++){ SendColorByte(sprite27[t]*VSlideOut,ColMain);PrecessQuit Start=DSprite->x4decalage;}
}
if (MainAnim==1) {
for(t=0;t<4;t++){ SendColorByte(sprite29[t]*VSlideOut,ColMain);PrecessQuit Start=DSprite->x4decalage;} 
}
if (MainAnim==2) {
for(t=0;t<4;t++){ SendColorByte(sprite31[t]*VSlideOut,ColMain);PrecessQuit Start=DSprite->x4decalage;}
}}
//fin main sprite
}else if ((Map[m][n]==1)&&(while1!=0)) {
for(t=Start;t<4;t++){  SendColorByte(sprite1[t],ColBrick);PrecessQuit}Start=0;
}else if ((Map[m][n]==2)&&(while1!=0)) {
for(t=Start;t<4;t++){ SendColorByte(sprite2[t],ColBrick);PrecessQuit}Start=0;
}else if ((Map[m][n]==3)&&(while1!=0)) {
for(t=Start;t<4;t++){ SendColorByte(sprite3[t],ColBrick2);PrecessQuit}Start=0;
}else if ((Map[m][n]==4)&&(while1!=0)) {
for(t=Start;t<4;t++){ SendColorByte(sprite4[t],ColBrick2);PrecessQuit}Start=0;
}else if ((Map[m][n]==8)&&(while1!=0)) {
for(t=Start;t<4;t++){SendColorByte(sprite8[t],ColBrick);PrecessQuit}Start=0;
}else if ((Map[m][n]==15)&&(while1!=0)) {
for(t=Start;t<4;t++){ SendColorByte(sprite15[t],ColFloor);PrecessQuit}Start=0;
}else if ((Map[m][n]==16)&&(while1!=0)) {
for(t=Start;t<4;t++){ SendColorByte(sprite16[t],ColFloor);PrecessQuit}Start=0;
}
else if ((Map[m][n]==11)&&(while1!=0)) {
if (timer>30){
for(t=Start;t<4;t++){
 SendColorByte(sprite11[t],Colkey);PrecessQuit}Start=0;
}else{
for(t=Start;t<4;t++){
SendColorByte(sprite12[t],Colkey);PrecessQuit}Start=0;
}}else if ((Map[m][n]==13)&&(while1!=0)) {
for(t=Start;t<4;t++){ SendColorByte(sprite13[t],ColDoor);PrecessQuit}Start=0;
}else if ((Map[m][n]==14)&&(while1!=0)) {
for(t=Start;t<4;t++){SendColorByte(sprite14[t],ColDoor);PrecessQuit}Start=0;
}
else{if (while1!=0){
for(t=Start;t<4;t++){
SendColorByte(0x00,color(0,0,0));PrecessQuit}Start=0;
}}
if (while1!=0) {n++; }
}

}
PICOKIT.DisplayColor();   
}
