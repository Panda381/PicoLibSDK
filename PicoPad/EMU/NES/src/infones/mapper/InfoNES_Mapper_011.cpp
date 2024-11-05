/*===================================================================*/
/*                                                                   */
/*                     Mapper 11 (Color Dreams)                      */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 11                                             */
/*-------------------------------------------------------------------*/
void Map11_Init()
{
  int nPage;

  /* Initialize Mapper */
  MapperInit = Map11_Init;

  /* Write to Mapper */
  MapperWrite = Map11_Write;

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
  if ( NesHeader.byRomSize > 1 )
  {
    ROMBANK0 = ROMPAGE( 0 );
    ROMBANK1 = ROMPAGE( 1 );
    ROMBANK2 = ROMPAGE( 2 );
    ROMBANK3 = ROMPAGE( 3 );    
  } else {
    ROMBANK0 = ROMPAGE( 0 );
    ROMBANK1 = ROMPAGE( 1 );
    ROMBANK2 = ROMPAGE( 0 );
    ROMBANK3 = ROMPAGE( 1 );
  }

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 )
  {
    for ( nPage = 0; nPage < 8; ++nPage )
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    InfoNES_SetupChr();
  }

#ifndef NES_MIRRORING	// use custom mirroring: 0=horiz, 1=vert, 2=2400, 3=2000, 4=four, 5=0001, 6=0111
  /* Name Table Mirroring */
  InfoNES_Mirroring( MIRROR_V );
#endif

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 11 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map11_Write)( WORD wAddr, BYTE byData )
{
  BYTE byPrgBank = ( byData & 0x03 ) << 2;
  BYTE byChrBank = ( ( byData & 0xf0 ) >> 4 ) << 3;

  /* Set ROM Banks */
  ROMBANK0 = ROMPAGE( ( byPrgBank + 0 ) % ( NesHeader.byRomSize << 1 ) );
  ROMBANK1 = ROMPAGE( ( byPrgBank + 1 ) % ( NesHeader.byRomSize << 1 ) );
  ROMBANK2 = ROMPAGE( ( byPrgBank + 2 ) % ( NesHeader.byRomSize << 1 ) );
  ROMBANK3 = ROMPAGE( ( byPrgBank + 3 ) % ( NesHeader.byRomSize << 1 ) );

  /* Set PPU Banks */
  PPUBANK[ 0 ] = VROMPAGE( ( byChrBank + 0 ) % ( NesHeader.byVRomSize << 3 ) );
  PPUBANK[ 1 ] = VROMPAGE( ( byChrBank + 1 ) % ( NesHeader.byVRomSize << 3 ) );
  PPUBANK[ 2 ] = VROMPAGE( ( byChrBank + 2 ) % ( NesHeader.byVRomSize << 3 ) );
  PPUBANK[ 3 ] = VROMPAGE( ( byChrBank + 3 ) % ( NesHeader.byVRomSize << 3 ) );
  PPUBANK[ 4 ] = VROMPAGE( ( byChrBank + 4 ) % ( NesHeader.byVRomSize << 3 ) );
  PPUBANK[ 5 ] = VROMPAGE( ( byChrBank + 5 ) % ( NesHeader.byVRomSize << 3 ) );
  PPUBANK[ 6 ] = VROMPAGE( ( byChrBank + 6 ) % ( NesHeader.byVRomSize << 3 ) );
  PPUBANK[ 7 ] = VROMPAGE( ( byChrBank + 7 ) % ( NesHeader.byVRomSize << 3 ) );
  InfoNES_SetupChr();
}
