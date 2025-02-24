
// ****************************************************************************
//                                 
//                              Includes
//
// ****************************************************************************

// ----------------------------------------------------------------------------
//                                   Includes
// ----------------------------------------------------------------------------

#include INCLUDES_H		// all includes

typedef unsigned int uint_fast8_t;
typedef unsigned int uint_fast16_t;
typedef unsigned int uint_fast32_t;

typedef int int_fast8_t;
typedef int int_fast16_t;
typedef int int_fast32_t;

#define INT_FAST16_MAX (0x7fffffff)
#define INT16_MIN 	(-32768)
#define INT16_MAX 	 (32767)
#define UINT16_MAX 	 (65535)

struct tm
{
  int	tm_sec;
  int	tm_min;
  int	tm_hour;
  int	tm_mday;
  int	tm_mon;
  int	tm_year;
  int	tm_wday;
  int	tm_yday;
  int	tm_isdst;
};

extern const u8 gameRom[];

// configuration
#define EMU_CLKSYS_GB		252000 	// system clock in [kHz] for Game Boy
#define EMU_CLKSYS_GBC		300000 	// system clock in [kHz] for Game Boy Color

#include "src/table.h"
#include "src/minigb_apu.h"
#include "src/peanut_gb.h"
#include "src/emu_gb_msg.h"
#include "src/emu_gb_menu.h"
#include "src/main.h"		// main code
