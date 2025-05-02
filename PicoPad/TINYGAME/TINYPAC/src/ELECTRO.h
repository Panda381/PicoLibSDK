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


#ifndef _ELECTRO_H
#define _ELECTRO_H

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

void SoundSet(void);
uint8_t Mymap(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max);
void DrawBuffer(void);
uint8_t ExploreMap(uint8_t x_, uint8_t y_) ;
uint8_t Select_Byte(uint8_t y_) ;
void DirectDraw(uint8_t Byte_,int16_t XPASS,int16_t YPASS,uint16_t Col_);
uint16_t ColorBuf(uint8_t x_, uint8_t y_);
uint16_t color(uint8_t R, uint8_t G, uint8_t B);
uint32_t pico_millis();
void Sound(uint8_t freq_, uint8_t dur);
void DigitalControl(uint8_t gpio_pin, bool state);

#endif 

