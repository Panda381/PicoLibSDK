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


#ifndef _CHAMBER8_H
#define _CHAMBER8_H

//#include "../include.h"
//#include "Chamber7.h"

const int8_t RopeVec8[]  = {
VERTICAL,79,3,6,
VERTICAL,9,3,18,
VERTICAL,86,18,16,
VERTICAL,86,42,15,
VERTICAL,9,30,15,

HORIZONTAL,15,5,58,

EXIT};

const int8_t Vec8[]  = {
POS,0,5,
VEC,2,3,
ROOF,93,3,
VEC,96,5,

WALL,96,11,
VEC,96, 17,

FLOOR,30,17,
POS,90,17,
VEC,91,18,
VEC,92,20,
VEC,96,23,
WALL,96,41,

FLOOR,20,41,
POS,90,41,
VEC,91,42,
VEC,92,44,
VEC,96,47,
WALL,96,63,
POS,95,63,
FLOOR,0,63,

VEC,0,56,
VEC,3,54,
VEC,5,53,
VEC,5,52,
VEC,6,52,
POS,0,52,
FLOOR,76,52,

WALL,0,33,

VEC,3,31,
VEC,5,30,
VEC,5,30,
VEC,6,29,
POS,0,29,
FLOOR,76,29,
WALL,0,5,

EXIT
};

const int8_t SPK8[]  = {
DOOR,-7,56,18,
DOOR,98,10,14,

KEY,20,13,17,
KEY,45,7,18,

GOLD2,78,16,24,
GOLD2,78,40,25,
GOLD,16,28,26,

EXIT
};

const int8_t AcidDrop8[]  = {
37,
6,3,
12,3,
23,2,
35,2,
47,2,
59,2,
71,2,
76,3,
82,3,
89,3,
30,19,
43,19,
56,19,
69,19,
83,18,
89,19,
6,31,
12,31,
24,31,
37,31,
50,31,
63,31,
76,31,
20,43,
33,43,
46,43,
59,43,
72,43,
83,42,
89,43,
5,55,
12,54,
24,54,
37,54,
50,54,
63,54,
76,54,
};

#endif
