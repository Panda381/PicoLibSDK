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


#ifndef _CHAMBER4_H
#define _CHAMBER4_H

//#include "../include.h"
//#include "Chamber3.h"

const int8_t RopeVec4[]  = {
VERTICAL,26,28,26,
VERTICAL,86,5,37,

VERTICAL,39,54,3,
VERTICAL,74,54,3,

HORIZONTAL,42,30,28,
EXIT};

const int8_t Vec4[]  = {
BIG_ILOT,36,38,
BIG_ILOT,71,38,
POS,0,5,
VEC,2,3,
ROOF,94,3,
VEC,96,6,
WALL,96,63,
FLOOR,0,63,

VEC,0,57,

WALL,0,51,
VEC,5,48,
VEC,6,46,
FLOOR,0,46,

VEC,0,41,

WALL,0,35,
VEC,5,32,
VEC,6,30,
FLOOR,0,30,

VEC,0,24,
VEC,4,20,
ROOF,18,20,
VEC,26,28,
ROOF,77,28,
WALL,77,18,
FLOOR,28,18,

VEC,28,13,
FLOOR,0,13,
VEC,0,5,

POS,42,30,
VEC,39,27,

POS,72,30,
VEC,76,26,
EXIT
};

const int8_t Big_Ilot[]  = {
POS,0,0,
FLOOR,6,0,
WALL,6,13,
POS,0,0,
WALL,0,13,
VEC,3,15,
VEC,6,13,
EXIT};

const int8_t SPK4[]  = {
  DOOR,-7,6,112,
  DOOR,-7,23,113,
  DOOR,-7,39,114,
  DOOR,-7,56,115,

GOLD2,78,3,14,
GOLD2,45,50,12,
GOLD,80,50,13,

KEY,4,17,6,
KEY,4,33,9,

EXIT
};

const int8_t AcidDrop4[]  = {
20,
7,4,
19,4,
28,3,
47,2,
71,2,
83,2,
89,2,
6,19,
6,32,
6,48,
23,28,
29,27,
39,29,
47,27,
65,27,
74,30,
36,53,
42,53,
71,53,
77,53
};

#endif
