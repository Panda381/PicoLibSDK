/*===================================================================*/
/*                                                                   */
/*                        Mapper 7 (AOROM)                           */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 7                                              */
/*-------------------------------------------------------------------*/
void Map7_Init()
{
  /* Initialize Mapper */
  MapperInit = Map7_Init;

  /* Write to Mapper */
  MapperWrite = Map7_Write;

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
  if (NesHeader.byVRomSize > 0)
  {
    for (int nPage = 0; nPage < 8; ++nPage)
      PPUBANK[nPage] = VROMPAGE(nPage);
    InfoNES_SetupChr();
  }

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 

  // default mirroring
#ifndef NES_MIRRORING	// use custom mirroring: 0=horiz, 1=vert, 2=2400, 3=2000, 4=four, 5=0001, 6=0111
  InfoNES_Mirroring( MIRROR_0 );
#endif
}

/*-------------------------------------------------------------------*/
/*  Mapper 7 Write Function                                          */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map7_Write)( WORD wAddr, BYTE byData )
{
  BYTE byBank;

  /* Set ROM Banks */
  if (NesHeader.byRomSize > 0)
  {
    byBank = ( byData & 0x07 ) << 2;
    int m = ( NesHeader.byRomSize << 1 ) - 1;

    ROMBANK0 = ROMPAGE( byBank & m );
    ROMBANK1 = ROMPAGE( (byBank + 1) & m );
    ROMBANK2 = ROMPAGE( (byBank + 2) & m );
    ROMBANK3 = ROMPAGE( (byBank + 3) & m );
  }

  /* Name Table Mirroring */
  InfoNES_Mirroring( (byData & 0x10) ? MIRROR_1 : MIRROR_0 );
}
