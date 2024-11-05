/*===================================================================*/
/*                                                                   */
/*                    Mapper 206 (Namcot 109)                        */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map206_Reg;	// 0x00
	u8	res[3];		// 0x01
} sMap206_Data;

STATIC_ASSERT((sizeof(sMap206_Data) == 0x04) && (sizeof(sMap206_Data) <= MAPDATA_SIZE), "Incorrect sMap206_Data!");

#define MAPDATA206	((sMap206_Data*)NES->mapdata)	// mapper data

#define Map206_Reg	MAPDATA206->md_Map206_Reg

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 206                                            */
/*-------------------------------------------------------------------*/
void Map206_Init()
{
  /* Initialize Mapper */
  MapperInit = Map206_Init;

  /* Write to Mapper */
  MapperWrite = Map206_Write;

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

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 206 Write Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map206_Write)( WORD wAddr, BYTE byData )
{
  switch ( wAddr & 0xe001 )
  {
    case 0x8000:
      Map206_Reg = byData;
      break;

    case 0x8001:
      switch ( Map206_Reg & 0x07 )
      {
        case 0x00:
          byData &= 0x3f;
          PPUBANK[ 0 ] = VROMPAGE( (byData + 0) % ( NesHeader.byVRomSize << 3 ));
          PPUBANK[ 1 ] = VROMPAGE( (byData + 1) % ( NesHeader.byVRomSize << 3 ) );
          InfoNES_SetupChr();
          break;

        case 0x01:
          byData &= 0x3f;
          PPUBANK[ 2 ] = VROMPAGE( (byData + 0) % ( NesHeader.byVRomSize << 3 ));
          PPUBANK[ 3 ] = VROMPAGE( (byData + 1) % ( NesHeader.byVRomSize << 3 ) );
          InfoNES_SetupChr();
          break;

        case 0x02:
          byData &= 0x3f;
          byData %= ( NesHeader.byVRomSize << 3 );
          PPUBANK[ 4 ] = VROMPAGE( byData );
          InfoNES_SetupChr();
          break;

        case 0x03:
          byData &= 0x3f;
          byData %= ( NesHeader.byVRomSize << 3 );
          PPUBANK[ 5 ] = VROMPAGE( byData );
          InfoNES_SetupChr();
          break;

        case 0x04:
          byData &= 0x3f;
          byData %= ( NesHeader.byVRomSize << 3 );
          PPUBANK[ 6 ] = VROMPAGE( byData );
          InfoNES_SetupChr();
          break;

        case 0x05:
          byData &= 0x3f;
          byData %= ( NesHeader.byVRomSize << 3 );
          PPUBANK[ 7 ] = VROMPAGE( byData );
          InfoNES_SetupChr();
          break;

        case 0x06:
          byData &= 0x0f;
          byData %= ( NesHeader.byRomSize << 1 );
          ROMBANK0 = ROMPAGE( byData );
          break;

        case 0x07:
          byData &= 0x0f;
          byData %= ( NesHeader.byRomSize << 1 );
          ROMBANK1 = ROMPAGE( byData );
          break;
      }
      break;
  }  
}
