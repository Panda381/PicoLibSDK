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


#ifndef _CHAMBER10_H
#define _CHAMBER10_H

//#include "../include.h"
//#include "Chamber9.h"

const int8_t RopeVec10[]  = {
EXIT};

const int8_t Vec10[]  = {
POS,0,5,
VEC,2,3,
ROOF,93,3,
VEC,96,5,
WALL,96,63,
FLOOR,0,63,
WALL,0,5,
EXIT
};

const int8_t SPK10[]  = {
EXIT
};

const int8_t AcidDrop10[]  = {
15,
3,3,
11,2,
18,3,
23,2,
28,3,
35,2,
39,4,
47,2,
53,2,
57,4,
65,2,
70,3,
78,3,
85,4,
89,2
};

#endif
