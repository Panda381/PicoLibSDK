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

#ifndef _CHAMBER1_H
#define _CHAMBER1_H

//#include "../include.h"
//#include "Chamber0.h"

const int8_t RopeVec1[]  = {
VERTICAL,85,36,19,
VERTICAL,9,18,26,
VERTICAL,39,18,36,
VERTICAL,66,21,6,
VERTICAL,80,5,6,
EXIT};


const int8_t Vec1[]  = {
POS,0,5,
VEC,6,3,
ROOF,42,3,
VEC,50,6,
WALL,50,13,
POS,50,13,
FLOOR,0,13,
VEC,0,5,

POS,58,5,
VEC,62,3,
ROOF,90,3,
VEC,96,6,
VEC,96,13,

VEC,90,13,
VEC,92,15,
VEC,96,18,
WALL,96,24,
VEC,96,29,
FLOOR,78,29,

VEC,80,31,
VEC,83,33,
VEC,85,35,
ROOF,92,35,
VEC,96,39,
WALL,96,63,
FLOOR,29,63,

POS,20,29,
FLOOR,29,29,
WALL,29,63,

POS,20,29,
WALL,20,57,
FLOOR,8,57,

POS,0,20,
WALL,0,44,
VEC,0,52,
FLOOR,7,52,
WALL,7,56,
VEC,7,57,

POS,0,20,
ROOF,44,20,
VEC,50,22,
WALL,50,47,
VEC,55,52,
ROOF,71,52,

POS,58,6,
WALL,58,13,
VEC,70,13,
WALL,70,19,
VEC,64,20,
VEC,60,19,
VEC,58,21,
WALL,58,33,
FLOOR,63,33,
WALL,63,36,
VEC,63,38,
FLOOR,68,38,
WALL,68,42,
VEC,68,43,
FLOOR,73,43,
VEC,73,45,
VEC,69,50,
EXIT
};

const int8_t SPK1[]  = {
GOLD2,45,48,3,
GOLD2,71,48,4,
GOLD2,81,14,5,

KEY,60,7,4,
KEY,42,7,14,

DOOR,-7,6,21,
DOOR,-7,45,100,
DOOR,98,6,101,
DOOR,98,22,102,

EXIT
};

const int8_t AcidDrop1[]  = {
21,
6,22,
12,22,
20,20,
29,21,
36,22,
42,22,
54,53,
62,53,
70,51,
82,34,
88,34,
63,21,
69,21,
83,2,
77,2,
70,3,
90,3,
63,4,
40,3,
32,3,
7,4
};

#endif
