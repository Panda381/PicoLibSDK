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

//PROTOTYPE
void Create_New_Intercept_TMISSILE(int8_t X_,int8_t Y_);
void SNDBOX_TMISSILE(uint8_t Snd_);
void Tiny_Flip_TMISSILE(void);

//class ARMY
void ARMY_TMISSILE::RESET_WEAPON(uint8_t LEVEL_){
 if (LEVEL_>4) {SPARE=4;}else{SPARE=3;}
ROCKET=10;
}

uint8_t ARMY_TMISSILE::USE_WEAPON(void){
if (ROCKET>0) {ROCKET--;}else{
if (SPARE>0) {SPARE--;ROCKET=9;}else{return 0;} 
}return 1;}

void ARMY_TMISSILE::ATTACK_WEAPON(void){
if (ROCKET>0) {
while(1){
if (ROCKET>0) {USE_WEAPON();SNDBOX_TMISSILE(5);}else{goto Exit_;} 
}
Exit_:;
}else{
if (SPARE>0) {SPARE--;SNDBOX_TMISSILE(3);} 
}
Tiny_Flip_TMISSILE();
}

uint8_t ARMY_TMISSILE::GET_SPARE(void){
return SPARE; 
}

uint8_t ARMY_TMISSILE::GET_ROCKET(void){
return ROCKET; 
}
//class ARMY END

//CLASS STATIC SPRITE 
  void STATIC_SPRITE_TMISSILE::INIT(int8_t X_,int8_t Y_,uint8_t ACTIVE_){X=X_;Y=Y_;ACTIVE=ACTIVE_;}
  void STATIC_SPRITE_TMISSILE::PUT_X(int8_t X_){X=X_;}
  void STATIC_SPRITE_TMISSILE::PUT_Y(int8_t Y_){Y=Y_;}
  void STATIC_SPRITE_TMISSILE::PUT_ACTIVE(uint8_t ACTIVE_){ACTIVE=ACTIVE_;} 
  int8_t STATIC_SPRITE_TMISSILE::GET_X(void){return X;}
  int8_t STATIC_SPRITE_TMISSILE::GET_Y(void){return Y;} 
  uint8_t STATIC_SPRITE_TMISSILE::GET_ACTIVE(void){return ACTIVE;}
//CLASS STATIC SPRITE END

//CLASS STATIC ANIM SPRITE 
void STATIC_SPRITE_ANIM_TMISSILE::INIT_DOME(int8_t X_,int8_t Y_,uint8_t ACTIVE_){INIT(X_,Y_,ACTIVE_);FRAME=0;}

uint8_t STATIC_SPRITE_ANIM_TMISSILE::PROGRESS_ANIM(void){
if (FRAME<6) {FRAME++;}else{
FRAME=0;
PUT_ACTIVE(0);
}
return FRAME;
}

void STATIC_SPRITE_ANIM_TMISSILE::PUT_FRAME(uint8_t FRAME_){
FRAME=FRAME_;
}

uint8_t STATIC_SPRITE_ANIM_TMISSILE::GET_FRAME(void){
return FRAME;
}
//CLASS STATIC ANIM SPRITE END

//CLASS LINE TMISSILE
void LINE_TMISSILE::RESET(void){
ACTIVE=0;
}

void LINE_TMISSILE::INIT(int8_t START_X_,int8_t END_X_,uint8_t ACTIVE_){
  START_X=START_X_;
  POS_X=START_X;
  POS_Y=11;
  END_X=END_X_;
  ACTIVE=ACTIVE_;
  Track=POS_Y;
  yPass=0;
  yDeca=0;
  }

void LINE_TMISSILE::DOME_COLLISION(STATIC_SPRITE_ANIM_TMISSILE *DOME_,ARMY_TMISSILE *ARM_){
if ((END_X>54)&&(END_X<73)) {ARM_->ATTACK_WEAPON();}
for(uint8_t t=0;t<NUM_of_DOME_TMISSILE;t++){
  if  (DOME_[t].GET_ACTIVE()) { 
    if (((END_X)>(DOME_[t].GET_X()))&&((END_X)<(DOME_[t].GET_X()+14))) {
    DOME_[t].PUT_FRAME(1);
    goto Out_;
    }
  }
}
Out_:;
}

uint8_t LINE_TMISSILE::PROGRESS(STATIC_SPRITE_ANIM_TMISSILE *DOME_,ARMY_TMISSILE *ARM_){ 
if (Track<55) {Track++;}else{DOME_COLLISION(DOME_,ARM_);ACTIVE=0;return 0;}
yPass=RecupeLineY(Track);
yDeca=(0xff>>(7-RecupeDecalageY(Track)));
return 0;
}

int8_t LINE_TMISSILE::GET_START_X(void){return START_X;}
int8_t LINE_TMISSILE::GET_POS_X(void){return POS_X;}
int8_t LINE_TMISSILE::GET_POS_Y(void){return POS_Y;}
uint8_t LINE_TMISSILE::GET_yPass(void){return yPass;}
uint8_t LINE_TMISSILE::GET_yDeca(void){return yDeca;}
int8_t LINE_TMISSILE::GET_END_X(void){return END_X;}
uint8_t LINE_TMISSILE::GET_ACTIVE(void){return ACTIVE;} 
uint8_t LINE_TMISSILE::GET_TRACK(void){return Track;}
void LINE_TMISSILE::PUT_START_X(int8_t START_X_){START_X=START_X_;}
void LINE_TMISSILE::PUT_POS_X(int8_t POS_X_){POS_X=POS_X_;}
void LINE_TMISSILE::PUT_POS_Y(int8_t POS_Y_){POS_Y=POS_Y_;}
void LINE_TMISSILE::PUT_END_X(int8_t END_X_){END_X=END_X_;}
void LINE_TMISSILE::PUT_ACTIVE(uint8_t ACTIVE_){ACTIVE=ACTIVE_;} 
void LINE_TMISSILE::PUT_TRACK(uint8_t TRACK_){Track=TRACK_;} 
//CLASS LINE TMISSILE END

//CLASS SHIELD:LINE_TMISSILE
void DEFENCE::UPDATE_DEFENCE(void){
if (Count>0) {
  XCF=XCF+(XF);
  YCF=YCF+(YF);
  PUT_X((XCF));
  PUT_Y((YCF));
 Count--; 
  }else{
    PUT_ACTIVE(0);
    Create_New_Intercept_TMISSILE(GET_X()-3,GET_Y()-6);
}}

void DEFENCE::INIT(void){
PUT_X(0);
PUT_Y(0); 
Count=0;
PUT_ACTIVE(0); 
XCF=0;
YCF=0;
}

void DEFENCE::NEW(int8_t xEND_,int8_t yEND_){
XCF=63.00;
YCF=51.00;
float tXF,tYF;
XF=(xEND_-XCF);
YF=(yEND_-YCF);
tXF=abs(XF);
tYF=abs(YF);
Count=(tXF>=tYF)?(tXF/SPEED_SHOOT_TMISSILE):(tYF/SPEED_SHOOT_TMISSILE);
XF=((XF)/Count);
YF=((YF)/Count);  
PUT_ACTIVE(1); 
}
//CLASS SHIELD:LINE_TMISSILE END

//CLASS Clk
void CLK::INIT(uint8_t Start_,uint8_t End_,uint8_t Loop_){Start=Start_;End=End_;Pos=Start;Trig=0;Loop=Loop_;}

uint8_t CLK::PROGRESS(void){
  if ((Loop==0)&&(Trig==1)) {return Trig;}
  if (Pos<End) {
    Pos++;
    }else{
      Trig=!Trig;
      Pos=(Loop==1)?Start:End;
      }
      return Trig;
      }
      
void CLK::RESET(void){Pos=Start;Trig=0;}
//CLASS Clk END

//Class CROSS
void CROSS::INIT(void){
PUT_X(62);
PUT_Y(30); 
PUT_ACTIVE(1);
}

void CROSS::RIGHT(void){
PUT_X(GET_X()+3);
LIMITR();
}

void CROSS::LEFT(void){
PUT_X(GET_X()-3);
LIMITL();
}

void CROSS::DOWN(void){
PUT_Y(GET_Y()+2);
LIMITD();
}

void CROSS::UP(void){
PUT_Y(GET_Y()-2);
LIMITU();
}

void CROSS::LIMITR(void){
if (GET_X()>123) {PUT_X(123);}
}
void CROSS::LIMITL(void){
if (GET_X()<2) {PUT_X(2);} 
}
void CROSS::LIMITU(void){
if (GET_Y()<14) {PUT_Y(14);}
}
void CROSS::LIMITD(void){
if (GET_Y()>48) {PUT_Y(48);} 
}
//Class CROSS END
