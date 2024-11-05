/*===================================================================*/
/*                                                                   */
/*                         Mapper 99                                 */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map99_Old;		// 0x00
	u8	md_res[3];		// 0x02
} sMap99_Data;

STATIC_ASSERT((sizeof(sMap99_Data) == 0x04) && (sizeof(sMap99_Data) <= MAPDATA_SIZE), "Incorrect sMap99_Data!");

#define MAPDATA99		((sMap99_Data*)NES->mapdata)	// mapper data

#define Map99_Old		MAPDATA99->md_Map99_Old

//#define MAP99_XRAM_SIZE	8192
//STATIC_ASSERT(MAP99_XRAM_SIZE <= XRAM_SIZE, "Incorrect MAP99_XRAM_SIZE!");

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 99                                             */
/*-------------------------------------------------------------------*/
void Map99_Init()
{
  /* Initialize Mapper */
  MapperInit = Map99_Init;

  /* Write to Mapper */
  MapperWrite = Map0_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map99_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  /* Set ROM Banks */
  u8 m = (NesHeader.byRomSize << 1) - 1;
  ROMBANK0 = ROMPAGE( 0 );
  ROMBANK1 = ROMPAGE( 1 & m);
  ROMBANK2 = ROMPAGE( 2 & m);
  ROMBANK3 = ROMPAGE( 3 & m);

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 )
  {
    for ( int nPage = 0; nPage < 8; ++nPage )
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    InfoNES_SetupChr();
  }

  /* Initialize IRQ Reg */
  Map99_Old = 0xea;

#ifndef NES_MIRRORING	// use custom mirroring: 0=horiz, 1=vert, 2=2400, 3=2000, 4=four, 5=0001, 6=0111
  InfoNES_Mirroring( MIRROR_4 );
  ROM_FourScr = 1;
#endif

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 99 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map99_HSync)()
{
	u8 d = APU_Reg[0x16];
	if (d != Map99_Old)
	{
		Map99_Old = d;
		d = (d >> 2) & 1; // select 8 KB ROM and VROM

		u8 m = (NesHeader.byRomSize << 1) - 1;
		ROMBANK0 = ROMPAGE((d << 2) & m);

		m = (NesHeader.byVRomSize << 3) - 1;
		d <<= 3;
		int i;
		for (i = 0; i < 8; i++) PPUBANK[i] = VROMPAGE((d + i) & m);
		InfoNES_SetupChr();
	}
}
