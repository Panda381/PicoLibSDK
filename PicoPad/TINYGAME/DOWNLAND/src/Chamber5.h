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


#ifndef _CHAMBER5_H
#define _CHAMBER5_H

//#include "../include.h"
//#include "Chamber4.h"

const int8_t RopeVec5[]  = {
VERTICAL,10,3,34,
VERTICAL,29,34,10,
VERTICAL,41,34,20,
VERTICAL,69,3,9,
EXIT};


const int8_t Vec5[]  = {
POS,0,5,
VEC,2,3,
ROOF,94,3,
VEC,96,6,
VEC,96,13,
FLOOR,78,13,
VEC,80,15,
VEC,80,16,
VEC,84,19,
VEC,88,20,
ROOF,94,20,
VEC,96,21,
VEC,96,29,
FLOOR,70,29,
VEC,70,24,
VEC,65,24,
VEC,65,19,
VEC,60,19,
VEC,60,14,
VEC,21,14,
WALL,21,32,
VEC,24,35,
VEC,26,35,
ROOF,90,35,
VEC,93,36,
VEC,96,39,
WALL,96,44,
POS,96,43,
VEC,96,50,
FLOOR,84,50,
VEC,84,45,
FLOOR,52,45,
VEC,54,47,
VEC,54,48,
VEC,58,51,
ROOF,72,51,
VEC,76,54,
WALL,76,63,
FLOOR,0,63,
VEC,0,55,
VEC,11,50,
VEC,14,53,
VEC,17,51,
WALL,17,45,
FLOOR,0,45,
WALL,0,5,
EXIT
};

const int8_t SPK5[]  = {
DOOR,-7,56,10,
DOOR,98,22,9,
DOOR,98,43,11,

GOLD,-2,26,17,
GOLD2,65,50,15,
GOLD,16,19,16,

KEY,-2,11,10,
KEY,87,2,11,

BALLONSPK,79,32,0,

EXIT
};

const int8_t AcidDrop5[]  = {
23,
7,4,
13,4,
23,2,
41,2,
53,2,
26,37,
32,37,
38,37,
44,37,
7,54,
17,53,
66,3,
72,3,
78,3,
89,2,
81,19,
89,21,
66,35,
77,35,
90,35,
58,53,
65,52,
71,53,
};

#endif
