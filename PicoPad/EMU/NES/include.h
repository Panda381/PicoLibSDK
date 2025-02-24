
// ****************************************************************************
//                                 
//                              Includes
//
// ****************************************************************************

// ----------------------------------------------------------------------------
//                                   Includes
// ----------------------------------------------------------------------------

#include INCLUDES_H		// all includes

// Type definition
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;

#include "src/setup.h"		// emulator setup

#include "src/infones/NES_APU.h" // sound

#include "src/main.h"		// main code
#include "src/emu_nes_msg.h"
#include "src/emu_nes_menu.h"

#include "src/infones/InfoNES.h"
#include "src/infones/InfoNES_System.h"
#include "src/infones/InfoNES_Mapper.h"
#include "src/infones/K6502.h"
#include "src/infones/K6502_rw.h"
