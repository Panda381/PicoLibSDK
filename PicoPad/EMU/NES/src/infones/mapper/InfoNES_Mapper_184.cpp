/*===================================================================*/
/*                                                                   */
/*                           Mapper 184                              */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 184                                            */
/*-------------------------------------------------------------------*/
void Map184_Init()
{
  /* Initialize Mapper */
  MapperInit = Map184_Init;

  /* Write to Mapper */
  MapperWrite = Map0_Write;

  /* Write to SRAM */
  MapperSram = Map184_Sram;

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
  ROMBANK2 = ROMPAGE( 2 );
  ROMBANK3 = ROMPAGE( 3 );

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
/*  Mapper 184 Write to Sram Function                                */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map184_Sram)( WORD wAddr, BYTE byData )
{
  u8 d = byData & 7;
  d <<= 2;
  d %= ( NesHeader.byVRomSize << 3 );
  PPUBANK[ 0 ] = VROMPAGE(d + 0); 
  PPUBANK[ 1 ] = VROMPAGE(d + 1); 
  PPUBANK[ 2 ] = VROMPAGE(d + 2); 
  PPUBANK[ 3 ] = VROMPAGE(d + 3); 

  d = (byData >> 4) & 7;
  d <<= 2;
  d %= ( NesHeader.byVRomSize << 3 );
  PPUBANK[ 4 ] = VROMPAGE(d + 0); 
  PPUBANK[ 5 ] = VROMPAGE(d + 1); 
  PPUBANK[ 6 ] = VROMPAGE(d + 2); 
  PPUBANK[ 7 ] = VROMPAGE(d + 3); 

  InfoNES_SetupChr();
}
