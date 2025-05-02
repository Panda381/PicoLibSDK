//   >>>>> T-I-N-Y  I-N-V-A-D-E-R-S for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2018-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  tiny-Invaders is free software: you can redistribute it and/or modify
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


// The tiny-Invaders source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "invaders.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the invaders.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK


#ifndef PICOKIT_H
#define PICOKIT_H

//#include "../include.h"

#define BLACK 0
#define WHITE 1

#define LOW 0
#define HIGH 1
#define uint8_t u8
#define int8 s8
#define uint16_t u16
#define int16_t s16

#define TINYJOYPAD_LEFT   (KeyPressed(KEY_LEFT))
#define TINYJOYPAD_RIGHT (KeyPressed(KEY_RIGHT))
#define TINYJOYPAD_DOWN (KeyPressed(KEY_DOWN))
#define TINYJOYPAD_UP  (KeyPressed(KEY_UP))
#define BUTTON_DOWN ((KeyPressed(KEY_A))||(KeyPressed(KEY_B)))
#define BUTTON_UP ((!KeyPressed(KEY_A))&&(!KeyPressed(KEY_B)))

#define delay(Var_0) WaitMs(Var_0)
#define _delay_ms(Var_1) WaitMs(Var_1)
#define _delay_us(Var_2) WaitUs(Var_2)

#define My_delay_ms(Var_3) WaitMs(Var_3)
#define My_delay_us(Var_4) WaitUs(Var_4)

class PICOCOMPATIBILITY {
private:
    // Variables membres
    uint8_t T;
    uint8_t Buffer[1024];
    uint8_t FPS_Break;
    unsigned long curtMil = 0;
    unsigned long MemMil = 0;

    // Méthodes privées
    uint8_t Select_Byte(uint8_t y_);
    int8_t ESP_RecupeLineY(int8_t Valeur);
    int8_t ESP_RecupeDecalageY(uint8_t Valeur);
    uint8_t ESP_SplitSpriteDecalageY(uint8_t decalage, uint8_t Input, uint8_t UPorDOWN);
    uint8_t ESP_blitzSprite(int8_t xPos, int8_t yPos, int8_t HSPRITE_, uint8_t xPASS, uint8_t yPASS, uint8_t FRAME, const uint8_t *SPRITES);

public:

    // Méthodes publiques
    void TinyOLED_Send(uint8_t x_,uint8_t y_,uint8_t Byte_);
    void drawPixel(int16_t x_, int16_t y_, uint8_t Col_);
    uint8_t getPixel(int16_t x_, int16_t y_);
    uint8_t *getBuffer(void);
    uint8_t RBuffer(uint16_t pos_);
    void sBuffer(uint16_t pos_, uint8_t Val_);
    void SetFPS(uint8_t Val_);
    void FPS_Temper(void);
    void Init(void);
    void clear(void);
    void Begin(void);
    void display(void);
    void fillScreen(uint8_t t);
    void ClearBuffer(void);
    uint8_t nextFrame(void);
    uint8_t pressed(uint8_t);
    void drawSelfMasked(int8_t x_, int8_t y_, uint8_t const *PRGM_, uint8_t F_);
    void drawSelfMasked_OverScan(int8_t x_, int8_t y_, uint8_t const *PRGM_, uint8_t F_);
    void drawErase(int8_t x_, int8_t y_, uint8_t const *PRGM_, uint8_t F_);
    void drawInvertPixel(int8_t x_, int8_t y_, uint8_t const *PRGM_, uint8_t F_);
    void drawOverwrite(int8_t x_, int8_t y_, uint8_t const *PRGM_, uint8_t F_);
    uint8_t BlackSquare(int8_t y, uint8_t yPASS, const uint8_t *PRGM_);
    uint8_t ExploreMap(uint8_t x_, uint8_t y_);
    void DrawBuffer(void);
    void DirectDraw(uint8_t Byte_,int16_t XPASS,int16_t YPASS,uint16_t Col_);
    void transferBufferToColorDisplay_320x240() ;
    void clearPicoBuffer(void);
};

uint16_t MainBufferColor(uint8_t add2_);
void SoundSet(void);
uint32_t pico_millis(void);
void DigitalControl(uint8_t gpio_pin, bool state);
uint8_t Mymap(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max);
uint16_t color(uint8_t R, uint8_t G, uint8_t B);
void Sound(uint8_t freq_, uint8_t dur);
void MEGA_PLAY_MUSIC(const uint8_t *Music_);

#endif // PICOKIT_H

