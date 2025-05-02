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

#include "../include.h"


#define MAX_FOOD_TPLAQUE 4
#define MAX_FOOD_D_TPLAQUE 8

GAME_DATA_TPLAQUE GD;
PICOCOMPATIBILITY PICOKIT;

const uint16_t FruitCol[6]={color(31,0,0),color(31,45,14),color(13,36,31),color(31,31,16),color(31,50,19),color(28,51,16)};
//PUBLIC VAR
unsigned long currentMillis=0;
unsigned long MemMillis=0;
#define FRAME_CONTROL  while((currentMillis-MemMillis)<90){currentMillis=pico_millis();}MemMillis=currentMillis
#define GAME_FRAME_CONTROL  while((currentMillis-MemMillis)<40){currentMillis=pico_millis();}MemMillis=currentMillis
uint8_t Scan_Teeth_TPLAQUE=0;
uint8_t SCAN_COLLISION_TPLAQUE=0;
uint8_t BLINK_START_TPLAQUE=0;
uint8_t TC_TPLAQUE=0;
uint8_t TPC_TPLAQUE=0;
uint8_t TSIA_TPLAQUE=0;
bool ATTAQUE1_TPLAQUE=false;
bool ATTAQUE2_TPLAQUE=false;
uint8_t M10000_TPLAQUE=0;
uint8_t M1000_TPLAQUE=0;
uint8_t M100_TPLAQUE=0;
uint8_t M10_TPLAQUE=0;
uint8_t M1_TPLAQUE=0;

//PUBLIC OBJ
Main_Sprite_TPLAQUE MainSprite_TPLAQUE;
Food_Sprite_TPLAQUE M_Spr[MAX_FOOD_D_TPLAQUE];
Sprite_TPLAQUE TEETH_UP[8];
Sprite_TPLAQUE TEETH_DOWN[8];
Weapon_Sprite_TPLAQUE weapon;

void INIT_NEW_GAME_TPLAQUE(void){
GD.EXTRA_TEETH=0;
GD.EXTRA_TEETH_COMP=0;
GD.Scores_TPLAQUE=0;
GD.Level=0;
GD.END_OF_GAME=0;
GD.Food_Type=0;
GD.Skip_Frame=0;
TEETH_RESET_TPLAQUE();
FOOD_RESET_TPLAQUE();
}
 
////////////////////////////////// main  ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////  

int main() {
PICOKIT.Init();
uint8_t LooP;
NEW_GAME:;
while(1){
if (BUTTON_DOWN) {break;}
Tiny_Flip_TPLAQUE(3);
delay(22);
BLINK_START_TPLAQUE=(BLINK_START_TPLAQUE>00)?BLINK_START_TPLAQUE-1:22;
}
INIT_NEW_GAME_TPLAQUE();
LOAD_LEVEL_TPLAQUE();
BLINK_START_TPLAQUE=0;
COMPIL_SCO_TPLAQUE();
Tiny_Flip_TPLAQUE(0);
Sound(100,250);Sound(20,250);
delay(1000);
RESTORE_TEETH_TPLAQUE(); 
delay(1000);
while(1){
if (TINYJOYPAD_RIGHT) {MainSprite_TPLAQUE.MDROITE();RND_POS_TPLAQUE();}
else if (TINYJOYPAD_LEFT) {MainSprite_TPLAQUE.MGAUCHE();RND_POS_TPLAQUE();}else{MainSprite_TPLAQUE.DECEL_X();}
if (TINYJOYPAD_UP) {MainSprite_TPLAQUE.MHAUT();RND_POS_TPLAQUE();}
else if (TINYJOYPAD_DOWN) {MainSprite_TPLAQUE.MBAS();RND_POS_TPLAQUE();}else{MainSprite_TPLAQUE.DECEL_Y();}
if ((BUTTON_DOWN)&&(GD.TUBE_FUEL_TIMER>0)) {weapon.Start(MainSprite_TPLAQUE);}
FOOD_MOVE_UPDATE_TPLAQUE();
HIT_BOX_TPLAQUE();
if (GAME_PLAY_TPLAQUE()==1) {
  Tiny_Flip_TPLAQUE(0);
  for(LooP=1;LooP<20;LooP++){
  Sound(4,80);
  Sound(100,80);
  }
  delay(1000);
  goto NEW_GAME;
  }
switch (GD.Skip_Frame) {
  case 0:ATTAQUE1_TPLAQUE=!ATTAQUE1_TPLAQUE;Tiny_Flip_TPLAQUE(0);GAME_FRAME_CONTROL; break;
  case 2:ADJUST_TUBE_TIMER_TPLAQUE();ADJUST_TUBE_TPLAQUE();break;
  case 3:GD.Tube_Refresh=(GD.Tube_Refresh>0)?GD.Tube_Refresh-1:8;
  default:break;
  }   
GD.Skip_Frame=(GD.Skip_Frame<5)?GD.Skip_Frame+1:0;
}}

////////////////////////////////// main end /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void ADJUST_TUBE_TIMER_TPLAQUE(void){
GD.TUBE_FUEL_TIMER=(GD.TUBE_FUEL_TIMER>0)?GD.TUBE_FUEL_TIMER-1:0;  
ADJUST_TUBE_TPLAQUE();
}

void ADJUST_TUBE_TPLAQUE(void){
if (GD.Tube_Refresh==0) {
GD.TUBE_FUEL=Mymap(GD.TUBE_FUEL_TIMER,GD.TUBE_FUEL_TIMER_REF,0,0,4);
}
}

void SCORE_ADD_TPLAQUE(uint8_t SC_){
GD.EXTRA_TEETH_COMP=GD.EXTRA_TEETH_COMP+SC_; 
if ((GD.EXTRA_TEETH_COMP)>99) {
GD.EXTRA_TEETH_COMP=GD.EXTRA_TEETH_COMP-100;
GD.EXTRA_TEETH++;
}
GD.Scores_TPLAQUE+=SC_;
}

void UPDATE_PANNEL_TPLAQUE(void){
COMPIL_SCO_TPLAQUE();
Tiny_Flip_TPLAQUE(2);
}

void COMPIL_SCO_TPLAQUE(void){
M10000_TPLAQUE=(GD.Scores_TPLAQUE/10000);
M1000_TPLAQUE=(((GD.Scores_TPLAQUE)-(M10000_TPLAQUE*10000))/1000);
M100_TPLAQUE=(((GD.Scores_TPLAQUE)-(M1000_TPLAQUE*1000)-(M10000_TPLAQUE*10000))/100);
M10_TPLAQUE=(((GD.Scores_TPLAQUE)-(M100_TPLAQUE*100)-(M1000_TPLAQUE*1000)-(M10000_TPLAQUE*10000))/10);
M1_TPLAQUE=((GD.Scores_TPLAQUE)-(M10_TPLAQUE*10)-(M100_TPLAQUE*100)-(M1000_TPLAQUE*1000)-(M10000_TPLAQUE*10000));
}

uint8_t GAME_PLAY_TPLAQUE(void){
CHECK_NUMBER_OF_TEETH_TPLAQUE();
uint8_t t,q=0;
if (GD.NOT_MOVE>0) {
if (GD.NOT_MOVE>=GD.TIMER_TEETH) {
  DELETE_TEETH_TPLAQUE();
  GD.NOT_MOVE=0;
  INVERT_FOOD_DIRECTION_TPLAQUE();
}else{
switch(GD.up_down){
case 0:for (t=0;t<8;t++){Collision_WTEETH_AGAIN_TPLAQUE(q,TEETH_UP[t]);}break; 
case 1:for (t=0;t<8;t++){Collision_WTEETH_AGAIN_TPLAQUE(q,TEETH_DOWN[t]);}break;  
default:break;
}
GD.NOT_MOVE++;
if (q==0) {GD.NOT_MOVE=0;INVERT_FOOD_DIRECTION_TPLAQUE();}
}}
if (GD.Delay_Direction_Change<1) {
  if (GD.x_move_active==1) {UPDATE_CHANGEX_TPLAQUE();}
  GD.Delay_Direction_Change=GD.DELAY_DIRECTION_CHANGE;
  }else{
    GD.Delay_Direction_Change--;
    }
if (END_OF_LEVEL_TPLAQUE()) {return 1;}
return 0;
}

uint8_t  END_OF_LEVEL_TPLAQUE(void){
uint8_t T_V=0;
if (GD.REGEN_NO==0) {
for (uint8_t t=0;t<8;t++){ 
if (M_Spr[t].ACTIVE()!=0) {T_V=1;}
}
if (T_V==0) {
  weapon.PUT_ACTIVE(0);
  DECOUNT_TPLAQUE();
  if (GD.END_OF_GAME) {if (GD.EXTRA_TEETH>0) {GD.END_OF_GAME=0;}}
  if (GD.END_OF_GAME) {return 1;}
  NEXT_LEVEL_TPLAQUE();
  delay(400);
  RESTORE_TEETH_TPLAQUE();
  delay(400);
  ADD_TEETH_TPLAQUE();
  }
}
return 0;
}

void DSOUND_TPLAQUE(uint8_t T_AD){
Sound(5+T_AD,20);
Sound(100,10);
}

void DSOUND2_TPLAQUE(void){
Sound(100,10);
Sound(50,30);
}

void DECOUNT_TPLAQUE(void){
uint8_t t;
GD.END_OF_GAME=1;
while(1){
if (GD.TUBE_FUEL<4) {
  for(t=0;t<4;t++) {
    Tiny_Flip_TPLAQUE(1);
    DSOUND2_TPLAQUE();
    SCORE_ADD_TPLAQUE(1);
    UPDATE_PANNEL_TPLAQUE();
    FRAME_CONTROL;
    }
  GD.TUBE_FUEL++;
  Tiny_Flip_TPLAQUE(1);
  DSOUND2_TPLAQUE();
  FRAME_CONTROL;
  }else{
    Tiny_Flip_TPLAQUE(1);
    delay(200);
    break;
    }
}
for (t=0;t<8;t++){
if (TEETH_UP[t].ACTIVE()==1) {
    GD.END_OF_GAME=0;
    TEETH_UP[t].PUT_ACTIVE(-1);
    SCORE_ADD_TPLAQUE(2);
    COMPIL_SCO_TPLAQUE();
    DSOUND_TPLAQUE(0); 
    Tiny_Flip_TPLAQUE(1);
    FRAME_CONTROL;
    }
}
for (t=0;t<8;t++){
if (TEETH_DOWN[t].ACTIVE()==1) {
    GD.END_OF_GAME=0;
    TEETH_DOWN[t].PUT_ACTIVE(-1);
    SCORE_ADD_TPLAQUE(2);
    COMPIL_SCO_TPLAQUE();
    DSOUND_TPLAQUE(0); 
    Tiny_Flip_TPLAQUE(1);
    FRAME_CONTROL;
  }
}
}

void RESTORE_TEETH_TPLAQUE(void){
uint8_t t,T_ADD=0;
for (t=0;t<8;t++){
if (TEETH_UP[t].ACTIVE()==-1) {
  TEETH_UP[t].PUT_ACTIVE(1);
  Tiny_Flip_TPLAQUE(1);
  DSOUND_TPLAQUE(T_ADD+=10);
  delay(10);
  }
}
for (t=0;t<8;t++){
if (TEETH_DOWN[t].ACTIVE()==-1) {
  TEETH_DOWN[t].PUT_ACTIVE(1);
  Tiny_Flip_TPLAQUE(1);
  DSOUND_TPLAQUE(T_ADD+=10);
  delay(10);
  }
}
}

void Sound_ADDTEETH_TPLAQUE(void){
Sound(100,20);
Sound(1,20);
Sound(100,20);
}

void ADD_TEETH_TPLAQUE(void){
 uint8_t t;
for (t=0;t<4;t++){
if (TEETH_UP[t+4].ACTIVE()==0) {
  if (PUT_TEETH_TPLAQUE(TEETH_UP[t+4])) {
    goto ENDING;
    }
    }
if (TEETH_DOWN[t+4].ACTIVE()==0) {
  if (PUT_TEETH_TPLAQUE(TEETH_DOWN[t+4])) {
    goto ENDING;
    }
    }
if (TEETH_UP[(3-t)].ACTIVE()==0) {
  if (PUT_TEETH_TPLAQUE(TEETH_UP[(3-t)])) {
    goto ENDING;
    }
    }
if (TEETH_DOWN[(3-t)].ACTIVE()==0) {
  if (PUT_TEETH_TPLAQUE(TEETH_DOWN[(3-t)])) {
    goto ENDING;
    }
    }
}
ENDING:;
delay(400);
}

uint8_t PUT_TEETH_TPLAQUE(Sprite_TPLAQUE &TEETH_){
if (GD.EXTRA_TEETH>0) {
  TEETH_.PUT_ACTIVE(1);
  Sound_ADDTEETH_TPLAQUE();
  GD.EXTRA_TEETH=(GD.EXTRA_TEETH>0)?GD.EXTRA_TEETH-1:0;
  Tiny_Flip_TPLAQUE(1);
  delay(400);
}
if (GD.EXTRA_TEETH>0) {
  return 0;
  }else{
    return 1;
    }
return 1;
}

void NEXT_LEVEL_TPLAQUE(void){
GD.Level=(GD.Level<MAX_LEVEL_TPLAQUE)?GD.Level+1:MAX_LEVEL_TPLAQUE;
GD.Food_Type=(GD.Food_Type<5)?GD.Food_Type+1:0;
LOAD_LEVEL_TPLAQUE();
}

void LOAD_LEVEL_TPLAQUE(void){
GD.TEETH_COUNT_UP=0;
GD.TEETH_COUNT_DOWN=0;
GD.REGEN_NO=RENEW_NUMBER_TPLAQUE;
GD.up_down=1;
GD.SCAN_CHANGE_DIRECTION=0;
GD.DELAY_DIRECTION_CHANGE=(Mymap(GD.Level,0,MAX_LEVEL_TPLAQUE,25,4));
GD.sp=(Mymap(GD.Level,0,MAX_LEVEL_TPLAQUE,5,15));
if (GD.Level>2) {GD.x_move_active=1;}else{GD.x_move_active=0;}
GD.RENEW_FOOD=(Mymap(GD.Level,0,MAX_LEVEL_TPLAQUE,180,32));
GD.TIMER_TEETH=(Mymap(GD.Level,0,MAX_LEVEL_TPLAQUE,200,64));
GD.renew=GD.RENEW_FOOD;
GD.NOT_MOVE=0;
GD.Delay_Direction_Change=GD.DELAY_DIRECTION_CHANGE;
MainSprite_TPLAQUE.PUT_DIRECTION_Y(1);
MainSprite_TPLAQUE.INIT();
GD.TUBE_FUEL_TIMER_REF=(Mymap(GD.Level,0,MAX_LEVEL_TPLAQUE,1200,200));
GD.TUBE_FUEL_TIMER=GD.TUBE_FUEL_TIMER_REF;
GD.TUBE_FUEL=Mymap(GD.TUBE_FUEL_TIMER,GD.TUBE_FUEL_TIMER_REF,0,0,4);
GD.Tube_Refresh=8;
weapon.PUT_ACTIVE(0);
}

void DELETE_TEETH_TPLAQUE(void){
uint8_t t;
for (t=0;t<8;t++){
if (TEETH_UP[t].ACTIVE()==2) {TEETH_UP[t].PUT_ACTIVE(0);}
if (TEETH_DOWN[t].ACTIVE()==2) {TEETH_DOWN[t].PUT_ACTIVE(0);}
}}

void HIT_BOX_TPLAQUE(void){
weapon.Update();
SCAN_COLLISION_TPLAQUE=(SCAN_COLLISION_TPLAQUE<7)?SCAN_COLLISION_TPLAQUE+1:0;
for(uint8_t t=0;t<8;t++){
Check_Collision_WBALISTIC_TPLAQUE(t); 
}}

void CHECK_NUMBER_OF_TEETH_TPLAQUE(void){
GD.TEETH_COUNT_UP=GD.TEETH_COUNT_UP+TEETH_UP[SCAN_COLLISION_TPLAQUE].ACTIVE();
GD.TEETH_COUNT_DOWN=GD.TEETH_COUNT_DOWN+TEETH_DOWN[SCAN_COLLISION_TPLAQUE].ACTIVE();
if (SCAN_COLLISION_TPLAQUE==7) {
 if ((GD.TEETH_COUNT_UP!=0)&&(GD.TEETH_COUNT_DOWN==0)) {
switch (GD.up_down){
case 1:INVERT_FOOD_DIRECTION_TPLAQUE();break;
default:break;
}}
if ((GD.TEETH_COUNT_UP==0)&&(GD.TEETH_COUNT_DOWN!=0)) {
switch (GD.up_down){
case 0:INVERT_FOOD_DIRECTION_TPLAQUE();break;
default:break;
}}
GD.TEETH_COUNT_UP=0;
GD.TEETH_COUNT_DOWN=0;
}}

void Check_Collision_WBALISTIC_TPLAQUE(uint8_t T_V){
if (M_Spr[T_V].ACTIVE()) {
int8_t FOODX_=M_Spr[T_V].X();
int8_t FOODW_=FOODX_+5;
int8_t FOODY_=M_Spr[T_V].Y()+1;
int8_t FOODH_=FOODY_+5;
int8_t BALISTICX_=MainSprite_TPLAQUE.X()+3;
int8_t BALISTICY_=weapon.Y();
int8_t BALISTICH_=BALISTICY_+7;
if (BALISTICX_>FOODW_) {goto SUITE;}
if (BALISTICX_<FOODX_) {goto SUITE;}
if (BALISTICY_>FOODH_) {goto SUITE;}
if (BALISTICH_<FOODY_) {goto SUITE;}
if (weapon.ACTIVE()==1){
M_Spr[T_V].PUT_ACTIVE(0);
SCORE_ADD_TPLAQUE(1);
UPDATE_PANNEL_TPLAQUE();
weapon.PUT_ACTIVE(2);
}
goto ENDING;
SUITE:;
switch(GD.up_down){
case 0:Check_Collision_WTEETH_TPLAQUE(FOODX_,FOODW_,FOODY_,FOODH_,TEETH_UP[SCAN_COLLISION_TPLAQUE]);break;
case 1:Check_Collision_WTEETH_TPLAQUE(FOODX_,FOODW_,FOODY_,FOODH_,TEETH_DOWN[SCAN_COLLISION_TPLAQUE]);break;
default:break;
}}
ENDING:;
}

uint8_t Check_Collision_WTEETH_TPLAQUE(int8_t FX,int8_t FW,int8_t FY,int8_t FH,Sprite_TPLAQUE &SP_){
if (SP_.ACTIVE()==1) {
int8_t TEETHX_=SP_.X();
int8_t TEETHW_=TEETHX_+8;
int8_t TEETHY_=SP_.Y();
int8_t TEETHH_=TEETHY_+7;
FX=FX+1;
FW=FX+2;
if (FX>TEETHW_) return 0;
if (FW<TEETHX_) return 0;
if (FY>TEETHH_) return 0;
if (FH<TEETHY_) return 0;
if (GD.NOT_MOVE==0) {GD.NOT_MOVE=1;}
SP_.PUT_ACTIVE(2);
return 0;
}
return 0;
}

uint8_t Collision_WTEETH_AGAIN_TPLAQUE(uint8_t &Count,Sprite_TPLAQUE &TEETH_){
if (GD.NOT_MOVE>0){
if (TEETH_.ACTIVE()==2) {
int8_t TEETHX_=TEETH_.X();
int8_t TEETHW_=TEETHX_+8;
int8_t TEETHY_=TEETH_.Y();
int8_t TEETHH_=TEETHY_+7;
for (uint8_t t=0;t<8;t++){
if (M_Spr[t].ACTIVE()==1) {
int8_t FOODX_=M_Spr[t].X()+1;
int8_t FOODW_=FOODX_+2;
int8_t FOODY_=M_Spr[t].Y()+1;
int8_t FOODH_=FOODY_+5;
if (FOODX_>TEETHW_) goto ENDING;
if (FOODW_<TEETHX_) goto ENDING;
if (FOODY_>TEETHH_) goto ENDING;
if (FOODH_<TEETHY_) goto ENDING;
Count=1;
return 0;
ENDING:;
}}
if (TEETH_.ACTIVE()==2) {TEETH_.PUT_ACTIVE(1);}
}}
return 0;
}

void INVERT_FOOD_DIRECTION_TPLAQUE(void){
int8_t T_V=-1;
GD.up_down=(GD.up_down==1)?0:1;
switch(GD.up_down){
case (0):T_V=-1;break;
case (1):T_V=1;break;
default:break;
}
for(uint8_t t=0;t<8;t++){
 if (M_Spr[t].ACTIVE()==1){
M_Spr[t].PUT_DIRECTION_Y(T_V);
M_Spr[t].PUT_SOMY(0);
M_Spr[t].PUT_SY(0);
}}
}

void TEETH_RESET_TPLAQUE(void){
for (uint8_t t=0;t<8;t++){
TEETH_UP[t].PUT_ACTIVE(0);
TEETH_UP[t].PUT_X((t*11)+21);
TEETH_UP[t].PUT_Y(8);
TEETH_DOWN[t].PUT_ACTIVE(0);
TEETH_DOWN[t].PUT_X((t*11)+21);
TEETH_DOWN[t].PUT_Y(54);
}
for (uint8_t t=2;t<6;t++){
TEETH_UP[t].PUT_ACTIVE(-1);
TEETH_DOWN[t].PUT_ACTIVE(-1);
}}

void FOOD_RESET_TPLAQUE(void){
 for (uint8_t t=0;t<MAX_FOOD_D_TPLAQUE;t++){
M_Spr[t].DISABLE_RESET();
}}

void FOOD_MOVE_UPDATE_TPLAQUE(void){
if (GD.NOT_MOVE==0) {
for (uint8_t t=0;t<MAX_FOOD_D_TPLAQUE;t++){
 if (M_Spr[t].Y()<18) {M_Spr[t].PUT_DIRECTION_X(0);}
 if (M_Spr[t].Y()>45) {M_Spr[t].PUT_DIRECTION_X(0);} 
M_Spr[t].MOVE_UPDATE();
}
UPDATE_FOOD_TRIGER_TPLAQUE();}
ADD_FOOD_TPLAQUE();
}

void ADD_FOOD_TPLAQUE(void){
if ((GD.renew==0)&&(GD.REGEN_NO!=0)) {
for (uint8_t t=0;t<MAX_FOOD_TPLAQUE;t=t+1){
if ((M_Spr[t*2].ACTIVE()==0)&&(M_Spr[(t*2)+1].ACTIVE()==0)) {
  GD.REGEN_NO=(GD.REGEN_NO>0)?GD.REGEN_NO-1:0;
  M_Spr[(t*2)].CREAT_FOOD();
  M_Spr[(t*2)+1].CREAT_FOOD();
  COLAPS_FOOD_TPLAQUE(t*2);
  GD.renew=GD.RENEW_FOOD;
  goto ENDING;
  }
}}
ENDING:;
}

void COLAPS_FOOD_TPLAQUE(uint8_t T_V){
M_Spr[T_V+1].COPY_OBJ(M_Spr[T_V]);
}

void UPDATE_FOOD_TRIGER_TPLAQUE(void){
GD.renew=(GD.renew>0)?GD.renew-1:GD.RENEW_FOOD;
}

void UPDATE_CHANGEX_TPLAQUE(void){
GD.SCAN_CHANGE_DIRECTION=(GD.SCAN_CHANGE_DIRECTION<(MAX_FOOD_TPLAQUE-1))?GD.SCAN_CHANGE_DIRECTION+1:0;
int8_t T_V=RND_TPLAQUE();
 if (M_Spr[GD.SCAN_CHANGE_DIRECTION*2].ACTIVE()==1) {
  M_Spr[GD.SCAN_CHANGE_DIRECTION*2].PUT_DIRECTION_X(T_V);
  M_Spr[GD.SCAN_CHANGE_DIRECTION*2].PUT_SOMX(0);
}
 if (M_Spr[(GD.SCAN_CHANGE_DIRECTION*2)+1].ACTIVE()==1) {
  M_Spr[(GD.SCAN_CHANGE_DIRECTION*2)+1].PUT_DIRECTION_X(T_V); 
    M_Spr[(GD.SCAN_CHANGE_DIRECTION*2)+1].PUT_SOMX(0);
  }
}

uint8_t blitzSprite_TPLAQUE(int8_t xPos,int8_t yPos,uint8_t xPASS,uint8_t yPASS,uint8_t FRAME,const uint8_t *SPRITES){
uint8_t OUTBYTE;
uint8_t WSPRITE=(SPRITES[0]);
uint8_t HSPRITE=(SPRITES[1]);
uint16_t Wmax=((HSPRITE*WSPRITE)+1);
uint16_t PICBYTE=FRAME*(Wmax-1);
int8_t RECUPELINEY=RecupeLineY_TPLAQUE(yPos);
if ((xPASS>((xPos+(WSPRITE-1))))||(xPASS<xPos)||((RECUPELINEY>yPASS)||((RECUPELINEY+(HSPRITE))<yPASS))) {return 0x00;}
int8_t SPRITEyLINE=(yPASS-(RECUPELINEY));
uint8_t SPRITEyDECALAGE=(RecupeDecalageY_TPLAQUE(yPos));
uint16_t ScanA=(((xPASS-xPos)+(SPRITEyLINE*WSPRITE))+2);
uint16_t ScanB=(((xPASS-xPos)+((SPRITEyLINE-1)*WSPRITE))+2);
if (ScanA>Wmax) {OUTBYTE=0x00;}else{OUTBYTE=SplitSpriteDecalageY_TPLAQUE(SPRITEyDECALAGE,SPRITES[ScanA+(PICBYTE)],1);}
if ((SPRITEyLINE>0)) {
uint8_t OUTBYTE2=SplitSpriteDecalageY_TPLAQUE(SPRITEyDECALAGE,SPRITES[ScanB+(PICBYTE)],0);
if (ScanB>Wmax) {return OUTBYTE;}else{return OUTBYTE|OUTBYTE2;}
}else{return OUTBYTE;}
}

uint8_t BACK_TPLAQUE(uint8_t xPASS,uint8_t yPASS){
  if (xPASS>115) return 0;
  if (xPASS<12) return 0;
  if (yPASS<2) {
return BACK2_TPLAQUE[(xPASS-12)+(104*yPASS)];
  }else if (yPASS>5) {
    return BACK1_TPLAQUE[(xPASS-12)+(104*(yPASS-6))];
    }
    return 0;
}

uint8_t TEETH_TPLAQUE(uint8_t xPASS){
uint8_t T_BYTE;
uint8_t GRIS=0xff;
if (xPASS<21) return 0;
if (xPASS>106) return 0;
T_BYTE=TPC_TPLAQUE;
if (TPC_TPLAQUE<10) {TPC_TPLAQUE++;}else{TPC_TPLAQUE=0;TC_TPLAQUE++;}
if (TEETH_UP[TC_TPLAQUE].ACTIVE()==1) {
  return teeth_up_TPLAQUE[T_BYTE];
}else if (TEETH_UP[TC_TPLAQUE].ACTIVE()>1) {
  if (ATTAQUE1_TPLAQUE==true) {GRIS=(ATTAQUE2_TPLAQUE)?0b10101010:0b01010101;}
  return GRIS&teeth_up_TPLAQUE[T_BYTE];
  }
  return 0;
}

uint8_t TEETH_DOWN_TPLAQUE(uint8_t xPASS,uint8_t MULT){
uint8_t T_BYTE;
uint8_t GRIS=0xff;
if (xPASS<21) return 0;
if (xPASS>106) return 0;
T_BYTE=TPC_TPLAQUE;
if (TPC_TPLAQUE<10) {TPC_TPLAQUE++;}else{TPC_TPLAQUE=0;TC_TPLAQUE++;}
if (TEETH_DOWN[TC_TPLAQUE].ACTIVE()==1) {
   return teeth_DOWN_TPLAQUE[T_BYTE+(MULT)];
}else if (TEETH_DOWN[TC_TPLAQUE].ACTIVE()>1) {
   if (ATTAQUE1_TPLAQUE==true) {GRIS=(ATTAQUE2_TPLAQUE)?0b10101010:0b01010101;}
   return GRIS&teeth_DOWN_TPLAQUE[T_BYTE+(MULT)];
}
return 0;
}

uint8_t TUBE_TPLAQUE(uint8_t xPASS,uint8_t yPASS){
 if (MainSprite_TPLAQUE.X()>xPASS) {return 0;} 
 if (MainSprite_TPLAQUE.Y()-7>(yPASS*8)) {return 0;} 
 if ((MainSprite_TPLAQUE.X()+9)<xPASS) {return 0;} 
 if ((MainSprite_TPLAQUE.Y()+16)<(yPASS*8)) {return 0;} 
return blitzSprite_TPLAQUE(MainSprite_TPLAQUE.X(),MainSprite_TPLAQUE.Y(),xPASS,yPASS,((MainSprite_TPLAQUE.DIRECTION_Y()==-1)?0:1)+(GD.TUBE_FUEL*2),tube_TPLAQUE);
}

uint8_t Food_Recupe_TPLAQUE(uint8_t xPASS,uint8_t yPASS){
uint8_t BYTE_t=0;
if ((yPASS==0)||(yPASS==7)) return 0;
for (uint8_t t=0;t<MAX_FOOD_D_TPLAQUE;t++){
  if (M_Spr[t].ACTIVE()==0) {goto SUITE;}
 if (M_Spr[t].X()>xPASS) {goto SUITE;} 
 if(M_Spr[t].Y()-7>(yPASS*8)) {goto SUITE;} 
 if ((M_Spr[t].X()+8)<xPASS) {goto SUITE;} 
 if((M_Spr[t].Y()+8)<(yPASS*8)) {goto SUITE;} 
BYTE_t=BYTE_t|blitzSprite_TPLAQUE(M_Spr[t].X(),M_Spr[t].Y(),xPASS,yPASS,GD.Food_Type,FOOD_TPLAQUE);
SUITE:;
}
return BYTE_t;
}

void Recupe_TPLAQUE(uint8_t xPASS,uint8_t yPASS){
  if (xPASS>115) return;
  if (xPASS<12) return;
 switch(yPASS){
  case 0:Draw_BACKGROUND(xPASS,yPASS);break;
  case 1:Draw_BACKGROUND(xPASS,yPASS);PICOKIT.DirectDraw(xPASS,yPASS,TEETH_TPLAQUE(xPASS),color(31,63,31));break;
  case 6:Draw_BACKGROUND(xPASS,yPASS);PICOKIT.DirectDraw(xPASS,yPASS,TEETH_DOWN_TPLAQUE(xPASS,0),color(31,63,31));break;
  case 7:Draw_BACKGROUND(xPASS,yPASS);PICOKIT.DirectDraw(xPASS,yPASS,TEETH_DOWN_TPLAQUE(xPASS,11),color(31,63,31));break;
  default:break;
 }
Draw_Food(xPASS,yPASS);
return ;
}

uint8_t SplitSpriteDecalageY_TPLAQUE(uint8_t decalage,uint8_t Input,uint8_t UPorDOWN){
if (UPorDOWN) {return Input<<decalage;}
return Input>>(8-decalage);
}

int8_t RecupeLineY_TPLAQUE(int8_t Valeur){
return (Valeur>>3); 
}

uint8_t RecupeDecalageY_TPLAQUE(uint8_t Valeur){
return (Valeur-((Valeur>>3)<<3));
}

void Draw_Tube(uint8_t xPASS,uint8_t yPASS){
PICOKIT.DirectDraw(xPASS,yPASS,TUBE_TPLAQUE(xPASS,yPASS),color(0,63,31));
}

void Draw_Food(uint8_t xPASS,uint8_t yPASS){
	PICOKIT.DirectDraw(xPASS,yPASS,Food_Recupe_TPLAQUE(xPASS,yPASS),FruitCol[GD.Food_Type]);
}

void Draw_Extra_Teeth(uint8_t xPASS,uint8_t yPASS){
Recupe_ExtraTeeth_TPLAQUE(xPASS,yPASS);
}


void Draw_BACKGROUND(uint8_t xPASS,uint8_t yPASS){
PICOKIT.DirectDraw(xPASS,yPASS,BACK_TPLAQUE(xPASS,yPASS),color(31,31,16));
}

void Draw_Scores(uint8_t xPASS,uint8_t yPASS){
PICOKIT.DirectDraw(xPASS,yPASS,recupe_SCORES_TPLAQUE(xPASS,yPASS),color(0,63,31));
}


void Draw_Waepon(uint8_t xPASS,uint8_t yPASS){
if ((weapon.ACTIVE()==1)&&(xPASS==MainSprite_TPLAQUE.X()+3)) {PICOKIT.DirectDraw(xPASS,yPASS,blitzSprite_TPLAQUE(xPASS,weapon.Y(),xPASS,yPASS,0,BALISTIC_TPLAQUE),color(31,63,31));}
}

void Tiny_Flip_TPLAQUE(uint8_t T_FLIP){
uint8_t y,x;
Scan_Teeth_TPLAQUE=0;
PICOKIT.clearPicoBuffer();
for (y = 0; y < 8; y++){
ATTAQUE2_TPLAQUE=0;
TPC_TPLAQUE=0;
TC_TPLAQUE=0;
    for (x = 0; x < 128; x++){ 
ATTAQUE2_TPLAQUE=!ATTAQUE2_TPLAQUE;
switch (T_FLIP){
 case 0:case 1: case 2:
 Recupe_TPLAQUE(x,y);
 Draw_Tube(x,y);
 Draw_Extra_Teeth(x,y);
 Draw_Scores(x,y);
 Draw_Extra_Teeth(x,y);
 Draw_Waepon(x,y);
break;
 case 3:
 
 Draw_BACKGROUND(x,y);
  Draw_Scores(x,y);
  INTRO_TPLAQUE(x,y);

  break;
 default:break;
}}
}
PICOKIT.DisplayColor();

}

void INTRO_TPLAQUE(uint8_t xPASS,uint8_t yPASS){
uint8_t BYTE_=0x00;
if (BLINK_START_TPLAQUE>11) {PICOKIT.DirectDraw(xPASS,yPASS,blitzSprite_TPLAQUE(38,26,xPASS,yPASS,0,START_TPLAQUE),color(31,63,31));}
}

uint8_t TRACE_TPLAQUE(uint8_t xPASS,uint8_t yPASS){
 if (yPASS>0) return 0;
if (xPASS<12) return 0;
if (xPASS>115) return 0; 
return BACK2_TPLAQUE[(xPASS-12)];}
  
uint8_t recupe_SCORES_TPLAQUE(uint8_t xPASS,uint8_t yPASS){
if (xPASS<12) {return 0;}
if (xPASS>34){return 0;}
if (yPASS>0) {return 0;}
return 
(SPEED_BLITZ_TPLAQUE(12,0,xPASS,yPASS,M10000_TPLAQUE,police_TPLAQUE)|
 SPEED_BLITZ_TPLAQUE(16,0,xPASS,yPASS,M1000_TPLAQUE,police_TPLAQUE)|
 SPEED_BLITZ_TPLAQUE(20,0,xPASS,yPASS,M100_TPLAQUE,police_TPLAQUE)|
 SPEED_BLITZ_TPLAQUE(24,0,xPASS,yPASS,M10_TPLAQUE,police_TPLAQUE)|
 SPEED_BLITZ_TPLAQUE(28,0,xPASS,yPASS,M1_TPLAQUE,police_TPLAQUE)|
 SPEED_BLITZ_TPLAQUE(32,0,xPASS,yPASS,0,police_TPLAQUE));
}


void Recupe_ExtraTeeth_TPLAQUE(uint8_t xPASS,uint8_t yPASS){
if (yPASS>0) return;
if (xPASS>116) return;
uint8_t tt=(GD.EXTRA_TEETH*6);
if (xPASS<(117-tt)) return;
for (uint8_t z=0;z<(GD.EXTRA_TEETH+1);z++){
PICOKIT.DirectDraw(xPASS,yPASS, SPEED_BLITZ_TPLAQUE(116-(z*6),0,xPASS,yPASS,0,ExtraTeeth_TPLAQUE),color(31,63,31));
}
}

/*
uint8_t Recupe_ExtraTeeth_TPLAQUE(uint8_t xPASS,uint8_t yPASS){
if (yPASS>0) return 0;
if (xPASS>116) return 0;
if (xPASS<(117-(GD.EXTRA_TEETH*6))) return 0;
Scan_Teeth_TPLAQUE=(Scan_Teeth_TPLAQUE<5)?Scan_Teeth_TPLAQUE+1:0;
return ExtraTeeth_TPLAQUE[Scan_Teeth_TPLAQUE];
}*/

uint8_t SPEED_BLITZ_TPLAQUE(int8_t xPos,int8_t yPos,uint8_t xPASS,uint8_t yPASS,uint8_t FRAME,const uint8_t *SPRITES){
uint8_t WSPRITE=(SPRITES[0]);
uint8_t HSPRITE=(SPRITES[1]);
if ((xPASS>((xPos+(WSPRITE-1))))||(xPASS<xPos)||((yPASS<yPos)||(yPASS>(yPos+(HSPRITE-1))))) {return 0x00;}
return SPRITES[2+(((xPASS-xPos)+((yPASS-yPos)*(WSPRITE)))+(FRAME*(HSPRITE*WSPRITE)))];
}
