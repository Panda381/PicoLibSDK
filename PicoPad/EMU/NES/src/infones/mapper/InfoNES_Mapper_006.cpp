/*===================================================================*/
/*                                                                   */
/*                       Mapper 6 (FFE)                              */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map6_IRQ_Enable;	// 0x00
	u8	md_res[3];		// 0x01
	DWORD	md_Map6_IRQ_Cnt;	// 0x04
} sMap6_Data;

STATIC_ASSERT((sizeof(sMap6_Data) == 0x08) && (sizeof(sMap6_Data) <= MAPDATA_SIZE), "Incorrect sMap6_Data!");

#define MAPDATA6		((sMap6_Data*)NES->mapdata)	// mapper data

#define Map6_IRQ_Enable		MAPDATA6->md_Map6_IRQ_Enable
#define Map6_IRQ_Cnt		MAPDATA6->md_Map6_IRQ_Cnt

// BYTE Map6_Chr_Ram[0x2000 * 4];
#define Map6_Chr_Ram	NES->xram

#define MAP6_XRAM_SIZE	(0x2000 * 4)
STATIC_ASSERT(MAP6_XRAM_SIZE <= XRAM_SIZE, "Incorrect MAP6_XRAM_SIZE!");

/* The address of 1Kbytes unit of the Map6 Chr RAM */
#define Map6_VROMPAGE(a) &Map6_Chr_Ram[(a)*0x400]

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 6                                              */
/*-------------------------------------------------------------------*/
void Map6_Init()
{
  int nPage;

  NES_XRamSize = MAP6_XRAM_SIZE;

  /* Initialize Mapper */
  MapperInit = Map6_Init;

  /* Write to Mapper */
  MapperWrite = Map6_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map6_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map6_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  /* Set ROM Banks */
  ROMBANK0 = ROMPAGE(0);
  ROMBANK1 = ROMPAGE(1);
  ROMBANK2 = ROMPAGE(14);
  ROMBANK3 = ROMPAGE(15);

  /* Set PPU Banks */
  if (NesHeader.byVRomSize > 0)
  {
    for (nPage = 0; nPage < 8; ++nPage)
    {
      PPUBANK[nPage] = VROMPAGE(nPage);
    }
    InfoNES_SetupChr();
  }
  else
  {
    for (nPage = 0; nPage < 8; ++nPage)
    {
      PPUBANK[nPage] = Map6_VROMPAGE(nPage);
    }
    InfoNES_SetupChr();
  }

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring(1, 1);
}

/*-------------------------------------------------------------------*/
/*  Mapper 6 Write to APU Function                                   */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map6_Apu)(WORD wAddr, BYTE byData)
{
  switch (wAddr)
  {
  /* Name Table Mirroring */
  case 0x42fe:
    if (byData & 0x10)
    {
      InfoNES_Mirroring(MIRROR_1);
    }
    else
    {
      InfoNES_Mirroring(MIRROR_0);
    }
    break;

  case 0x42ff:
    if (byData & 0x10)
    {
      InfoNES_Mirroring(MIRROR_H);
    }
    else
    {
      InfoNES_Mirroring(MIRROR_V);
    }
    break;

  case 0x4501:
    Map6_IRQ_Enable = 0;
    break;

  case 0x4502:
    Map6_IRQ_Cnt = (Map6_IRQ_Cnt & 0xff00) | (DWORD)byData;
    break;

  case 0x4503:
    Map6_IRQ_Cnt = (Map6_IRQ_Cnt & 0x00ff) | ((DWORD)byData << 8);
    Map6_IRQ_Enable = 1;
    break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 6 Write Function                                          */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map6_Write)(WORD wAddr, BYTE byData)
{
  BYTE byPrgBank = (byData & 0x3c) >> 2;
  BYTE byChrBank = byData & 0x03;

  /* Set ROM Banks */
  byPrgBank <<= 1;
  byPrgBank %= (NesHeader.byRomSize << 1);

  ROMBANK0 = ROMPAGE(byPrgBank);
  ROMBANK1 = ROMPAGE(byPrgBank + 1);

  /* Set PPU Banks */
  PPUBANK[0] = &Map6_Chr_Ram[byChrBank * 0x2000 + 0 * 0x400];
  PPUBANK[1] = &Map6_Chr_Ram[byChrBank * 0x2000 + 1 * 0x400];
  PPUBANK[2] = &Map6_Chr_Ram[byChrBank * 0x2000 + 2 * 0x400];
  PPUBANK[3] = &Map6_Chr_Ram[byChrBank * 0x2000 + 3 * 0x400];
  PPUBANK[4] = &Map6_Chr_Ram[byChrBank * 0x2000 + 4 * 0x400];
  PPUBANK[5] = &Map6_Chr_Ram[byChrBank * 0x2000 + 5 * 0x400];
  PPUBANK[6] = &Map6_Chr_Ram[byChrBank * 0x2000 + 6 * 0x400];
  PPUBANK[7] = &Map6_Chr_Ram[byChrBank * 0x2000 + 7 * 0x400];
  InfoNES_SetupChr();
}

/*-------------------------------------------------------------------*/
/*  Mapper 6 H-Sync Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map6_HSync)()
{
  if (Map6_IRQ_Enable)
  {
    Map6_IRQ_Cnt += 133;
    if (Map6_IRQ_Cnt >= 0xffff)
    {
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
      IRQ_REQ;
#endif
      Map6_IRQ_Cnt = 0;
    }
  }
}
