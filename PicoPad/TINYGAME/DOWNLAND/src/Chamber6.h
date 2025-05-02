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


#ifndef _CHAMBER6_H
#define _CHAMBER6_H

//#include "../include.h"
//#include "Chamber5.h"

const int8_t RopeVec6[]  = {
VERTICAL,11,3,53,
VERTICAL,80,3,9,
VERTICAL,80,30,26,
EXIT};

const int8_t Vec6[]  = {
POS,0,5,
VEC,2,3,
ROOF,94,3,
VEC,96,6,
WALL,96,18,
VEC,96,23,
FLOOR,23,23,
VEC,24,24,
VEC,25,26,
VEC,31,29,
ROOF,93,29,
VEC,96,32,
WALL,96,38,
VEC,96,43,

FLOOR,90,43,
VEC,91,44,
VEC,92,46,
VEC,96,48,
VEC,96,52,
VEC,95,52,
VEC,95,53,
VEC,96,54,
WALL,96,63,
FLOOR,0,63,
WALL,0,5,

EXIT
};

const int8_t SPK6[]  = {
DOOR,98,36,13,
DOOR,98,16,12,

KEY,16,2,12,
KEY,88,30,13,

GOLD,16,26,20,
GOLD,38,47,18,
GOLD,50,7,19,

BALLONSPK,83,30,0,

EXIT
};

const int8_t AcidDrop6[]  = {
18,
8,5,
14,5,
23,3,
35,3,
47,2,
53,2,
65,2,
77,2,
83,2,
89,2,
31,31,
46,28,
52,28,
64,28,
77,29,
83,29,
90,30,
90,45,
};

#endif
