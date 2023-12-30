
// ****************************************************************************
//
//                            Keyboard - PicoinoMini
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

#if USE_PICOINO

#if USE_PICOINOMINI

#include "../../_sdk/inc/sdk_gpio.h"
#include "../../_sdk/inc/sdk_timer.h"
#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_lib/inc/lib_print.h"
#include "picoino_keymini.h"

// state of User LED
Bool UserLedState = False;

// remap key to characters
const char KeyMapToChar[KEY_NUM+1] = {
	NOCHAR,		// 0 NOKEY
	CH_UP,		// 1 KEY_UP
	CH_LEFT,	// 2 KEY_LEFT
	CH_RIGHT,	// 3 KEY_RIGHT
	CH_DOWN,	// 4 KEY_DOWN
	CH_TAB,		// 5 KEY_X
	CH_ESC,		// 6 KEY_Y
	CH_CR,		// 7 KEY_A
	CH_SPC,		// 8 KEY_B
};

// map keys to GPIO-IN
const u8 KeyGpioTab[KEY_NUM] = {
// left pad - OUT is HIGH, IN is pull-down
	KEY_UP_GPIO,		// UP
	KEY_LEFT_GPIO,		// LEFT
	KEY_RIGHT_GPIO,		// RIGHT
	KEY_DOWN_GPIO,		// DOWN
// right pad - OUT is LOW, IN is pull-up
	KEY_X_GPIO,		// X
	KEY_Y_GPIO,		// Y
	KEY_A_GPIO,		// A
	KEY_B_GPIO,		// B
};

// time of last press/release
u16 KeyLastPress[KEY_NUM];
u16 KeyLastRelease[KEY_NUM];

// keys are currently pressed (index = button code - 1)
volatile Bool KeyPressMap[KEY_NUM];

// keyboard buffer
#define KEYBUF_SIZE	10 // size of keyboard buffer
char KeyBuf[KEYBUF_SIZE]; // keyboard buffer
char KeyRetBuf = NOKEY;	// returned key to keyboard buffer (NOKEY = no key)
volatile u8 KeyWriteOff = 0; // write offset to keyboard buffer
volatile u8 KeyReadOff = 0;	// read offset from keyboard buffer

// initialize keyboard
void KeyInit()
{
	// output to keyboard
	GPIO_Init(KEY_OUT_GPIO); // output to keyboard
	GPIO_DirOut(KEY_OUT_GPIO);

	// inputs from keyboard
	GPIO_Init(KEY_UP_GPIO);
	GPIO_Init(KEY_LEFT_GPIO);
	GPIO_Init(KEY_RIGHT_GPIO);
	GPIO_Init(KEY_DOWN_GPIO);

	// clear pressed map
	memset((void*)KeyPressMap, 0, sizeof(KeyPressMap));
	
	// initialize  keyboard buffer
	KeyRetBuf = NOKEY;
	KeyWriteOff = 0;
	KeyReadOff = 0;
}

// terminate keyboard
void KeyTerm()
{
	GPIO_Reset(KEY_OUT_GPIO);
	GPIO_Reset(KEY_UP_GPIO);
	GPIO_Reset(KEY_LEFT_GPIO);
	GPIO_Reset(KEY_RIGHT_GPIO);
	GPIO_Reset(KEY_DOWN_GPIO);
}

// write key to keyboard buffer
void KeyWriteKey(char key)
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

#define WAITDELAY	7 // short delay in [us] for discharging diode capacity (2 us is enough)

// scan keyboard (called from SysTick)
void KeyScan()
{
	u16 t = TimeMs(); // time in [ms]

	// left pad: set OUT to HIGH, set IN to pull-down
	GPIO_Out1(KEY_OUT_GPIO);
	GPIO_PullDown(KEY_UP_GPIO);
	GPIO_PullDown(KEY_LEFT_GPIO);
	GPIO_PullDown(KEY_RIGHT_GPIO);
	GPIO_PullDown(KEY_DOWN_GPIO);
	WaitUs(WAITDELAY); // short delay for discharging diode capacity

	// scan left pad
	int i;
	for (i = 0; i < 4; i++)
	{
		// check if button is pressed
		if (GPIO_In(KeyGpioTab[i]) == 1)
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

	// right pad: set OUT to LOW, set IN to pull-up
	GPIO_Out0(KEY_OUT_GPIO);
	GPIO_PullUp(KEY_X_GPIO);
	GPIO_PullUp(KEY_Y_GPIO);
	GPIO_PullUp(KEY_A_GPIO);
	GPIO_PullUp(KEY_B_GPIO);
	WaitUs(WAITDELAY); // short delay for discharging diode capacity

	// scan right pad
	for (; i < 8; i++)
	{
		// check if button is pressed
		if (GPIO_In(KeyGpioTab[i]) == 0)
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

	// set user LED
	GPIO_Out(KEY_OUT_GPIO, !UserLedState);
}

// check if button KEY_* is currently pressed
Bool KeyPressed(u8 key)
{
#if !SYSTICK_KEYSCAN	// call KeyScan() function from SysTick system timer
	KeyScan();
#endif
	if ((key < 1) || (key > KEY_NUM)) return False;
	return KeyPressMap[key-1];
}

// get virtual key code from keyboard buffer (returns NOKEY if no scan code)
u8 KeyGet()
{
#if !SYSTICK_KEYSCAN	// call KeyScan() function from SysTick system timer
	// scan keyboard
	KeyScan();
#endif

	// get key from temporary 1-key buffer
	char ch = KeyRetBuf;
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

// get character from keyboard buffer (returns NOCHAR if no valid character)
char KeyChar()
{
	return KeyMapToChar[KeyGet()];
}

// flush keyboard buffer
void KeyFlush()
{
	KeyReadOff = KeyWriteOff;
	KeyRetBuf = NOKEY;
}

// return key to keyboard buffer (can hold only 1 key)
void KeyRet(char key)
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

#endif // USE_PICOINOMINI

#endif // USE_PICOINO
