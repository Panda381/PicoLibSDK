
// ****************************************************************************
//
//                             PicoPad Buttons
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

#if USE_PICOPAD

#include "../../_sdk/inc/sdk_gpio.h"
#include "../../_sdk/inc/sdk_timer.h"
#if USE_USBPAD		// simulate keypad with USB keyboard
#include "../../_sdk/usb_inc/sdk_usb_phy.h"
#include "../../_sdk/usb_inc/sdk_usb_hid.h"
#include "../../_sdk/usb_inc/sdk_usb_host.h"
#include "../../_sdk/usb_inc/sdk_usb_host_hid.h"
#endif // USE_USBPAD
#include "../../_display/minivga/minivga.h"
#include "picopad_key.h"

#if !USE_PICOPADVGA
// buttons GPIOs
const u8 KeyGpioTab[KEY_NUM] = {
	BTN_UP_PIN,		// up
	BTN_LEFT_PIN,		// left
	BTN_RIGHT_PIN,		// right
	BTN_DOWN_PIN,		// down
	BTN_X_PIN,		// X
	BTN_Y_PIN,		// Y
	BTN_A_PIN,		// A
	BTN_B_PIN,		// B
};
#endif

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

// time of last press/release
u16 KeyLastPress[KEY_NUM];
u16 KeyLastRelease[KEY_NUM];

// keys are currently pressed (index = button code - 1)
volatile Bool KeyPressMap[KEY_NUM];

// keyboard buffer
#define KEYBUF_SIZE	16 // size of keyboard buffer
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
#if !USE_PICOPADVGA
		GPIO_Init(KeyGpioTab[i]); // initialize GPIO to input
		GPIO_PullUp(KeyGpioTab[i]); // pull-up
#endif
		KeyPressMap[i] = False;
	}

	KeyRetBuf = NOKEY;
	KeyWriteOff = 0;
	KeyReadOff = 0;

#if USE_USBPAD		// simulate keypad with USB keyboard
	// initialize USB
	UsbHostInit();
#endif
}

// terminate keys
void KeyTerm()
{
#if USE_USBPAD		// simulate keypad with USB keyboard
	// terminate USB
	UsbTerm();
#endif

#if !USE_PICOPADVGA
	int i;
	for (i = 0; i < KEY_NUM; i++)
	{
		GPIO_Reset(KeyGpioTab[i]); // reset GPIO
	}
#endif
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
#if USE_PICOPADVGA
		if ((VgaKeyScan & (1UL << i)) != 0)
#else
		if (GPIO_In(KeyGpioTab[i]) == 0)
#endif
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
					KeyWriteKey((i+1) | KEY_RELEASE);
				}
			}
		}
	}
}

// check if key KEY_* is currently pressed
Bool KeyPressed(u8 key)
{
	key &= KEY_MASK; // clear release flag

#if !SYSTICK_KEYSCAN	// call KeyScan() function from SysTick system timer
	KeyScan();
#endif
	if ((key < 1) || (key > KEY_NUM)) return False;

#if USE_USBPAD		// simulate keypad with USB keyboard
	if (UsbKeyIsMounted())
	{
		switch (key)
		{
		case KEY_UP:
			if (	UsbKeyIsPressed(HID_KEY_ARROW_UP) ||
				UsbKeyIsPressed(HID_KEY_KEYPAD_8)) return True;
			break;

		case KEY_LEFT:
			if (	UsbKeyIsPressed(HID_KEY_ARROW_LEFT) ||
				UsbKeyIsPressed(HID_KEY_KEYPAD_4)) return True;
			break;

		case KEY_RIGHT:
			if (	UsbKeyIsPressed(HID_KEY_ARROW_RIGHT) ||
				UsbKeyIsPressed(HID_KEY_KEYPAD_6)) return True;
			break;

		case KEY_DOWN:
			if (	UsbKeyIsPressed(HID_KEY_ARROW_DOWN) ||
				UsbKeyIsPressed(HID_KEY_KEYPAD_2) ||
				UsbKeyIsPressed(HID_KEY_KEYPAD_5)) return True;
			break;

		case KEY_X:
			if (	UsbKeyIsPressed(HID_KEY_TAB) ||
				UsbKeyIsPressed(HID_KEY_KEYPAD_SUBTRACT) ||
				UsbKeyIsPressed(HID_KEY_SHIFT_LEFT) ||
				UsbKeyIsPressed(HID_KEY_SHIFT_RIGHT) ||
				UsbKeyIsPressed(HID_KEY_X)) return True;
			break;

		case KEY_Y:
			if (	UsbKeyIsPressed(HID_KEY_ESCAPE) ||
				UsbKeyIsPressed(HID_KEY_BACKSPACE) ||
				UsbKeyIsPressed(HID_KEY_KEYPAD_MULTIPLY) ||
				UsbKeyIsPressed(HID_KEY_Y) ||
				UsbKeyIsPressed(HID_KEY_Z)) return True;
			break;

		case KEY_A:
			if (	UsbKeyIsPressed(HID_KEY_CONTROL_LEFT) ||
				UsbKeyIsPressed(HID_KEY_CONTROL_RIGHT) ||
				UsbKeyIsPressed(HID_KEY_ENTER) ||
				UsbKeyIsPressed(HID_KEY_KEYPAD_ENTER) ||
//				UsbKeyIsPressed(HID_KEY_SPACE) ||
				UsbKeyIsPressed(HID_KEY_KEYPAD_0) ||
				UsbKeyIsPressed(HID_KEY_A)) return True;
			break;

		case KEY_B:
			if (	UsbKeyIsPressed(HID_KEY_ALT_LEFT) ||
				UsbKeyIsPressed(HID_KEY_ALT_RIGHT) ||
				UsbKeyIsPressed(HID_KEY_KEYPAD_ADD) ||
				UsbKeyIsPressed(HID_KEY_KEYPAD_DECIMAL) ||
				UsbKeyIsPressed(HID_KEY_B)) return True;
			break;
		}
	}
#endif // USE_USBPAD

	return KeyPressMap[key-1];
}

// get button from keyboard buffer, including release keys (returns NOKEY if no scan code; B7 = KEY_RELEASE = release flag)
u8 KeyGetRel()
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

#if USE_USBPAD		// simulate keypad with USB keyboard
	if (UsbKeyIsMounted())
	{
		for (;;)
		{
			u32 key = UsbGetKeyRel();
			if (key == 0) break;

			ch = key & 0xff;
			u8 rel = ((key & B24) == 0) ? 0 : KEY_RELEASE; // release flag
			switch (ch)
			{
			case HID_KEY_ARROW_UP:
			case HID_KEY_KEYPAD_8:
				return KEY_UP | rel;

			case HID_KEY_ARROW_LEFT:
			case HID_KEY_KEYPAD_4:
				return KEY_LEFT | rel;

			case HID_KEY_ARROW_RIGHT:
			case HID_KEY_KEYPAD_6:
				return KEY_RIGHT | rel;

			case HID_KEY_ARROW_DOWN:
			case HID_KEY_KEYPAD_2:
			case HID_KEY_KEYPAD_5:
				return KEY_DOWN | rel;

			case HID_KEY_TAB:
			case HID_KEY_KEYPAD_SUBTRACT:
			case HID_KEY_SHIFT_LEFT:
			case HID_KEY_SHIFT_RIGHT:
			case HID_KEY_X:
				return KEY_X | rel;

			case HID_KEY_ESCAPE:
			case HID_KEY_BACKSPACE:
			case HID_KEY_KEYPAD_MULTIPLY:
			case HID_KEY_Y:
			case HID_KEY_Z:
				return KEY_Y | rel;

			case HID_KEY_CONTROL_LEFT:
			case HID_KEY_CONTROL_RIGHT:
			case HID_KEY_ENTER:
			case HID_KEY_KEYPAD_ENTER:
//			case HID_KEY_SPACE:
			case HID_KEY_KEYPAD_0:
			case HID_KEY_A:
				return KEY_A | rel;

			case HID_KEY_ALT_LEFT:
			case HID_KEY_ALT_RIGHT:
			case HID_KEY_KEYPAD_ADD:
			case HID_KEY_KEYPAD_DECIMAL:
			case HID_KEY_B:
				return KEY_B | rel;
			}
		}
	}
#endif // USE_USBPAD

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

// get button from keyboard buffer (returns NOKEY if no scan code)
u8 KeyGet()
{
	u8 ch;
	for (;;)
	{
		ch = KeyGetRel();
		if ((ch & KEY_RELEASE) == 0) return ch; // release key of NOKEY
	}
}

// get character from local keyboard
char KeyChar()
{
#if USE_USBPAD		// simulate keypad with USB keyboard
	// get USB character (returns NOCHAR if no character)
	char ch = UsbGetChar();
	if (ch != NOCHAR) return ch;
#endif // USE_USBPAD

	return KeyMapToChar[(u8)KeyGet()];
}

// flush keyboard buffer
void KeyFlush()
{
	KeyReadOff = KeyWriteOff;
	KeyRetBuf = NOKEY;

#if USE_USBPAD		// simulate keypad with USB keyboard
	// flush USB keys
	UsbFlushKey();
#endif // USE_USBPAD
}

// return key to keyboard buffer (can hold only 1 key)
void KeyRet(u8 key)
{
	KeyRetBuf = key;
}

// check no pressed key
Bool KeyNoPressed()
{
#if !SYSTICK_KEYSCAN	// call KeyScan() function from SysTick system timer
	// scan keyboard
	KeyScan();
#endif

	int i;
	for (i = 0; i < KEY_NUM; i++) if (KeyPressMap[i]) return False;
	return True;
}

// wait for no key pressed
void KeyWaitNoPressed()
{
	while (!KeyNoPressed()) {}
}

#endif // USE_PICOPAD
