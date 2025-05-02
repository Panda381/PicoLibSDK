//     >>>>>  T-I-N-Y  T-R-I-S v2.1 for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                     Programmer: Daniel C 2019-2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Tiny-tris is free software: you can redistribute it and/or modify
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


// The Tiny-tris source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "TinyTris.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "ReadMe.txt".

#include "../include.h"

uint8_t SnD_;
uint8_t BUFFER_SSD[1024]={0xff};
		
uint8_t ExploreMap(uint8_t x_, uint8_t y_){
  uint8_t MaxX = 128;
  uint8_t ByteNo = Select_Byte(y_);
  uint8_t ByTe = (BUFFER_SSD[(x_ + (ByteNo * MaxX))]);
  return ((ByTe & (1 << (y_ - (ByteNo << 3)))) == 0) ? 0 : 1;
}

uint8_t Select_Byte(uint8_t y_) {
  return (y_ >> 3);
}

uint8_t Mymap(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint16_t color(uint8_t R, uint8_t G, uint8_t B) {
    // Décaler R de 11 bits (puisque R occupe les bits 11 à 15)
    // Décaler G de 5 bits (puisque G occupe les bits 5 à 10)
    // B occupe les bits 0 à 4, donc pas de décalage nécessaire
    return ((R & 0x1F) << 11) | ((G & 0x3F) << 5) | (B & 0x1F);
}

uint16_t ColorBuf(uint8_t x_, uint8_t y_) {
    // Vérifie si y_ est 63 ou si x_ est à l'extérieur de l'intervalle [46, 82]
    if (y_ == 63 || x_ < 46 || x_ > 82) {
        return color(0, 255 - y_, 255);
    }
    return color(32, 255, 255);
}

void DrawBuffer(void) {
    #define Off_setX 32
    #define Off_setY 48
    // Dimension des buffers
    const uint16_t WIDTH_MOD = WIDTH; // Eviter la répétition
    uint8_t y, x;

    for (y = 0; y < 64; y++) {
        for (x = 0; x < 128; x++) {
            uint16_t Pix_ = (ExploreMap(x, y)) ? ColorBuf(x, y) : 0;
            uint16_t x2 = (x * 2) + Off_setX;
            uint16_t y2 = (y * 2) + Off_setY;

            // Utilisé un seul accès à FrameBuf pour mettre à jour les pixels
            FrameBuf[x2 + (y2 * WIDTH_MOD)] = Pix_;
            FrameBuf[(x2 + 1) + (y2 * WIDTH_MOD)] = Pix_;
            FrameBuf[x2 + ((y2 + 1) * WIDTH_MOD)] = Pix_;
            FrameBuf[(x2 + 1) + ((y2 + 1) * WIDTH_MOD)] = Pix_;
        }
    }
}

uint32_t pico_millis() {
    // Obtenez le nombre de millisecondes depuis le démarrage de la Pico
    return to_ms_since_boot(get_absolute_time());
}

void DigitalControl(uint8_t gpio_pin, bool state) {
    // Initialiser le GPIO
    gpio_init(gpio_pin);
    // Définir le GPIO comme sortie
    gpio_set_dir(gpio_pin, GPIO_OUT);
    // Mettre le GPIO à l'état spécifié (true pour ON, false pour OFF)
    gpio_put(gpio_pin, state);
}

void SoundSet(void){
	if (Config.volume>0) {SnD_=true;}else{SnD_=false;}
}

void Sound(uint8_t freq_, uint8_t dur){
uint8_t Invert_=(255-freq_);
uint8_t H_L=(freq_!=0)?SnD_:0;
uint8_t HiDur=Mymap(Config.volume,0,254,0,Invert_);
uint16_t LoDur=Invert_+(Invert_-HiDur);

for (uint8_t t=0;t<dur;t++){
  DigitalControl(PWMSND_GPIO,H_L);
for (uint8_t t=0;t<HiDur;t++){
_delay_us(1);}
    DigitalControl(PWMSND_GPIO, false);
for (uint16_t t=0;t<LoDur;t++){
_delay_us(1); }}
    DigitalControl(PWMSND_GPIO, false);
}

