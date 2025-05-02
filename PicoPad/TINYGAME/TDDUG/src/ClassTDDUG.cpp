//       >>>>> T-I-N-Y D-D-U-G for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-DDug is free software: you can redistribute it and/or modify
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


// The Tiny-DDug source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "tddug.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the tddug.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#include "../include.h"

extern GAME_DATA_TDDUG GD_DDUG;
extern Main_Sprite_TDDUG DUG_TDDUG;
extern uint8_t MAIN_SPEED_STEP_TDDUG;

int8_t Sprite_TDDUG::X(void){return x;}
int8_t Sprite_TDDUG::Y(void){return y;}
int8_t Sprite_TDDUG::ACTIVE(void){return active;}
void Sprite_TDDUG::PUT_X(int8_t XX){x=XX;}
void Sprite_TDDUG::PUT_Y(int8_t YY){y=YY;}
void Sprite_TDDUG::PUT_ACTIVE(int8_t ACT){active=ACT;}
int8_t Sprite_TDDUG::DIRECTION_X(void){return direction_x;}
int8_t Sprite_TDDUG::DIRECTION_Y(void){return direction_y;}
void Sprite_TDDUG::PUT_DIRECTION_X(int8_t D_X){direction_x=D_X;}
void Sprite_TDDUG::PUT_DIRECTION_Y(int8_t D_Y){direction_y=D_Y;}
//end sprite


int8_t Moving_Sprite_TDDUG::SOMX(void){return Som_x;}
int8_t Moving_Sprite_TDDUG::SOMY(void){return Som_y;}
int8_t Moving_Sprite_TDDUG::SX(void){return sx;}
int8_t Moving_Sprite_TDDUG::SY(void){return sy;}
void Moving_Sprite_TDDUG::PUT_SX(int8_t SX_){sx=SX_;}
void Moving_Sprite_TDDUG::PUT_SY(int8_t SY_){sy=SY_;}
void Moving_Sprite_TDDUG::PUT_SOMX(int8_t PUT_SOMX_){Som_x=PUT_SOMX_;}
void Moving_Sprite_TDDUG::PUT_SOMY(int8_t PUT_SOMY_){Som_y=PUT_SOMY_;}
void Moving_Sprite_TDDUG::Ou_suis_je(uint8_t &x_,uint8_t &y_){uint8_t PX=x_-20;int8_t PY=y_-8;x_=PX>>2;y_=PY>>2;}
//end moving sprite


uint8_t Enemy_Sprite_TDDUG::BIG_ZIP(void){
return Big_zip;
}

void Enemy_Sprite_TDDUG::PUT_BIG_ZIP(uint8_t Fir_){
Big_zip=Fir_;
}

uint8_t Enemy_Sprite_TDDUG::FIRST(void){
return First;
}

void Enemy_Sprite_TDDUG::PUT_FIRST(uint8_t Fir_){
First=Fir_;
}

uint8_t Enemy_Sprite_TDDUG::ANIM_DIRECT(void){
return Anim_Direct;
}

void Enemy_Sprite_TDDUG::PUT_ANIM_DIRECT(uint8_t H_Dir_){
Anim_Direct=H_Dir_;
}

void Enemy_Sprite_TDDUG::PUT_TYPE(uint8_t TY_){
Type=TY_;
}

uint8_t Enemy_Sprite_TDDUG::TYPE(void){
return Type; 
}

void Enemy_Sprite_TDDUG::PUT_ANIM(uint8_t Ani_){
Anim=Ani_;
}

uint8_t Enemy_Sprite_TDDUG::ANIM(void){
return Anim; 
}

void Enemy_Sprite_TDDUG::PUT_TRACKING(int8_t TR_){
Tracking=TR_;
}

int8_t Enemy_Sprite_TDDUG::TRACKING(void){
return Tracking;
}

void Enemy_Sprite_TDDUG::INIT(int8_t X_,int8_t Y_,int8_t Type_){
PUT_TRACKING(1);
PUT_SOMX(0);
PUT_SOMY(0);
PUT_SX(0);
PUT_SY(0);
PUT_X(X_);
PUT_Y(Y_);
PUT_DIRECTION_X(RND_TDDUG());
PUT_DIRECTION_Y(RND_TDDUG());
PUT_ACTIVE(1);
First=0;
Type=Type_;
Big_zip=0;
}

void Enemy_Sprite_TDDUG::HAUT(void){
int8_t Sy_=SY(),y_=Y(),Somy_=SOMY();
int8_t T_SOMY=SOMY();
if ((T_SOMY+(-ENEMY_SPEED_STEP_TDDUG))<=-SPRITE_ACCEL_SPEED_TDDUG) {
  PUT_SOMY((T_SOMY+(-ENEMY_SPEED_STEP_TDDUG))+SPRITE_ACCEL_SPEED_TDDUG);
  PUT_Y(Y()-1);
  }else{
    PUT_SOMY(T_SOMY+(-ENEMY_SPEED_STEP_TDDUG));
    }
if ((E_GRID_UPDATE_UP())||(Y()<0)) {
PUT_SY(Sy_);
PUT_Y(y_);
PUT_SOMY(Somy_);
NEW_LIMITE_DIRECTION(1);
}else{
  NEW_DIRECTION(1);
  PUT_DIRECTION_Y(-1);
}}

void Enemy_Sprite_TDDUG::DROITE(void){
int8_t Sx_=SX(),x_=X(),Somx_=SOMX();
int8_t T_SOMX=SOMX();
if ((T_SOMX+ENEMY_SPEED_STEP_TDDUG)>=SPRITE_ACCEL_SPEED_TDDUG) {
  PUT_SOMX((T_SOMX+ENEMY_SPEED_STEP_TDDUG)-SPRITE_ACCEL_SPEED_TDDUG);
  PUT_X(X()+1);
  }else{
    PUT_SOMX(T_SOMX+ENEMY_SPEED_STEP_TDDUG);
    }
if ((E_GRID_UPDATE_RIGHT())||(X()>100)) {
PUT_SX(Sx_);
PUT_X(x_);
PUT_SOMX(Somx_);
NEW_LIMITE_DIRECTION(0);
}else{
   NEW_DIRECTION(0);
   PUT_DIRECTION_X(1);
}
if (X()!=x_) {PUT_ANIM_DIRECT(1);}
}  

void Enemy_Sprite_TDDUG::BAS(void){
int8_t Sy_=SY(),y_=Y(),Somy_=SOMY();
int8_t T_SOMY=SOMY();
if ((T_SOMY+ENEMY_SPEED_STEP_TDDUG)>=SPRITE_ACCEL_SPEED_TDDUG) {
  PUT_SOMY((T_SOMY+ENEMY_SPEED_STEP_TDDUG)-SPRITE_ACCEL_SPEED_TDDUG);
  PUT_Y(Y()+1);
  }else{
    PUT_SOMY(T_SOMY+ENEMY_SPEED_STEP_TDDUG);
    }
if ((E_GRID_UPDATE_DOWN())||(Y()>48)) {
PUT_SY(Sy_);
PUT_Y(y_);
PUT_SOMY(Somy_);
NEW_LIMITE_DIRECTION(1);
}else{
  NEW_DIRECTION(1);
  PUT_DIRECTION_Y(1);
}}

void Enemy_Sprite_TDDUG::GAUCHE(void){
int8_t Sx_=SX(),x_=X(),Somx_=SOMX();
int8_t T_SOMX=SOMX();
if ((T_SOMX+(-ENEMY_SPEED_STEP_TDDUG))<=-SPRITE_ACCEL_SPEED_TDDUG) {
  PUT_SOMX((T_SOMX+(-ENEMY_SPEED_STEP_TDDUG))+SPRITE_ACCEL_SPEED_TDDUG);
  PUT_X(X()-1);
  }else{
    PUT_SOMX(T_SOMX+(-ENEMY_SPEED_STEP_TDDUG));
    }
if ((E_GRID_UPDATE_LEFT())||(X()<20)) {
PUT_SX(Sx_);
PUT_X(x_);
PUT_SOMX(Somx_);
NEW_LIMITE_DIRECTION(0);
}else{
NEW_DIRECTION(0);  
PUT_DIRECTION_X(-1);
}
if (X()!=x_) {PUT_ANIM_DIRECT(0);}
}

void Enemy_Sprite_TDDUG::NEW_LIMITE_DIRECTION(int8_t dir_){
switch(dir_){
  case(0):if (DIRECTION_X()==1) {PUT_DIRECTION_X(-1);}else{PUT_DIRECTION_X(1);}break;
  case(1):if (DIRECTION_Y()==1) {PUT_DIRECTION_Y(-1);}else{PUT_DIRECTION_Y(1);}break;  
default:break;
}}

void Enemy_Sprite_TDDUG::NEW_DIRECTION(int8_t dir_){
switch(dir_){
  case(0):if (RecupeDecalageY_TDDUG(X()-20)!=0) {PUT_DIRECTION_Y(RND_TDDUG());}break;  
  case(1):if (RecupeDecalageY_TDDUG(Y())!=0) {PUT_DIRECTION_X(RND_TDDUG());}break;  
}}

uint8_t Enemy_Sprite_TDDUG::E_GRID_UPDATE_RIGHT(void){
 uint8_t XX_=X()+7,YY_=Y();
 Ou_suis_je(XX_,YY_); 
 if (READ_GRID_TDDUG(XX_,YY_)==1) {return Tracking;}
 if (READ_GRID_TDDUG(XX_,YY_+1)==1) {return Tracking;}
 return 0;
  }

uint8_t Enemy_Sprite_TDDUG::E_GRID_UPDATE_LEFT(void){
  uint8_t XX_=X(),YY_=Y();
 Ou_suis_je(XX_,YY_); 
 if (READ_GRID_TDDUG(XX_,YY_)==1) {return Tracking;}
 if (READ_GRID_TDDUG(XX_,YY_+1)==1) {return Tracking;}
 return 0;
  }

uint8_t Enemy_Sprite_TDDUG::E_GRID_UPDATE_UP(void){
  uint8_t XX_=X(),YY_=Y();
 Ou_suis_je(XX_,YY_); 
 if (READ_GRID_TDDUG(XX_,YY_)==1) {return Tracking;}
 if (READ_GRID_TDDUG(XX_+1,YY_)==1) {return Tracking;} 
 return 0;
  }

uint8_t Enemy_Sprite_TDDUG::E_GRID_UPDATE_DOWN(void){
  uint8_t XX_=X(),YY_=Y()+7;
 Ou_suis_je(XX_,YY_); 
 if (READ_GRID_TDDUG(XX_,YY_)==1) {return Tracking;}
 if (READ_GRID_TDDUG(XX_+1,YY_)==1) {return Tracking;} 
 return 0;
  }

void Main_Sprite_TDDUG::INIT(void){
PUT_X(60);
PUT_Y(32);
PUT_ACTIVE(1);
PUT_DIRECTION_Y(0);
PUT_DIRECTION_X(1);
PUT_SOMX(0);
PUT_SOMY(0);
PUT_SX(0);
PUT_SY(0);
}

void Main_Sprite_TDDUG::MHAUT(void){
int8_t T_SOMY=SOMY();
if ((T_SOMY+(-MAIN_SPEED_STEP_TDDUG))<=-MAIN_ACCEL_SPEED_TDDUG) {
  PUT_SOMY((T_SOMY+(-MAIN_SPEED_STEP_TDDUG))+MAIN_ACCEL_SPEED_TDDUG);
  PUT_Y(Y()-1);
  }else{
    PUT_SOMY(T_SOMY+(-MAIN_SPEED_STEP_TDDUG));
    }
ANIM_UPDATE_MAIN_TDDUG(3);
if(Y()<0) {PUT_Y(0);PUT_SY(0);PUT_SOMY(0);}
PUT_DIRECTION_Y(-1);
GRID_UPDATE_UP();
}

void Main_Sprite_TDDUG::MDROITE(void){
int8_t T_SOMX=SOMX();
if ((T_SOMX+MAIN_SPEED_STEP_TDDUG)>=MAIN_ACCEL_SPEED_TDDUG) {
  PUT_SOMX((T_SOMX+MAIN_SPEED_STEP_TDDUG)-MAIN_ACCEL_SPEED_TDDUG);
  PUT_X(X()+1);
  }else{
    PUT_SOMX(T_SOMX+MAIN_SPEED_STEP_TDDUG);
    }
ANIM_UPDATE_MAIN_TDDUG(0);
if(X()>100){PUT_X(100);PUT_SX(0);PUT_SOMX(0);}
PUT_DIRECTION_X(1);
GRID_UPDATE_RIGHT();
}  

void Main_Sprite_TDDUG::MBAS(void){
int8_t T_SOMY=SOMY();
if ((T_SOMY+MAIN_SPEED_STEP_TDDUG)>=MAIN_ACCEL_SPEED_TDDUG) {
  PUT_SOMY((T_SOMY+MAIN_SPEED_STEP_TDDUG)-MAIN_ACCEL_SPEED_TDDUG);
  PUT_Y(Y()+1);
  }else{
    PUT_SOMY(T_SOMY+MAIN_SPEED_STEP_TDDUG);
    }
ANIM_UPDATE_MAIN_TDDUG(1);
if(Y()>48) {PUT_Y(48);PUT_SY(0);PUT_SOMY(0);}  
PUT_DIRECTION_Y(1);
GRID_UPDATE_DOWN();
}

void Main_Sprite_TDDUG::MGAUCHE(void){
int8_t T_SOMX=SOMX();
if ((T_SOMX+(-MAIN_SPEED_STEP_TDDUG))<=-MAIN_ACCEL_SPEED_TDDUG) {
  PUT_SOMX((T_SOMX+(-MAIN_SPEED_STEP_TDDUG))+MAIN_ACCEL_SPEED_TDDUG);
  PUT_X(X()-1);
  }else{
    PUT_SOMX(T_SOMX+(-MAIN_SPEED_STEP_TDDUG));
    }
ANIM_UPDATE_MAIN_TDDUG(2);
if(X()<20) {PUT_X(20);PUT_SX(0);PUT_SOMX(0);}
PUT_DIRECTION_X(-1);
GRID_UPDATE_LEFT();
}

void Main_Sprite_TDDUG::WALK_RIGHT(void){
if (RecupeDecalageY_TDDUG(Y())==0) {MDROITE();
}else{
switch(DIRECTION_Y()){
  case  (1):MBAS(); break;
  case (-1):MHAUT();break;
  default:break;
  }}}

void Main_Sprite_TDDUG::WALK_LEFT(void){
if (RecupeDecalageY_TDDUG(Y())==0) {MGAUCHE();
}else{
switch(DIRECTION_Y()){
  case  (1):MBAS(); break;
  case (-1):MHAUT();break;
  default:break;
  }}}

void Main_Sprite_TDDUG::WALK_UP(void){
if (RecupeDecalageY_TDDUG(X()-20)==0) {MHAUT();
}else{
switch(DIRECTION_X()){
  case  (1):MDROITE(); break;
  case (-1):MGAUCHE();break;
  default:break;
  }}}

void Main_Sprite_TDDUG::WALK_DOWN(void){
if (RecupeDecalageY_TDDUG(X()-20)==0) {MBAS(); 
}else{
switch(DIRECTION_X()){
  case  (1):MDROITE(); break;
  case (-1):MGAUCHE();break;
  default:break;
  }}}

void Main_Sprite_TDDUG::GRID_UPDATE_RIGHT(void){
  uint8_t Snd=0;
  uint8_t XX_=X()+7,YY_=Y();
 Ou_suis_je(XX_,YY_); 
 if (READ_GRID_TDDUG(XX_,YY_)==1) {Snd=1;SCORES_ADD_TDDUG();WRITE_GRID_TDDUG(XX_,YY_);}
 if (READ_GRID_TDDUG(XX_,YY_+1)==1) {Snd=1;SCORES_ADD_TDDUG();WRITE_GRID_TDDUG(XX_,YY_+1);}
 if (Snd) SND_DDUG();
  }
  
void Main_Sprite_TDDUG::GRID_UPDATE_LEFT(void){
    uint8_t Snd=0;
  uint8_t XX_=X(),YY_=Y();
 Ou_suis_je(XX_,YY_); 
 if (READ_GRID_TDDUG(XX_,YY_)==1) {Snd=1;SCORES_ADD_TDDUG();WRITE_GRID_TDDUG(XX_,YY_);}
 if (READ_GRID_TDDUG(XX_,YY_+1)==1) {Snd=1;SCORES_ADD_TDDUG();WRITE_GRID_TDDUG(XX_,YY_+1);}
   if (Snd) SND_DDUG();
  }
  
void Main_Sprite_TDDUG::GRID_UPDATE_UP(void){
    uint8_t Snd=0;
  uint8_t XX_=X(),YY_=Y();
 Ou_suis_je(XX_,YY_); 
 if (READ_GRID_TDDUG(XX_,YY_)==1) {Snd=1;SCORES_ADD_TDDUG();WRITE_GRID_TDDUG(XX_,YY_);}
 if (READ_GRID_TDDUG(XX_+1,YY_)==1) {Snd=1;SCORES_ADD_TDDUG();WRITE_GRID_TDDUG(XX_+1,YY_);} 
  if (Snd) SND_DDUG();
  }
  
void Main_Sprite_TDDUG::GRID_UPDATE_DOWN(void){
    uint8_t Snd=0;
  uint8_t XX_=X(),YY_=Y()+7;
 Ou_suis_je(XX_,YY_); 
 if (READ_GRID_TDDUG(XX_,YY_)==1) {Snd=1;SCORES_ADD_TDDUG();WRITE_GRID_TDDUG(XX_,YY_);}
 if (READ_GRID_TDDUG(XX_+1,YY_)==1) {Snd=1;SCORES_ADD_TDDUG();WRITE_GRID_TDDUG(XX_+1,YY_);} 
  if (Snd) SND_DDUG();
  }
//end main sprite



void WEAPON_TDDUG::INIT_WEAPON(void){
PUT_ACTIVE(0); 
}

void WEAPON_TDDUG::ADJUST_WEAPON(Main_Sprite_TDDUG &MS_){
Sound_TDDUG(100,1);
Sound_TDDUG(200,12);
switch(GD_DDUG.DIRECTION_ANIM){
 case(0):PUT_ANIM_OR(0);PUT_X(MS_.X()+8);PUT_Y(MS_.Y()+2);break; 
 case(1):PUT_ANIM_OR(1);PUT_X(MS_.X()+2);PUT_Y(MS_.Y()+8);break; 
 case(2):PUT_ANIM_OR(0);PUT_X(MS_.X()-4);PUT_Y(MS_.Y()+2);break; 
 case(3):PUT_ANIM_OR(1);PUT_X(MS_.X()+2);PUT_Y(MS_.Y()-4);break; 
 default:break;
}
WEAPON_TDDUG NUl;
WEAPON_COLISION_TDDUG(NUl,0);
}

void WEAPON_TDDUG::ADJUST_WEAPON2(WEAPON_TDDUG &W_,Main_Sprite_TDDUG &MS_){
uint8_t A_,B_,C_;
switch(GD_DDUG.DIRECTION_ANIM){
 case(0):A_=0;B_=(MS_.X()+12);C_=(MS_.Y()+2);break; 
 case(1):A_=(1);B_=(MS_.X()+2);C_=(MS_.Y()+12);break; 
 case(2):A_=(0);B_=(MS_.X()-8);C_=(MS_.Y()+2);break; 
 case(3):A_=(1);B_=(MS_.X()+2);C_=(MS_.Y()-8);break; 
 default:A_=0;B_=0;C_=0;break;
}
PUT_ANIM_OR(A_);PUT_X(B_);PUT_Y(C_);
WEAPON_COLISION_TDDUG(W_,1);
}

void WEAPON_TDDUG::WEAPON_COLISION_TDDUG(WEAPON_TDDUG W_,uint8_t Nu_){
uint8_t XX_=X()+2,YY_=Y()+2;
Ou_suis_je(XX_,YY_); 
if (READ_GRID_TDDUG(XX_,YY_)==1) {
 if (Nu_==0) {
  W_.PUT_ACTIVE(5);
  }else{PUT_ACTIVE(0);}
}}

uint8_t WEAPON_TDDUG::ANIM_OR(void){
return anim_or;
}

void WEAPON_TDDUG::PUT_ANIM_OR(uint8_t WA_){
anim_or=WA_;
}

void WEAPON_TDDUG::Ou_suis_je(uint8_t &x_,uint8_t &y_){
uint8_t PX=x_-20;
int8_t PY=y_-8;
x_=PX>>2;
y_=PY>>2;
}
