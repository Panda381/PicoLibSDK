/*===================================================================*/
/*                                                                   */
/*                              Mapper 156                           */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map154_Regs[ 1 ];	// 0x00
	u8	md_res[3];		// 0x01
} sMap154_Data;

STATIC_ASSERT((sizeof(sMap154_Data) == 0x04) && (sizeof(sMap154_Data) <= MAPDATA_SIZE), "Incorrect sMap154_Data!");

#define MAPDATA154	((sMap154_Data*)NES->mapdata)	// mapper data

#define Map154_Regs	MAPDATA154->md_Map154_Regs

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 154                                             */
/*-------------------------------------------------------------------*/
void Map154_Init()
{
  /* Initialize Mapper */
  MapperInit = Map154_Init;

  /* Write to Mapper */
  MapperWrite = Map154_Write;

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

#ifndef NES_MIRRORING	// use custom mirroring: 0=horiz, 1=vert, 2=2400, 3=2000, 4=four, 5=0001, 6=0111
  InfoNES_Mirroring( MIRROR_0 );
#endif

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 154 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map154_Write)( WORD wAddr, BYTE byData )
{
  switch ( wAddr & 0x8001)
  {
    case 0x8000:
      Map154_Regs[ 0 ] = byData;
      byData = (byData >> 6) & 1;
      InfoNES_Mirroring( byData ? MIRROR_1 : MIRROR_0 );
      break;

    case 0x8001:
      switch ( Map154_Regs[ 0 ] & 0x07 )
      { 
        case 0x00:
          PPUBANK[ 0 ] = VROMPAGE( ( ( byData & 0xfe ) + 0 ) % ( NesHeader.byVRomSize << 3 ) );
          PPUBANK[ 1 ] = VROMPAGE( ( ( byData & 0xfe ) + 1 ) % ( NesHeader.byVRomSize << 3 ) );
          InfoNES_SetupChr();
          break;

        case 0x01:
          PPUBANK[ 2 ] = VROMPAGE( ( ( byData & 0xfe ) + 0 ) % ( NesHeader.byVRomSize << 3 ) );
          PPUBANK[ 3 ] = VROMPAGE( ( ( byData & 0xfe ) + 1 ) % ( NesHeader.byVRomSize << 3 ) );
          InfoNES_SetupChr();
          break;

        case 0x02:
          PPUBANK[ 4 ] = VROMPAGE( ( byData + 0x40 ) % ( NesHeader.byVRomSize << 3 ) );
          InfoNES_SetupChr();
          break;

        case 0x03:
          PPUBANK[ 5 ] = VROMPAGE( ( byData + 0x40 ) % ( NesHeader.byVRomSize << 3 ) );
          InfoNES_SetupChr();
          break;

        case 0x04:
          PPUBANK[ 6 ] = VROMPAGE( ( byData + 0x40 ) % ( NesHeader.byVRomSize << 3 ) );
          InfoNES_SetupChr();
          break;

        case 0x05:
          PPUBANK[ 7 ] = VROMPAGE( ( byData + 0x40 ) % ( NesHeader.byVRomSize << 3 ) );
          InfoNES_SetupChr();
          break;

        case 0x06:
          ROMBANK0 = ROMPAGE( byData % ( NesHeader.byRomSize << 1 ) );
          break;

        case 0x07:
          ROMBANK1 = ROMPAGE( byData % ( NesHeader.byRomSize << 1 ) );
          break;
      }
      break;
  }
}
