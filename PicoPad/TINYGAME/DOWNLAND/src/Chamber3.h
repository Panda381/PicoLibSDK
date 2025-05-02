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


#ifndef _CHAMBER3_H
#define _CHAMBER3_H

//#include "../include.h"
//#include "Chamber2.h"

const int8_t RopeVec3[]  = {
VERTICAL,24,37,19,
VERTICAL,55,19,37,
VERTICAL,79,19,37,
VERTICAL,67,19,11,
EXIT};

const int8_t Vec3[]  = {
POS,0,5,
VEC,2,3,
ROOF,94,3,
VEC,96,6,
VEC,96,13,
FLOOR,0,13,
VEC,0,5,

POS,0,21,
VEC,3,19,
ROOF,34,19,
VEC,36,23,
WALL,36,29,
VEC,20,29,
VEC,20,32,
VEC,23,35,
ROOF,34,36,
VEC,36,38,
WALL,36,53,
VEC,39,56,
VEC,42,53,


WALL,42,26,
VEC,44,19,
ROOF,90,19,
VEC,96,24,
VEC,96,30,

FLOOR,90,30,
VEC,91,31,
VEC,92,33,
VEC,96,36,
VEC,96,38,
VEC,95,38,
VEC,95,39,
VEC,96,40,
VEC,96,46,

FLOOR,90,46,
VEC,91,47,
VEC,92,49,
VEC,96,51,
VEC,96,55,
VEC,95,55,
VEC,95,56,
VEC,96,57,
VEC,96,63,

FLOOR,71,63,
VEC,71,62,
WALL,71,46,
VEC,68,47,
VEC,64,50,
WALL,64,63,

FLOOR,0,63,
VEC,0,57,
VEC,3,53,
VEC,10,50,
VEC,11,48,
VEC,12,46,
FLOOR,0,46,
VEC,0,39,
VEC,3,35,
VEC,6,33,
VEC,9,35,
VEC,12,33,
WALL,12,29,
FLOOR,0,29,
VEC,0,21,
EXIT
};

const int8_t SPK3[]  = {
GOLD2,64,34,9,
GOLD2,88,34,10,
GOLD,5,52,11,

KEY,28,22,8,
KEY,88,17,5,

DOOR,-7,6,107,
DOOR,-7,22,108,
DOOR,-7,39,109,

DOOR,98,6,110,
DOOR,98,23,5,
DOOR,98,39,8,
DOOR,98,56,111,

EXIT
};

const int8_t AcidDrop3[]  = {
24,
6,3,
12,3,
24,3,
6,18,
20,20,
30,18,
52,19,
6,35,
21,35,
27,36,
8,53,
35,55,
42,55,
58,19,
64,19,
70,19,
76,19,
82,19,
90,18,
90,32,
90,48,
91,4,
81,4,
61,4,
};

#endif
