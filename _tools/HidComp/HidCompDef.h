
#define U16_U8_LE(nn)	((u8)(nn)), ((u8)((nn)>>8))	// convert u16 to 2x u8 entries, little endian format (Intel)
#define U32_U8_LE(nn)	((u8)(nn)), ((u8)((nn)>>8)), ((u8)((nn)>>16)), ((u8)((nn)>>24)) // convert u32 to 4x u8 entries

#define	B0 (1<<0)
#define	B1 (1<<1)
#define	B2 (1<<2)
#define	B3 (1<<3)
#define	B4 (1<<4)
#define	B5 (1<<5)
#define	B6 (1<<6)
#define	B7 (1<<7)
#define	B8 (1U<<8)
#define	B9 (1U<<9)
#define	B10 (1U<<10)
#define	B11 (1U<<11)
#define	B12 (1U<<12)
#define	B13 (1U<<13)
#define	B14 (1U<<14)
#define	B15 (1U<<15)
#define B16 (1UL<<16)
#define B17 (1UL<<17)
#define B18 (1UL<<18)
#define	B19 (1UL<<19)
#define B20 (1UL<<20)
#define B21 (1UL<<21)
#define B22 (1UL<<22)
#define B23 (1UL<<23)
#define B24 (1UL<<24)
#define B25 (1UL<<25)
#define B26 (1UL<<26)
#define B27 (1UL<<27)
#define B28 (1UL<<28)
#define B29 (1UL<<29)
#define B30 (1UL<<30)
#define B31 (1UL<<31)

#define BIT(pos) (1UL<<(pos))

// === report descriptor
// HID report descriptor comes as collection of short and long items

// Short item:
// 0: (1) prefix byte
//  bit 0..1: (2 bits) size of optional data in bytes (0, 1, 2 or 4 bytes)
//  bit 2..3: (2 bits) type of this report descriptor (0=main, 1=global, 2=local, 3=reserved for long item)
//  bit 4..7: (4 bits) short item tag: function of the item (subtype)
// 1: (0, 1, 2 or 4) optional item data

// Long item:
// 0: (1) prefix byte = 0xfe
//  bit 0..1: (2 bits) size = 2 bytes
//  bit 2..3: (2 bits) type = 3
//  bit 4..7: (4 bits) tag = 0x0f
// 1: (1) data size in bytes
// 2: (1) long item tag
// 3: (x) item data

// data array on end of report field
#define HID_REPORT_DATA_0(data)						// +0x00
#define HID_REPORT_DATA_1(data)	, (data)			// +0x01
#define HID_REPORT_DATA_2(data)	, U16_U8_LE(data)	// +0x02
#define HID_REPORT_DATA_3(data)	, U32_U8_LE(data)	// +0x03

// prefix byte of report item
//  size ... size of data 0, 1, 2, 3 (3 means 4 bytes)
//  type ... type of this report descriptor HID_TYPE_MAIN=0, HID_TYPE_GLOBAL=1 or HID_TYPE_LOCAL=2
//  tag ... item tag (function, subtype)
//  data ... data
#define HID_REPORT_ITEM(size, type, tag, data) \
	((size) | ((type) << 2) | ((tag) << 4)) HID_REPORT_DATA_##size(data)

// type of report descriptor
#define HID_TYPE_MAIN	0		// +0x00
#define HID_TYPE_GLOBAL	1		// +0x04
#define HID_TYPE_LOCAL	2		// +0x08

// 'main' report descriptor item tags
#define HID_MAIN_INPUT			8		// input		+0x80
#define HID_MAIN_OUTPUT			9		// output		+0x90
#define HID_MAIN_COLLECTION		10		// collection		+0xA0
#define HID_MAIN_FEATURE		11		// feature		+0xB0
#define HID_MAIN_COLLECTION_END	12		// end of collection	+0xC0

#define HID_INPUT0			HID_REPORT_ITEM(0, HID_TYPE_MAIN, HID_MAIN_INPUT, 0)			// 0x00+0x00+0x80 = 0x80
#define HID_OUTPUT0			HID_REPORT_ITEM(0, HID_TYPE_MAIN, HID_MAIN_OUTPUT, 0)			// 0x00+0x00+0x90 = 0x90
#define HID_COLLECTION0		HID_REPORT_ITEM(0, HID_TYPE_MAIN, HID_MAIN_COLLECTION, 0)		// 0x00+0x00+0xA0 = 0xA0
#define HID_FEATURE0		HID_REPORT_ITEM(0, HID_TYPE_MAIN, HID_MAIN_FEATURE, 0)			// 0x00+0x00+0xB0 = 0xB0
//#define HID_COLLECTION_END0	HID_REPORT_ITEM(0, HID_TYPE_MAIN, HID_MAIN_COLLECTION_END, 0)	// 0x00+0x00+0xC0 = 0xC0

#define HID_INPUT(x)		HID_REPORT_ITEM(1, HID_TYPE_MAIN, HID_MAIN_INPUT, (x))			// 0x01+0x00+0x80 = 0x81 xx
#define HID_OUTPUT(x)		HID_REPORT_ITEM(1, HID_TYPE_MAIN, HID_MAIN_OUTPUT, (x))			// 0x01+0x00+0x90 = 0x91 xx
#define HID_COLLECTION(x)	HID_REPORT_ITEM(1, HID_TYPE_MAIN, HID_MAIN_COLLECTION, (x))		// 0x01+0x00+0xA0 = 0xA1 xx
#define HID_FEATURE(x)		HID_REPORT_ITEM(1, HID_TYPE_MAIN, HID_MAIN_FEATURE, (x))		// 0x01+0x00+0xB0 = 0xB1 xx
#define HID_COLLECTION_END	HID_REPORT_ITEM(0, HID_TYPE_MAIN, HID_MAIN_COLLECTION_END, 0)	// 0x00+0x00+0xC0 = 0xC0

#define HID_INPUT2(x)		HID_REPORT_ITEM(2, HID_TYPE_MAIN, HID_MAIN_INPUT, (x))			// 0x02+0x00+0x80 = 0x82 xx xx
#define HID_OUTPUT2(x)		HID_REPORT_ITEM(2, HID_TYPE_MAIN, HID_MAIN_OUTPUT, (x))			// 0x02+0x00+0x90 = 0x92 xx xx
#define HID_COLLECTION2(x)	HID_REPORT_ITEM(2, HID_TYPE_MAIN, HID_MAIN_COLLECTION, (x))		// 0x02+0x00+0xA0 = 0xA2 xx xx
#define HID_FEATURE2(x)		HID_REPORT_ITEM(2, HID_TYPE_MAIN, HID_MAIN_FEATURE, (x))		// 0x02+0x00+0xB0 = 0xB2 xx xx

#define HID_INPUT3(x)		HID_REPORT_ITEM(3, HID_TYPE_MAIN, HID_MAIN_INPUT, (x))			// 0x03+0x00+0x80 = 0x83 xx xx xx xx
#define HID_OUTPUT3(x)		HID_REPORT_ITEM(3, HID_TYPE_MAIN, HID_MAIN_OUTPUT, (x))			// 0x03+0x00+0x90 = 0x93 xx xx xx xx
#define HID_COLLECTION3(x)	HID_REPORT_ITEM(3, HID_TYPE_MAIN, HID_MAIN_COLLECTION, (x))		// 0x03+0x00+0xA0 = 0xA3 xx xx xx xx
#define HID_FEATURE3(x)		HID_REPORT_ITEM(3, HID_TYPE_MAIN, HID_MAIN_FEATURE, (x))		// 0x03+0x00+0xB0 = 0xB3 xx xx xx xx

// data following INPUT, OUTPUT and FEATURE (0, 1, 2 or 4 bytes - missing data are assumed to be 0):
//	bit 0: 0=data, 1=constant
//	bit 1: 0=array, 1=variable
//	bit 2: 0=absolute, 1=relative
//	bit 3: 0=no wrap, 1=wrap
//	bit 4: 0=linear, 1=non linear
//	bit 5: 0=preferred state, 1=no preffered
//	bit 6: 0=no null position, 1=null state
//	bit 7: 0=non volatile, 1=volatile
//	bit 8: 0=bit field, 1=buffered bytes
//	bit 9..31: 0=reserved

#define HID_DATA				0	// 0x00
#define HID_CONSTANT			B0	// 0x01

#define HID_ARRAY				0	// 0x00
#define HID_VARIABLE			B1	// 0x02

#define HID_ABSOLUTE			0	// 0x00
#define HID_RELATIVE			B2	// 0x04

#define HID_NO_WRAP				0	// 0x00
#define HID_WRAP				B3	// 0x08

#define HID_LINEAR				0 	// 0x00
#define HID_NON_LINEAR			B4	// 0x10

#define HID_PREFERRED_STATE		0	// 0x00
#define HID_NO_PREFERRED		B5	// 0x20

#define HID_NO_NULL_POSITION	0	// 0x00
#define HID_NULL_STATE			B6	// 0x40

#define HID_NON_VOLATILE		0	// 0x00
#define HID_VOLATILE			B7	// 0x80 if used, data size must be 2 !

#define HID_BITFIELD			0	// 0x00
#define HID_BUFFERED_BYTES		B8	// 0x1000 if used, data size must be 2 !

// data following COLLECTION:
//	0x00: physical (group of axes)
//	0x01: application (mouse, keyboard)
//	0x02: logical (interrelated data)
//	0x03: report
//	0x04: named array
//	0x05: usage switch
//	0x06: usage modifier

#define HID_COLLECTION_PHYSICAL			0	// 0x00
#define HID_COLLECTION_APPLICATION		1	// 0x01
#define HID_COLLECTION_LOGICAL			2	// 0x02
#define HID_COLLECTION_REPORT			3	// 0x03
#define HID_COLLECTION_NAMED_ARRAY		4	// 0x04
#define HID_COLLECTION_USAGE_SWITCH		5	// 0x05
#define HID_COLLECTION_USAGE_MODIFIER	6	// 0x06

// 'global' report item tags
#define HID_GLOBAL_USAGE_PAGE			0	// usage page					+0x00
#define HID_GLOBAL_LOGICAL_MIN			1	// logical minimum value		+0x10
#define HID_GLOBAL_LOGICAL_MAX			2	// logical maximum value		+0x20
#define HID_GLOBAL_PHYSICAL_MIN			3	// physical minimum value		+0x30
#define HID_GLOBAL_PHYSICAL_MAX			4	// physical maximum value		+0x40
#define HID_GLOBAL_UNIT_EXPONENT		5	// unit exponent in base 2		+0x50
#define HID_GLOBAL_UNIT					6	// unit values					+0x60
#define HID_GLOBAL_REPORT_SIZE			7	// report size in bits			+0x70
#define HID_GLOBAL_REPORT_ID			8	// report ID					+0x80
#define HID_GLOBAL_REPORT_COUNT			9	// report count					+0x90
#define HID_GLOBAL_PUSH					10	// push global state on stack	+0xA0
#define HID_GLOBAL_POP					11	// pop global state from stack	+0xB0

#define HID_USAGE_PAGE0			HID_REPORT_ITEM(0, HID_TYPE_GLOBAL, HID_GLOBAL_USAGE_PAGE, 0)		// 0x00+0x04+0x00 = 0x04
#define HID_USAGE_PAGE(x)		HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_USAGE_PAGE, (x))		// 0x01+0x04+0x00 = 0x05 xx
#define HID_USAGE_PAGE2(x)		HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_USAGE_PAGE, (x))		// 0x02+0x04+0x00 = 0x06 xx xx
#define HID_USAGE_PAGE3(x)		HID_REPORT_ITEM(3, HID_TYPE_GLOBAL, HID_GLOBAL_USAGE_PAGE, (x))		// 0x03+0x04+0x00 = 0x07 xx xx xx xx

#define HID_LOGICAL_MIN0		HID_REPORT_ITEM(0, HID_TYPE_GLOBAL, HID_GLOBAL_LOGICAL_MIN, 0)		// 0x00+0x04+0x10 = 0x14
#define HID_LOGICAL_MIN(x)		HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_LOGICAL_MIN, (x))	// 0x01+0x04+0x10 = 0x15 xx
#define HID_LOGICAL_MIN2(x)		HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_LOGICAL_MIN, (x))	// 0x02+0x04+0x10 = 0x16 xx xx
#define HID_LOGICAL_MIN3(x)		HID_REPORT_ITEM(3, HID_TYPE_GLOBAL, HID_GLOBAL_LOGICAL_MIN, (x))	// 0x03+0x04+0x10 = 0x17 xx xx xx xx

#define HID_LOGICAL_MAX0		HID_REPORT_ITEM(0, HID_TYPE_GLOBAL, HID_GLOBAL_LOGICAL_MAX, 0)		// 0x00+0x04+0x20 = 0x24
#define HID_LOGICAL_MAX(x)		HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_LOGICAL_MAX, (x))	// 0x01+0x04+0x20 = 0x25 xx
#define HID_LOGICAL_MAX2(x)		HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_LOGICAL_MAX, (x))	// 0x02+0x04+0x20 = 0x26 xx xx
#define HID_LOGICAL_MAX3(x)		HID_REPORT_ITEM(3, HID_TYPE_GLOBAL, HID_GLOBAL_LOGICAL_MAX, (x))	// 0x03+0x04+0x20 = 0x27 xx xx xx xx

#define HID_PHYSICAL_MIN0		HID_REPORT_ITEM(0, HID_TYPE_GLOBAL, HID_GLOBAL_PHYSICAL_MIN, 0)		// 0x00+0x04+0x30 = 0x34
#define HID_PHYSICAL_MIN(x)		HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_PHYSICAL_MIN, (x))	// 0x01+0x04+0x30 = 0x35 xx
#define HID_PHYSICAL_MIN2(x)	HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_PHYSICAL_MIN, (x))	// 0x02+0x04+0x30 = 0x36 xx xx
#define HID_PHYSICAL_MIN3(x)	HID_REPORT_ITEM(3, HID_TYPE_GLOBAL, HID_GLOBAL_PHYSICAL_MIN, (x))	// 0x03+0x04+0x30 = 0x37 xx xx xx xx

#define HID_PHYSICAL_MAX0		HID_REPORT_ITEM(0, HID_TYPE_GLOBAL, HID_GLOBAL_PHYSICAL_MAX, 0)		// 0x00+0x04+0x40 = 0x44
#define HID_PHYSICAL_MAX(x)		HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_PHYSICAL_MAX, (x))	// 0x01+0x04+0x40 = 0x45 xx
#define HID_PHYSICAL_MAX2(x)	HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_PHYSICAL_MAX, (x))	// 0x02+0x04+0x40 = 0x46 xx xx
#define HID_PHYSICAL_MAX3(x)	HID_REPORT_ITEM(3, HID_TYPE_GLOBAL, HID_GLOBAL_PHYSICAL_MAX, (x))	// 0x03+0x04+0x40 = 0x47 xx xx xx xx

#define HID_UNIT_EXPONENT0		HID_REPORT_ITEM(0, HID_TYPE_GLOBAL, HID_GLOBAL_UNIT_EXPONENT, 0)	// 0x00+0x04+0x50 = 0x54
#define HID_UNIT_EXPONENT(x)	HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_UNIT_EXPONENT, (x))	// 0x01+0x04+0x50 = 0x55 xx
#define HID_UNIT_EXPONENT2(x)	HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_UNIT_EXPONENT, (x))	// 0x02+0x04+0x50 = 0x56 xx xx
#define HID_UNIT_EXPONENT3(x)	HID_REPORT_ITEM(3, HID_TYPE_GLOBAL, HID_GLOBAL_UNIT_EXPONENT, (x))	// 0x03+0x04+0x50 = 0x57 xx xx xx xx

#define HID_UNIT0				HID_REPORT_ITEM(0, HID_TYPE_GLOBAL, HID_GLOBAL_UNIT, 0)				// 0x00+0x04+0x60 = 0x64
#define HID_UNIT(x)				HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_UNIT, (x))			// 0x01+0x04+0x60 = 0x65 xx
#define HID_UNIT2(x)			HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_UNIT, (x))			// 0x02+0x04+0x60 = 0x66 xx xx
#define HID_UNIT3(x)			HID_REPORT_ITEM(3, HID_TYPE_GLOBAL, HID_GLOBAL_UNIT, (x))			// 0x03+0x04+0x60 = 0x67 xx xx xx xx

#define HID_REPORT_SIZE0		HID_REPORT_ITEM(0, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_SIZE, 0)		// 0x00+0x04+0x70 = 0x74
#define HID_REPORT_SIZE(x)		HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_SIZE, (x))	// 0x01+0x04+0x70 = 0x75 xx
#define HID_REPORT_SIZE2(x)		HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_SIZE, (x))	// 0x02+0x04+0x70 = 0x76 xx xx
#define HID_REPORT_SIZE3(x)		HID_REPORT_ITEM(3, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_SIZE, (x))	// 0x03+0x04+0x70 = 0x77 xx xx xx xx

#define HID_REPORT_ID0			HID_REPORT_ITEM(0, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_ID, 0)		// 0x00+0x04+0x80 = 0x84
#define HID_REPORT_ID(x)		HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_ID, (x))		// 0x01+0x04+0x80 = 0x85 xx
#define HID_REPORT_ID2(x)		HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_ID, (x))		// 0x02+0x04+0x80 = 0x86 xx xx
#define HID_REPORT_ID3(x)		HID_REPORT_ITEM(3, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_ID, (x))		// 0x03+0x04+0x80 = 0x87 xx xx xx xx

#define HID_REPORT_COUNT0		HID_REPORT_ITEM(0, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_COUNT, 0)		// 0x00+0x04+0x90 = 0x94
#define HID_REPORT_COUNT(x)		HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_COUNT, (x))	// 0x01+0x04+0x90 = 0x95 xx
#define HID_REPORT_COUNT2(x)	HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_COUNT, (x))	// 0x02+0x04+0x90 = 0x96 xx xx
#define HID_REPORT_COUNT3(x)	HID_REPORT_ITEM(3, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_COUNT, (x))	// 0x03+0x04+0x90 = 0x97 xx xx xx xx

#define HID_PUSH				HID_REPORT_ITEM(0, HID_TYPE_GLOBAL, HID_GLOBAL_PUSH, 0)				// 0x00+0x04+0xA0 = 0xA4
#define HID_POP					HID_REPORT_ITEM(0, HID_TYPE_GLOBAL, HID_GLOBAL_POP, 0)				// 0x00+0x04+0xB0 = 0xB4

#define HID_PUSH0				HID_PUSH															// 0x00+0x04+0xA0 = 0xA4
#define HID_POP0				HID_POP																// 0x00+0x04+0xB0 = 0xB4

// 'local' report descriptor item tags
#define HID_LOCAL_USAGE				0	// item usage			+0x00
#define HID_LOCAL_USAGE_MIN			1	// usage minimum		+0x10
#define HID_LOCAL_USAGE_MAX			2	// usage maximum		+0x20
#define HID_LOCAL_DESIGNATOR_INX	3	// designator index		+0x30
#define HID_LOCAL_DESIGNATOR_MIN	4	// designator minimum	+0x40
#define HID_LOCAL_DESIGNATOR_MAX	5	// designator maximum	+0x50
#define HID_LOCAL_STRING_INX		7	// string index			+0x70
#define HID_LOCAL_STRING_MIN		8	// string minimum		+0x80
#define HID_LOCAL_STRING_MAX		9	// string maximum		+0x90
#define HID_LOCAL_DELIMITER			10	// delimiter			+0xA0

#define HID_USAGE0				HID_REPORT_ITEM(0, HID_TYPE_LOCAL, HID_LOCAL_USAGE, 0)			// 0x00+0x08+0x00 = 0x08
#define HID_USAGE(x)			HID_REPORT_ITEM(1, HID_TYPE_LOCAL, HID_LOCAL_USAGE, (x))		// 0x01+0x08+0x00 = 0x09 xx
#define HID_USAGE2(x)			HID_REPORT_ITEM(2, HID_TYPE_LOCAL, HID_LOCAL_USAGE, (x))		// 0x02+0x08+0x00 = 0x0A xx xx
#define HID_USAGE3(x)			HID_REPORT_ITEM(3, HID_TYPE_LOCAL, HID_LOCAL_USAGE, (x))		// 0x03+0x08+0x00 = 0x0B xx xx xx xx

#define HID_USAGE_MIN0			HID_REPORT_ITEM(0, HID_TYPE_LOCAL, HID_LOCAL_USAGE_MIN, 0)		// 0x00+0x08+0x10 = 0x18
#define HID_USAGE_MIN(x)		HID_REPORT_ITEM(1, HID_TYPE_LOCAL, HID_LOCAL_USAGE_MIN, (x))	// 0x01+0x08+0x10 = 0x19 xx
#define HID_USAGE_MIN2(x)		HID_REPORT_ITEM(2, HID_TYPE_LOCAL, HID_LOCAL_USAGE_MIN, (x))	// 0x02+0x08+0x10 = 0x1A xx xx
#define HID_USAGE_MIN3(x)		HID_REPORT_ITEM(3, HID_TYPE_LOCAL, HID_LOCAL_USAGE_MIN, (x))	// 0x03+0x08+0x10 = 0x1B xx xx xx xx

#define HID_USAGE_MAX0			HID_REPORT_ITEM(0, HID_TYPE_LOCAL, HID_LOCAL_USAGE_MAX, 0)		// 0x00+0x08+0x20 = 0x28
#define HID_USAGE_MAX(x)		HID_REPORT_ITEM(1, HID_TYPE_LOCAL, HID_LOCAL_USAGE_MAX, (x))	// 0x01+0x08+0x20 = 0x29 xx
#define HID_USAGE_MAX2(x)		HID_REPORT_ITEM(2, HID_TYPE_LOCAL, HID_LOCAL_USAGE_MAX, (x))	// 0x02+0x08+0x20 = 0x2A xx xx
#define HID_USAGE_MAX3(x)		HID_REPORT_ITEM(3, HID_TYPE_LOCAL, HID_LOCAL_USAGE_MAX, (x))	// 0x03+0x08+0x20 = 0x2B xx xx xx xx

// usage page
#define HID_USAGE_PAGE_DESKTOP			0x01
#define HID_USAGE_PAGE_SIMULATE			0x02
#define HID_USAGE_PAGE_VIRTUAL_REALITY	0x03
#define HID_USAGE_PAGE_SPORT			0x04
#define HID_USAGE_PAGE_GAME				0x05
#define HID_USAGE_PAGE_GENERIC_DEVICE	0x06
#define HID_USAGE_PAGE_KEYBOARD			0x07
#define HID_USAGE_PAGE_LED				0x08
#define HID_USAGE_PAGE_BUTTON			0x09
#define HID_USAGE_PAGE_ORDINAL			0x0a
#define HID_USAGE_PAGE_TELEPHONY		0x0b
#define HID_USAGE_PAGE_CONSUMER			0x0c
#define HID_USAGE_PAGE_DIGITIZER		0x0d
#define HID_USAGE_PAGE_PID				0x0f
#define HID_USAGE_PAGE_UNICODE			0x10
#define HID_USAGE_PAGE_ALPHA_DISPLAY	0x14
#define HID_USAGE_PAGE_MEDICAL			0x40
#define HID_USAGE_PAGE_MONITOR			0x80	// 0x80 - 0x83 ... if used, data size must be 2 !
#define HID_USAGE_PAGE_POWER			0x84	// 0x84 - 0x87 ... if used, data size must be 2 !
#define HID_USAGE_PAGE_BARCODE_SCANNER	0x8c	// if used, data size must be 2 !
#define HID_USAGE_PAGE_SCALE			0x8d	// if used, data size must be 2 !
#define HID_USAGE_PAGE_MSR				0x8e	// if used, data size must be 2 !
#define HID_USAGE_PAGE_CAMERA			0x90	// if used, data size must be 2 !
#define HID_USAGE_PAGE_ARCADE			0x91	// if used, data size must be 2 !
#define HID_USAGE_PAGE_FIDO				0xF1D0	// FIDO alliance HID usage page ... if used, data size must be 3 !
#define HID_USAGE_PAGE_VENDOR			0xFF00	// 0xFF00 - 0xFFFF ... if used, data size must be 3 !

// generic desktop page
#define HID_USAGE_DESKTOP_POINTER					0x01
#define HID_USAGE_DESKTOP_MOUSE						0x02
#define HID_USAGE_DESKTOP_JOYSTICK					0x04
#define HID_USAGE_DESKTOP_GAMEPAD					0x05
#define HID_USAGE_DESKTOP_KEYBOARD					0x06
#define HID_USAGE_DESKTOP_KEYPAD					0x07
#define HID_USAGE_DESKTOP_MULTI_AXIS_CONTROLLER		0x08
#define HID_USAGE_DESKTOP_TABLET_PC_SYSTEM			0x09
#define HID_USAGE_DESKTOP_X							0x30
#define HID_USAGE_DESKTOP_Y							0x31
#define HID_USAGE_DESKTOP_Z							0x32
#define HID_USAGE_DESKTOP_RX						0x33
#define HID_USAGE_DESKTOP_RY						0x34
#define HID_USAGE_DESKTOP_RZ						0x35
#define HID_USAGE_DESKTOP_SLIDER					0x36
#define HID_USAGE_DESKTOP_DIAL						0x37
#define HID_USAGE_DESKTOP_WHEEL						0x38
#define HID_USAGE_DESKTOP_HAT_SWITCH				0x39
#define HID_USAGE_DESKTOP_COUNTED_BUFFER			0x3A
#define HID_USAGE_DESKTOP_BYTE_COUNT				0x3B
#define HID_USAGE_DESKTOP_MOTION_WAKEUP				0x3C
#define HID_USAGE_DESKTOP_START						0x3D
#define HID_USAGE_DESKTOP_SELECT					0x3E
#define HID_USAGE_DESKTOP_VX						0x40
#define HID_USAGE_DESKTOP_VY						0x41
#define HID_USAGE_DESKTOP_VZ						0x42
#define HID_USAGE_DESKTOP_VBRX						0x43
#define HID_USAGE_DESKTOP_VBRY						0x44
#define HID_USAGE_DESKTOP_VBRZ						0x45
#define HID_USAGE_DESKTOP_VNO						0x46
#define HID_USAGE_DESKTOP_FEATURE_NOTIFICATION		0x47
#define HID_USAGE_DESKTOP_RESOLUTION_MULTIPLIER		0x48
#define HID_USAGE_DESKTOP_SYSTEM_CONTROL			0x80	 // if used, or all following codes, data size must be 2 !
#define HID_USAGE_DESKTOP_SYSTEM_POWER_DOWN			0x81
#define HID_USAGE_DESKTOP_SYSTEM_SLEEP				0x82
#define HID_USAGE_DESKTOP_SYSTEM_WAKE_UP			0x83
#define HID_USAGE_DESKTOP_SYSTEM_CONTEXT_MENU		0x84
#define HID_USAGE_DESKTOP_SYSTEM_MAIN_MENU			0x85
#define HID_USAGE_DESKTOP_SYSTEM_APP_MENU			0x86
#define HID_USAGE_DESKTOP_SYSTEM_MENU_HELP			0x87
#define HID_USAGE_DESKTOP_SYSTEM_MENU_EXIT			0x88
#define HID_USAGE_DESKTOP_SYSTEM_MENU_SELECT		0x89
#define HID_USAGE_DESKTOP_SYSTEM_MENU_RIGHT			0x8A
#define HID_USAGE_DESKTOP_SYSTEM_MENU_LEFT			0x8B
#define HID_USAGE_DESKTOP_SYSTEM_MENU_UP			0x8C
#define HID_USAGE_DESKTOP_SYSTEM_MENU_DOWN			0x8D
#define HID_USAGE_DESKTOP_SYSTEM_COLD_RESTART		0x8E
#define HID_USAGE_DESKTOP_SYSTEM_WARM_RESTART		0x8F
#define HID_USAGE_DESKTOP_DPAD_UP					0x90
#define HID_USAGE_DESKTOP_DPAD_DOWN					0x91
#define HID_USAGE_DESKTOP_DPAD_RIGHT				0x92
#define HID_USAGE_DESKTOP_DPAD_LEFT					0x93
#define HID_USAGE_DESKTOP_SYSTEM_DOCK				0xA0
#define HID_USAGE_DESKTOP_SYSTEM_UNDOCK				0xA1
#define HID_USAGE_DESKTOP_SYSTEM_SETUP				0xA2
#define HID_USAGE_DESKTOP_SYSTEM_BREAK				0xA3
#define HID_USAGE_DESKTOP_SYSTEM_DEBUGGER_BREAK		0xA4
#define HID_USAGE_DESKTOP_APPLICATION_BREAK			0xA5
#define HID_USAGE_DESKTOP_APPLICATION_DEBUGGER_BREAK 0xA6
#define HID_USAGE_DESKTOP_SYSTEM_SPEAKER_MUTE		0xA7
#define HID_USAGE_DESKTOP_SYSTEM_HIBERNATE			0xA8
#define HID_USAGE_DESKTOP_SYSTEM_DISPLAY_INVERT		0xB0
#define HID_USAGE_DESKTOP_SYSTEM_DISPLAY_INTERNAL	0xB1
#define HID_USAGE_DESKTOP_SYSTEM_DISPLAY_EXTERNAL	0xB2
#define HID_USAGE_DESKTOP_SYSTEM_DISPLAY_BOTH		0xB3
#define HID_USAGE_DESKTOP_SYSTEM_DISPLAY_DUAL		0xB4
#define HID_USAGE_DESKTOP_SYSTEM_DISPLAY_TOGGLE_INT_EXT	0xB5
#define HID_USAGE_DESKTOP_SYSTEM_DISPLAY_SWAP_PRIMARY_SECONDARY	0xB6
#define HID_USAGE_DESKTOP_SYSTEM_DISPLAY_LCD_AUTOSCALE	0xB7
#define HID_USAGE_DESKTOP_THROTTLE					0xBB

// consumer page (part)
// - Generic Control
#define HID_USAGE_CONSUMER_CONTROL					0x0001
// - Power Control
#define HID_USAGE_CONSUMER_POWER					0x0030
#define HID_USAGE_CONSUMER_RESET					0x0031
#define HID_USAGE_CONSUMER_SLEEP					0x0032
// - Screen Brightness
#define HID_USAGE_CONSUMER_BRIGHTNESS_INCREMENT		0x006F
#define HID_USAGE_CONSUMER_BRIGHTNESS_DECREMENT		0x0070
// - Mobile systems with Windows 8
#define HID_USAGE_CONSUMER_WIRELESS_RADIO_CONTROLS	0x000C
#define HID_USAGE_CONSUMER_WIRELESS_RADIO_BUTTONS	0x00C6	 // if used, or all following codes, data size must be 2 !
#define HID_USAGE_CONSUMER_WIRELESS_RADIO_LED		0x00C7
#define HID_USAGE_CONSUMER_WIRELESS_RADIO_SLIDER_SWITCH 0x00C8
// - Media Control
#define HID_USAGE_CONSUMER_PLAY_PAUSE				0x00CD
#define HID_USAGE_CONSUMER_SCAN_NEXT				0x00B5
#define HID_USAGE_CONSUMER_SCAN_PREVIOUS			0x00B6
#define HID_USAGE_CONSUMER_STOP						0x00B7
#define HID_USAGE_CONSUMER_VOLUME					0x00E0
#define HID_USAGE_CONSUMER_MUTE						0x00E2
#define HID_USAGE_CONSUMER_BASS						0x00E3
#define HID_USAGE_CONSUMER_TREBLE					0x00E4
#define HID_USAGE_CONSUMER_BASS_BOOST				0x00E5
#define HID_USAGE_CONSUMER_VOLUME_INCREMENT			0x00E9
#define HID_USAGE_CONSUMER_VOLUME_DECREMENT			0x00EA
#define HID_USAGE_CONSUMER_BASS_INCREMENT			0x0152
#define HID_USAGE_CONSUMER_BASS_DECREMENT			0x0153
#define HID_USAGE_CONSUMER_TREBLE_INCREMENT			0x0154
#define HID_USAGE_CONSUMER_TREBLE_DECREMENT			0x0155
// - Application Launcher
#define HID_USAGE_CONSUMER_AL_CONSUMER_CONTROL_CONFIGURATION 0x0183
#define HID_USAGE_CONSUMER_AL_EMAIL_READER			0x018A
#define HID_USAGE_CONSUMER_AL_CALCULATOR			0x0192
#define HID_USAGE_CONSUMER_AL_LOCAL_BROWSER			0x0194
// - Browser/Explorer Specific
#define HID_USAGE_CONSUMER_AC_SEARCH				0x0221
#define HID_USAGE_CONSUMER_AC_HOME					0x0223
#define HID_USAGE_CONSUMER_AC_BACK					0x0224
#define HID_USAGE_CONSUMER_AC_FORWARD				0x0225
#define HID_USAGE_CONSUMER_AC_STOP					0x0226
#define HID_USAGE_CONSUMER_AC_REFRESH				0x0227
#define HID_USAGE_CONSUMER_AC_BOOKMARKS				0x022A
// - Mouse Horizontal scroll
#define HID_USAGE_CONSUMER_AC_PAN					0x0238

// FIDO alliance page (0xF1D0)
#define HID_USAGE_FIDO_U2FHID		0x01	// U2FHID usage for top-level collection
#define HID_USAGE_FIDO_DATA_IN		0x20	// Raw IN data report
#define HID_USAGE_FIDO_DATA_OUT		0x21	// Raw OUT data report
