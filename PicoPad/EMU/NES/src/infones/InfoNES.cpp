/*===================================================================*/
/*===================================================================*/
/*                                                                   */
/*  InfoNES.cpp : NES Emulator for Win32, Linux(x86), Linux(PS2)     */
/*                                                                   */
/*  2000/05/18  InfoNES Project ( based on pNesX )                   */
/*                                                                   */
/*===================================================================*/

#include "../../include.h"

/* ROM */
BYTE *ROM;
int NES_ROMSize; // ROM size (bytes)

u32 NES_OldSyncTime; // old time of HSYNC

/* Current Scanline */
WORD PPU_Scanline;

/* Display Buffer */
u8 *WorkLine;

/* Table for Mirroring */
BYTE PPU_MirrorTable[][4] =
{
        {NAME_TABLE0, NAME_TABLE0, NAME_TABLE1, NAME_TABLE1},	// 0: MIRROR_H horizontal
        {NAME_TABLE0, NAME_TABLE1, NAME_TABLE0, NAME_TABLE1},	// 1: MIRROR_V vertical
        {NAME_TABLE1, NAME_TABLE1, NAME_TABLE1, NAME_TABLE1},	// 2: MIRROR_1 one screen 0x2400
        {NAME_TABLE0, NAME_TABLE0, NAME_TABLE0, NAME_TABLE0},	// 3: MIRROR_0 one screen 0x2000
        {NAME_TABLE0, NAME_TABLE1, NAME_TABLE2, NAME_TABLE3},	// 4: MIRROR_4 four screen
        {NAME_TABLE0, NAME_TABLE0, NAME_TABLE0, NAME_TABLE1},	// 5: MIRROR_0001 special (Mapper #233)
        {NAME_TABLE0, NAME_TABLE1, NAME_TABLE1, NAME_TABLE1},	// 6: MIRROR_0111 special (Mapper #150)
};

/* Pad data */
DWORD PAD1_Latch;
DWORD PAD2_Latch;
DWORD PAD_System;
DWORD PAD1_Bit;
DWORD PAD2_Bit;

/*-------------------------------------------------------------------*/
/*  Mapper Function                                                  */
/*-------------------------------------------------------------------*/

/* Initialize Mapper */
void (*MapperInit)();
/* Write to Mapper */
void (*MapperWrite)(WORD wAddr, BYTE byData);
/* Read from Mapper */
void (*MapperRead)(WORD wAddr, BYTE byData);
/* Write to SRAM */
void (*MapperSram)(WORD wAddr, BYTE byData);
/* Write to Apu */
void (*MapperApu)(WORD wAddr, BYTE byData);
/* Read from Apu */
BYTE(*MapperReadApu)(WORD wAddr);
/* Callback at VSync */
void (*MapperVSync)();
/* Callback at HSync */
void (*MapperHSync)();
/* Callback at PPU read/write */
void (*MapperPPU)(WORD wAddr); // mapper 96
/* Callback at Rendering Screen 1:BG, 0:Sprite */
void (*MapperRenderScreen)(BYTE byMode);

/*-------------------------------------------------------------------*/
/*  ROM information                                                  */
/*-------------------------------------------------------------------*/

/* .nes File Header */
struct NesHeader_tag NesHeader;

// NES header format: 0=NES0, 1=NES1, 2=NES2
BYTE NesFormat;

/* Mapper Number */
BYTE MapperNo;
BYTE SubMapperNo;

/* Mirroring MIRROR_H or MIRROR_V */
BYTE ROM_Mirroring;
/* It has SRAM */
BYTE ROM_SRAM;
/* It has Trainer */
BYTE ROM_Trainer;
/* Four screen VRAM  */
BYTE ROM_FourScr;

/*===================================================================*/
/*                                                                   */
/*                InfoNES_Init() : Initialize InfoNES                */
/*                                                                   */
/*===================================================================*/
void InfoNES_Init()
{
  // Initialize 6502
  K6502_Init();
}

/*===================================================================*/
/*                                                                   */
/*                  InfoNES_Load() : Load a cassette                 */
/*                                                                   */
/*===================================================================*/
// returns -1 on error, or 0 on success
int InfoNES_Load(const char *pszFileName)
{
  // Reset InfoNES
  if (InfoNES_Reset() < 0) return -1;

  // Successful
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*                 InfoNES_Reset() : Reset InfoNES                   */
/*                                                                   */
/*===================================================================*/
// returns -1 on non supported mapper, or 0 on success
int InfoNES_Reset()
{
  int nIdx;

  /*-------------------------------------------------------------------*/
  /*  Get information on the cassette                                  */
  /*-------------------------------------------------------------------*/

  // get mapper number NES0
#ifdef NES_MAPPERNO	// if defined, use custom mapper number
  MapperNo = NES_MAPPERNO;
#else // NES_MAPPERNO
  MapperNo = NesHeader.byInfo1 >> 4;
  if (NesFormat == 2) // NES2 extension
    MapperNo |= (NesHeader.byInfo2 & 0xf0) | ((NesHeader.byReserve[0] & 0x0f) << 8);
  else if (NesFormat == 1) // NES1 extension
    MapperNo |= (NesHeader.byInfo2 & 0xf0);
#endif // NES_MAPPERNO

  // sub-mapper
#ifdef NES_SUBMAPPER	// use submapper 0, 1.. (0 is default)
  SubMapperNo = NES_SUBMAPPER;
#else
  SubMapperNo = 0;
#endif

#ifdef DEB_REMAP			// Debug - enable mapper number incrementing using B key in menu
  if (ReMapperNo >= 0)
  {
    do {
      MapperNo = (BYTE)(ReMapperNo + 1);
      ReMapperNo = MapperNo;
      for (nIdx = 0; MapperTable[nIdx].nMapperNo != -1; ++nIdx)
      {
        if (MapperTable[nIdx].nMapperNo == MapperNo) break;
      }
    } while (MapperTable[nIdx].nMapperNo == -1);
  }
  ReMapperNo = MapperNo;
#endif

  // Get information on the ROM
#ifdef NES_MIRRORING	// use custom mirroring: 0=horiz, 1=vert, 2=2400, 3=2000, 4=four, 5=0001, 6=0111
  ROM_Mirroring = NES_MIRRORING;
  ROM_FourScr = (ROM_Mirroring == MIRROR_4);
#else
  ROM_Mirroring = (NesHeader.byInfo1 & 1) ? MIRROR_V : MIRROR_H; // mirroring
  ROM_FourScr = NesHeader.byInfo1 & 8;
#endif

  ROM_SRAM = NesHeader.byInfo1 & 2;
  ROM_Trainer = NesHeader.byInfo1 & 4;

  /*-------------------------------------------------------------------*/
  /*  Initialize resources                                             */
  /*-------------------------------------------------------------------*/

  // Clear RAM
  InfoNES_MemorySet(RAM, 0, WRAM_SIZE);

  // Reset palette table
  InfoNES_MemorySet(PalTable, 0, sizeof PalTable);

  // Reset joypad
  PAD1_Latch = PAD2_Latch = PAD_System = 0;
  PAD1_Bit = PAD2_Bit = 0;

  /*-------------------------------------------------------------------*/
  /*  Initialize PPU                                                   */
  /*-------------------------------------------------------------------*/

  InfoNES_SetupPPU();

  NES_OldSyncTime = Time();

  /*-------------------------------------------------------------------*/
  /*  Initialize pAPU                                                  */
  /*-------------------------------------------------------------------*/

  InfoNES_ApuInit();

  /*-------------------------------------------------------------------*/
  /*  Initialize Mapper                                                */
  /*-------------------------------------------------------------------*/

  // Get Mapper Table Index
  for (nIdx = 0; MapperTable[nIdx].nMapperNo != -1; ++nIdx)
  {
    if (MapperTable[nIdx].nMapperNo == MapperNo) break;
  }

  // Non support mapper
  if (MapperTable[nIdx].nMapperNo == -1) return -1;

  // Set up a mapper initialization function
  NES_XRamSize = 0; // default size of extra RAM
  PPU_hook = NULL; // PPU hook - access to PPU memory 0x0000 - 0x3FFF (NULL = not used)
  MapperRead = NULL;
  MapperTable[nIdx].pMapperInit();

  /*-------------------------------------------------------------------*/
  /*  Reset CPU                                                        */
  /*-------------------------------------------------------------------*/

  K6502_Reset();

  // Successful
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*                InfoNES_SetupPPU() : Initialize PPU                */
/*                                                                   */
/*===================================================================*/
void InfoNES_SetupPPU()
{
  int nPage;

  // Clear PPU and Sprite Memory
  InfoNES_MemorySet(PPURAM, 0, PPURAM_SIZE);
  InfoNES_MemorySet(SPRRAM, 0, SPRRAM_SIZE);

  // Reset PPU Register
  PPU_R0 = PPU_R1 = PPU_R2 = PPU_R3 = PPU_R7 = 0;

  // Reset latch flag
  PPU_Latch_Flag = 0;

  // Reset up and down clipping flag
  PPU_UpDown_Clip = 0;

  FrameStep = 0;
  FrameIRQ_Enable = 0;

  // Reset Scroll values
  PPU_Scr_H_Byte = PPU_Scr_H_Bit = 0;

  // Reset PPU address
  PPU_Addr = 0;		// current address in PPU video memory (0x0000-0x3FFF)
  PPU_Temp = 0;		// nametable address 0xF3FF, 0xF7FF, 0xFBFF, 0xFFFF

  // Reset scanline
  PPU_Scanline = 0;

  // Reset hit position of sprite #0
  SpriteJustHit = 0;

  // Reset information on PPU_R0	// PPUCTRL
  PPU_Increment = 1;			// VRAM address increment 1 or 32
  PPU_NameTableBank = NAME_TABLE0;	// base index of nametable bank (8=0x2000, 9=0x2400, 10=0x2800, 11=0x2C00)
  PPU_BG_Base = ChrBuf;			// background base 0x0000 or 0x4000
  PPU_SP_Base = ChrBuf + 256 * 64;	// sprite base 0x0000 or 0x4000
  PPU_SP_Height = 8;			// sprite height 8 or 16

  // Reset PPU banks
  for (nPage = 0; nPage < 16; ++nPage)
    // u8* PPUBANK[16] = pointers to PPU banks (...0x2000, 0x2400, 0x2800, 0x2C00...)
    PPUBANK[nPage] = &PPURAM[nPage * 0x400];

  /* Mirroring of Name Table MIRROR_* */
  InfoNES_Mirroring(ROM_Mirroring);

  /* Reset VRAM Write Enable */
  byVramWriteEnable = (NesHeader.byVRomSize == 0) ? 1 : 0;
}

/*===================================================================*/
/*                                                                   */
/*       InfoNES_Mirroring() : Set up a Mirroring of Name Table      */
/*                                                                   */
/*===================================================================*/
// nType = mirroring type MIRROR_*
void FASTCODE NOFLASH(InfoNES_Mirroring)(int nType)
{
  // u8* PPUBANK[16] = pointers to PPU banks (...0x2000, 0x2400, 0x2800, 0x2C00...)
  PPUBANK[NAME_TABLE0] = &PPURAM[PPU_MirrorTable[nType][0] * 0x400];
  PPUBANK[NAME_TABLE1] = &PPURAM[PPU_MirrorTable[nType][1] * 0x400];
  PPUBANK[NAME_TABLE2] = &PPURAM[PPU_MirrorTable[nType][2] * 0x400];
  PPUBANK[NAME_TABLE3] = &PPURAM[PPU_MirrorTable[nType][3] * 0x400];
}

/*===================================================================*/
/*                                                                   */
/*              InfoNES_Cycle() : The loop of emulation              */
/*                                                                   */
/*===================================================================*/

int ScanlineClockH = 0;	// scanline clock HighRes

// - one frame
void FASTCODE NOFLASH(InfoNES_Cycle)()
{
  // Emulation loop
  do
  {
    // get HighRes scanline clocks
    int clkh = ScanlineClockH; // scanline clock HighRes
    clkh += STEP_PER_SCANLINE_H; // increase scanline clocks
    int clk = clkh >> STEP_PER_SCANLINE_SHIFT; // clock LowRes
    clkh -= clk << STEP_PER_SCANLINE_SHIFT; // subtract clock LowRes
    ScanlineClockH = clkh;

    // Set a flag if a scanning line is a hit in the sprite #0
    if ((SpriteJustHit == PPU_Scanline) && (PPU_Scanline < SCAN_UNKNOWN_START))
    {
      // # of Steps to execute before sprite #0 hit
      // - NES_HIT_SHIFT correction (recommended 30):
      //      3 - 100 to avoid flickering in "Race America" (line in intro image, line under the helicopter at start)
      //      15 - 100 to avoid flickering in "Saint Seiya" (line above menu in game and in level selection)
      //      28 - 33 to avoid flickering in "Advanced Dungeons & Dragons - Dragon Strike" (line in intro image)
      int nStep = (SPRRAM[SPR_X]*STEP_PER_SCANLINE_H)/(NES_DISP_WIDTH<<STEP_PER_SCANLINE_SHIFT) + NES_HIT_SHIFT;
      if (nStep < 0) nStep = 0;
      if (nStep > clk) nStep = clk;

      // Execute instructions
      K6502_Step(nStep);

      // Set a sprite hit flag
      if ((PPU_R1 & R1_SHOW_SP) && (PPU_R1 & R1_SHOW_SCR))	// PPUMASK
          PPU_R2 |= R2_HIT_SP;					// PPUSTATUS

      // NMI is required if there is necessity
#ifndef NES_NOIRQ_HIT0		// if defined, disable IRQ on sprite #0 hit
      if ((PPU_R0 & R0_NMI_SP) && (PPU_R1 & R1_SHOW_SP))	// PPUCTRL	// PPUMASK
          NMI_REQ;
#endif

      // Execute rest of instructions
      K6502_Step(clk - nStep);
    }
    else
    {
      // Execute instructions
      K6502_Step(clk);
    }

    // shift APU frame counter
    FrameStep += clk;

    // prepare length of APU frame
    int framelen = NES_ApuFrameLen(APU);

#ifdef NES_FRAMELEN_ADD		// if defined, add to APU frame length (base value is 29824 or 37280)
    framelen += NES_FRAMELEN_ADD;
#endif

    // check APU frame counter end
    if (FrameStep >= framelen)
    {
         FrameStep -= framelen;
         if (FrameIRQ_Enable)
         {
#ifndef NES_NOIRQ_FRAME		// if defined, disable IRQ on APU frame
            IRQ_REQ;
#endif
            APU_Reg[0x15] |= 0x40;
         }
    }

    // A mapper function in H-Sync
    MapperHSync();

    // A function in H-Sync
    InfoNES_HSync();

    // HSYNC Wait
    u32 dif;
    for (;;)
    {
       dif = Time() - NES_OldSyncTime;
       if (dif >= NES_HSYNCTIME) break;
    }

    if (dif > 2000*NES_HSYNCTIME)
      NES_OldSyncTime = Time();
    else
      NES_OldSyncTime += NES_HSYNCTIME;

    // end of frame
  } while (PPU_Scanline != 0);
}

/*===================================================================*/
/*                                                                   */
/*              InfoNES_HSync() : A function in H-Sync               */
/*                                                                   */
/*===================================================================*/
void FASTCODE NOFLASH(InfoNES_HSync)()
{
  // PPU_Scr_H_Byte horizontal byte index X of PPU address (0..31)
  PPU_Scr_H_Byte = PPU_Addr & 31;	// horizontal index X of PPU address (0..31)

  // index of name table bank (8=0x2000, 9=0x2400, 10=0x2800, 11=0x2C00)
  PPU_NameTableBank = NAME_TABLE0 + ((PPU_Addr >> 10) & 3);	// base nametable bank (8=0x2000, 9=0x2400, 10=0x2800, 11=0x2C00)

  /*-------------------------------------------------------------------*/
  /*  Render a scanline                                                */
  /*-------------------------------------------------------------------*/

  // current scanline to render
  int y = PPU_Scanline;

  // valid scanline?
  if ((y < NES_DISP_HEIGHT) && (NES_DispMap[y] == 0))	// check scanline map
  {
    // set scanline map
    NES_DispMap[y] = 1;

    // destination buffer
    WorkLine = &NES->framebuf[y*NES_DISP_WIDTH]; // prepare pointer to the line in frame buffer

    // render line
    InfoNES_DrawLine();
  }

  /*-------------------------------------------------------------------*/
  /*  Set new scroll values                                            */
  /*-------------------------------------------------------------------*/

  // if show sprites or show background
  if ((PPU_R1 & R1_SHOW_SP) || (PPU_R1 & R1_SHOW_SCR))		// PPUMASK
  {
    // end of V-blank, pre-render scanline
    if (PPU_Scanline == SCAN_VBLANK_END)
    {
      // PPU_Addr = current address in PPU video memory (0x0000-0x3FFF)
      PPU_Addr = PPU_Temp;		// nametable address 0xF3FF, 0xF7FF, 0xFBFF, 0xFFFF
    }

    // valid image
    else if (PPU_Scanline < SCAN_UNKNOWN_START)
    {
      // PPU_Addr = current address in PPU video memory (0x0000-0x3FFF)
      PPU_Addr = (PPU_Addr & ~0b10000011111) |
                 (PPU_Temp & 0b10000011111);		// nametable address 0xF3FF, 0xF7FF, 0xFBFF, 0xFFFF

      int v = (PPU_Addr >> 12) | ((PPU_Addr >> 2) & (31 << 3));
      if (v == 29 * 8 + 7)
      {
        v = 0;
        PPU_Addr ^= 0x800;
      }
      else if (v == 31 * 8 + 7)
      {
        v = 0;
      }
      else
        ++v;
      PPU_Addr = (PPU_Addr & ~0b111001111100000) |
                 ((v & 7) << 12) | (((v >> 3) & 31) << 5);
    }
  }

  /*-------------------------------------------------------------------*/
  /*  Next Scanline                                                    */
  /*-------------------------------------------------------------------*/

  PPU_Scanline = (PPU_Scanline == SCAN_VBLANK_END) ? 0 : (PPU_Scanline + 1);

  /*-------------------------------------------------------------------*/
  /*  Operation in the specific scanning line                          */
  /*-------------------------------------------------------------------*/
  switch (PPU_Scanline)
  {
  // start of image (scanline 0)
  case SCAN_TOP_OFF_SCREEN:
    // Reset a PPU status
    PPU_R2 = 0;		// PPUSTATUS

    // Get position of sprite #0
    InfoNES_GetSprHitY();
    break;

  // end of image, post-render blanking line
//#if SCAN_UNKNOWN_START != SCAN_VBLANK_START
//  case SCAN_UNKNOWN_START:
//    break;
//#endif

  // start of V-blank, send NMI IRQ
  case SCAN_VBLANK_START:

    // Set a V-Blank flag
    PPU_R2 |= R2_IN_VBLANK;		// PPUSTATUS

    // A mapper function in V-Sync
    MapperVSync();

    // Get the condition of the joypad
    InfoNES_PadState(&PAD1_Latch, &PAD2_Latch, &PAD_System);

#ifndef NES_NOIRQ_VBLANK	// if defined, disable IRQ on V-blank
    // NMI on V-Blank
    if (PPU_R0 & R0_NMI_VB)	// PPUCTRL
    {
      NMI_REQ;
    }
#endif
    break;
  }
}

namespace
{
  void FASTCODE NOFLASH(compositeSprite)(const u8 *pal,
                                            const u8 *spr,
                                            u8 *buf)
  {
    auto sprEnd = spr + NES_DISP_WIDTH;
    do
    {
      auto proc = [=](int i) __attribute__((always_inline))
      {
        int v = spr[i];
        //if (v && ((v >> 7) || (buf[i] >> 15))   
        // Are alpha bits zero? (set in K6502_rw.h)
        if (v && ((v >> 7) || !(buf[i] & 0x80)))
        {
          buf[i] = pal[v & 0xf];
        }
        // set alpha bits back to 1
        buf[i] |= 0x80;
      };

      proc(0);
      proc(1);
      proc(2);
      proc(3);
      buf += 4;
      spr += 4;
    } while (spr < sprEnd);
  }
}

/*===================================================================*/
/*                                                                   */
/*              InfoNES_DrawLine() : Render a scanline               */
/*                                                                   */
/*===================================================================*/
void FASTCODE NOFLASH(InfoNES_DrawLine)()
{
  int nX;
  int nY;
  int nY4;
  int nYBit;
  WORD *pPalTbl;
  BYTE *pAttrBase;
  u8 *pPoint;
  int nNameTable;	// index of name table bank (8=0x2000, 9=0x2400, 10=0x2800, 11=0x2C00)
  BYTE *pbyNameTable;	// pointer into name table bank
  BYTE *pbyChrData;
  BYTE *pSPRRAM;
  int nAttr;
  int nSprCnt;
  int nIdx;
  int nSprData;
  BYTE bySprCol;
  BYTE pSprBuf[NES_DISP_WIDTH + 7];
  BYTE pDispBuf[NES_DISP_WIDTH + 16];

  /*-------------------------------------------------------------------*/
  /*  Render Background                                                */
  /*-------------------------------------------------------------------*/

  /* MMC5 VROM switch */
  MapperRenderScreen(1);

  // Pointer to the render position
  //  pPoint = &WorkFrame[PPU_Scanline * NES_DISP_WIDTH];
  assert(WorkLine);
  pPoint = pDispBuf + 8; //WorkLine;

  // Clear a scanline if screen is off
  if (!(PPU_R1 & R1_SHOW_SCR)) // show background		// PPUMASK
  {
    InfoNES_MemorySet(pPoint, NES_BLACK, NES_DISP_WIDTH);
  }
  else
  {
    // index of name table bank (8=0x2000, 9=0x2400, 10=0x2800, 11=0x2C00)
    nNameTable = PPU_NameTableBank;	// base nametable bank (8=0x2000, 9=0x2400, 10=0x2800, 11=0x2C00)

    // PPU_Addr = current address in PPU video memory (0x0000-0x3FFF)
    nY = (PPU_Addr >> 5) & 31; // // vertical index Y of PPU address (0..31)
    const int yOfsModBG = PPU_Addr >> 12;
    nYBit = yOfsModBG << 3;

    // horizontal byte index X of PPU address (0..31)
    nX = PPU_Scr_H_Byte; // PPU_Scr_H_Byte horizontal byte index X of PPU address (0..31)

    nY4 = ((nY & 2) << 1);

    // background pattern table address 0=0x0000, 1=0x1000
    const int patternTableIdBG = PPU_R0 & R0_BG_ADDR ? 1 : 0;	// PPUCTRL

    // background pattern table offset 0=0x0000, 4=0x1000
    const int bankOfsBG = patternTableIdBG << 2;

    /*-------------------------------------------------------------------*/
    /*  Rendering of the block of the left end                           */
    /*-------------------------------------------------------------------*/

    // prepare pointer into name table
    //   nNameTable = index of name table bank (8=0x2000, 9=0x2400, 10=0x2800, 11=0x2C00)
    //   u8* PPUBANK[16] = pointers to PPU banks (...0x2000, 0x2400, 0x2800, 0x2C00...)
    pbyNameTable = PPUBANK[nNameTable] + nY*32 + nX; // pointer into name table bank

    pbyChrData = PPU_BG_Base + (*pbyNameTable << 6) + nYBit;	// BG_Base = background base 0x0000 or 0x4000

    // prepare pointer into attributes
    //   nNameTable = index of name table bank (8=0x2000, 9=0x2400, 10=0x2800, 11=0x2C00)
    //   u8* PPUBANK[16] = pointers to PPU banks (...0x2000, 0x2400, 0x2800, 0x2C00...)
    pAttrBase = PPUBANK[nNameTable] + 0x3c0 + (nY / 4) * 8;

    {
      // shift to end of first tile
      //   PPU_Scr_H_Bit = bit index X in byte of PPU scroll address (0..7)
      pPoint += 8 - PPU_Scr_H_Bit;

      // palette
      const u8* pal = &PalTable[(((pAttrBase[nX >> 2] >> ((nX & 2) + nY4)) & 3) << 2)];
      const int ch = *pbyNameTable; // read tile index from name table
      const int bank = (ch >> 6) + bankOfsBG; // bankOfsBG = background pattern table offset 0=0x0000, 4=0x1000
      const int addrOfs = ((ch & 63) << 4) + yOfsModBG; // offset of tile pattern
      const u8* data = PPUBANK[bank] + addrOfs; // pointer to tile data
      const auto pl0 = data[0]; // 
      const auto pl1 = data[8];
      const auto pat0 = (pl0 & 0x55) | ((pl1 << 1) & 0xaa);
      const auto pat1 = ((pl0 >> 1) & 0x55) | (pl1 & 0xaa);

      // PPU_Scr_H_Bit = bit index X in byte of PPU scroll address (0..7)
      switch (PPU_Scr_H_Bit)
      {
      case 0:
        pPoint[-8] = pal[(pat1 >> 6) & 3];
      case 1:
        pPoint[-7] = pal[(pat0 >> 6) & 3];
      case 2:
        pPoint[-6] = pal[(pat1 >> 4) & 3];
      case 3:
        pPoint[-5] = pal[(pat0 >> 4) & 3];
      case 4:
        pPoint[-4] = pal[(pat1 >> 2) & 3];
      case 5:
        pPoint[-3] = pal[(pat0 >> 2) & 3];
      case 6:
        pPoint[-2] = pal[(pat1 >> 0) & 3];
      case 7:
        pPoint[-1] = pal[(pat0 >> 0) & 3];
      default:
        break;
      }
    }

    // Callback at PPU read/write
    MapperPPU(PATTBL(pbyChrData));

    ++nX;
    ++pbyNameTable; // pointer into name table bank

    /*-------------------------------------------------------------------*/
    /*  Rendering of the left table                                      */
    /*-------------------------------------------------------------------*/

    auto putBG = [&](int nX) __attribute__((always_inline))
    {
      const u8* pal = &PalTable[(((pAttrBase[nX >> 2] >> ((nX & 2) + nY4)) & 3) << 2)];
      //const auto palAddr = reinterpret_cast<uintptr_t>(pal);
      const int ch = *pbyNameTable;
      const int bank = (ch >> 6) + bankOfsBG; // bankOfsBG = background pattern table offset 0=0x0000, 4=0x1000
      const int addrOfs = ((ch & 63) << 4) + yOfsModBG;
      const auto data = PPUBANK[bank] + addrOfs;
      const auto pl0 = data[0];
      const auto pl1 = data[8];

      const auto pat0 = (pl0 & 0x55) | ((pl1 & 0x55) << 1);
      const auto pat1 = ((pl0 & 0xaa) >> 1) | (pl1 & 0xaa);
      pPoint[0] = pal[(pat1 >> 6) & 3];
      pPoint[1] = pal[(pat0 >> 6) & 3];
      pPoint[2] = pal[(pat1 >> 4) & 3];
      pPoint[3] = pal[(pat0 >> 4) & 3];
      pPoint[4] = pal[(pat1 >> 2) & 3];
      pPoint[5] = pal[(pat0 >> 2) & 3];
      pPoint[6] = pal[(pat1 >> 0) & 3];
      pPoint[7] = pal[(pat0 >> 0) & 3];
      pPoint += 8;
    };

    for (; nX < 32; ++nX)
    {
      putBG(nX);

      // Callback at PPU read/write
      MapperPPU(PATTBL(pbyChrData));

      ++pbyNameTable; // increase pointer into name table bank
    }

    // Horizontal Mirror
    //   nNameTable = index of name table bank (8=0x2000, 9=0x2400, 10=0x2800, 11=0x2C00)
    nNameTable ^= NAME_TABLE_H_MASK;

    // u8* PPUBANK[16] = pointers to PPU banks (...0x2000, 0x2400, 0x2800, 0x2C00...)
    pbyNameTable = PPUBANK[nNameTable] + nY*32; // pointer into name table bank
    pAttrBase = PPUBANK[nNameTable] + 0x3c0 + (nY / 4) * 8;

    /*-------------------------------------------------------------------*/
    /*  Rendering of the right table                                     */
    /*-------------------------------------------------------------------*/

    // PPU_Scr_H_Byte horizontal byte index X of PPU address (0..31)
    for (nX = 0; nX < PPU_Scr_H_Byte; ++nX)
    {
      putBG(nX);

      // Callback at PPU read/write
      MapperPPU(PATTBL(pbyChrData));

      ++pbyNameTable; // increase pointer into name table bank
    }

    /*-------------------------------------------------------------------*/
    /*  Rendering of the block of the right end                          */
    /*-------------------------------------------------------------------*/

    {
      const u8* pal = &PalTable[(((pAttrBase[nX >> 2] >> ((nX & 2) + nY4)) & 3) << 2)];
      const int ch = *pbyNameTable;
      const int bank = (ch >> 6) + bankOfsBG; // bankOfsBG = background pattern table offset 0=0x0000, 4=0x1000
      const int addrOfs = ((ch & 63) << 4) + yOfsModBG;
      // u8* PPUBANK[16] = pointers to PPU banks (...0x2000, 0x2400, 0x2800, 0x2C00...)
      const u8* data = PPUBANK[bank] + addrOfs;
      const auto pl0 = data[0];
      const auto pl1 = data[8];
      const auto pat0 = (pl0 & 0x55) | ((pl1 << 1) & 0xaa);
      const auto pat1 = ((pl0 >> 1) & 0x55) | (pl1 & 0xaa);
      //      const auto [pat0, pat1] = getPatBG(ch);

      // PPU_Scr_H_Bit = bit index X in byte of PPU scroll address (0..7)
      switch (PPU_Scr_H_Bit)
      {
      case 8:
        pPoint[7] = pal[(pat0 >> 0) & 3];
      case 7:
        pPoint[6] = pal[(pat1 >> 0) & 3];
      case 6:
        pPoint[5] = pal[(pat0 >> 2) & 3];
      case 5:
        pPoint[4] = pal[(pat1 >> 2) & 3];
      case 4:
        pPoint[3] = pal[(pat0 >> 4) & 3];
      case 3:
        pPoint[2] = pal[(pat1 >> 4) & 3];
      case 2:
        pPoint[1] = pal[(pat0 >> 6) & 3];
      case 1:
        pPoint[0] = pal[(pat1 >> 6) & 3];
      default:
        break;
      }
    }

    // Callback at PPU read/write
    MapperPPU(PATTBL(pbyChrData));
  }

  /*-------------------------------------------------------------------*/
  /*  Render a sprite                                                  */
  /*-------------------------------------------------------------------*/

  /* MMC5 VROM switch */
  MapperRenderScreen(0);

  if (PPU_R1 & R1_SHOW_SP)		// PPUMASK
  {
    // Reset Scanline Sprite Count
    PPU_R2 &= ~R2_MAX_SP;		// PPUSTATUS

    // Reset sprite buffer
    InfoNES_MemorySet(pSprBuf, 0, sizeof pSprBuf);

    const int patternTableIdSP88 = PPU_R0 & R0_SP_ADDR ? 1 : 0;		// PPUCTRL
    const int bankOfsSP88 = patternTableIdSP88 << 2;

    // Render a sprite to the sprite buffer
    nSprCnt = 0;
    for (pSPRRAM = SPRRAM + (63 << 2); pSPRRAM >= SPRRAM; pSPRRAM -= 4)
    {
      nY = pSPRRAM[SPR_Y] + 1;
      if (nY > PPU_Scanline || nY + PPU_SP_Height <= PPU_Scanline)  // sprite height 8 or 16
        continue; // Next sprite

      /*-------------------------------------------------------------------*/
      /*  A sprite in scanning line                                        */
      /*-------------------------------------------------------------------*/

      // Horizontal Sprite Count +1
      ++nSprCnt;

      nAttr = pSPRRAM[SPR_ATTR];
      nYBit = PPU_Scanline - nY;
      nYBit = (nAttr & SPR_ATTR_V_FLIP) ? (PPU_SP_Height - nYBit - 1) : nYBit;  // sprite height 8 or 16
      const int yOfsModSP = nYBit;
      nYBit <<= 3;

      int ch = pSPRRAM[SPR_CHR];

      int bankOfs;
      if (PPU_R0 & R0_SP_SIZE)		// PPUCTRL
      {
        // 8x16
        bankOfs = (ch & 1) << 2;
        ch &= 0xfe;
      }
      else
      {
        // 8x8
        bankOfs = bankOfsSP88;
      }

      const int bank = (ch >> 6) + bankOfs;
      const int addrOfs = ((ch & 63) << 4) + ((yOfsModSP & 8) << 1) + (yOfsModSP & 7);
      // u8* PPUBANK[16] = pointers to PPU banks (...0x2000, 0x2400, 0x2800, 0x2C00...)
      const u8* data = PPUBANK[bank] + addrOfs;
      const uint32_t pl0 = data[0];
      const uint32_t pl1 = data[8];
      const auto pat0 = ((pl0 & 0x55) << 24) | ((pl1 & 0x55) << 25);
      const auto pat1 = ((pl0 & 0xaa) << 23) | ((pl1 & 0xaa) << 24);

      nAttr ^= SPR_ATTR_PRI;
      bySprCol = (nAttr & (SPR_ATTR_COLOR | SPR_ATTR_PRI)) << 2;
      nX = pSPRRAM[SPR_X];
      const auto dst = pSprBuf + nX;

      if (nAttr & SPR_ATTR_H_FLIP)
      {
        // h flip
        if (int v = (pat1 << 0) >> 30)
        {
          dst[7] = bySprCol | v;
        }
        if (int v = (pat0 << 0) >> 30)
        {
          dst[6] = bySprCol | v;
        }
        if (int v = (pat1 << 2) >> 30)
        {
          dst[5] = bySprCol | v;
        }
        if (int v = (pat0 << 2) >> 30)
        {
          dst[4] = bySprCol | v;
        }
        if (int v = (pat1 << 4) >> 30)
        {
          dst[3] = bySprCol | v;
        }
        if (int v = (pat0 << 4) >> 30)
        {
          dst[2] = bySprCol | v;
        }
        if (int v = (pat1 << 6) >> 30)
        {
          dst[1] = bySprCol | v;
        }
        if (int v = (pat0 << 6) >> 30)
        {
          dst[0] = bySprCol | v;
        }
      }
      else
      {
        // non flip
        if (int v = (pat1 << 0) >> 30)
        {
          dst[0] = bySprCol | v;
        }
        if (int v = (pat0 << 0) >> 30)
        {
          dst[1] = bySprCol | v;
        }
        if (int v = (pat1 << 2) >> 30)
        {
          dst[2] = bySprCol | v;
        }
        if (int v = (pat0 << 2) >> 30)
        {
          dst[3] = bySprCol | v;
        }
        if (int v = (pat1 << 4) >> 30)
        {
          dst[4] = bySprCol | v;
        }
        if (int v = (pat0 << 4) >> 30)
        {
          dst[5] = bySprCol | v;
        }
        if (int v = (pat1 << 6) >> 30)
        {
          dst[6] = bySprCol | v;
        }
        if (int v = (pat0 << 6) >> 30)
        {
          dst[7] = bySprCol | v;
        }
      }
    }

    // Rendering sprite
    pPoint = pDispBuf + 8;

    compositeSprite(PalTable + 0x10, pSprBuf, pPoint);

    if (nSprCnt >= 8)
      PPU_R2 |= R2_MAX_SP; // Set a flag of maximum sprites on scanline // PPUSTATUS
  }

  // transfer image, clipping
  pPoint = pDispBuf + 8; 					// start of source image
  u8* dst = WorkLine;						// destination buffer

  int n = NES_DISP_WIDTH; 					// length of source image
  memcpy(dst, pPoint, n);					// copy source image
}

/*===================================================================*/
/*                                                                   */
/* InfoNES_GetSprHitY() : Get a position of scanline hits sprite #0  */
/*                                                                   */
/*===================================================================*/
void FASTCODE NOFLASH(InfoNES_GetSprHitY)()
{
  const int patternTableIdSP88 = PPU_R0 & R0_SP_ADDR ? 1 : 0;	// PPUCTRL
  const int bankOfsSP88 = patternTableIdSP88 << 2;

  int yOfsMod;
  int stride;
  if (SPRRAM[SPR_ATTR] & SPR_ATTR_V_FLIP)
  {
    // Vertical flip
    yOfsMod = PPU_SP_Height - 1;	// sprite height 8 or 16
    stride = -1;
  }
  else
  {
    // Non flip
    yOfsMod = 0;
    stride = 1;
  }

  int ch = SPRRAM[SPR_CHR];

  int bankOfs;
  if (PPU_R0 & R0_SP_SIZE)		// PPUCTRL
  {
    // 8x16
    bankOfs = (ch & 1) << 2;
    ch &= 0xfe;
  }
  else
  {
    // 8x8
    bankOfs = bankOfsSP88;
  }

  const int bank = (ch >> 6) + bankOfs;
  const int addrOfs = ((ch & 63) << 4) + ((yOfsMod & 8) << 1) + (yOfsMod & 7);

  // u8* PPUBANK[16] = pointers to PPU banks (...0x2000, 0x2400, 0x2800, 0x2C00...)
  u8* data = PPUBANK[bank] + addrOfs;

  if ((SPRRAM[SPR_Y] < SCAN_UNKNOWN_START) && (SPRRAM[SPR_Y] > 0))
  {
    for (int nLine = 0; nLine < PPU_SP_Height; nLine++)	// sprite height 8 or 16
    {
      if (data[0] | data[8])
      {
        // Scanline hits sprite #0
        SpriteJustHit = SPRRAM[SPR_Y] + 1 + nLine;
        nLine = SCAN_VBLANK_END;
      }
      data += stride;
    }
  }
  else
  {
    // Scanline didn't hit sprite #0
    SpriteJustHit = SCAN_UNKNOWN_START + 1;
  }
}
