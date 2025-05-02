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


#ifndef _CHAMBER9_H
#define _CHAMBER9_H

//#include "../include.h"
//#include "Chamber8.h"

const int8_t RopeVec9[]  = {
VERTICAL,40,37,8,
VERTICAL,52,37,8,
VERTICAL,64,37,8,

VERTICAL,40,3,8,
VERTICAL,52,3,8,
VERTICAL,64,3,8,

VERTICAL,76,3,42,

VERTICAL,8,18,34,

EXIT};

const int8_t Vec9[]  = {
ILOT,37,29, 
ILOT,49,29, 
ILOT,61,29, 

POS,0,5,
VEC,2,3,
ROOF,93,3,
VEC,96,5,
WALL,96,63,
FLOOR,12,63,
VEC,12,59,
FLOOR,0,59,
VEC,0,58,
WALL,0,27,
VEC,0,25,
VEC,3,22,
VEC,5,21,
ROOF,28,21,
VEC,30,16,
VEC,30,15,
VEC,31,14,
FLOOR,0,14,
VEC,0,5,

POS,19,44,
FLOOR,31,44,
VEC,30,45,
VEC,30,47,
VEC,25,50,
VEC,19,47,
VEC,19,44,
VEC,18,44,
EXIT
};

const int8_t SPK9[]  = {
DOOR,-7,7,19,
DOOR,-7,52,20,

DOOR,97,56,16,

KEY,31,2,19,
KEY,38,22,20,

GOLD,67,48,27,
GOLD2,37,48,28,
GOLD2,12,21,29,

EXIT
};

const int8_t AcidDrop9[]  = {
26,
5,2,
11,2,
17,2,
23,2,
29,2,
37,4,
43,4,
49,4,
55,4,
61,4,
67,4,
73,4,
79,4,
5,23,
11,23,
18,22,
24,22,
30,18,
19,49,
30,49,
37,37,
43,37,
49,37,
55,37,
61,37,
67,37,
};

#endif
