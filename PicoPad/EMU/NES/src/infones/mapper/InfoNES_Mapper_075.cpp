/*===================================================================*/
/*                                                                   */
/*           Mapper 75 (Konami VRC 1 and Jaleco SS8805)              */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map75_Regs[ 2 ];	// 0x00
	u8	md_res[2];		// 0x02
} sMap75_Data;

STATIC_ASSERT((sizeof(sMap75_Data) == 0x04) && (sizeof(sMap75_Data) <= MAPDATA_SIZE), "Incorrect sMap75_Data!");

#define MAPDATA75		((sMap75_Data*)NES->mapdata)	// mapper data

#define Map75_Regs	MAPDATA75->md_Map75_Regs

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 75                                             */
/*-------------------------------------------------------------------*/
void Map75_Init()
{
  /* Initialize Mapper */
  MapperInit = Map75_Init;

  /* Write to Mapper */
  MapperWrite = Map75_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

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

  /* Initialize State Flag */
  Map75_Regs[ 0 ] = 0;
  Map75_Regs[ 1 ] = 1;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 75 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map75_Write)( WORD wAddr, BYTE byData )
{
  switch ( wAddr & 0xf000 )
  {
    /* Set ROM Banks */
    case 0x8000:
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK0 = ROMPAGE( byData );
      break;

    case 0x9000:
      /* Set Mirroring */
      if ( byData & 0x01 )
      {
        InfoNES_Mirroring( MIRROR_H );
      } else {
        InfoNES_Mirroring( MIRROR_V );
      }

      /* Set PPU Banks */
      Map75_Regs[ 0 ] = ( Map75_Regs[ 0 ] & 0x0f ) | ( ( byData & 0x02 ) << 3 );
      PPUBANK[ 0 ] = VROMPAGE( ( Map75_Regs[ 0 ] << 2 ) + 0 );
      PPUBANK[ 1 ] = VROMPAGE( ( Map75_Regs[ 0 ] << 2 ) + 1 );
      PPUBANK[ 2 ] = VROMPAGE( ( Map75_Regs[ 0 ] << 2 ) + 2 );
      PPUBANK[ 3 ] = VROMPAGE( ( Map75_Regs[ 0 ] << 2 ) + 3 );

      Map75_Regs[ 1 ] = ( Map75_Regs[ 1 ] & 0x0f ) | ( ( byData & 0x04 ) << 2 );
      PPUBANK[ 4 ] = VROMPAGE( ( Map75_Regs[ 1 ] << 2 ) + 0 );
      PPUBANK[ 5 ] = VROMPAGE( ( Map75_Regs[ 1 ] << 2 ) + 1 );
      PPUBANK[ 6 ] = VROMPAGE( ( Map75_Regs[ 1 ] << 2 ) + 2 );
      PPUBANK[ 7 ] = VROMPAGE( ( Map75_Regs[ 1 ] << 2 ) + 3 );
      InfoNES_SetupChr();
      break;

    /* Set ROM Banks */
    case 0xA000:
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK1 = ROMPAGE( byData );
      break;

    /* Set ROM Banks */
    case 0xC000:
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK2 = ROMPAGE( byData );
      break;

    case 0xE000:
      /* Set PPU Banks */
      Map75_Regs[ 0 ] = ( Map75_Regs[ 0 ] & 0x10 ) | ( byData & 0x0f );
      PPUBANK[ 0 ] = VROMPAGE( ( Map75_Regs[ 0 ] << 2 ) + 0 );
      PPUBANK[ 1 ] = VROMPAGE( ( Map75_Regs[ 0 ] << 2 ) + 1 );
      PPUBANK[ 2 ] = VROMPAGE( ( Map75_Regs[ 0 ] << 2 ) + 2 );
      PPUBANK[ 3 ] = VROMPAGE( ( Map75_Regs[ 0 ] << 2 ) + 3 );
      InfoNES_SetupChr();
      break;

    case 0xF000:
      /* Set PPU Banks */
      Map75_Regs[ 1 ] = ( Map75_Regs[ 1 ] & 0x10 ) | ( byData & 0x0f );
      PPUBANK[ 4 ] = VROMPAGE( ( Map75_Regs[ 1 ] << 2 ) + 0 );
      PPUBANK[ 5 ] = VROMPAGE( ( Map75_Regs[ 1 ] << 2 ) + 1 );
      PPUBANK[ 6 ] = VROMPAGE( ( Map75_Regs[ 1 ] << 2 ) + 2 );
      PPUBANK[ 7 ] = VROMPAGE( ( Map75_Regs[ 1 ] << 2 ) + 3 );
      InfoNES_SetupChr();
      break;
  }
}
