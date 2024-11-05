/*===================================================================*/
/*                                                                   */
/*         Mapper 193 : MEGA SOFT (NTDEC) : Fighting Hero            */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 193                                            */
/*-------------------------------------------------------------------*/
void Map193_Init()
{
  /* Initialize Mapper */
  MapperInit = Map193_Init;

  /* Write to Mapper */
  MapperWrite = Map0_Write;

  /* Write to SRAM */
  MapperSram = Map193_Sram;

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
  ROMBANK1 = ROMLASTPAGE(2);
  ROMBANK2 = ROMLASTPAGE(1);
  ROMBANK3 = ROMLASTPAGE(0);

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 ) {
    for ( int nPage = 0; nPage < 8; ++nPage )
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    InfoNES_SetupChr();
  }

#ifndef NES_MIRRORING	// use custom mirroring: 0=horiz, 1=vert, 2=2400, 3=2000, 4=four, 5=0001, 6=0111
  InfoNES_Mirroring(MIRROR_V);
#endif

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 193 Write to SRAM Function                                */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map193_Sram)( WORD wAddr, BYTE byData )
{
  switch( wAddr ) {
  case	0x6000:
    byData &= 0xfc;
    PPUBANK[ 0 ] = VROMPAGE( (byData + 0 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 1 ] = VROMPAGE( (byData + 1 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 2 ] = VROMPAGE( (byData + 2 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 3 ] = VROMPAGE( (byData + 3 ) % ( NesHeader.byVRomSize << 3 ) );
    InfoNES_SetupChr();
    break;

  case	0x6001:
    byData &= 0xfe;
    PPUBANK[ 4 ] = VROMPAGE( ( byData + 0 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 5 ] = VROMPAGE( ( byData + 1 ) % ( NesHeader.byVRomSize << 3 ) );
    InfoNES_SetupChr();
    break;

  case	0x6002:
    byData &= 0xfe;
    PPUBANK[ 6 ] = VROMPAGE( ( byData + 0 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 7 ] = VROMPAGE( ( byData + 1 ) % ( NesHeader.byVRomSize << 3 ) );
    InfoNES_SetupChr();
    break;

  case	0x6003:
    ROMBANK0 = ROMPAGE( byData % ( NesHeader.byRomSize << 1 ));
    break;

  case 0x6004:
    InfoNES_Mirroring( (byData & 1) ? MIRROR_H : MIRROR_V);
    break;
  }
}
