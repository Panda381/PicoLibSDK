//      >>>>> T-I-N-Y  P-A-C-M-A-N for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2019-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  tiny-pacman is free software: you can redistribute it and/or modify
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


// The Tiny-Pacman source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tinypac.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "ReadMe.txt".

#include "../include.h"

// var public
uint16_t ColorPrint=0;
uint8_t LevelColor=0;
const uint8_t LvlColR[4]={0,31,31,0};
const uint8_t LvlColB[4]={31,31,0,0};
extern uint8_t BUFFER_SSD[1024];
extern uint8_t SnD_;
uint8_t LEVELSPEED;
uint8_t GobbingEND;
uint8_t LIVE;
uint8_t INGAME;
uint8_t Gobeactive;
uint8_t TimerGobeactive;
uint8_t add;
uint8_t dotsMem[9];
int8_t dotscount;
uint8_t Frame;
unsigned long currentMillis=0;
unsigned long MemMillis=0;
//15
#define FRAME_CONTROL while((currentMillis-MemMillis)<50){currentMillis=pico_millis();}MemMillis=currentMillis


// fin var public


typedef struct PERSONAGE{
uint8_t anim=0;
uint8_t guber=0;
uint8_t type=0;
uint8_t DirectionAnim=0;
uint8_t DirectionH=0;
uint8_t DirectionV=0;
uint8_t x=0;
uint8_t Decalagey=0;
int8_t y=0;
uint8_t switchanim=0;
}PERSONAGE;
 
enum {PACMAN=0,FANTOME=1,FRUIT=2};

int main() {
SoundSet();
uint8_t t;
PERSONAGE Sprite[5];
NEWGAME:
LevelColor=0;
ResetVar();
LIVE=3;
goto New;
NEWLEVEL:
if (LEVELSPEED>10) {LEVELSPEED=LEVELSPEED-10;
if ((LEVELSPEED==160)||(LEVELSPEED==120)||(LEVELSPEED==80)||(LEVELSPEED==40)||(LEVELSPEED==10)){    
if (LIVE<3) {LIVE++; for(t=0;t<=4;t++){
Sound(80,100);delay(300);
}}}}
New:
GobbingEND=(LEVELSPEED/2);
for(t=0;t<9;t++){
dotsMem[t]=0xff;
}
RESTARTLEVEL:
Gobeactive=0;
Sprite[0].type=PACMAN;
Sprite[0].x=64;
Sprite[0].y=3;
Sprite[0].Decalagey=5;
Sprite[0].DirectionV=2;
Sprite[0].DirectionH=2;
Sprite[0].DirectionAnim=0;
Sprite[1].type=FANTOME;
Sprite[1].x=76;
Sprite[1].y=4;
Sprite[1].guber=0;
Sprite[2].type=FANTOME;
Sprite[2].x=75;
Sprite[2].y=5;
Sprite[2].guber=0;
Sprite[3].type=FANTOME;
Sprite[3].x=77;
Sprite[3].y=4;
Sprite[3].guber=0;
Sprite[4].type=FANTOME;
Sprite[4].x=76;
Sprite[4].y=5;
Sprite[4].guber=0;
while(1){
//joystick
if (BUTTON_DOWN) {StartGame(&Sprite[0]);}
if (INGAME) {
if (TINYJOYPAD_LEFT) {Sprite[0].DirectionV=0;}
else if (TINYJOYPAD_RIGHT) {Sprite[0].DirectionV=1;}
if (TINYJOYPAD_DOWN) {Sprite[0].DirectionH=1;}
else if (TINYJOYPAD_UP) {Sprite[0].DirectionH=0;}
//fin joystick
if (TimerGobeactive>1)  {TimerGobeactive--;}else{if (TimerGobeactive==1) {TimerGobeactive=0;Gobeactive=0;}}}
if (Frame<24) {Frame++;}else{Frame=0;}
if (CollisionPac2Caracter(&Sprite[0])==0) {RefreshCaracter(&Sprite[0]);}else{
Sound(100,200);Sound(75,200);Sound(50,200);Sound(25,200);Sound(12,200);delay(400);
if (LIVE>0) {LIVE--;goto RESTARTLEVEL;}else{goto NEWGAME;}}
if (Frame%2==0) {
Tiny_Flip(0,&Sprite[0]);
FRAME_CONTROL;	
if (INGAME==1){
for (uint8_t t=0;t<=139;t=t+2){
Sound(Music[t]-48,(Music[t+1])-90); 
}INGAME=2;}
}else{
for(t=0;t<63;t++){
if (checkDotPresent(t)) {break;}else{if (t==62) { for(uint8_t r=0;r<60;r++){Sound(2+r,10);Sound(255-r,20);}delay(1000);if (LevelColor<3) {LevelColor++;}else{LevelColor=0;}  goto NEWLEVEL;}}
}}
if ((Gobeactive)&&(Frame%2==0)) {Sound((255-TimerGobeactive),1);}
        #if USE_SCREENSHOT
		switch (KeyGet())
		{
		case KEY_Y: SmallScreenShot();
		}
		#else
		if (KeyPressed(KEY_Y)) ResetToBootLoader();
		#endif
	
}}
////////////////////////////////// main end /////////////////////////////////

void StartGame(struct PERSONAGE *Spk_){
if (INGAME==0) {
Spk_[1].x=76;
Spk_[1].y=3;
Spk_[2].x=75;
Spk_[2].y=4;
Spk_[3].x=77;
Spk_[3].y=3;
Spk_[4].x=76;
Spk_[4].y=4;
INGAME=1;
}
}

void ResetVar(void){
LEVELSPEED=200;
GobbingEND=0;
LIVE=3;
Gobeactive=0;
TimerGobeactive=0;
add=0;
INGAME=0;
for(uint8_t t=0;t<9;t++){
dotsMem[t]=0xff;}
dotscount=0;
Frame=0;}


uint8_t CollisionPac2Caracter(struct PERSONAGE *Spk_){
uint8_t ReturnCollision=0;
#define xmax(I) (Spk_[I].x+6)
#define xmin(I) (Spk_[I].x)
#define ymax(I) ((Spk_[I].y*8)+Spk_[I].Decalagey+6)
#define ymin(I) ((Spk_[I].y*8)+Spk_[I].Decalagey)
if ((INGAME)) {    
for (uint8_t t=1;t<=4;t++){
if ((xmax(0)<xmin(t))||(xmin(0)>xmax(t))||(ymax(0)<ymin(t))||(ymin(0)>ymax(t))) {}else{ 
if (Gobeactive) {if (Spk_[t].guber!=1) {Sound(20,100);Sound(2,100);}Spk_[t].guber=1;ReturnCollision=0;}else{ if (Spk_[t].guber==1) {ReturnCollision=0;}else{ReturnCollision=1;}}
}}}return ReturnCollision;}

void RefreshCaracter(struct PERSONAGE *Spk_){
uint8_t memx,memy,memdecalagey;
for (uint8_t t=0;t<=4;t++){
memx=Spk_[t].x;
memy=Spk_[t].y;
memdecalagey=Spk_[t].Decalagey;
if ((Spk_[t].y>-1)&&(Spk_[t].y<8)) {
if ((Frame%2==0)||(t==0)||(LEVELSPEED<=160)) {
if (Spk_[t].DirectionV==1) {Spk_[t].x++;}
if (Spk_[t].DirectionV==0) {
if (t==0) {
if ((Spk_[0].y==3)&&(Spk_[0].x==86)){}else{Spk_[t].x--;}
}else{Spk_[t].x--;}
}}}
if (CheckCollisionWithBack(t,1,Spk_)) {
if (t!=0) {Spk_[t].DirectionV=random()%2;}else{ Spk_[t].DirectionV=2;}
Spk_[t].x=memx;
}
if ((Frame%2==0)||(t==0)||(LEVELSPEED<=160)) {
if (Spk_[t].DirectionH==1) {if (Spk_[t].Decalagey<7) {Spk_[t].Decalagey++;}else{Spk_[t].Decalagey=0;Spk_[t].y++;if (Spk_[t].y==9) {Spk_[t].y=-1;}}}
if (Spk_[t].DirectionH==0) {if (Spk_[t].Decalagey>0) {Spk_[t].Decalagey--;}else{Spk_[t].Decalagey=7;Spk_[t].y--;if (Spk_[t].y==-2) {Spk_[t].y=8;}}}
}
if (CheckCollisionWithBack(t,0,Spk_)) {
if (t!=0) {Spk_[t].DirectionH=random()%2;}else{Spk_[t].DirectionH=2;}
Spk_[t].y=memy;
Spk_[t].Decalagey=memdecalagey;
}
if (t==0) {
if (Frame%2==0) {
if (Spk_[t].DirectionH==1) {Spk_[t].DirectionAnim=0;}
if (Spk_[t].DirectionH==0) {Spk_[t].DirectionAnim=(2*3);} 
if (Spk_[t].DirectionV==1) {Spk_[t].DirectionAnim=(3*3);}
if (Spk_[t].DirectionV==0) {Spk_[t].DirectionAnim=(1*3);}
}}else{
if ((Frame==0)||(Frame==12)) {
Spk_[t].DirectionAnim=0;
if (Spk_[t].DirectionH==1) {Spk_[t].DirectionAnim=0;}
if (Spk_[t].DirectionH==0) {Spk_[t].DirectionAnim=2;}
}}
if (t==0) {
if (Frame%2==0) {
if (Spk_[0].switchanim==0) {
if (Spk_[0].anim<2) {Spk_[0].anim++;}else{Spk_[0].switchanim=1;} 
}else{
if (Spk_[0].anim>0) {Spk_[0].anim--;}else{Spk_[0].switchanim=0;}  
}}}else{
if ((Spk_[t].guber==1)&&(Spk_[t].x>=74)&&(Spk_[t].x<=76)&&(Spk_[t].y>=2)&&(Spk_[t].y<=4)) {Spk_[t].guber=0;}
if  (Frame%2==0) {
if (Spk_[t].anim<1) {Spk_[t].anim++;}else{Spk_[t].anim=0; } 
}}}}

uint8_t CheckCollisionWithBack(uint8_t SpriteCheck,uint8_t HorVcheck,struct PERSONAGE *Spk_){
uint8_t BacktoComp;
if (HorVcheck==1) {
BacktoComp=RecupeBacktoCompV(SpriteCheck,Spk_); 
}else{
BacktoComp=RecupeBacktoCompH(SpriteCheck,Spk_);}
if ((BacktoComp)!=0) {return 1;}else{return 0;}}

uint8_t RecupeBacktoCompV(uint8_t SpriteCheck,struct PERSONAGE *Spk_){
uint8_t Y1=0b00000000;
uint8_t Y2=Y1;
#define SpriteWide 6
#define MAXV (Spk_[SpriteCheck].x+SpriteWide)
#define MINV (Spk_[SpriteCheck].x)
if (Spk_[SpriteCheck].DirectionV==1) {
Y1=back[((Spk_[SpriteCheck].y)*128)+(MAXV)];
Y2=back[((Spk_[SpriteCheck].y+1)*128)+(MAXV)];
}else if (Spk_[SpriteCheck].DirectionV==0) {
Y1=back[((Spk_[SpriteCheck].y)*128)+(MINV)];
Y2=back[((Spk_[SpriteCheck].y+1)*128)+(MINV)];
}else{Y1=0;Y2=0;}

Y1=Trim(0,Y1,Spk_[SpriteCheck].Decalagey);
Y2=Trim(1,Y2,Spk_[SpriteCheck].Decalagey);
if (((Y1)!=0b00000000)||((Y2)!=0b00000000) ) {return 1;}else{return 0;}
}

uint8_t Trim(uint8_t Y1orY2,uint8_t TrimValue,uint8_t Decalage){
uint8_t Comp;
if (Y1orY2==0) {
Comp=0b01111111<<Decalage;
return (TrimValue&Comp);
}else{
Comp=(0b01111111>>(8-Decalage));
return (TrimValue&Comp);
}}

uint8_t ScanHRecupe(uint8_t UporDown,uint8_t Decalage){
if (UporDown==0){
return 0b01111111<<Decalage;}
else{
return 0b01111111>>(8-Decalage);
}}

uint8_t RecupeBacktoCompH(uint8_t SpriteCheck,struct PERSONAGE *Spk_){
uint8_t TempPGMByte;
if (Spk_[SpriteCheck].DirectionH==0) {
uint8_t RECUPE=(ScanHRecupe(0,Spk_[SpriteCheck].Decalagey));
for(uint8_t t=0;t<=6;t++){
if ((((Spk_[SpriteCheck].y)*128)+(Spk_[SpriteCheck].x+t)>1023)||(((Spk_[SpriteCheck].y)*128)+(Spk_[SpriteCheck].x+t)<0)) {TempPGMByte=0x00;}else{
 TempPGMByte=(back[((Spk_[SpriteCheck].y)*128)+(Spk_[SpriteCheck].x+t)]); 
}
#define CHECKCOLLISION ((RECUPE)&(TempPGMByte))
if  (CHECKCOLLISION!=0) {return 1;}
}}else if (Spk_[SpriteCheck].DirectionH==1) {
uint8_t tadd=0;
if (Spk_[SpriteCheck].Decalagey>2) { tadd=1;}else{tadd=0;}
uint8_t RECUPE=(ScanHRecupe(tadd,Spk_[SpriteCheck].Decalagey));
for(uint8_t t=0;t<=6;t++){
if (((((Spk_[SpriteCheck].y+tadd)*128)+(Spk_[SpriteCheck].x+t))>1023)||((((Spk_[SpriteCheck].y+tadd)*128)+(Spk_[SpriteCheck].x+t))<0)) {TempPGMByte=0x00;}else{
TempPGMByte=back[((Spk_[SpriteCheck].y+tadd)*128)+(Spk_[SpriteCheck].x+t)];
}
#define CHECKCOLLISION2 ((RECUPE)&(TempPGMByte))
if  (CHECKCOLLISION2!=0) {return 1;}
}}return 0;}

void TinyOLED_Send(uint8_t x_,uint8_t y_,uint8_t Byte_){
BUFFER_SSD[x_+(y_*128)]=Byte_;
}

void Tiny_Flip(uint8_t render0_picture1,struct PERSONAGE *Spk_){
uint8_t y,x; 
dotscount=-1;

if (render0_picture1==0) {
if (INGAME) {
	
for (y = 0; y < 8; y++){ 
for (x = 0; x < 128; x++){
	TinyOLED_Send(x,y,(background(x,y)));
}}
DrawBuffer();
for (y = 0; y < 8; y++){ 
for (x = 0; x < 128; x++){
	DirectDraw(DotsWrite(x,y,Spk_),x,y,color(31, 63, 31));

	DirectDraw(	SpriteWrite(x,y,0,Spk_),x,y,color(31, 63, 0));

	DirectDraw(	SpriteWrite(x,y,1,Spk_),x,y,color(31, 31, 0));
	DirectDraw(	SpriteWrite(x,y,2,Spk_),x,y,color(0, 63, 31));
	DirectDraw(	SpriteWrite(x,y,3,Spk_),x,y,color(31, 0, 31));
	DirectDraw(	SpriteWrite(x,y,4,Spk_),x,y,color(31, 0, 0));
	
}}
for (y = 0; y < 8; y++){ 
for (x = 0; x < 8; x++){
	DirectDraw(FruitWrite(x,y),x,y,color(31, 63, 31));
	DirectDraw(LiveWrite(x,y),x,y,color(31, 63, 0));
}}
    }else{
for (y = 0; y < 8; y++){ 
for (x = 0; x < 128; x++){
TinyOLED_Send(x,y,0xff-(background(x,y)|SpriteWrite2(x,y,Spk_)));
}}
DrawBuffer();
}

}else if (render0_picture1==1){
	for (y = 0; y < 8; y++){ 
for (x = 0; x < 128; x++){
TinyOLED_Send(x,y,back[x+(y*128)]);
}}
DrawBuffer();
}
DispDirtyRect(32,48,256,128);
DispUpdate();
}

uint8_t FruitWrite(uint8_t x,uint8_t y){
switch(y){
  case 7:if (x<=7) {return fruits[x];}break;
  case 6:if ((LEVELSPEED<=190)&&(x<=7)) {return fruits[x+8];}break;
  case 5:if ((LEVELSPEED<=180)&&(x<=7)) {return fruits[x+16];}break;
  case 4:if ((LEVELSPEED<=170)&&(x<=7)) {return fruits[x+24];}break;
}return 0;}

uint8_t LiveWrite(uint8_t x,uint8_t y){
if (y<LIVE) {if (x<=7) {return caracters[x+(1*8)];}else{return 0;}
}return 0x00;}

uint8_t DotsWrite(uint8_t x,uint8_t y,struct PERSONAGE *Spk_){
uint8_t Menreturn=0;
uint8_t mem1=dots[x+(128*y)];
if (mem1!=0b00000000) {
dotscount++;
 switch(dotscount){
  case 0:  
  case 1: 
  case 12: 
  case 13: 
  case 50:
  case 51:
  case 62:
  case 63:Menreturn=1;break;
  default:Menreturn=0;break;
}
if (checkDotPresent(dotscount)==0b00000000) {mem1=0x00;}else{
if ((Spk_[0].type==PACMAN)&&((Spk_[0].x<x)&&(Spk_[0].x>x-6))&&(((Spk_[0].y==y)&&(Spk_[0].Decalagey<6))||((Spk_[0].y==y-1)&&(Spk_[0].Decalagey>5)))) {DotsDestroy(dotscount);if (Menreturn==1) {TimerGobeactive=LEVELSPEED;Gobeactive=1;}else{Sound(10,10);Sound(50,10);}}
}}
if (Menreturn==1) {
if (((Frame>=6)&&(Frame<=12))||((Frame>=18)&&(Frame<=24))) {
return 0x00;
}else{return mem1;}
}else{return mem1;}}

uint8_t checkDotPresent(uint8_t  DotsNumber){
uint8_t REST=DotsNumber;
uint8_t DOTBOOLPOSITION=0;
DECREASE:
if (REST>=8) {REST=REST-8;DOTBOOLPOSITION++;goto DECREASE;}
return ((dotsMem[DOTBOOLPOSITION])&(0b10000000>>REST));
}

void DotsDestroy(uint8_t DotsNumber){
uint8_t REST=DotsNumber;
uint8_t DOTBOOLPOSITION=0;
uint8_t SOUSTRAIRE;
DECREASE:
if (REST>=8) {REST=REST-8;DOTBOOLPOSITION=DOTBOOLPOSITION+1;goto DECREASE;}
switch(REST){
  case (0):SOUSTRAIRE=0b01111111;break;
  case (1):SOUSTRAIRE=0b10111111;break;
  case (2):SOUSTRAIRE=0b11011111;break;
  case (3):SOUSTRAIRE=0b11101111;break;
  case (4):SOUSTRAIRE=0b11110111;break;
  case (5):SOUSTRAIRE=0b11111011;break;
  case (6):SOUSTRAIRE=0b11111101;break;
  case (7):SOUSTRAIRE=0b11111110;break;
}
dotsMem[DOTBOOLPOSITION]=dotsMem[DOTBOOLPOSITION]&SOUSTRAIRE;
}

uint8_t SplitSpriteDecalageY(uint8_t decalage,uint8_t Input,uint8_t UPorDOWN){
if (UPorDOWN) {
return Input<<decalage;
}else{
return Input>>(8-decalage); 
}}

uint8_t SpriteWrite(uint8_t x,uint8_t y,uint8_t var1,struct PERSONAGE  *Spk_){
uint8_t AddBin=0b00000000;
if (Spk_[var1].y==y) {
AddBin=AddBin|SplitSpriteDecalageY(Spk_[var1].Decalagey,return_if_sprite_present(x,Spk_,var1),1);
}else if (((Spk_[var1].y+1)==y)&&(Spk_[var1].Decalagey!=0)) {
AddBin=AddBin|SplitSpriteDecalageY(Spk_[var1].Decalagey,return_if_sprite_present(x,Spk_,var1),0);
}
return AddBin;}

uint8_t SpriteWrite2(uint8_t x,uint8_t y,PERSONAGE  *Spk_){
uint8_t var1=0;
uint8_t AddBin=0b00000000;
while(1){ 
if (Spk_[var1].y==y) {
AddBin=AddBin|SplitSpriteDecalageY(Spk_[var1].Decalagey,return_if_sprite_present(x,Spk_,var1),1);
}else if (((Spk_[var1].y+1)==y)&&(Spk_[var1].Decalagey!=0)) {
AddBin=AddBin|SplitSpriteDecalageY(Spk_[var1].Decalagey,return_if_sprite_present(x,Spk_,var1),0);
}
var1++;
if (var1==5) {break;}
}return AddBin;}

uint8_t return_if_sprite_present(uint8_t x,struct PERSONAGE  *Spk_,uint8_t SpriteNumber){
uint8_t ADDgobActive;
uint8_t ADDGober;
ColorPrint=0;
if  ((x>=Spk_[SpriteNumber].x)&&(x<(Spk_[SpriteNumber].x+8))) { 
if (SpriteNumber!=0) { 
if (Spk_[SpriteNumber].guber==1) {
ADDgobActive=1*(4*8);
ADDGober=Spk_[SpriteNumber].guber*(4*8);
}else{
if ((((Frame>=6)&&(Frame<=12))||((Frame>=18)&&(Frame<=24)))||(TimerGobeactive>GobbingEND)) {ADDgobActive=Gobeactive*(4*8);}else{ADDgobActive=0;}
ADDGober=0;
}}else{
ADDGober=0;
ADDgobActive=0;
}
if ((INGAME==0)&&(SpriteNumber==0)) {  return 0;}     

return caracters[((x-Spk_[SpriteNumber].x)+(8*(Spk_[SpriteNumber].type*12)))+(Spk_[SpriteNumber].anim*8)+(Spk_[SpriteNumber].DirectionAnim*8)+(ADDgobActive)+(ADDGober)];
}return 0;}

uint8_t background(uint8_t x,uint8_t y){
return BackBlitz[((y)*128)+((x))];
}
