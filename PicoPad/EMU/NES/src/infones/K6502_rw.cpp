/*===================================================================*/
/*                                                                   */
/*  K6502_RW.h : 6502 Reading/Writing Operation for NES              */
/*               This file is included in K6502.cpp                  */
/*                                                                   */
/*  2000/5/23   InfoNES Project ( based on pNesX )                   */
/*                                                                   */
/*===================================================================*/

#include "../../include.h"

// PPU hook - access to PPU memory 0x0000 - 0x3FFF (NULL = not used)
void (*PPU_hook)(u16 addr);

/*===================================================================*/
/*                                                                   */
/*               K6502_Read() : Reading operation                    */
/*                                                                   */
/*===================================================================*/
BYTE FASTCODE NOFLASH(K6502_Read)(WORD wAddr)
{
  /*
   *  Reading operation
   *
   *  Parameters
   *    WORD wAddr              (Read)
   *      Address to read
   *
   *  Return values
   *    Read data
   *
   *  Remarks
   *    0x0000 - 0x1fff  RAM ( 0x800 - 0x1fff is mirror of 0x0 - 0x7ff )
   *    0x2000 - 0x3fff  PPU
   *    0x4000 - 0x5fff  Sound
   *    0x6000 - 0x7fff  SRAM ( Battery Backed )
   *    0x8000 - 0xffff  ROM
   *
   */
  BYTE byRet;

  if (wAddr >= 0x8000)
  {
    byRet = ROMBANK[(wAddr - 0x8000) >> 13][wAddr & 0x1fff];
    if ((wAddr >= 0xff00) && (MapperRead != NULL)) MapperRead(wAddr, byRet);
    return byRet;
  }

  switch (wAddr & 0xe000)
  {
  case 0x0000: /* RAM */
    return RAM[wAddr & 0x7ff];

  case 0x2000:                /* PPU */

    // 0x2007 read data from PPU video memory
    if ((wAddr & 0x7) == 0x7) /* PPU Memory */
    {
      WORD addr = PPU_Addr;

      // Increment PPU Address
      PPU_Addr += PPU_Increment;	// VRAM address increment 1 or 32
      addr &= 0x3fff;

      // PPU hook - access to PPU memory 0x0000 - 0x3FFF (NULL = not used)
      if (PPU_hook != NULL) PPU_hook(addr);

      // Set return value;
      byRet = PPU_R7; // PPU_R7 = temporary register during read from PPU memory

      // Read PPU Memory, save into temporary register
      // u8* PPUBANK[16] = pointers to PPU banks (...0x2000, 0x2400, 0x2800, 0x2C00...)
      PPU_R7 = PPUBANK[addr >> 10][addr & 0x3ff];

      return byRet;
    }

    // 0x2004 read data from OAM memory
    else if ((wAddr & 0x7) == 0x4) /* SPR_RAM I/O Register */
    {
      // PPU_R3 = OAMADDR address in OAM memory
      return SPRRAM[PPU_R3++];		// OAMADDR
    }

    // 0x2002 get PPU status
    else if ((wAddr & 0x7) == 0x2) /* PPU Status */
    {
      // Set return value
      byRet = PPU_R2;		// PPUSTATUS

      // Reset a V-Blank flag
      PPU_R2 &= ~R2_IN_VBLANK;		// PPUSTATUS

      // Reset address latch
      PPU_Latch_Flag = 0;

      // Make a Nametable 0 in V-Blank
      //  - do not use, this dysfunctional games "The Addams Family" and "The Adventures of Gilligan's Island"
/*
      if (PPU_Scanline >= SCAN_VBLANK_START && !(PPU_R0 & R0_NMI_VB))		// PPUCTRL
      {
        PPU_R0 &= ~R0_NAME_ADDR;		// PPUCTRL
        PPU_NameTableBank = NAME_TABLE0;	// base nametable bank (8=0x2000, 9=0x2400, 10=0x2800, 11=0x2C00)
        PPU_Temp = PPU_Temp & 0xF3FF;		// nametable address 0xF3FF, 0xF7FF, 0xFBFF, 0xFFFF
      }
*/
      return byRet;
    }
    break;

  case 0x4000: /* Sound */
    if (wAddr == 0x4015)
    {
      // read sound status 0x4015
      return NES_ApuReadState();
    }
    else if (wAddr == 0x4016)
    {
      // Set Joypad1 data
      byRet = (BYTE)((PAD1_Latch >> PAD1_Bit) & 1) | 0x40;
      PAD1_Bit = (PAD1_Bit == 23) ? 0 : (PAD1_Bit + 1);
      return byRet;
    }
    else if (wAddr == 0x4017)
    {
      // Set Joypad2 data
      byRet = (BYTE)((PAD2_Latch >> PAD2_Bit) & 1) | 0x40;
      PAD2_Bit = (PAD2_Bit == 23) ? 0 : (PAD2_Bit + 1);
      return byRet;
    }
    else
    {
      /* Return Mapper Register*/
      return MapperReadApu(wAddr);
    }
    break;
    // The other sound registers are not readable.

  case 0x6000: /* SRAM */
    if (ROM_SRAM)
    {
      return SRAM[wAddr & 0x1fff];
    }
    else
    { /* SRAM BANK */
      return SRAMBANK[wAddr & 0x1fff];
    }
  }

  return (wAddr >> 8); /* when a register is not readable the upper half
                            address is returned. */
}

/*===================================================================*/
/*                                                                   */
/*               K6502_Write() : Writing operation                    */
/*                                                                   */
/*===================================================================*/
void FASTCODE NOFLASH(K6502_Write)(WORD wAddr, BYTE byData)
{
  /*
   *  Writing operation
   *
   *  Parameters
   *    WORD wAddr              (Read)
   *      Address to write
   *
   *    BYTE byData             (Read)
   *      Data to write
   *
   *  Remarks
   *    0x0000 - 0x1fff  RAM ( 0x800 - 0x1fff is mirror of 0x0 - 0x7ff )
   *    0x2000 - 0x3fff  PPU
   *    0x4000 - 0x5fff  Sound
   *    0x6000 - 0x7fff  SRAM ( Battery Backed )
   *    0x8000 - 0xffff  ROM
   *
   */
  WORD waddr2;

  switch (wAddr & 0xe000)
  {
  case 0x0000: /* RAM */
  {
    auto addr = wAddr & 0x7ff;
    RAM[addr] = byData;
  }
  break;

  case 0x2000: /* PPU */
    switch (wAddr & 0x7)
    {
    // PPUCTRL
    case 0: /* 0x2000 */
      PPU_R0 = byData;	// PPUCTRL
      PPU_Increment = (PPU_R0 & R0_INC_ADDR) ? 32 : 1; // VRAM address increment 1 or 32
      PPU_NameTableBank = NAME_TABLE0 + (PPU_R0 & R0_NAME_ADDR); // base nametable bank (8=0x2000, 9=0x2400, 10=0x2800, 11=0x2C00)
      PPU_BG_Base = (PPU_R0 & R0_BG_ADDR) ? ChrBuf + 256 * 64 : ChrBuf;	// background base 0x0000 or 0x4000
      PPU_SP_Base = (PPU_R0 & R0_SP_ADDR) ? ChrBuf + 256 * 64 : ChrBuf;	// sprite base 0x0000 or 0x4000
      PPU_SP_Height = (PPU_R0 & R0_SP_SIZE) ? 16 : 8;	// sprite height 8 or 16

      // Account for Loopy's scrolling discoveries
      //  PPU_TEMP <- save base nametable address 0x2000, 0x2400, 0x2800, 0x2C00
      PPU_Temp = (PPU_Temp & 0xF3FF) | ((((WORD)byData) & 0x0003) << 10); // nametable address 0xF3FF, 0xF7FF, 0xFBFF, 0xFFFF
      break;

    case 1: /* 0x2001, PPUMASK, PPU_R1 */
// B0: greyscale 0=color, 1=greyscale
// B1: 1=show background leftmost 8 pixels, 0=hide
// B2: 1=show sprites leftmost 8 pixels, 0=hide
// B3: 1=show backhround
// B4: 1=show sprites
// B5: emphasize red
// B6: emphasize green
// B7: emphasize blue
      PPU_R1 = byData;
      break;

    case 2: /* 0x2002 */
#if 0	  
          PPU_R2 = byData;     // 0x2002 is not writable	// PPUSTATUS
#endif
      break;

    case 3: /* 0x2003 */
      // Sprite RAM Address
      // PPU_R3 = OAMADDR address in OAM memory
      PPU_R3 = byData;		// OAMADDR
      break;

    case 4: /* 0x2004 */
      // Write data to Sprite RAM
      // PPU_R3 = OAMADDR address in OAM memory
      SPRRAM[PPU_R3++] = byData;	// OAMADDR
      break;

    case 5: /* 0x2005 */
      // Set Scroll Register
      if (PPU_Latch_Flag)
      {
        // V-Scroll Register
        // PPU_Scr_V_Next = (byData > 239) ? 0 : byData;
        // PPU_Scr_V_Byte_Next = PPU_Scr_V_Next >> 3;
        // PPU_Scr_V_Bit_Next = PPU_Scr_V_Next & 7;

        // Added : more Loopy Stuff
        PPU_Temp = (PPU_Temp & 0xFC1F) | ((((WORD)byData) & 0xF8) << 2);	// nametable address 0xF3FF, 0xF7FF, 0xFBFF, 0xFFFF
        PPU_Temp = (PPU_Temp & 0x8FFF) | ((((WORD)byData) & 0x07) << 12);

        // PPU_Addr = current address in PPU video memory (0x0000-0x3FFF)
//        PPU_Addr = PPU_Temp;		// nametable address 0xF3FF, 0xF7FF, 0xFBFF, 0xFFFF
      }
      else
      {
        // H-Scroll Register
        // PPU_Scr_H_Next = byData;
        // PPU_Scr_H_Byte_Next = PPU_Scr_H_Next >> 3;	// PPU_Scr_H_Byte horizontal byte index X of PPU address (0..31)
        // PPU_Scr_H_Bit_Next = PPU_Scr_H_Next & 7;
        PPU_Scr_H_Bit = byData & 7;

        // Added : more Loopy Stuff
        PPU_Temp = (PPU_Temp & 0xFFE0) | ((((WORD)byData) & 0xF8) >> 3);	// nametable address 0xF3FF, 0xF7FF, 0xFBFF, 0xFFFF
      }
      PPU_Latch_Flag ^= 1;
      break;

    case 6: /* 0x2006 */
      // Set PPU Address
      if (PPU_Latch_Flag)
      {
        /* Low */
#if 0
            PPU_Addr = ( PPU_Addr & 0xff00 ) | ( (WORD)byData );
#else
        PPU_Temp = (PPU_Temp & 0xFF00) | (((WORD)byData) & 0x00FF);	// nametable address 0xF3FF, 0xF7FF, 0xFBFF, 0xFFFF
        PPU_Addr = PPU_Temp;
#endif
        InfoNES_SetupScr();
      }
      else
      {
        /* High */
#if 0
            PPU_Addr = ( PPU_Addr & 0x00ff ) | ( (WORD)( byData & 0x3f ) << 8 );
            InfoNES_SetupScr();
#else
        PPU_Temp = (PPU_Temp & 0x00FF) | ((((WORD)byData) & 0x003F) << 8);	// nametable address 0xF3FF, 0xF7FF, 0xFBFF, 0xFFFF
#endif
      }
      PPU_Latch_Flag ^= 1;
      break;

    case 7: /* 0x2007 */
    {
      WORD addr = PPU_Addr;

      // Increment PPU Address
      PPU_Addr += PPU_Increment;	// VRAM address increment 1 or 32
      addr &= 0x3fff;

      // PPU hook - access to PPU memory 0x0000 - 0x3FFF (NULL = not used)
      if (PPU_hook != NULL) PPU_hook(addr);

      // Write to PPU Memory
      if (addr < 0x2000 && byVramWriteEnable)
      {
        // Pattern Data
        // u8* PPUBANK[16] = pointers to PPU banks (...0x2000, 0x2400, 0x2800, 0x2C00...)

#if 1		// check video-RAM write address
        u8* d = &PPUBANK[addr >> 10][0];
	if ((d >= &NES->vram[0]) && (d < &NES->framebuf[0]))
	        d[addr & 0x3ff] = byData;
#else
        PPUBANK[addr >> 10][addr & 0x3ff] = byData;
#endif

      }
      else if (addr < 0x3f00) /* 0x2000 - 0x3eff */
      {
        // Name Table and mirror
        // u8* PPUBANK[16] = pointers to PPU banks (...0x2000, 0x2400, 0x2800, 0x2C00...)

#if 1		// check video-RAM write address
        u8* d = &PPUBANK[addr >> 10][0];
	if ((d >= &NES->vram[0]) && (d < &NES->framebuf[0]))
	        d[addr & 0x3ff] = byData;

        d = &PPUBANK[(addr ^ 0x1000) >> 10][0];
	if ((d >= &NES->vram[0]) && (d < &NES->framebuf[0]))
	        d[addr & 0x3ff] = byData;
#else
        PPUBANK[addr >> 10][addr & 0x3ff] = byData;
        PPUBANK[(addr ^ 0x1000) >> 10][addr & 0x3ff] = byData;
#endif

      }
      else if (!(addr & 0xf)) /* 0x3f00 or 0x3f10 */
      {
        // Palette mirror
        PPURAM[0x3f10] = PPURAM[0x3f14] = PPURAM[0x3f18] = PPURAM[0x3f1c] =
            PPURAM[0x3f00] = PPURAM[0x3f04] = PPURAM[0x3f08] = PPURAM[0x3f0c] = byData;

        PalTable[0x00] = PalTable[0x04] = PalTable[0x08] = PalTable[0x0c] =
            PalTable[0x10] = PalTable[0x14] = PalTable[0x18] = PalTable[0x1c] = byData;
      }
      else if (addr & 3)
      {
        // Palette
        PPURAM[addr] = byData;
        PalTable[addr & 0x1f] = byData | 0x80;
      }
    }
    break;
    }
    break;

  case 0x4000: /* Sound */
    waddr2 = wAddr & 0x1f;

    switch (waddr2)
    {
    // write sound register
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
    case 0x0a:
    case 0x0b:
    case 0x0c:
    case 0x0d:
    case 0x0e:
    case 0x0f:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
      NES_ApuWriteReg((u8)waddr2, byData);
      break;

    case 0x14: /* 0x4014 */
      // Sprite DMA
      switch (byData >> 5)
      {
      case 0x0: /* RAM */
        InfoNES_MemoryCopy(SPRRAM, &RAM[((WORD)byData << 8) & 0x7ff], SPRRAM_SIZE);
        break;

      case 0x3: /* SRAM */
        InfoNES_MemoryCopy(SPRRAM, &SRAM[((WORD)byData << 8) & 0x1fff], SPRRAM_SIZE);
        break;

      case 0x4: /* ROM BANK 0 */
        InfoNES_MemoryCopy(SPRRAM, &ROMBANK0[((WORD)byData << 8) & 0x1fff], SPRRAM_SIZE);
        break;

      case 0x5: /* ROM BANK 1 */
        InfoNES_MemoryCopy(SPRRAM, &ROMBANK1[((WORD)byData << 8) & 0x1fff], SPRRAM_SIZE);
        break;

      case 0x6: /* ROM BANK 2 */
        InfoNES_MemoryCopy(SPRRAM, &ROMBANK2[((WORD)byData << 8) & 0x1fff], SPRRAM_SIZE);
        break;

      case 0x7: /* ROM BANK 3 */
        InfoNES_MemoryCopy(SPRRAM, &ROMBANK3[((WORD)byData << 8) & 0x1fff], SPRRAM_SIZE);
        break;
      }
//      APU_Reg[0x14] = byData;
      break;

    case 0x15: /* 0x4015 */
      NES_ApuWriteEn(byData);
      break;

    case 0x16: /* 0x4016 */
      // Reset joypad
      if (!(APU_Reg[0x16] & 1) && (byData & 1))
      {
        PAD1_Bit = 0;
        PAD2_Bit = 0;
      }
/*
      if (MapperNo == 99)
      {
        APU_Reg[0x16] = byData;
	Map99_HSync();
      }
*/
      break;

    case 0x17: /* 0x4017 */
      NES_ApuWriteFrame(byData);

      // reset frame counter
      FrameStep = 0;

      // Frame IRQ
      if (!(byData & 0xc0))
      {
        FrameIRQ_Enable = 1;
      }
      else
      {
        FrameIRQ_Enable = 0;
      }
      break;
    }

    if (wAddr <= 0x4017)
    {
      APU_Reg[waddr2] = byData;
    }
    else
    {
      /* Write to APU */
      MapperApu(wAddr, byData);
    }
   break;

  case 0x6000: /* SRAM */
    SRAM[wAddr & 0x1fff] = byData;
    SRAMwritten = true;

    /* Write to SRAM, when no SRAM */
    if (!ROM_SRAM || (MapperNo == 80) /*|| (MapperNo == 150)*/)
    {
      MapperSram(wAddr, byData);
    }
    break;

  case 0x8000: /* ROM BANK 0 */
  case 0xa000: /* ROM BANK 1 */
  case 0xc000: /* ROM BANK 2 */
  case 0xe000: /* ROM BANK 3 */
    // Write to Mapper
    MapperWrite(wAddr, byData);
    break;
  }
}

// Reading/Writing operation (WORD version)
WORD FASTCODE NOFLASH(K6502_ReadW)(WORD wAddr)
{
	return K6502_Read(wAddr) | (WORD)K6502_Read(wAddr + 1) << 8;
};

void FASTCODE NOFLASH(K6502_WriteW)(WORD wAddr, WORD wData)
{
  K6502_Write(wAddr, wData & 0xff);
  K6502_Write(wAddr + 1, wData >> 8);
};

WORD FASTCODE NOFLASH(K6502_ReadZpW)(BYTE byAddr)
{
	return K6502_ReadZp(byAddr) | (K6502_ReadZp(byAddr + 1) << 8);
};

// 6502's indirect absolute jmp(opcode: 6C) has a bug (added at 01/08/15 )
WORD FASTCODE NOFLASH(K6502_ReadW2)(WORD wAddr)
{
  if (0x00ff == (wAddr & 0x00ff))
  {
    return K6502_Read(wAddr) | (WORD)K6502_Read(wAddr - 0x00ff) << 8;
  }
  else
  {
    return K6502_Read(wAddr) | (WORD)K6502_Read(wAddr + 1) << 8;
  }
}
