/*===================================================================*/
/*                                                                   */
/*                    Mapper 207 (modified X1-005)                   */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map207_mcache[8];	// 0x00 mirroring cache
	BYTE	md_Map207_lastppu;	// 0x08 last PPU page
	BYTE	md_Map207_mlast;	// 0x09 last selected mirrorring (255 = not selected)
	u8	md_res[2];		// 0x0B
} sMap207_Data;

STATIC_ASSERT((sizeof(sMap207_Data) == 0x0C) && (sizeof(sMap207_Data) <= MAPDATA_SIZE), "Incorrect sMap207_Data!");

#define MAPDATA207	((sMap207_Data*)NES->mapdata)	// mapper data

#define Map207_mcache	MAPDATA207->md_Map207_mcache
#define Map207_lastppu	MAPDATA207->md_Map207_lastppu
#define Map207_mlast	MAPDATA207->md_Map207_mlast

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 80                                             */
/*-------------------------------------------------------------------*/
void Map207_Init()
{
  /* Initialize Mapper */
  MapperInit = Map207_Init;

  /* Write to Mapper */
  MapperWrite = Map0_Write;

  /* Write to SRAM */
  MapperSram = Map207_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map0_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  // PPU hook - access to PPU memory 0x0000 - 0x3FFF (NULL = not used)
  PPU_hook = Map207_PPU_hook;

  // invalid mirroring cache
  Map207_mlast = 255;

  /* Set ROM Banks */
  ROMBANK0 = ROMPAGE( 0 );
  ROMBANK1 = ROMPAGE( 1 );
  ROMBANK2 = ROMLASTPAGE( 1 );
  ROMBANK3 = ROMLASTPAGE( 0 );

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 )
  {
    for ( int nPage = 0; nPage < 8; ++nPage )
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    InfoNES_SetupChr();
  }

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

// Sync
void FASTCODE NOFLASH(Map207_Sync)()
{
	// set mirroring - page 0x2400 or 0x2000
	InfoNES_Mirroring(Map207_mcache[Map207_lastppu] ? MIRROR_1 : MIRROR_0);
}

// PPU hook - access to PPU memory 0x0000 - 0x3FFF (NULL = not used)
void FASTCODE NOFLASH(Map207_PPU_hook)(u16 addr)
{
	if (addr < 0x2000)
	{
		Map207_lastppu = (u8)(addr >> 10);
		u8 mir = Map207_mcache[Map207_lastppu];
		if (mir != Map207_mlast)
		{
			Map207_mlast = mir;
			InfoNES_Mirroring(mir ? MIRROR_1 : MIRROR_0);
		}
	}
}

/*-------------------------------------------------------------------*/
/*  Mapper 80 Write to SRAM Function                                 */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map207_Sram)( WORD wAddr, BYTE byData )
{
  switch ( wAddr )
  {
    /* Set PPU Banks */
    case 0x7ef0:
      Map207_mcache[0] = Map207_mcache[1] = byData >> 7;
      Map207_Sync();

      byData &= 0x7f;
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 0 ] = VROMPAGE( byData );
      PPUBANK[ 1 ] = VROMPAGE( byData + 1 );
      InfoNES_SetupChr();
      break;

    case 0x7ef1:
      Map207_mcache[2] = Map207_mcache[3] = byData >> 7;
      Map207_Sync();

      byData &= 0x7f;
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 2 ] = VROMPAGE( byData );
      PPUBANK[ 3 ] = VROMPAGE( byData + 1 );
      InfoNES_SetupChr();
      break;
  
    case 0x7ef2:
      Map207_mcache[4] = byData >> 7;
      Map207_Sync();

      byData &= 0x7f;
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 4 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;      

    case 0x7ef3:
      Map207_mcache[5] = byData >> 7;
      Map207_Sync();

      byData &= 0x7f;
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 5 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;  

    case 0x7ef4:
      Map207_mcache[6] = byData >> 7;
      Map207_Sync();

      byData &= 0x7f;
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 6 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break; 

    case 0x7ef5:
      Map207_mcache[7] = byData >> 7;
      Map207_Sync();

      byData &= 0x7f;
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 7 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break; 

    /* Set ROM Banks */
    case 0x7efa:
    case 0x7efb:
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK0 = ROMPAGE( byData );
      break;

    case 0x7efc:
    case 0x7efd:
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK1 = ROMPAGE( byData );
      break;

    case 0x7efe:
    case 0x7eff:
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK2 = ROMPAGE( byData );
      break;
  }
}
