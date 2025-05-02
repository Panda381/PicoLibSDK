//  >>>>> GILBERT IN THE DOWNLAND for PicoPad RP2040 & RP2350+ GPL v3 <<<<<
//                    Programmer: Daniel C 2025
//               Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                      https://WWW.TINYJOYPAD.COM
//           https://sites.google.com/view/arduino-collection

//  Gilbert in the Downland is free software: you can redistribute it and/or modify
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


// The Gilbert in the Downland source code include commands referencing to librairy 
// PicoLibSDK who is not include in the source code.

// A uf2 file "downland.uf2" is provided with the source code which includes
// compiled code from the PicoLibSDK library.
// Reference in the file "PicoLibSDK_Readme.txt".

// Please note that the downland.uf2 file is a compiled version with code from
// the PicoLibSDK library pack by Miroslav Nemecek. Thank you to him!
// https://github.com/Panda381/PicoLibSDK

#ifndef _CHAMBER2_H
#define _CHAMBER2_H

//#include "../include.h"
//#include "Chamber1.h"

const int8_t RopeVec2[]  = {
VERTICAL,40,38,17,
VERTICAL,12,18,22,
VERTICAL,69,40,3,

VERTICAL,79,3,8,
VERTICAL,67,3,19,

VERTICAL,45,19,3,
VERTICAL,26,4,3,
EXIT};

const int8_t Vec2[]  = {
POS,0,5,
VEC,2,3,
ROOF,94,3,
VEC,96,6,
VEC,96,13,
VEC,90,13,
VEC,92,15,
VEC,96,18,
WALL,96,24,
VEC,96,29,
FLOOR,38,29,
POS,0,5,
VEC,0,13,
VEC,56,13,
VEC,50,15,
VEC,50,16,
VEC,46,18,
VEC,42,17,
VEC,38,19,
WALL,38,29,

POS,0,20,
VEC,3,18,
ROOF,28,18,
VEC,30,20,
WALL,30,29,
VEC,22,29,
VEC,24,31,
VEC,27,39,

ROOF,90,39,
VEC,96,41,
VEC,96,48,
VEC,80,48,
VEC,80,53,

VEC,64,53,
VEC,64,58,

VEC,48,58,
VEC,48,63,

VEC,0,63,

POS,0,20,
WALL,0,39,
VEC,0,48,
FLOOR,33,48,
WALL,33,51,
POS,0,54,
ROOF,31,54,
VEC,33,51,
POS,0,54,
VEC,0,63,
EXIT
};


const int8_t SPK2[]  = {
GOLD2,43,40,6,
GOLD,77,35,7,
GOLD2,40,21,8,

KEY,22,18,3,
KEY,76,17,7,

DOOR,-7,6,6,
DOOR,-7,41,3,
DOOR,-7,56,104,
DOOR,98,6,105,
DOOR,98,22,7,
DOOR,98,41,106,

BALLONSPK,77,36,0,
EXIT
};

const int8_t AcidDrop2[]  = {
23,
6,3,
15,4,
23,2,
29,2,
45,4,
52,3,

64,3,
70,3,
76,3,
82,3,

90,3,
42,19,
48,19,
9,20,
15,20,
28,40,
37,39,
43,39,
57,41,
66,38,
72,38,
14,54,
33,53
};

#endif
