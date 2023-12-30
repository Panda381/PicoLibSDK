
// ****************************************************************************
//
//                            DemoVGA Keyboard
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#include "../../global.h"	// globals

#if USE_DEMOVGA

#include "../../_sdk/inc/sdk_gpio.h"
#include "../../_sdk/inc/sdk_timer.h"
#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_lib/inc/lib_print.h"
#include "demovga_key.h"

// map keys to GPIO-IN
const u8 KeyGpioTab[KEY_NUM] = {
	KEY_A_GPIO,		// input key A
	KEY_B_GPIO,		// input key B
	KEY_C_GPIO,		// input key C
};

// time of last press/release
u16 KeyLastPress[KEY_NUM];
u16 KeyLastRelease[KEY_NUM];

// keys are currently pressed (index = button code - 1)
volatile Bool KeyPressMap[KEY_NUM];

// keyboard buffer
#define KEYBUF_SIZE	8 // size of keyboard buffer
u8 KeyBuf[KEYBUF_SIZE]; // keyboard buffer
u8 KeyRetBuf = NOKEY;	// returned key to keyboard buffer (NOKEY = no key)
volatile u8 KeyWriteOff = 0; // write offset to keyboard buffer
volatile u8 KeyReadOff = 0;	// read offset from keyboard buffer

// initialize keys
void KeyInit()
{
	int i;
	for (i = 0; i < KEY_NUM; i++)
	{
		GPIO_Init(KeyGpioTab[i]); // initialize GPIO to input
		GPIO_PullDown(KeyGpioTab[i]); // pull-down
		KeyPressMap[i] = False;
	}

	KeyRetBuf = NOKEY;
	KeyWriteOff = 0;
	KeyReadOff = 0;
}

// terminate keys
void KeyTerm()
{
	int i;
	for (i = 0; i < KEY_NUM; i++)
	{
		GPIO_Reset(KeyGpioTab[i]); // reset GPIO
	}
}

// write key to keyboard buffer
void KeyWriteKey(u8 key)
{
	u8 w = KeyWriteOff;
	u8 w2 = w + 1;
	if (w2 >= KEYBUF_SIZE) w2 = 0;
	dmb();
	if (w2 != KeyReadOff)
	{
		KeyBuf[w] = key;
		KeyWriteOff = w2;
	}
}

// scan keys
void KeyScan()
{
	int i;
	u16 t = TimeMs(); // time in [ms]
	for (i = 0; i < KEY_NUM; i++)
	{
		// check if button is pressed
		if (GPIO_In(KeyGpioTab[i]) != 0)
		{
			// button is pressed for the first time
			if (!KeyPressMap[i])
			{
				KeyLastPress[i] = t + (KEY_REP_TIME1 - KEY_REP_TIME2);
				KeyPressMap[i] = True;
				KeyWriteKey(i+1);
			}

			// button is already pressed - check repeat interval
			else
			{
				if ((s32)(t - KeyLastPress[i]) >= (s16)KEY_REP_TIME2)
				{
					KeyLastPress[i] = t;
					KeyWriteKey(i+1);
				}
			}
			KeyLastRelease[i] = t;
		}

		// button is release - check stop of press
		else
		{
			if (KeyPressMap[i])
			{
				if ((s16)(t - KeyLastRelease[i]) >= (s16)KEY_REL_TIME)
				{
					KeyPressMap[i] = False;
				}
			}
		}
	}
}

// check if key KEY_* is currently pressed
Bool KeyPressed(u8 key)
{
#if !SYSTICK_KEYSCAN	// call KeyScan() function from SysTick system timer
	KeyScan();
#endif
	if ((key < 1) || (key > KEY_NUM)) return False;
	return KeyPressMap[key-1];
}

// get button from keyboard buffer (returns NOKEY if no scan code)
u8 KeyGet()
{
#if !SYSTICK_KEYSCAN	// call KeyScan() function from SysTick system timer
	// scan keyboard
	KeyScan();
#endif

	// get key from temporary 1-key buffer
	u8 ch = KeyRetBuf;
	if (ch != NOKEY)
	{
		KeyRetBuf = NOKEY;
		return ch;
	}

	// check if keyboard buffer is empty
	u8 r = KeyReadOff;
	dmb();
	if (r == KeyWriteOff) return NOKEY;

	// get key from keyboard buffer
	ch = KeyBuf[r];

	// write new read offset
	r++;
	if (r >= KEYBUF_SIZE) r = 0;
	KeyReadOff = r;

	return ch;
}

// flush keyboard buffer
void KeyFlush()
{
	KeyReadOff = KeyWriteOff;
	KeyRetBuf = NOKEY;
}

// return key to keyboard buffer (can hold only 1 key)
void KeyRet(u8 key)
{
	KeyRetBuf = key;
}

// check no pressed key
Bool KeyNoPressed()
{
	int i;
	for (i = 0; i < KEY_NUM; i++) if (KeyPressMap[i]) return False;
	return True;
}

// wait for no key pressed
void KeyWaitNoPressed()
{
	while (!KeyNoPressed()) {}
}

#endif // USE_DEMOVGA
