//   >>>>> T-I-N-Y  T-R-I-C-K for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  tiny-Trick is free software: you can redistribute it and/or modify
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


// The tiny-Trick source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "ttrick.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the ttrick.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#include "../include.h"

// var public
uint8_t Player1_Rotate;
uint8_t Computer_Rotate;
uint8_t COM_MAX_SPEED_x;
uint8_t COM_MAX_SPEED_y;
uint8_t ORIx;
uint8_t ORIy;
uint8_t SKIP_FRAME;
uint8_t Drag_Puck;
uint8_t Puck_x;
uint8_t Puck_y;
uint8_t X_Drag;
uint8_t Y_drag;
uint8_t GOAL_;
uint8_t Points_CPU;
uint8_t Points_Pl1;
// fin var public

PICOCOMPATIBILITY PICOKIT;

Sprite sprite[5];

void Reset_new_game(void){
Drag_Puck=255;
GOAL_=255;
Points_CPU=0;
Points_Pl1=0;
}

void ResetAllSprites(void){
uint8_t t;
for (t=0;t<4;t++){  
sprite[t].Xpos=0;   
sprite[t].Ypos=0;  
sprite[t].CumuleX=0;
sprite[t].CumuleY=0;
sprite[t].SpeedXpositif=0;
sprite[t].SpeedXnegatif=0;
sprite[t].SpeedYpositif=0;
sprite[t].SpeedYnegatif=0;
}}

void INIT_ALL_VAR(void){
ORIx=0;
ORIy=0;
Player1_Rotate=0;
Computer_Rotate=4;
sprite[0].Xpos=37;   
sprite[0].Ypos=23;  
sprite[1].Xpos=74;   
sprite[1].Ypos=24;  
sprite[3].Xpos=4;
sprite[3].Ypos=28;   
sprite[4].Xpos=119; 
sprite[4].Ypos=28;  
sprite[2].Xpos=63;
if (SKIP_FRAME){
  sprite[2].SpeedYpositif=26;
  sprite[2].Ypos=6;
}else{
  sprite[2].SpeedYnegatif=26;
  sprite[2].Ypos=56;
}}

#define PUCK_X sprite[2].Xpos
#define PUCK_Y sprite[2].Ypos

void Goal_Adj(uint8_t Y_POS,uint8_t SPR_Type){
if (Y_POS==1) {
if (sprite[SPR_Type].Ypos<34) {sprite[SPR_Type].Ypos+=2;}
}else{
if (sprite[SPR_Type].Ypos>20) {sprite[SPR_Type].Ypos-=2;}
}}

void SND(uint8_t Snd){
uint8_t q; 
  switch(Snd){
    case 0:for(q=1;q<30;q++){Sound(140,30);Sound(220,30);}break;
    case 1:for(q=1;q<30;q++){Sound(20,35);Sound(200,45);}break;
    case 2:Sound(3,5);Sound(10,10);Sound(3,5);break;
    case 3:Sound(3,2);break;
    case 4:Sound(200,10);Sound(80,4);break;
    case 5:for(q=1;q<20;q++){Sound(4,80);Sound(100,80);}break;
    case 6:for(q=200;q>10;q--){Sound(200-q,3);Sound(q,12);}break;
    case 7:Sound(100,250);Sound(20,250);break;
    default:break;
 }}

int main() {
PICOKIT.SetFPS(22);
PICOKIT.Init();
INTRO:;
PICOKIT.SetColorBuffer(31,63,21,0,52,31,0,0,0);
Reset_new_game();
{ 
uint8_t TIMER=0;
while(1){
       #if USE_SCREENSHOT
		switch (KeyGet())
		{
		case KEY_Y: SmallScreenShot();
		}
		#else
		if (KeyPressed(KEY_Y)) ResetToBootLoader();
		#endif
if (TIMER<255) {TIMER++;}else{TIMER=0;}
_delay_ms(4);
switch(TIMER){
case (0) : intro(0,0); break;
case (128) : intro(4,0);break;
default:break;}
if (BUTTON_DOWN) {SND(7);_delay_ms(100);goto Start_Game;}
}
}
Start_Game:;
PICOKIT.SetColorBuffer(26,56,31,20,37,15,0,0,0);
RESTART:;
ResetAllSprites();
INIT_ALL_VAR();
Tiny_Flip();
_delay_ms(600);
SND(0);
while(1){
COM_MAX_SPEED_x=28;
COM_MAX_SPEED_y=28;
if  (TINYJOYPAD_UP) {Goal_Adj(0,3);ORIx=2;Speed_adj_V_N(0);}
else if (TINYJOYPAD_DOWN) {Goal_Adj(1,3);ORIx=8;Speed_adj_V_P(0);}else{ORIx=0;Speed_V_Decel(0);}
if (TINYJOYPAD_RIGHT) {ORIy=4; Speed_adj_H_P(0);}
else if (TINYJOYPAD_LEFT) {ORIy=16;Speed_adj_H_N(0);}else{ORIy=0;Speed_H_Decel(0);}
Adjust_Cumule_Speed(0);
Player1_Rotate=Direction_Recupe(ORIy+ORIx);
if (BUTTON_DOWN) {if ((Drag_Puck==0)&&(sprite[2].Xpos<119)) {KICK_PUCK(Player1_Rotate);}}
collision_check_LimiteScreen(0);
COM_MAX_SPEED_x=Adjust_Speed_Computer(X_Drag,sprite[1].Xpos);
COM_MAX_SPEED_y=Adjust_Speed_Computer(Y_drag,sprite[1].Ypos);
Computer_Rotate=Update_Computer_Direction(); 
if ((sprite[2].Ypos+1)>(sprite[4].Ypos+5)) {Goal_Adj(1,4);}
if ((sprite[2].Ypos+1)<(sprite[4].Ypos+5)) {Goal_Adj(0,4);} 
Adjust_Cumule_Speed(1);
collision_check_LimiteScreen(1);
if (collision_check_Sprite2Sprite(0,1)) {BOUNCE_SPRITE();SND(2);}
Speed_Puck_Decel(2);
Adjust_Cumule_Speed(2);
Drag_Puck_test();
Drag_Puck_Update();
Tiny_Flip();
collision_check_LimiteScreen(2);
  if ((Drag_Puck==1)&&(sprite[1].Xpos<48)&&((Computer_Rotate>=3)&&(Computer_Rotate<=5))) {if ((sprite[2].Xpos>8)) {KICK_PUCK(Computer_Rotate);}}
  if (GOAL_!=255) {
   GOAL_=255;
   SND(1); 
   Drag_Puck=255;
   if (Points_CPU==10) {intro(3,0);SND(6);_delay_ms(1000);_delay_ms(1000);goto INTRO;}
   else if (Points_Pl1==10) {intro(2,0);SND(5);_delay_ms(1000);_delay_ms(1000);goto INTRO;}
   else{SCREEN_GOAL();} 
   goto RESTART;
    }
SKIP_FRAME=!SKIP_FRAME;
PICOKIT.FPS_Temper();
}}
////////////////////////////////// main end /////////////////////////////////

uint8_t check_Puck2Gard(uint8_t Gard){
#define X1 (sprite[2].Xpos)
#define Y1 (sprite[2].Ypos)
#define D1 2
#define X2 ((sprite[Gard].Xpos)+1)
#define Y2 ((sprite[Gard].Ypos)+1)
#define D2x 3 
#define D2y 7 
if ((X1>(X2+D2x))||((X1+D1)<X2)||(Y1>(Y2+D2y))||((Y1+D1)<Y2)) {return 0;}
return 1;
}

uint8_t GOAL(uint8_t Goal_0,uint8_t Goal_1){
#define Xt1 (sprite[2].Xpos)
if (Drag_Puck==255){
if (Xt1<64) {
if ((Xt1+2)<=(sprite[3].Xpos+1)&&(Goal_0==0)) {GOAL_=1;Points_CPU++;} }else{
if (((Xt1)>=((sprite[4].Xpos+5)-1))&&(Goal_1==0)) {GOAL_=0;Points_Pl1++;} }
}
return 1;
}

void Reverse_x_puck_2_pos(uint8_t SpriteTested){
 
  if (sprite[SpriteTested].SpeedXpositif==0) {
    Reverse_x_pla(SpriteTested);
}
}

void Reverse_x_puck_2_neg(uint8_t SpriteTested){
 if (sprite[SpriteTested].SpeedXnegatif==0) {
 Reverse_x_pla(SpriteTested);
}}

uint8_t collision_check_LimiteScreen(uint8_t SpriteTested){
uint8_t Gar0=0,Gar1=0;
#define XX sprite[SpriteTested].Xpos
#define YY sprite[SpriteTested].Ypos
if (SpriteTested==2) {
if (check_Puck2Gard(3)) {Gar0=1;Reverse_x_puck_2_pos(2);}
if (check_Puck2Gard(4)) {Gar1=1;Reverse_x_puck_2_neg(2);}
if ((YY>=23)&&(YY<=38)) {if (GOAL(Gar0,Gar1)) {return 0;}}
}
#define DIM (PLAYER_DIM[SpriteTested]*2)
if (XX<=4) {XX+=2;goto X_X;}
if ((XX+DIM)>=122) {XX-=2;goto X_X;}
if (YY<=4) {YY+=2;goto Y_Y;}
if ((YY+DIM)>=56) {YY-=2;goto Y_Y;}
return 0;
Y_Y:;
Reverse_y_pla(SpriteTested);
SND(3);
return 1;
X_X:;
Reverse_x_pla(SpriteTested);
SND(3);
return 1;
}

void KICK_PUCK(uint8_t Player_Rotate){
Drag_Puck=255;
uint8_t DXN=0;
uint8_t DYN=0;
uint8_t DXP=0;
uint8_t DYP=0;
#define SPD 46
switch(Player_Rotate){
  case(0):DXP=SPD;break;
  case(1):DXP=SPD;DYP=SPD;break;
  case(2):DYP=SPD;break;
  case(3):DYP=SPD;DXN=SPD;break;
  case(4):DXN=SPD;break;
  case(5):DXN=SPD;DYN=SPD;break;
  case(6):DYN=SPD;break;
  case(7):DXP=SPD;DYN=SPD;break;
  default:break;
}
sprite[2].SpeedXnegatif=DXN;
sprite[2].SpeedXpositif=DXP; 
sprite[2].SpeedYnegatif=DYN;
sprite[2].SpeedYpositif=DYP;
SND(4);
}

void Drag_Puck_Update(void){
uint8_t Rot;
if (Drag_Puck!=255) {  
Rot=(Drag_Puck==0)?Player1_Rotate:Computer_Rotate;
Puck_x=sprite[Drag_Puck].Xpos;
Puck_y=sprite[Drag_Puck].Ypos; 
sprite[2].Xpos=Puck_x+PLAYER_Puck[(Rot*2)];
sprite[2].Ypos=Puck_y+PLAYER_Puck[(Rot*2)+1];
}else{
Puck_x=sprite[2].Xpos;
Puck_y=sprite[2].Ypos;
}}

void Drag_Puck_test(void){
if (SKIP_FRAME==0) {
if (collision_check_Sprite2Sprite(2,0)) {
 Drag_Puck=0; 
  }}else{  
if (collision_check_Sprite2Sprite(2,1)) {
 Drag_Puck=1; 
  }} 
}

void Speed_Puck_Decel(uint8_t SPR_TYPE){
if (sprite[SPR_TYPE].SpeedXpositif>0) {sprite[SPR_TYPE].SpeedXpositif-=1;}
if (sprite[SPR_TYPE].SpeedXnegatif>0) {sprite[SPR_TYPE].SpeedXnegatif-=1;}
if (sprite[SPR_TYPE].SpeedYpositif>0) {sprite[SPR_TYPE].SpeedYpositif-=1;}
if (sprite[SPR_TYPE].SpeedYnegatif>0) {sprite[SPR_TYPE].SpeedYnegatif-=1;}
}

void BOUNCE_SPRITE(void){
if (sprite[0].Xpos<sprite[1].Xpos) {
if (sprite[0].SpeedXnegatif==0) {
Reverse_x_pla(0);
}
if (sprite[1].SpeedXpositif==0) {
Reverse_x_pla(1);
}}else{
if (sprite[0].SpeedXpositif==0) {
Reverse_x_pla(0);
}
if (sprite[1].SpeedXnegatif==0) {
Reverse_x_pla(1);
}}
if (sprite[0].Ypos<sprite[1].Ypos) {
if (sprite[0].SpeedYnegatif==0) {
Reverse_y_pla(0);
}
if (sprite[1].SpeedYpositif==0) {
Reverse_y_pla(1);
}
}else{
  if (sprite[0].SpeedYpositif==0) {
Reverse_y_pla(0);
}
if (sprite[1].SpeedYnegatif==0) {
Reverse_y_pla(1);
}}
}

void Reverse_y_pla(uint8_t SpriteTested){
uint8_t temp=sprite[SpriteTested].SpeedYnegatif;
sprite[SpriteTested].SpeedYnegatif=sprite[SpriteTested].SpeedYpositif;
sprite[SpriteTested].SpeedYpositif=temp; 
}

void Reverse_x_pla(uint8_t SpriteTested){
uint8_t temp=sprite[SpriteTested].SpeedXnegatif;
sprite[SpriteTested].SpeedXnegatif=sprite[SpriteTested].SpeedXpositif;
sprite[SpriteTested].SpeedXpositif=temp; 
}

uint8_t collision_check_Sprite2Sprite(uint8_t Sp_1,uint8_t Sp_2){
uint8_t EXCEP=(Sp_1==2)?0:5;
uint8_t EXCEP2=(Sp_1==2)?2:1;
#define XX_ (sprite[Sp_1].Xpos+EXCEP)
#define YY_ (sprite[Sp_1].Ypos+EXCEP)
#define DIM_ (PLAYER_DIM[Sp_1]*EXCEP2)
#define XX2_ (sprite[Sp_2].Xpos+EXCEP)
#define YY2_ (sprite[Sp_2].Ypos+EXCEP)
#define DIM2_ (PLAYER_DIM[Sp_2]*EXCEP2)  
if (XX_>(XX2_+DIM2_)) {return 0;}
if ((XX_+DIM_)<XX2_) {return 0;}
if (YY_>(YY2_+DIM2_)) {return 0;}
if ((YY_+DIM_)<YY2_) {return 0;}
return 1;
}

uint8_t Computer_rotation_filter_update(void){
uint8_t Direct_x=0;
uint8_t Direct_y=0;
uint8_t Mem_X_HIGH=0;
uint8_t Mem_Y_HIGH=0;
#define XP sprite[1].SpeedXpositif
#define XN sprite[1].SpeedXnegatif
#define YP sprite[1].SpeedYpositif
#define YN sprite[1].SpeedYnegatif
if (XP>XN) {Mem_X_HIGH=XP;Direct_x=4;}else{Direct_x=16;Mem_X_HIGH=XN;}
if (YP>YN) {Mem_Y_HIGH=YP;Direct_y=8;}else{Direct_y=2;Mem_Y_HIGH=YN;}
if (Mem_X_HIGH>Mem_Y_HIGH) {
 if ((Mem_X_HIGH-Mem_Y_HIGH)<14) {return Direction_Recupe(Direct_x+Direct_y);}else{return Direction_Recupe(Direct_x);}
  }else{
 if ((Mem_Y_HIGH-Mem_X_HIGH)<14) {return Direction_Recupe(Direct_x+Direct_y);}else{return Direction_Recupe(Direct_y);}
  }
}

uint8_t Direction_Recupe(uint8_t Con){
switch(Con){
  case(4):return 0;break;
  case(12):return 1;break;
  case(8):return 2;break;
  case(24):return 3;break;
  case(16):return 4;break;
  case(18):return 5;break;
  case(2):return 6;break;
  case(6):return 7;break;
  default:return Player1_Rotate;break;
}
return 0;
}

uint8_t Update_Computer_Direction(void){
#define W_PL2 PLAYER_DIM[1]
if (Drag_Puck==1) {
X_Drag=0;
Y_drag=30;
}else{
X_Drag=Puck_x;
Y_drag=Puck_y;}
if ((sprite[1].Xpos+W_PL2)<X_Drag) {Speed_adj_H_P(1);}else{Speed_adj_H_N(1);}
if ((sprite[1].Ypos+W_PL2)<Y_drag) {Speed_adj_V_P(1);}else{Speed_adj_V_N(1);}
return Computer_rotation_filter_update();
}

void Speed_H_Decel(uint8_t SPR_TYPE){
if (sprite[SPR_TYPE].SpeedXpositif>0) {sprite[SPR_TYPE].SpeedXpositif-=2;}
if (sprite[SPR_TYPE].SpeedXnegatif>0) {sprite[SPR_TYPE].SpeedXnegatif-=2;}
}

void Speed_V_Decel(uint8_t SPR_TYPE){
if (sprite[SPR_TYPE].SpeedYpositif>0) {sprite[SPR_TYPE].SpeedYpositif-=2;}
if (sprite[SPR_TYPE].SpeedYnegatif>0) {sprite[SPR_TYPE].SpeedYnegatif-=2;}
}

uint8_t Adjust_Speed_Computer(uint8_t Puck,uint8_t Val_Computer_2){
if (Val_Computer_2>Puck)  {
return ((Val_Computer_2-Puck)>10)?28:16;   
}else{return ((Puck-Val_Computer_2)>10)?28:16;     
}}

void Adjust_Cumule_Speed(uint8_t SPRITE_NO){
uint8_t MAX_SPEED=3;
if (SPRITE_NO==2) {MAX_SPEED=5;}
for(uint8_t t=0;t<MAX_SPEED;t++){
if (sprite[SPRITE_NO].SpeedXpositif>0) {
  if (t==0) {sprite[SPRITE_NO].CumuleX=sprite[SPRITE_NO].CumuleX+sprite[SPRITE_NO].SpeedXpositif;}
  if (sprite[SPRITE_NO].CumuleX>=10) {sprite[SPRITE_NO].CumuleX=sprite[SPRITE_NO].CumuleX-10;if (sprite[SPRITE_NO].Xpos<126) {sprite[SPRITE_NO].Xpos++;}}
}else if (sprite[SPRITE_NO].SpeedXnegatif>0) {
    if (t==0) {sprite[SPRITE_NO].CumuleX=sprite[SPRITE_NO].CumuleX+sprite[SPRITE_NO].SpeedXnegatif;}
  if (sprite[SPRITE_NO].CumuleX>=10) {sprite[SPRITE_NO].CumuleX=sprite[SPRITE_NO].CumuleX-10;if (sprite[SPRITE_NO].Xpos>0) {sprite[SPRITE_NO].Xpos--;}}
}else{sprite[SPRITE_NO].CumuleX=0;}
if (sprite[SPRITE_NO].SpeedYpositif>0) {
  if (t==0) {sprite[SPRITE_NO].CumuleY=sprite[SPRITE_NO].CumuleY+sprite[SPRITE_NO].SpeedYpositif;}
  if (sprite[SPRITE_NO].CumuleY>=10) {sprite[SPRITE_NO].CumuleY=sprite[SPRITE_NO].CumuleY-10;if (sprite[SPRITE_NO].Ypos<61) {sprite[SPRITE_NO].Ypos++;}}
}else if (sprite[SPRITE_NO].SpeedYnegatif>0) {
if (t==0) {sprite[SPRITE_NO].CumuleY=sprite[SPRITE_NO].CumuleY+sprite[SPRITE_NO].SpeedYnegatif;}
  if (sprite[SPRITE_NO].CumuleY>=10) {sprite[SPRITE_NO].CumuleY=sprite[SPRITE_NO].CumuleY-10;if (sprite[SPRITE_NO].Ypos>0) {sprite[SPRITE_NO].Ypos--;}}
}else{sprite[SPRITE_NO].CumuleY=0;}
}}

void Speed_adj_H_P(uint8_t SPRITE_NO){
if (sprite[SPRITE_NO].SpeedXnegatif==0) {if (sprite[SPRITE_NO].SpeedXpositif<COM_MAX_SPEED_x) {sprite[SPRITE_NO].SpeedXpositif=sprite[SPRITE_NO].SpeedXpositif+2;}}else{
sprite[SPRITE_NO].SpeedXnegatif=sprite[SPRITE_NO].SpeedXnegatif-2;  
}}

void Speed_adj_H_N(uint8_t SPRITE_NO){
if (sprite[SPRITE_NO].SpeedXpositif==0) {if (sprite[SPRITE_NO].SpeedXnegatif<COM_MAX_SPEED_x) {sprite[SPRITE_NO].SpeedXnegatif=sprite[SPRITE_NO].SpeedXnegatif+2;}}else{
sprite[SPRITE_NO].SpeedXpositif=sprite[SPRITE_NO].SpeedXpositif-2;  
}}

void Speed_adj_V_P(uint8_t SPRITE_NO){
if (sprite[SPRITE_NO].SpeedYnegatif==0) {if (sprite[SPRITE_NO].SpeedYpositif<COM_MAX_SPEED_y) {sprite[SPRITE_NO].SpeedYpositif=sprite[SPRITE_NO].SpeedYpositif+2;}}else{
sprite[SPRITE_NO].SpeedYnegatif=sprite[SPRITE_NO].SpeedYnegatif-2;  
}}

void Speed_adj_V_N(uint8_t SPRITE_NO){
if (sprite[SPRITE_NO].SpeedYpositif==0) {if (sprite[SPRITE_NO].SpeedYnegatif<COM_MAX_SPEED_y) {sprite[SPRITE_NO].SpeedYnegatif=sprite[SPRITE_NO].SpeedYnegatif+2;}}else{
sprite[SPRITE_NO].SpeedYpositif=sprite[SPRITE_NO].SpeedYpositif-2;  
}}

uint8_t blitzSprite(int8_t xPos,int8_t yPos,uint8_t xPASS,uint8_t yPASS,uint8_t FRAME,const uint8_t *SPRITES){
uint8_t OUTBYTE;
uint8_t WSPRITE=(SPRITES[0]);
uint8_t HSPRITE=(SPRITES[1]);
uint8_t Wmax ((HSPRITE*WSPRITE)+1);
uint16_t PICBYTE=FRAME*(Wmax-1);
uint8_t RECUPELINEY=RecupeLineY(yPos);
if ((xPASS>((xPos+(WSPRITE-1))))||(xPASS<xPos)||((RECUPELINEY>yPASS)||((RECUPELINEY+(HSPRITE))<yPASS))) {return 0x00;}
uint8_t SPRITEyLINE=(yPASS-(RECUPELINEY));
uint8_t SPRITEyDECALAGE=(RecupeDecalageY(yPos));
uint8_t ScanA=(((xPASS-xPos)+(SPRITEyLINE*WSPRITE))+2);
uint8_t ScanB=(((xPASS-xPos)+((SPRITEyLINE-1)*WSPRITE))+2);
if (ScanA>Wmax) {OUTBYTE=0x00;}else{OUTBYTE=SplitSpriteDecalageY(SPRITEyDECALAGE,SPRITES[ScanA+(PICBYTE)],1);}
if ((SPRITEyLINE>0)) {
uint8_t OUTBYTE2=SplitSpriteDecalageY(SPRITEyDECALAGE,SPRITES[ScanB+(PICBYTE)],0);
if (ScanB>Wmax) {return OUTBYTE;}else{return OUTBYTE|OUTBYTE2;}
}else{return OUTBYTE;}
}

uint8_t Recupe(uint8_t xPASS,uint8_t yPASS){
return (Patinoire1_2(xPASS,yPASS));
}

uint8_t Patinoire1_2(uint8_t xPASS,uint8_t yPASS){
if (xPASS>63) {return patinoire1_2[(127-xPASS)+(yPASS*64)];}
return patinoire1_2[(xPASS)+(yPASS*64)]; 
}

uint8_t SplitSpriteDecalageY(uint8_t decalage,uint8_t Input,uint8_t UPorDOWN){
if (UPorDOWN) {return Input<<decalage;}
return Input>>(8-decalage);
}

uint8_t RecupeLineY(uint8_t Valeur){
return (Valeur>>3); 
}

uint8_t RecupeDecalageY(uint8_t Valeur){
return (Valeur-((Valeur>>3)*8));
}

void Tiny_Flip(void){
	 #define XP1 sprite[0].Xpos   
 #define YP1 sprite[0].Ypos 
 #define XP2 sprite[1].Xpos 
 #define YP2 sprite[1].Ypos
 #define XP3 sprite[2].Xpos   
 #define YP3 sprite[2].Ypos 
 #define XP4 sprite[3].Xpos 
 #define YP4 sprite[3].Ypos
 #define XP5 sprite[4].Xpos 
 #define YP5 sprite[4].Ypos
uint8_t y,x; 
for (y = 0; y < 8; y++){ 
for (x = 0; x < 128; x++){PICOKIT.TinyOLED_Send(x,y,Recupe(x,y));}

}
PICOKIT.DrawSSDBuffer();
for (y = 0; y < 8; y++){ 

for (x = 0; x < 128; x++){
PICOKIT.DirectDraw(blitzSprite(XP1,YP1,x,y,Player1_Rotate,PLAYER_WHITE_B),x,y,Ply);
PICOKIT.DirectDraw(blitzSprite(XP2,YP2,x,y,Computer_Rotate,PLAYER_BLACK_B) ,x,y,Cpu);
PICOKIT.DirectDraw(blitzSprite(XP3,YP3,x,y,0,puck) ,x,y,color(31,63,0));

}}

for (y = 2; y < 6; y++){ 
for (x = 117; x < 124; x++){
	PICOKIT.DirectDraw(blitzSprite(XP5,YP5,x,y,0,GOALER_BLACK) ,x,y,Cpu);
}}

for (y = 2; y < 6; y++){ 
for (x = 4; x < 9; x++){
	PICOKIT.DirectDraw(blitzSprite(XP4,YP4,x,y,0,GOALER_WHITE) ,x,y,Ply);
}}

PICOKIT.DisplayColor();
}

void intro(uint8_t GOAL_SCREEN,uint8_t SLIDE){

uint8_t y,x; 
for (y = 0; y < 8; y++){ 
for (x = 0; x < 128; x++){
switch(GOAL_SCREEN){
  case 0:PICOKIT.TinyOLED_Send(x,y,(Intro[(x)+(y*128)]|blitzSprite(40,39,x,y,0,cadre40_37))); break;
  case 1:PICOKIT.TinyOLED_Send(x,y, ((Recupe(x,y)|(blitzSprite(40,39+SLIDE,x,y,0,screen)))&(0xff-blitzSprite(40,39+SLIDE,x,y,1,screen)))& ~(
blitzSprite(XP1,YP1,x,y,Player1_Rotate,PLAYER_WHITE_B)|
blitzSprite(XP2,YP2,x,y,Computer_Rotate,PLAYER_BLACK_B) |
blitzSprite(XP5,YP5,x,y,0,GOALER_BLACK)|
blitzSprite(XP4,YP4,x,y,0,GOALER_WHITE)
) 
 );
break;
  case 2:PICOKIT.TinyOLED_Send(x,y,Lost_win(x,y,win));break;
  case 3:PICOKIT.TinyOLED_Send(x,y,Lost_win(x,y,lost));break;
  case 4:PICOKIT.TinyOLED_Send(x,y,Intro[(x)+(y*128)]);break;
  default:break;
}}}

PICOKIT.DrawSSDBuffer();
//color

for (y = 0; y < 8; y++){ 
for (x = 0; x < 128; x++){
switch(GOAL_SCREEN){
  case 1:
  PICOKIT.DirectDraw( blitzSprite(40+(13),39+SLIDE+(13),x,y,Points_Pl1,NUM),x,y,Ply);
  PICOKIT.DirectDraw( blitzSprite(40+(33),39+SLIDE+(13),x,y,Points_CPU,NUM),x,y,Cpu);break;
  default:break;
}}}

PICOKIT.DisplayColor();//PICOKIT.DrawSSDBuffer();
}

uint8_t Lost_win(uint8_t xPASS,uint8_t yPASS,const uint8_t *SPRITES ){
return (((Recupe(xPASS,yPASS)|(blitzSprite(48,27,xPASS,yPASS,0,SPRITES))))&(0xff-blitzSprite(48,27,xPASS,yPASS,1,SPRITES)));
}

void SCREEN_GOAL(void){
uint8_t t;
for (t=6;t>0;t--){
intro(1,t*4);}
_delay_ms(900);
for (t=1;t<8;t++){
intro(1,t*4);}
_delay_ms(400);
}


