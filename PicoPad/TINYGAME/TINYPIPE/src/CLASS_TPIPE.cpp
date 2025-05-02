//       >>>>> T-I-N-Y P-I-P-E for PicoPad RP2040 & RP2350+ GPLv3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-Pipe is free software: you can redistribute it and/or modify
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


// Tiny-Pipe source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tinypipe.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the tinypipe.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#include "../include.h"

extern uint8_t RND_COUNTER_TPIPE;
extern PICOCOMPATIBILITY PICOKIT;
extern PASIVE_SPRITE_TPIPE Target;

//PROTOTYPE
uint8_t COLLISION_1VS1(int8_t x1,int8_t x2,int8_t y1,int8_t y2,int8_t sx1,int8_t sx2,int8_t sy1,int8_t sy2);
uint8_t FLOORS_VS_SPRITE(uint8_t Killed_,int8_t x1,int8_t x2,int8_t y1,int8_t y2);


void PASIVE_SPRITE_TPIPE::INIT(uint8_t Active_,int8_t X_,int8_t Y_){
x=X_;
y=Y_;
frame=0;
active=Active_;
killed=0;
width=7;
height=7;
}
uint8_t PASIVE_SPRITE_TPIPE::GET_K(void){return killed;}
void PASIVE_SPRITE_TPIPE::PUT_K(uint8_t K_){killed=K_;}
int8_t PASIVE_SPRITE_TPIPE::GET_X(void){return x;}
int8_t PASIVE_SPRITE_TPIPE::GET_Y(void){return y;}
uint8_t PASIVE_SPRITE_TPIPE::GET_W(void){return width;}
uint8_t PASIVE_SPRITE_TPIPE::GET_H(void){return height;}
uint8_t PASIVE_SPRITE_TPIPE::GET_F(void){return frame;}
void PASIVE_SPRITE_TPIPE::PUT_A(uint8_t F_){active=F_;}
uint8_t PASIVE_SPRITE_TPIPE::GET_A (void){return active;}
void PASIVE_SPRITE_TPIPE::PUT_F(uint8_t F_){frame=F_;}
void PASIVE_SPRITE_TPIPE::PUT_W(uint8_t Width_){width=Width_;}
void PASIVE_SPRITE_TPIPE::PUT_H(uint8_t height_){height=height_;}
void PASIVE_SPRITE_TPIPE::PUT_X(int8_t X_){x=X_;}
void PASIVE_SPRITE_TPIPE::PUT_Y(int8_t Y_){y=Y_;}
void PASIVE_SPRITE_TPIPE::ANIM(void){PUT_F((GET_F()<2)?(GET_F()+1):0);}





uint8_t SPRITE_TPIPE::GET_NOMOVETIMER(void){return NoMoveTimer;}
void SPRITE_TPIPE::PUT_NOMOVETIMER(uint8_t NoMoveTimer_){NoMoveTimer=NoMoveTimer_;}
int8_t SPRITE_TPIPE::GET_XSPEED(void){return xspeed;}
void SPRITE_TPIPE::PUT_XSPEED(int8_t XSPEED_){xspeed=XSPEED_;}
int8_t SPRITE_TPIPE::GET_YSPEED(void){return yspeed;}
void SPRITE_TPIPE::PUT_YSPEED(int8_t YSPEED_){yspeed=YSPEED_;}
uint8_t SPRITE_TPIPE::GET_AD(void){return Anim_Direction;}
void SPRITE_TPIPE::PUT_AD(uint8_t F_){Anim_Direction=F_;}

void SPRITE_TPIPE::DECEL(void){
if (xspeed<0){
xspeed++;
RunL();
}else if (xspeed>0){
xspeed--;
RunR(); 
}}

uint8_t SPRITE_TPIPE::GET_CJ(void){return Cancel_Jump;}
void SPRITE_TPIPE::PUT_CJ(uint8_t CJ_){Cancel_Jump=CJ_;}
void SPRITE_TPIPE::Reset_X_Speed(void){xspeed=0;xadd=0;}

void SPRITE_TPIPE::RunR(void){
xadd=abs(xspeed)+xadd;
if (xadd>10) {
xadd-=10;
if (GET_X()<114) {
PUT_X(GET_X()+1);
ANIM();
if (FLOORS_VS_SPRITE(GET_K(),GET_X()+1,GET_X()+GET_W()-1,GET_Y(),GET_Y()+GET_H())) {
PUT_X(GET_X()-1);
}}else{
PUT_X(8);;
}}}
  
void SPRITE_TPIPE::RunL(void){
xadd=abs(xspeed)+xadd;
if (xadd>10) {
xadd-=10;
if (GET_X()>8) {
PUT_X(GET_X()-1);
ANIM();
if (FLOORS_VS_SPRITE(GET_K(),GET_X()+1,GET_X()+GET_W()-1,GET_Y(),GET_Y()+GET_H())) {
PUT_X(GET_X()+1);
}}else{PUT_X(114);}}
}

void SPRITE_TPIPE::GRAVITY_REFRESH(void){
yspeed=(yspeed<30)?yspeed+2:30;
yadd=abs(yspeed)+yadd;}

void SPRITE_TPIPE::GRAVITY(uint8_t Main_){
if (GET_A()) {
GRAVITY_REFRESH();
if (yspeed>=0) {
while(yadd>10) {PUT_Y(GET_Y()+1);yadd-=10;}
while(1){
if (FLOORS_VS_SPRITE(GET_K(),GET_X()+1,GET_X()+GET_W()-1,GET_Y(),GET_Y()+GET_H())) {
PUT_Y(GET_Y()-1);
RESET_GRAVITY();
Cancel_Jump=(Cancel_Jump==1)?2:Cancel_Jump;
}else{if (GET_Y()>63) {PUT_A(0);}break;}}
}else{
while(yadd>10) {if (GET_Y()>-10) {PUT_Y(GET_Y()-1);yadd-=10;}else{RESET_GRAVITY();}}
if (FLOORS_VS_SPRITE(GET_K(),GET_X()+1,GET_X()+GET_W()-1,GET_Y(),GET_Y()+GET_H())) {
while(1){
if (FLOORS_VS_SPRITE(GET_K(),GET_X()+1,GET_X()+GET_W()-1,GET_Y(),GET_Y()+GET_H())) {
PUT_Y(GET_Y()+1);
RESET_GRAVITY();
}else{if ((Target.GET_A()==0)&&(Main_==1)){Target.INIT(1,GET_X(),GET_Y()-8);}break;}
}}}}}

void SPRITE_TPIPE::RESET_GRAVITY(void){
yadd=0;
yspeed=0;
}

void SPRITE_TPIPE::JUMP(void){
if (Cancel_Jump==0) {
Cancel_Jump=1; 
if (FLOORS_VS_SPRITE(GET_K(),GET_X()+1,GET_X()+GET_W()-1,GET_Y()+1,GET_Y()+GET_H()+1)) {
yspeed=-25;
yadd=10;
}}}

void SPRITE_TPIPE::RIGHTMOVE(void){
PUT_AD(0);
if (xspeed<0){
DECEL();
RunL();
}else{
xspeed=(xspeed<10)?xspeed+1:xspeed;
RunR();
}}
  
void SPRITE_TPIPE::LEFTMOVE(void){
PUT_AD(3);
  if (xspeed>0){
DECEL();
RunR();
}else{
xspeed=(xspeed>-10)?xspeed-1:xspeed;
RunL();
}}

void SPRITE_TPIPE::DIRECT_R_MOVE(void){
if (GET_X()>109){
if (GET_Y()>52){
PUT_X(24);PUT_Y(-3);
RESET_GRAVITY();
goto EnD_;}
}
PUT_AD(0);
RunR(); 
EnD_:;
}
  
void SPRITE_TPIPE::DIRECT_L_MOVE(void){
if (GET_X()<12){
if (GET_Y()>52){
PUT_X(97);PUT_Y(-3);
RESET_GRAVITY();
goto EnD_;}
}
PUT_AD(3);
RunL();  
EnD_:;
}

void SPRITE_TPIPE::AUTO_MOVE(void){
if (xspeed>0) {DIRECT_R_MOVE();}else{DIRECT_L_MOVE();}
}


uint8_t COLLISION_SIMPLIFIED(SPRITE_TPIPE *Main_,SPRITE_TPIPE *Other_){
return COLLISION_1VS1(Main_->GET_X(),Main_->GET_X()+Main_->GET_W(),Main_->GET_Y(),Main_->GET_Y()+Main_->GET_H(),
Other_->GET_X()+1,Other_->GET_X()+Other_->GET_W()-2,Other_->GET_Y()+5,Other_->GET_Y()+Other_->GET_H());
}

uint8_t COLLISION_1VS1(int8_t x1,int8_t x2,int8_t y1,int8_t y2,int8_t sx1,int8_t sx2,int8_t sy1,int8_t sy2){
if (x1>sx2) return 0;
if (x2<sx1) return 0;
if (y1>sy2) return 0;
if (y2<sy1) return 0;
return 1;
}

uint8_t FLOORS_VS_SPRITE(uint8_t Killed_,int8_t x1,int8_t x2,int8_t y1,int8_t y2){
if (!Killed_) {
if (COLLISION_1VS1(x1,x2,y1,y2,0,58,15,17)) return 1;
if (COLLISION_1VS1(x1,x2,y1,y2,69,127,15,17)) return 1;
if (COLLISION_1VS1(x1,x2,y1,y2,38,89,29,31)) return 1;
if (COLLISION_1VS1(x1,x2,y1,y2,0,28,34,36)) return 1;
if (COLLISION_1VS1(x1,x2,y1,y2,99,127,34,36)) return 1;
if (COLLISION_1VS1(x1,x2,y1,y2,0,48,46,48)) return 1;
if (COLLISION_1VS1(x1,x2,y1,y2,79,127,46,48)) return 1;
if (COLLISION_1VS1(x1,x2,y1,y2,0,127,61,61)) return 1;
}
return 0;
}

void RND_MIXER_TPIPE(void){
RND_COUNTER_TPIPE=(RND_COUNTER_TPIPE<29)?RND_COUNTER_TPIPE+1:0;
}

uint8_t PSEUDO_RND_TPIPE(void){
RND_MIXER_TPIPE();
return RND_TPIPE[RND_COUNTER_TPIPE];
}
