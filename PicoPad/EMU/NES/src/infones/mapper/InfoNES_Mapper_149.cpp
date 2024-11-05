/*===================================================================*/
/*                                                                   */
/*                    Mapper 149 (PC-Sachen/Hacker)                  */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 149                                            */
/*-------------------------------------------------------------------*/
void Map149_Init()
{
  /* Initialize Mapper */
  MapperInit = Map149_Init;

  /* Write to Mapper */
  MapperWrite = Map149_Write;

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
/*  Mapper 149 Write Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map149_Write)( WORD wAddr, BYTE byData )
{
  BYTE byChrBank;

  /* Set PPU Banks */
  byChrBank = byData >> 7;
  byChrBank <<= 3;
  u8 m = ( NesHeader.byVRomSize << 3 ) - 1;

  PPUBANK[ 0 ] = VROMPAGE( (byChrBank + 0) & m );
  PPUBANK[ 1 ] = VROMPAGE( (byChrBank + 1) & m );
  PPUBANK[ 2 ] = VROMPAGE( (byChrBank + 2) & m );
  PPUBANK[ 3 ] = VROMPAGE( (byChrBank + 3) & m );
  PPUBANK[ 4 ] = VROMPAGE( (byChrBank + 4) & m );
  PPUBANK[ 5 ] = VROMPAGE( (byChrBank + 5) & m );
  PPUBANK[ 6 ] = VROMPAGE( (byChrBank + 6) & m );
  PPUBANK[ 7 ] = VROMPAGE( (byChrBank + 7) & m );

  InfoNES_SetupChr();
}
