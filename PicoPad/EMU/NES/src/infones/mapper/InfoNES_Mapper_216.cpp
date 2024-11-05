/*===================================================================*/
/*                                                                   */
/*                    Mapper 216 (Bonza)                             */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 216                                            */
/*-------------------------------------------------------------------*/
void Map216_Init()
{
  /* Initialize Mapper */
  MapperInit = Map216_Init;

  /* Write to Mapper */
  MapperWrite = Map216_Write;

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
  ROMBANK2 = ROMPAGE( 2 );
  ROMBANK3 = ROMPAGE( 3 );

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
/*  Mapper 216 Read from APU Function                                  */
/*-------------------------------------------------------------------*/
//BYTE FASTCODE NOFLASH(Map216_ReadApu)(WORD wAddr)
//{
  /*
 *  Dummy Read from Apu
 *
 */
//  return 0;
//}

/*-------------------------------------------------------------------*/
/*  Mapper 216 Write Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map216_Write)( WORD wAddr, BYTE byData )
{
	int d = (wAddr & 1) << 2;
	int m = (NesHeader.byRomSize << 1) - 1;
	ROMBANK0 = ROMPAGE( (d + 0) & m );
	ROMBANK1 = ROMPAGE( (d + 1) & m );
	ROMBANK2 = ROMPAGE( (d + 2) & m );
	ROMBANK3 = ROMPAGE( (d + 3) & m );

  if ( NesHeader.byVRomSize > 0 )
  {
	d = ((wAddr & 0x0e) >> 1) << 3;
	m = (NesHeader.byVRomSize << 3) - 1;
	int i;
	for (i = 0; i < 8; i++) PPUBANK[i] = VROMPAGE((d + i) & m);
	InfoNES_SetupChr();
  }
}
