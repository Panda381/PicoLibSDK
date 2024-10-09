
// ****************************************************************************
//                                 
//                              Includes
//
// ****************************************************************************

// ----------------------------------------------------------------------------
//                                   Includes
// ----------------------------------------------------------------------------

#include "../../../includes.h"	// all includes

#define CHECK_NOINF	0		// 1 = disable check of infinity ranges

#undef USE_REAL32

#include "..\..\..\_lib\real\real.h"		// update flags
#include "..\..\..\_lib\real\real_def.h"
extern u8 Unit;		// current angle unit UNIT_* (used by functions ToRad() and FromRad())

#include "src/myrand.h"		// random numbers
#include "src/ref.h"		// reference functions
#include "src/test.h"		// tests
#include "src/main.h"		// main code


