/*===================================================================*/
/*                                                                   */
/*             Mapper 212 : Magic Dragon Mapper                      */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 212                                            */
/*-------------------------------------------------------------------*/
void Map212_Init()
{
  /* Initialize Mapper */
  MapperInit = Map212_Init;

  /* Write to Mapper */
  MapperWrite = Map212_Write;

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
  if (NesHeader.byRomSize > 1)
  {
    ROMBANK0 = ROMPAGE(0);
    ROMBANK1 = ROMPAGE(1);
    ROMBANK2 = ROMPAGE(2);
    ROMBANK3 = ROMPAGE(3);
  }
  else if (NesHeader.byRomSize > 0)
  {
    ROMBANK0 = ROMPAGE(0);
    ROMBANK1 = ROMPAGE(1);
    ROMBANK2 = ROMPAGE(0);
    ROMBANK3 = ROMPAGE(1);
  }
  else
  {
    ROMBANK0 = ROMPAGE(0);
    ROMBANK1 = ROMPAGE(0);
    ROMBANK2 = ROMPAGE(0);
    ROMBANK3 = ROMPAGE(0);
  }

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 ) {
    for ( int nPage = 0; nPage < 8; ++nPage )
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    InfoNES_SetupChr();
  }

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 212 Write Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map212_Write)( WORD wAddr, BYTE byData )
{
  u8 d, m;

  /* Set ROM Banks */
  m = (NesHeader.byRomSize << 1) - 1;
  if (wAddr & 0x4000)
  {
    d = (u8)((wAddr >> 1) & 3) << 2;
    ROMBANK0 = ROMPAGE( (d + 0) & m );
    ROMBANK1 = ROMPAGE( (d + 1) & m );
    ROMBANK2 = ROMPAGE( (d + 2) & m );
    ROMBANK3 = ROMPAGE( (d + 3) & m );
  }
  else
  {
    d = (u8)(wAddr & 7) << 1;
    ROMBANK0 = ROMPAGE( (d + 0) & m );
    ROMBANK1 = ROMPAGE( (d + 1) & m );
    ROMBANK2 = ROMPAGE( (d + 0) & m );
    ROMBANK3 = ROMPAGE( (d + 1) & m );
  }

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 )
  {
    m = (NesHeader.byVRomSize << 3) - 1;
    d = (u8)(wAddr & 7) << 3;
    PPUBANK[ 0 ] = VROMPAGE( (d + 0) & m );
    PPUBANK[ 1 ] = VROMPAGE( (d + 1) & m );
    PPUBANK[ 2 ] = VROMPAGE( (d + 2) & m );
    PPUBANK[ 3 ] = VROMPAGE( (d + 3) & m );
    PPUBANK[ 4 ] = VROMPAGE( (d + 4) & m );
    PPUBANK[ 5 ] = VROMPAGE( (d + 5) & m );
    PPUBANK[ 6 ] = VROMPAGE( (d + 6) & m );
    PPUBANK[ 7 ] = VROMPAGE( (d + 7) & m );
    InfoNES_SetupChr();
  }

  // set mirror
  InfoNES_Mirroring( ((wAddr >> 3) & 1) ? MIRROR_H : MIRROR_V );
}
