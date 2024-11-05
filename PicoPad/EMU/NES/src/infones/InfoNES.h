/*===================================================================*/
/*                                                                   */
/*  InfoNES.h : NES Emulator for Win32, Linux(x86), Linux(PS2)       */
/*                                                                   */
/*  2000/05/14  InfoNES Project ( based on pNesX )                   */
/*                                                                   */
/*===================================================================*/

#ifndef InfoNES_H_INCLUDED
#define InfoNES_H_INCLUDED

extern u32 NES_OldSyncTime; // old time of HSYNC

/* ROM */
extern BYTE *ROM;
extern int NES_ROMSize; // ROM size (bytes)

#define NAME_TABLE0 8
#define NAME_TABLE1 9
#define NAME_TABLE2 10
#define NAME_TABLE3 11

#define NAME_TABLE_V_MASK 2
#define NAME_TABLE_H_MASK 1

#define SPR_Y 0
#define SPR_CHR 1
#define SPR_ATTR 2
#define SPR_X 3
#define SPR_ATTR_COLOR 0x3
#define SPR_ATTR_V_FLIP 0x80
#define SPR_ATTR_H_FLIP 0x40
#define SPR_ATTR_PRI 0x20

/* PPU Register */
#define R0_NMI_VB 0x80
#define R0_NMI_SP 0x40
#define R0_SP_SIZE 0x20
#define R0_BG_ADDR 0x10
#define R0_SP_ADDR 0x08
#define R0_INC_ADDR 0x04
#define R0_NAME_ADDR 0x03

#define R1_BACKCOLOR 0xe0
#define R1_SHOW_SP 0x10
#define R1_SHOW_SCR 0x08
#define R1_CLIP_SP 0x04
#define R1_CLIP_BG 0x02
#define R1_MONOCHROME 0x01

#define R2_IN_VBLANK 0x80
#define R2_HIT_SP 0x40
#define R2_MAX_SP 0x20
#define R2_WRITE_FLAG 0x10

#define SCAN_TOP_OFF_SCREEN 0
#define SCAN_ON_SCREEN 1
#define SCAN_BOTTOM_OFF_SCREEN 2

/* Develop Scroll Registers */
#define InfoNES_SetupScr()

/* Current Scanline */
extern WORD PPU_Scanline;

//extern BYTE ChrBuf[];
#define ChrBuf ((u8*)NULL)

/*-------------------------------------------------------------------*/
/*  APU and Pad resources                                            */
/*-------------------------------------------------------------------*/

//extern BYTE APU_Reg[];
//extern int APU_Mute;

extern DWORD PAD1_Latch;
extern DWORD PAD2_Latch;
extern DWORD PAD_System;
extern DWORD PAD1_Bit;
extern DWORD PAD2_Bit;

#define PAD_SYS_QUIT 1
#define PAD_SYS_OK 2
#define PAD_SYS_CANCEL 4
#define PAD_SYS_UP 8
#define PAD_SYS_DOWN 0x10
#define PAD_SYS_LEFT 0x20
#define PAD_SYS_RIGHT 0x40

#define PAD_PUSH(a, b) (((a) & (b)) != 0)

/*-------------------------------------------------------------------*/
/*  Mapper Function                                                  */
/*-------------------------------------------------------------------*/

/* Initialize Mapper */
extern void (*MapperInit)();
/* Write to Mapper */
extern void (*MapperWrite)(WORD wAddr, BYTE byData);
/* Read from Mapper */
extern void (*MapperRead)(WORD wAddr, BYTE byData);
/* Write to SRAM */
extern void (*MapperSram)(WORD wAddr, BYTE byData);
/* Write to APU */
extern void (*MapperApu)(WORD wAddr, BYTE byData);
/* Read from Apu */
extern BYTE (*MapperReadApu)(WORD wAddr);
/* Callback at VSync */
extern void (*MapperVSync)();
/* Callback at HSync */
extern void (*MapperHSync)();
/* Callback at PPU read/write */
extern void (*MapperPPU)(WORD wAddr);
/* Callback at Rendering Screen 1:BG, 0:Sprite */
extern void (*MapperRenderScreen)(BYTE byMode);

/*-------------------------------------------------------------------*/
/*  ROM information                                                  */
/*-------------------------------------------------------------------*/

/* .nes File Header */
struct NesHeader_tag
{
  BYTE byID[4];
  BYTE byRomSize;
  BYTE byVRomSize;
  BYTE byInfo1;
  BYTE byInfo2;
  BYTE byReserve[8];
};

/* .nes File Header */
extern struct NesHeader_tag NesHeader;

// NES header format: 0=NES0, 1=NES1, 2=NES2
extern BYTE NesFormat;

/* Mapper No. */
extern BYTE MapperNo;
extern BYTE SubMapperNo;

/* Other */
extern BYTE ROM_Mirroring; // MIRROR_H or MIRROR_V
extern BYTE ROM_SRAM;
extern BYTE ROM_Trainer;
extern BYTE ROM_FourScr;

// mirroring (on change, update PPU_MirrorTable)
#define MIRROR_H	0	// horizontal
#define MIRROR_V	1	// vertical
#define MIRROR_1	2	// one screen 0x2400
#define MIRROR_0	3	// one screen 0x2000
#define MIRROR_4	4	// four screen
#define MIRROR_0001	5	// special (Mapper #233) 0-0-0-1
#define MIRROR_0111	6	// special (Mapper #150) 0-1-1-1

/*-------------------------------------------------------------------*/
/*  Function prototypes                                              */
/*-------------------------------------------------------------------*/

/* Initialize InfoNES */
void InfoNES_Init();

/* Load a cassette */
int InfoNES_Load(const char *pszFileName);

/* Reset InfoNES */
int InfoNES_Reset();

/* Initialize PPU */
void InfoNES_SetupPPU();

/* Set up a Mirroring of Name Table MIRROR_* */
void FASTCODE NOFLASH(InfoNES_Mirroring)(int nType);

/* The loop of emulation */
void FASTCODE NOFLASH(InfoNES_Cycle)();

/* A function in H-Sync */
void FASTCODE NOFLASH(InfoNES_HSync)();

/* Render a scanline */
void FASTCODE NOFLASH(InfoNES_DrawLine)();

/* Get a position of scanline hits sprite #0 */
void FASTCODE NOFLASH(InfoNES_GetSprHitY)();

/* Develop character data */
//void InfoNES_SetupChr();
INLINE void InfoNES_SetupChr() {}

extern u8 *WorkLine;

#endif /* !InfoNES_H_INCLUDED */