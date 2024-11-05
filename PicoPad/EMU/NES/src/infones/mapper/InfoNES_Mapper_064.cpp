/*===================================================================*/
/*                                                                   */
/*                    Mapper 64 (Tengen RAMBO-1)                     */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map64_Cmd;		// 0x00
	BYTE	md_Map64_IRQ_Enable;	// 0x01
	BYTE	md_Map64_IRQ_Cnt;	// 0x02
	BYTE	md_Map64_IRQ_Latch;	// 0x03
	BYTE	md_Map64_Regs[11];	// 0x04
	BYTE	md_Map64_Rmode;		// 0x0f reload mode
	BYTE	md_Map64_IRQmode;	// 0x10
	u8	res[3];			// 0x11
} sMap64_Data;

STATIC_ASSERT((sizeof(sMap64_Data) == 0x14) && (sizeof(sMap64_Data) <= MAPDATA_SIZE), "Incorrect sMap64_Data!");

#define MAPDATA64		((sMap64_Data*)NES->mapdata)	// mapper data

#define Map64_Cmd		MAPDATA64->md_Map64_Cmd
#define Map64_IRQ_Enable	MAPDATA64->md_Map64_IRQ_Enable
#define Map64_IRQ_Cnt		MAPDATA64->md_Map64_IRQ_Cnt
#define Map64_IRQ_Latch		MAPDATA64->md_Map64_IRQ_Latch
#define Map64_Regs		MAPDATA64->md_Map64_Regs
#define Map64_Rmode		MAPDATA64->md_Map64_Rmode
#define Map64_IRQmode		MAPDATA64->md_Map64_IRQmode

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 64                                             */
/*-------------------------------------------------------------------*/
void Map64_Init()
{
  /* Initialize Mapper */
  MapperInit = Map64_Init;

  /* Write to Mapper */
  MapperWrite = Map64_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map64_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  int i;
  for (i = 0; i < 11; i++) Map64_Regs[i] = 0xff;

  /* Set ROM Banks */
  ROMBANK0 = ROMLASTPAGE(0);
  ROMBANK1 = ROMLASTPAGE(0);
  ROMBANK2 = ROMLASTPAGE(0);
  ROMBANK3 = ROMLASTPAGE(0);

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 )
  {
    for ( int nPage = 0; nPage < 8; ++nPage )
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    InfoNES_SetupChr();
  }

  /* Initialize state flag */
  Map64_Cmd = 0x00;
  Map64_IRQ_Enable = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

void FASTCODE NOFLASH(Map64_Sync)()
{
	// set PPU banks
	u8 m = (NesHeader.byVRomSize << 3) - 1;
	if (Map64_Cmd & 0x80) // CHR A12 inversion
	{
		if (Map64_Cmd & 0x20)
		{
			PPUBANK[4] = VROMPAGE(Map64_Regs[0] & m);
			PPUBANK[5] = VROMPAGE(Map64_Regs[8] & m);
			PPUBANK[6] = VROMPAGE(Map64_Regs[1] & m);
			PPUBANK[7] = VROMPAGE(Map64_Regs[9] & m);
		}
		else
		{
			PPUBANK[4] = VROMPAGE(((Map64_Regs[0] & 0xfe) + 0) & m);
			PPUBANK[5] = VROMPAGE(((Map64_Regs[0] & 0xfe) + 1) & m);
			PPUBANK[6] = VROMPAGE(((Map64_Regs[1] & 0xfe) + 0) & m);
			PPUBANK[7] = VROMPAGE(((Map64_Regs[1] & 0xfe) + 1) & m);
		}
		PPUBANK[0] = VROMPAGE(Map64_Regs[2] & m);
		PPUBANK[1] = VROMPAGE(Map64_Regs[3] & m);
		PPUBANK[2] = VROMPAGE(Map64_Regs[4] & m);
		PPUBANK[3] = VROMPAGE(Map64_Regs[5] & m);
	}
	else
	{
		if (Map64_Cmd & 0x20)
		{
			PPUBANK[0] = VROMPAGE(Map64_Regs[0] & m);
			PPUBANK[1] = VROMPAGE(Map64_Regs[8] & m);
			PPUBANK[2] = VROMPAGE(Map64_Regs[1] & m);
			PPUBANK[3] = VROMPAGE(Map64_Regs[9] & m);
		}
		else
		{
			PPUBANK[0] = VROMPAGE(((Map64_Regs[0] & 0xfe) + 0) & m);
			PPUBANK[1] = VROMPAGE(((Map64_Regs[0] & 0xfe) + 1) & m);
			PPUBANK[2] = VROMPAGE(((Map64_Regs[1] & 0xfe) + 0) & m);
			PPUBANK[3] = VROMPAGE(((Map64_Regs[1] & 0xfe) + 1) & m);
		}
		PPUBANK[4] = VROMPAGE(Map64_Regs[2] & m);
		PPUBANK[5] = VROMPAGE(Map64_Regs[3] & m);
		PPUBANK[6] = VROMPAGE(Map64_Regs[4] & m);
		PPUBANK[7] = VROMPAGE(Map64_Regs[5] & m);
	}
          InfoNES_SetupChr();

	// set ROM banks
	m = (NesHeader.byRomSize << 1) - 1;
	if (Map64_Cmd & 0x40)
	{
		ROMBANK0 = ROMPAGE(Map64_Regs[10] & m);
		ROMBANK1 = ROMPAGE(Map64_Regs[7] & m);
		ROMBANK2 = ROMPAGE(Map64_Regs[6] & m);
	}
	else
	{
		ROMBANK0 = ROMPAGE(Map64_Regs[6] & m);
		ROMBANK1 = ROMPAGE(Map64_Regs[7] & m);
		ROMBANK2 = ROMPAGE(Map64_Regs[10] & m);
	}
}

/*-------------------------------------------------------------------*/
/*  Mapper 64 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map64_Write)( WORD wAddr, BYTE byData )
{
  switch ( wAddr & 0xf001 )
  {
    case 0x8000:
      Map64_Cmd = byData;
      break;

    case 0x8001:
	if ((Map64_Cmd & 0x0f) < 10)
		Map64_Regs[Map64_Cmd & 0x0f] = byData;
	else if ((Map64_Cmd & 0x0f) == 0x0f)
		Map64_Regs[10] = byData;
	Map64_Sync();
	break;

    case 0xa000:
        /* Name Table Mirroring */
        InfoNES_Mirroring( (byData & 0x01) ? MIRROR_H : MIRROR_V );
        break;

    case 0xc000:
	Map64_IRQ_Latch = byData;
	if (Map64_Rmode == 1) Map64_IRQ_Cnt = byData;
	break;

    case 0xc001:
	Map64_Rmode = 1;
	Map64_IRQ_Cnt = Map64_IRQ_Latch;
	Map64_IRQmode = byData & 1;
	break;

    case 0xe000:
	Map64_IRQ_Enable = 0;
	if (Map64_Rmode == 1) Map64_IRQ_Cnt = Map64_IRQ_Latch;
	break;

    case 0xe001:
	Map64_IRQ_Enable = 1;
	if (Map64_Rmode == 1) Map64_IRQ_Cnt = Map64_IRQ_Latch;
	break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 64 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map64_HSync)()
{
#define CPU_PER_SCANLINE	28	// CPU clock per scanline / 4

  if (Map64_IRQmode) // count CPU clock / 4
  {
	if (Map64_IRQ_Cnt < CPU_PER_SCANLINE)
	{
		Map64_IRQ_Cnt = 0xff;
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
		if (Map64_IRQ_Enable) IRQ_REQ;
#endif
	}
	else
		Map64_IRQ_Cnt -= CPU_PER_SCANLINE;
  }
  else
  {
    if ( 0 <= PPU_Scanline && PPU_Scanline <= 239 )
    {
      if ( PPU_R1 & R1_SHOW_SCR || PPU_R1 & R1_SHOW_SP )
      {
         Map64_Rmode = 0;
         Map64_IRQ_Cnt--;
         if ( Map64_IRQ_Cnt == 0xff )
         {
		if (Map64_IRQ_Enable)
		{
		         Map64_Rmode = 1; // do reload
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
			 IRQ_REQ;
#endif
		}
        }
      }
    }
  }
}
