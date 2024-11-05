/*===================================================================*/
/*                                                                   */
/*                           Mapper 210                              */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map210_IRQ_Enable;	// 0x00
	u8	md_res[3];		// 0x01
	DWORD	md_Map210_IRQ_Cnt;	// 0x04
} sMap210_Data;

STATIC_ASSERT((sizeof(sMap210_Data) == 0x08) && (sizeof(sMap210_Data) <= MAPDATA_SIZE), "Incorrect sMap210_Data!");

#define MAPDATA210		((sMap210_Data*)NES->mapdata)	// mapper data

#define Map210_IRQ_Enable	MAPDATA210->md_Map210_IRQ_Enable
#define Map210_IRQ_Cnt		MAPDATA210->md_Map210_IRQ_Cnt

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 210                                             */
/*-------------------------------------------------------------------*/
void Map210_Init()
{
  /* Initialize Mapper */
  MapperInit = Map210_Init;

  /* Write to Mapper */
  MapperWrite = Map210_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map210_Apu;

  /* Read from APU */
  MapperReadApu = Map210_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map210_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  /* Set ROM Banks */
  ROMBANK0 = ROMPAGE(0);
  ROMBANK1 = ROMPAGE(1);
  ROMBANK2 = ROMLASTPAGE(1);
  ROMBANK3 = ROMLASTPAGE(0);

  /* Set PPU Banks */
  if (NesHeader.byVRomSize > 0)
  {
    DWORD dwLastPage = (DWORD)NesHeader.byVRomSize << 3;
    PPUBANK[0] = VROMPAGE(dwLastPage - 8);
    PPUBANK[1] = VROMPAGE(dwLastPage - 7);
    PPUBANK[2] = VROMPAGE(dwLastPage - 6);
    PPUBANK[3] = VROMPAGE(dwLastPage - 5);
    PPUBANK[4] = VROMPAGE(dwLastPage - 4);
    PPUBANK[5] = VROMPAGE(dwLastPage - 3);
    PPUBANK[6] = VROMPAGE(dwLastPage - 2);
    PPUBANK[7] = VROMPAGE(dwLastPage - 1);
    InfoNES_SetupChr();
  }

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring(1, 1);
}

/*-------------------------------------------------------------------*/
/*  Mapper 210 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map210_Write)(WORD wAddr, BYTE byData)
{
  /* Set PPU Banks */
  switch (wAddr & 0xf800)
  {
  case 0x8000: /* $8000-87ff */
      byData %= (NesHeader.byVRomSize << 3);
      PPUBANK[0] = VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0x8800: /* $8800-8fff */
      byData %= (NesHeader.byVRomSize << 3);
      PPUBANK[1] = VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0x9000: /* $9000-97ff */
      byData %= (NesHeader.byVRomSize << 3);
      PPUBANK[2] = VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0x9800: /* $9800-9fff */
      byData %= (NesHeader.byVRomSize << 3);
      PPUBANK[3] = VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0xa000: /* $a000-a7ff */
      byData %= (NesHeader.byVRomSize << 3);
      PPUBANK[4] = VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0xa800: /* $a800-afff */
      byData %= (NesHeader.byVRomSize << 3);
      PPUBANK[5] = VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0xb000: /* $b000-b7ff */
      byData %= (NesHeader.byVRomSize << 3);
      PPUBANK[6] = VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0xb800: /* $b800-bfff */
      byData %= (NesHeader.byVRomSize << 3);
      PPUBANK[7] = VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0xe000: /* $e000-e7ff */
    switch (byData >> 6)
    {
    case 0x00:
      InfoNES_Mirroring(MIRROR_0);
      break;
    case 0x01:
      InfoNES_Mirroring(MIRROR_V);
      break;
    case 0x02:
      InfoNES_Mirroring(MIRROR_1);
      break;
    case 0x03:
      InfoNES_Mirroring(MIRROR_H);
      break;
    }
    byData &= 0x3f;
    byData %= (NesHeader.byRomSize << 1);
    ROMBANK0 = ROMPAGE(byData);
    break;

  case 0xe800: /* $e800-efff */
    byData &= 0x3f;
    byData %= (NesHeader.byRomSize << 1);
    ROMBANK1 = ROMPAGE(byData);
    break;

  case 0xf000: /* $f000-f7ff */
    byData &= 0x3f;
    byData %= (NesHeader.byRomSize << 1);
    ROMBANK2 = ROMPAGE(byData);
    break;

  case 0xf800: /* $e800-efff */
    if (wAddr == 0xf800)
    {
      // Extra Sound
    }
    break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 210 Write to APU Function                                  */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map210_Apu)(WORD wAddr, BYTE byData)
{
  switch (wAddr & 0xf800)
  {
  case 0x4800:
    if (wAddr == 0x4800)
    {
      // Extra Sound
    }
    break;

  case 0x5000: /* $5000-57ff */
    Map210_IRQ_Cnt = (((Map210_IRQ_Cnt >> STEP_PER_SCANLINE_SHIFT) & 0xff00) | byData) << STEP_PER_SCANLINE_SHIFT;
    break;

  case 0x5800: /* $5800-5fff */
    Map210_IRQ_Cnt = (((Map210_IRQ_Cnt >> STEP_PER_SCANLINE_SHIFT) & 0x00ff) | ((DWORD)(byData & 0x7f) << 8)) << STEP_PER_SCANLINE_SHIFT;
    Map210_IRQ_Enable = (byData & 0x80) >> 7;
    break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 210 Read from APU Function                                 */
/*-------------------------------------------------------------------*/
BYTE FASTCODE NOFLASH(Map210_ReadApu)(WORD wAddr)
{
  switch (wAddr & 0xf800)
  {
  case 0x4800:
    if (wAddr == 0x4800)
    {
      // Extra Sound
    }
    return (BYTE)(wAddr >> 8);

  case 0x5000: /* $5000-57ff */
    return (BYTE)((Map210_IRQ_Cnt >> STEP_PER_SCANLINE_SHIFT) & 0x00ff);

  case 0x5800: /* $5800-5fff */
    return (BYTE)(((Map210_IRQ_Cnt >> STEP_PER_SCANLINE_SHIFT) & 0x7f00) >> 8);

  default:
    return (BYTE)(wAddr >> 8);
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 210 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map210_HSync)()
{
  /*
 *  Callback at HSync
 *
 */
  if (Map210_IRQ_Enable)
  {
    if (Map210_IRQ_Cnt >= (DWORD)((0x7fff << STEP_PER_SCANLINE_SHIFT) - STEP_PER_SCANLINE_H))
    {
      Map210_IRQ_Cnt = (0x7fff << STEP_PER_SCANLINE_SHIFT);
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
      IRQ_REQ;
#endif
      Map210_IRQ_Enable = 0;
    }
    else
    {
      Map210_IRQ_Cnt += STEP_PER_SCANLINE_H;
    }
  }
}
