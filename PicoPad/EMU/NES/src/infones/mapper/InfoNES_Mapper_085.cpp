/*===================================================================*/
/*                                                                   */
/*                    Mapper 85 (Konami VRC7)                        */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map85_Regs[1];	// 0x00
	BYTE	md_Map85_IRQ_Enable;	// 0x01
	BYTE	md_Map85_IRQ_Cnt;	// 0x02
	BYTE	md_Map85_IRQ_Latch;	// 0x03
} sMap85_Data;

STATIC_ASSERT((sizeof(sMap85_Data) == 0x04) && (sizeof(sMap85_Data) <= MAPDATA_SIZE), "Incorrect sMap85_Data!");

#define MAPDATA85		((sMap85_Data*)NES->mapdata)	// mapper data

#define Map85_Regs		MAPDATA85->md_Map85_Regs
#define Map85_IRQ_Enable	MAPDATA85->md_Map85_IRQ_Enable
#define Map85_IRQ_Cnt		MAPDATA85->md_Map85_IRQ_Cnt
#define Map85_IRQ_Latch		MAPDATA85->md_Map85_IRQ_Latch

// BYTE Map85_Chr_Ram[0x100 * 0x400];
#define Map85_Chr_Ram	NES->xram

#define MAP85_XRAM_SIZE	(0x40 * 0x400)
#define MAP85_XRAM_MASK 0x3f
STATIC_ASSERT(MAP85_XRAM_SIZE <= XRAM_SIZE, "Incorrect MAP85_XRAM_SIZE!");

/* The address of 1Kbytes unit of the Map85 Chr RAM */
#define Map85_VROMPAGE(a) &Map85_Chr_Ram[((a) & MAP85_XRAM_MASK) * 0x400]

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 85                                             */
/*-------------------------------------------------------------------*/
void Map85_Init()
{
  NES_XRamSize = MAP85_XRAM_SIZE;

  /* Initialize Mapper */
  MapperInit = Map85_Init;

  /* Write to Mapper */
  MapperWrite = Map85_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map85_HSync;

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
  PPUBANK[0] = Map85_VROMPAGE(0);
  PPUBANK[1] = Map85_VROMPAGE(0);
  PPUBANK[2] = Map85_VROMPAGE(0);
  PPUBANK[3] = Map85_VROMPAGE(0);
  PPUBANK[4] = Map85_VROMPAGE(0);
  PPUBANK[5] = Map85_VROMPAGE(0);
  PPUBANK[6] = Map85_VROMPAGE(0);
  PPUBANK[7] = Map85_VROMPAGE(0);
  InfoNES_SetupChr();

  /* Initialize State Registers */
  Map85_Regs[0] = 0;
  Map85_IRQ_Enable = 0;
  Map85_IRQ_Cnt = 0;
  Map85_IRQ_Latch = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring(1, 1);
}

/*-------------------------------------------------------------------*/
/*  Mapper 85 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map85_Write)(WORD wAddr, BYTE byData)
{
  switch (wAddr & 0xf030)
  {
  case 0x8000:
    byData %= (NesHeader.byRomSize << 1);
    ROMBANK0 = ROMPAGE(byData);
    break;

  case 0x8010:
    byData %= (NesHeader.byRomSize << 1);
    ROMBANK1 = ROMPAGE(byData);
    break;

  case 0x9000:
    byData %= (NesHeader.byRomSize << 1);
    ROMBANK2 = ROMPAGE(byData);
    break;

  case 0x9010:
  case 0x9030:
    /* Extra Sound */

  case 0xa000:
    PPUBANK[0] = Map85_VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0xa010:
    PPUBANK[1] = Map85_VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0xb000:
    PPUBANK[2] = Map85_VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0xb010:
    PPUBANK[3] = Map85_VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0xc000:
    PPUBANK[4] = Map85_VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0xc010:
    PPUBANK[5] = Map85_VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0xd000:
    PPUBANK[6] = Map85_VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0xd010:
    PPUBANK[7] = Map85_VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  /* Name Table Mirroring */
  case 0xe000:
    switch (byData & 0x03)
    {
    case 0x00:
      InfoNES_Mirroring(MIRROR_V);
      break;
    case 0x01:
      InfoNES_Mirroring(MIRROR_H);
      break;
    case 0x02:
      InfoNES_Mirroring(MIRROR_0);
      break;
    case 0x03:
      InfoNES_Mirroring(MIRROR_1);
      break;
    }
    break;

  case 0xe010:
    Map85_IRQ_Latch = 0xff - byData;
    break;

  case 0xf000:
    Map85_Regs[0] = byData & 0x01;
    Map85_IRQ_Enable = (byData & 0x02) >> 1;
    Map85_IRQ_Cnt = Map85_IRQ_Latch;
    break;

  case 0xf010:
    Map85_IRQ_Enable = Map85_Regs[0];
    Map85_IRQ_Cnt = Map85_IRQ_Latch;
    break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 85 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map85_HSync)()
{
  if (Map85_IRQ_Enable)
  {
    if (Map85_IRQ_Cnt == 0)
    {
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
      IRQ_REQ;
#endif
      Map85_IRQ_Enable = 0;
    }
    else
    {
      Map85_IRQ_Cnt--;
    }
  }
}
