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



//Public VAR

extern PICOCOMPATIBILITY PICOKIT;

unsigned long currentMillis=0;
unsigned long MemMillis=0;
uint8_t RDLP=40;
uint8_t START_RDLP=64;

uint8_t REVERSE_TMISSILE;
uint8_t M10000_TMISSILE;
uint8_t M1000_TMISSILE;
uint8_t M100_TMISSILE;
uint8_t M10_TMISSILE;
uint8_t M1_TMISSILE;
uint16_t Scores_TMISSILE;
uint8_t SHOT_ADJ_TMISSILE;
uint8_t LEVEL_TMISSILE;

//CLASS DECLARATION
CLK RENEW;
CLK SPEED_MISSILE;
STATIC_SPRITE_ANIM_TMISSILE INTERCEPT[3];
STATIC_SPRITE_ANIM_TMISSILE DOME[NUM_of_DOME_TMISSILE];
LINE_TMISSILE Missile[NUM_of_MISSILE_TMISSILE];
CROSS Cross;
DEFENCE Dfence[3];
ARMY_TMISSILE ARM_TMISSILE;


////////////////////////////////// main  ////////////////////////////////////
int main() {
PICOKIT.Init();
PICOKIT.SetColorBuffer(15,41,15,15,30,21,0,0,0);
CLK BLINK;
BLINK.INIT(0,10,1);
NEWGAME:;
while(1){
if (BUTTON_DOWN) {
while(1){if (BUTTON_UP) {SNDBOX_TMISSILE(0);goto START_;}}  
  }
INTRO_TMISSILE(BLINK.PROGRESS());
CONTROL_FRAMERATE(46);
}
START_:;
NEW_GAME_TMISSILE();
NEXTLEVEL:;
ADJ_LEVEL_TMISSILE(LEVEL_TMISSILE);
RESTORE_DOME_TMISSILE();
uint8_t OneDrop=0;
Tiny_Flip_TMISSILE();
while(1){
if (TINYJOYPAD_RIGHT) {Cross.RIGHT();RDLP_MIX();}
else if (TINYJOYPAD_LEFT) {Cross.LEFT();RDLP_MIX();}
if (TINYJOYPAD_UP) {Cross.UP();Start_RDLP_MIX();}
else if (TINYJOYPAD_DOWN) {Cross.DOWN();Start_RDLP_MIX();}
if (BUTTON_DOWN) {
 if (OneDrop==0) {
   Renew_Shield_TMISSILE(Cross.GET_X(),Cross.GET_Y());
   OneDrop=1;
  }
  }else{OneDrop=0;}
switch (UPDATE_ENGINE_TMISSILE()) {
case 1:
if (ALL_ANIM_END_TMISSILE()){
  if (Score_Dome_Munition_TMISSILE()) {
    SNDBOX_TMISSILE(2);
    goto NEWGAME;
    }else{
     SNDBOX_TMISSILE(1);
     NEXTLEVEL_TMISSILE();
     goto NEXTLEVEL;
      }
}break;
default:break;
  }
CONTROL_FRAMERATE(46);
Tiny_Flip_TMISSILE();
RDLP_MIX();
Start_RDLP_MIX(); 
}}
////////////////////////////////// main end /////////////////////////////////

uint8_t ALL_ANIM_END_TMISSILE(void){
for (uint8_t t=0;t<NUM_of_DOME_TMISSILE;t++){
if (DOME[t].GET_FRAME()!=0) {return 0;}
}return 1;}

void RESTORE_DOME_TMISSILE(void){
for (uint8_t t=0;t<NUM_of_DOME_TMISSILE;t++){
if (DOME[t].GET_ACTIVE()==2) {DOME[t].PUT_ACTIVE(1);}
}}

uint8_t Score_Dome_Munition_TMISSILE(void){
_delay_ms(500);
while(1){
if (ARM_TMISSILE.USE_WEAPON()) {Scores_TMISSILE++;COMPIL_SCO_TMISSILE();SNDBOX_TMISSILE(3);Tiny_Flip_TMISSILE();}else{break;}
}
_delay_ms(500);
uint8_t Count_=0;
for (uint8_t t=0;t<NUM_of_DOME_TMISSILE;t++){
if (DOME[t].GET_ACTIVE()==1) {
if (DOME[t].GET_FRAME()==0) {
Scores_TMISSILE+=5; 
COMPIL_SCO_TMISSILE();
DOME[t].PUT_ACTIVE(2);
Count_++;
SNDBOX_TMISSILE(4);
}else{DOME[t].PUT_ACTIVE(0);}
}
Tiny_Flip_TMISSILE();
Tiny_Flip_TMISSILE();
}
_delay_ms(500);
if (Count_==0) return 1;
return 0;
}

void ADJ_LEVEL_TMISSILE(uint8_t Level_){
uint8_t t;
RESET_ALL_MISSILE_TMISSILE();
for (t=0;t<3;t++){
Dfence[t].INIT();
INTERCEPT[t].INIT(0,0,0);
}
ARM_TMISSILE.RESET_WEAPON(Level_);
SHOT_ADJ_TMISSILE=Mymap(Level_,0,10,20,60);
SPEED_MISSILE.INIT(0,Mymap(Level_,0,10,2,0),0);
RENEW.INIT(0,Mymap(Level_,0,10,6,12),0);
Cross.INIT();
}

void NEW_GAME_TMISSILE(void){
REVERSE_TMISSILE=0;
M10000_TMISSILE=0;
M1000_TMISSILE=0;
M100_TMISSILE=0;
M10_TMISSILE=0;
M1_TMISSILE=0;
Scores_TMISSILE=0;
LEVEL_TMISSILE=0;
for (uint8_t t=0;t<NUM_of_DOME_TMISSILE;t++){
DOME[t].INIT_DOME(DOME_ORDER_TMISSILE[t],7,1);
}}

void NEXTLEVEL_TMISSILE(void){
LEVEL_TMISSILE=(LEVEL_TMISSILE<10)?LEVEL_TMISSILE+1:10;
}

void INC_SCORES_TMISSILE(void){
Scores_TMISSILE+=13;  
COMPIL_SCO_TMISSILE();
REFRESH_TOPBAR_TMISSILE();
}

uint8_t UPDATE_ENGINE_TMISSILE(void){
uint8_t CHECK_IF_EXIST=255;
uint8_t t;
UPDATE_DOME_TMISSILE();
UPDATE_DFENCE_TMISSILE();
UPDATE_Intercept_TMISSILE();
if (SPEED_MISSILE.PROGRESS()) {
CHECK_IF_EXIST=0;
for (t=0;t<NUM_of_MISSILE_TMISSILE;t++){
if (Missile[t].GET_ACTIVE()) {
CHECK_IF_EXIST++;
Start_RDLP_MIX();
Missile[t].PROGRESS(&DOME[0],&ARM_TMISSILE);}  
}
if (RENEW.PROGRESS()) {
  if (SHOT_ADJ_TMISSILE){
  RENEW_MISSILE();
  SHOT_ADJ_TMISSILE=(SHOT_ADJ_TMISSILE>0)?SHOT_ADJ_TMISSILE-1:0;
  RENEW.RESET();
  }}
SPEED_MISSILE.RESET();
}
if (CHECK_IF_EXIST==0) {
if (SHOT_ADJ_TMISSILE==0) {
  for (t=0;t<3;t++){
Dfence[t].PUT_ACTIVE(0);
INTERCEPT[t].PUT_ACTIVE(0); 
}
Tiny_Flip_TMISSILE();   
return 1;}
}
return 0;
}

void UPDATE_DOME_TMISSILE(void){
uint8_t REFRESH=0;
uint8_t TOTAL_=0;
for (uint8_t t=0;t<NUM_of_DOME_TMISSILE;t++){
if (DOME[t].GET_ACTIVE()) {
if (DOME[t].GET_FRAME()>0) {
  SNDBOX_TMISSILE(5);
DOME[t].PROGRESS_ANIM();
  REFRESH=1;
  }
 TOTAL_++; 
  }}
if (TOTAL_==0) {
  SHOT_ADJ_TMISSILE=0;
}
 if (REFRESH) Tiny_Flip_TMISSILE();
}


void UPDATE_Intercept_TMISSILE(void){
uint8_t t;
for (t=0;t<3;t++){
if (INTERCEPT[t].GET_ACTIVE()) {
switch(INTERCEPT[t].PROGRESS_ANIM()){
  case(4):Destroy_TMISSILE(t);break;
  default:break;
}}}}

void Destroy_TMISSILE(uint8_t t_){
uint8_t t;
uint8_t Sn_=255;
for (t=0;t<NUM_of_MISSILE_TMISSILE;t++){
if (Missile[t].GET_ACTIVE()) {
if ((INTERCEPT[t_].GET_Y()+15)<(Missile[t].GET_TRACK())) goto NexT;
if ((INTERCEPT[t_].GET_Y())>(Missile[t].GET_TRACK())) goto NexT;
int8_t XPOS_=Mymap((Missile[t].GET_TRACK()),11,55,Missile[t].GET_START_X(),Missile[t].GET_END_X());
if (XPOS_< INTERCEPT[t_].GET_X()) {goto NexT;}
if (XPOS_> (INTERCEPT[t_].GET_X()+9)) {goto NexT;}
Missile[t].PUT_ACTIVE(0);
INC_SCORES_TMISSILE();}
NexT:;
TinySound(Sn_=Sn_-45,4);
}}

void Create_New_Intercept_TMISSILE(int8_t X_,int8_t Y_){
uint8_t t;
for (t=0;t<3;t++){
if (INTERCEPT[t].GET_ACTIVE()==0) {
INTERCEPT[t].INIT(X_,Y_,1);
INTERCEPT[t].PUT_FRAME(0);
break;
}}}

void NEW_POS_TMISSILE(int8_t *ST_,int8_t *END_){
int8_t A_,B_=0;
A_=START_RDLP;
if (REVERSE_TMISSILE==1){
B_=((A_+RDLP)>126)?(A_-RDLP):(A_+RDLP);}else
{B_=((A_-RDLP)<3)?(A_+RDLP):(A_-RDLP);}
*ST_=A_;
*END_=B_;
}

void Start_RDLP_MIX(void){
START_RDLP=(START_RDLP>3)?START_RDLP-3:125-START_RDLP;
}

void RDLP_MIX(void){
RDLP=(RDLP<60)?RDLP+3:22;
}

void UPDATE_DFENCE_TMISSILE(void){
for (uint8_t t=0;t<3;t++){
if (Dfence[t].GET_ACTIVE()==1) {Dfence[t].UPDATE_DEFENCE();}
}}

void Renew_Shield_TMISSILE(int8_t x_END,int8_t y_END){
for (uint8_t t=0;t<3;t++){
if (Dfence[t].GET_ACTIVE()==0) {    
  if (ARM_TMISSILE.USE_WEAPON()) {REFRESH_TOPBAR_TMISSILE();Dfence[t].NEW(x_END,y_END); }
  break;
  }}
}

void REFRESH_TOPBAR_TMISSILE(void){
Tiny_Flip_TMISSILE();
}

void RESET_ALL_MISSILE_TMISSILE(void){
int8_t A_,B_;
for (uint8_t t=0;t<NUM_of_MISSILE_TMISSILE;t++){
RDLP_MIX();
Start_RDLP_MIX();
NEW_POS_TMISSILE(&A_,&B_); 
Missile[t].INIT(A_,B_,0);}
}

void RENEW_MISSILE(void){
int8_t A_,B_;
RDLP_MIX();
Start_RDLP_MIX();
 NEW_POS_TMISSILE(&A_,&B_);
for (uint8_t t=0;t<NUM_of_MISSILE_TMISSILE;t++){
if (Missile[t].GET_ACTIVE()==0) { REVERSE_TMISSILE=(REVERSE_TMISSILE==1)?0:1;Missile[t].INIT(A_,B_,1);goto Ending;}
}  
Ending:;
}

uint8_t RECUP_LINE_TMISSILE(uint8_t xPASS,uint8_t yPASS){
uint8_t Byte_=0x00;
uint8_t TByte_=0x00;
for (uint8_t t=0;t<NUM_of_MISSILE_TMISSILE;t++){
if (Missile[t].GET_ACTIVE()==1) {
if (yPASS<=Missile[t].GET_yPass()){
TByte_=Trace_LINE(0,Missile[t].GET_START_X(),11,Missile[t].GET_END_X(),55,xPASS,yPASS);
if (Missile[t].GET_yPass()==yPASS) {
Byte_|=(Missile[t].GET_yDeca()&TByte_);
}else{
Byte_|=TByte_;
}}}}
return Byte_;
}

uint8_t RECUP_DOME_TMISSILE(uint8_t xPASS,uint8_t yPASS){
if (yPASS!=7) {return 0x00;}
uint8_t BYTE_OUT;
for (uint8_t t=0;t<NUM_of_DOME_TMISSILE;t++){
if (DOME[t].GET_ACTIVE()==1) {
BYTE_OUT=SPEED_BLITZ(DOME[t].GET_X(),DOME[t].GET_Y(),xPASS,yPASS,DOME[t].GET_FRAME(),DOME_SPRITE_TMISSILE);
if (BYTE_OUT!=0) return BYTE_OUT;
}}
return 0x00;
}

uint8_t RECUP_CROSS_TMISSILE(uint8_t xPASS,uint8_t yPASS){
return blitzSprite(Cross.GET_X(),Cross.GET_Y(),xPASS,yPASS,0,CROSS_TMISSILE);
}

uint8_t RECUP_SHIELD_TMISSILE(uint8_t xPASS,uint8_t yPASS){
uint8_t Byte_=0;
uint8_t t;
for (t=0;t<3;t++){
if (Dfence[t].GET_ACTIVE()) Byte_|=blitzSprite(Dfence[t].GET_X(),Dfence[t].GET_Y(),xPASS,yPASS,0,ROCKET_TMISSILE);
}
return Byte_;
}

uint8_t RECUPE_TOPBAR_TMISSILE(uint8_t xPASS,uint8_t yPASS){
return TOP_PANEL_TMISSILE[xPASS+(yPASS*128)]; 
}

uint8_t RECUP_INTERCEPT_TMISSILE(uint8_t xPASS,uint8_t yPASS){
uint8_t t,BYTE_=0;
for (t=0;t<3;t++){
if (INTERCEPT[t].GET_ACTIVE()){
if (xPASS<INTERCEPT[t].GET_X()) goto NexT_;
if (xPASS>INTERCEPT[t].GET_X()+10) goto NexT_;
BYTE_|=blitzSprite(INTERCEPT[t].GET_X(),INTERCEPT[t].GET_Y(),xPASS,yPASS,INTERCEPT[t].GET_FRAME() ,INTERCEPT_TMISSILE);
NexT_:;
}}
return BYTE_;
}

uint8_t Recupe_TMISSILE(uint8_t xPASS,uint8_t yPASS){
return (
RECUP_LINE_TMISSILE(xPASS,yPASS)|
BACKGROUND_TMISSILE(xPASS,yPASS)|
RECUP_DOME_TMISSILE(xPASS,yPASS)|
RECUP_CROSS_TMISSILE(xPASS,yPASS)|
RECUP_SHIELD_TMISSILE(xPASS,yPASS)|
RECUP_INTERCEPT_TMISSILE(xPASS,yPASS)|
RECUP_PANEL_TMISSILE(xPASS,yPASS)
);}

uint8_t BACKGROUND_TMISSILE(uint8_t xPASS,uint8_t yPASS){
switch(yPASS){
  case 0:return 0;break;
  case 1:return Y1_TMISSILE[xPASS];break; 
  case 2 ... 5 :return CENTER_TMISSILE[xPASS];break; 
  case 6:return Y6_TMISSILE[xPASS];break; 
  case 7:return Y7_TMISSILE[xPASS];break; 
  default:return 0;break;
}}

void Tiny_Flip_TMISSILE(void){
PICOKIT.clearPicoBuffer();
uint8_t y,x; 
for (y = 0; y < 8; y++){ 
    for (x = 0; x < 128; x++){ 

PICOKIT.DirectDraw(x,y,RECUP_LINE_TMISSILE(x,y),color(31,0,0));
PICOKIT.DirectDraw(x,y,BACKGROUND_TMISSILE(x,y),color(21,41,21));
PICOKIT.DirectDraw(x,y,RECUP_DOME_TMISSILE(x,y),color(0,41,21));
PICOKIT.DirectDraw(x,y,RECUP_CROSS_TMISSILE(x,y),color(0,41,0));
PICOKIT.DirectDraw(x,y,RECUP_SHIELD_TMISSILE(x,y),color(21,41,0));
PICOKIT.DirectDraw(x,y,RECUP_INTERCEPT_TMISSILE(x,y),color(21,41,0));
PICOKIT.DirectDraw(x,y,RECUP_PANEL_TMISSILE(x,y),color(11,41,11));
    }
}
PICOKIT.DisplayColor();
}

void INTRO_TMISSILE(uint8_t FL_){
uint8_t y,x; 
for (y = 0; y < 8; y++){ 

    for (x = 0; x < 128; x++){ 
PICOKIT.TinyOLED_Send(x,y,RECUPE_INTRO_TMISSILE(x,y,FL_)|((y==7)?NUMERIC_TMISSILE(x):0x00));
    }

}
PICOKIT.display();
}

uint8_t RECUPE_INTRO_TMISSILE(uint8_t xPASS,uint8_t yPASS,uint8_t Fl_){
if (Fl_==1){
if ((xPASS<68)&&(xPASS>60)){
switch (yPASS) {
  case 4:return 0;break;
  case 5:return 0;break;
  }}}
return SPEED_BLITZ(21,0,xPASS,yPASS,0,intro_TMISSILE);
}

uint8_t RECUP_PANEL_TMISSILE(uint8_t xPASS,uint8_t yPASS){
if (yPASS>0) return 0;
uint8_t Byte1_=RECUPE_TOPBAR_TMISSILE(xPASS,yPASS);
uint8_t Byte2_=0;
switch (xPASS){
  case 2 ... 33:Byte2_=Byte1_&INVENTORY_TMISSILE(xPASS);break;
  case 40 ... 99:Byte2_=Byte1_&MUNITION_TMISSILE(xPASS);break;
  case 106 ... 124:Byte2_=Byte1_|NUMERIC_TMISSILE(xPASS);break;
  default:return Byte1_;break;
}return Byte2_;}

uint8_t INVENTORY_TMISSILE(uint8_t xPASS){
uint8_t X_=(ARM_TMISSILE.GET_SPARE()*8)+2;
if (xPASS<X_) return 0xff;
return 0x03;}

uint8_t MUNITION_TMISSILE(uint8_t xPASS){
uint8_t X_=(ARM_TMISSILE.GET_ROCKET()*6)+40;
if (xPASS<X_) return 0xff;
return 0x01;}

uint8_t NUMERIC_TMISSILE(uint8_t xPASS){
return recupe_SCORES_TMISSILE(xPASS);}

uint8_t recupe_SCORES_TMISSILE(uint8_t xPASS){
if (xPASS<(105)) {return 0;}
if (xPASS>(125)){return 0;}
return 
(SPEED_BLITZ(106,0,xPASS,0,M10000_TMISSILE,police_TMISSILE)|
 SPEED_BLITZ(110,0,xPASS,0,M1000_TMISSILE,police_TMISSILE)|
 SPEED_BLITZ(114,0,xPASS,0,M100_TMISSILE,police_TMISSILE)|
 SPEED_BLITZ(118,0,xPASS,0,M10_TMISSILE,police_TMISSILE)|
 SPEED_BLITZ(122,0,xPASS,0,M1_TMISSILE,police_TMISSILE));
}

void COMPIL_SCO_TMISSILE(void){
M10000_TMISSILE=(Scores_TMISSILE/10000);
M1000_TMISSILE=(((Scores_TMISSILE)-(M10000_TMISSILE*10000))/1000);
M100_TMISSILE=(((Scores_TMISSILE)-(M1000_TMISSILE*1000)-(M10000_TMISSILE*10000))/100);
M10_TMISSILE=(((Scores_TMISSILE)-(M100_TMISSILE*100)-(M1000_TMISSILE*1000)-(M10000_TMISSILE*10000))/10);
M1_TMISSILE=((Scores_TMISSILE)-(M10_TMISSILE*10)-(M100_TMISSILE*100)-(M1000_TMISSILE*1000)-(M10000_TMISSILE*10000));
}

void SNDBOX_TMISSILE(uint8_t Snd_){
uint8_t t;
switch(Snd_){
  case(0):TinySound(100,250);TinySound(20,250);break;
  case(1):for(t=1;t<255;t++){TinySound(50,2);TinySound(t,2);}break;
  case(2):for(t=200;t>10;t--){TinySound(200-t,3);TinySound(t,12);}break;
  case(3):TinySound(200,140);TinySound(100,140);break;
  case(4):TinySound(2,140);TinySound(200,14);break;
  case(5):TinySound(200,6);TinySound(150,6);break;
  default:break; 
}}
