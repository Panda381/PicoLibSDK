/*===================================================================*/
/*                                                                   */
/*                          Mapper 244                               */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 244                                            */
/*-------------------------------------------------------------------*/
void Map244_Init()
{
  /* Initialize Mapper */
  MapperInit = Map244_Init;

  /* Write to Mapper */
  MapperWrite = Map244_Write;

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

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

static BYTE prg_perm[4][4] = {
	{ 0, 1, 2, 3, },
	{ 3, 2, 1, 0, },
	{ 0, 2, 1, 3, },
	{ 3, 1, 2, 0, },
};

static BYTE chr_perm[8][8] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7, },
	{ 0, 2, 1, 3, 4, 6, 5, 7, },
	{ 0, 1, 4, 5, 2, 3, 6, 7, },
	{ 0, 4, 1, 5, 2, 6, 3, 7, },
	{ 0, 4, 2, 6, 1, 5, 3, 7, },
	{ 0, 2, 4, 6, 1, 3, 5, 7, },
	{ 7, 6, 5, 4, 3, 2, 1, 0, },
	{ 7, 6, 5, 4, 3, 2, 1, 0, },
};

/*-------------------------------------------------------------------*/
/*  Mapper 244 Write Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map244_Write)( WORD wAddr, BYTE byData )
{
#if 1

	if (byData & 8)
	{
		// setup CHR ROM
		byData = chr_perm[(byData >> 4) & 7][byData & 7] << 3;

		PPUBANK[ 0 ] = VROMPAGE( ( byData + 0 ) % ( NesHeader.byVRomSize << 3 ) );
		PPUBANK[ 1 ] = VROMPAGE( ( byData + 1 ) % ( NesHeader.byVRomSize << 3 ) );
		PPUBANK[ 2 ] = VROMPAGE( ( byData + 2 ) % ( NesHeader.byVRomSize << 3 ) );
		PPUBANK[ 3 ] = VROMPAGE( ( byData + 3 ) % ( NesHeader.byVRomSize << 3 ) );
		PPUBANK[ 4 ] = VROMPAGE( ( byData + 4 ) % ( NesHeader.byVRomSize << 3 ) );
		PPUBANK[ 5 ] = VROMPAGE( ( byData + 5 ) % ( NesHeader.byVRomSize << 3 ) );
		PPUBANK[ 6 ] = VROMPAGE( ( byData + 6 ) % ( NesHeader.byVRomSize << 3 ) );
		PPUBANK[ 7 ] = VROMPAGE( ( byData + 7 ) % ( NesHeader.byVRomSize << 3 ) );
		InfoNES_SetupChr();
	}
	else
	{
		// setup PRG ROM
		byData = prg_perm[(byData >> 4) & 3][byData & 3] << 2;

		ROMBANK0 = ROMPAGE( ( byData + 0 ) % ( NesHeader.byRomSize << 1 ) );
		ROMBANK1 = ROMPAGE( ( byData + 1 ) % ( NesHeader.byRomSize << 1 ) );
		ROMBANK2 = ROMPAGE( ( byData + 2 ) % ( NesHeader.byRomSize << 1 ) );
		ROMBANK3 = ROMPAGE( ( byData + 3 ) % ( NesHeader.byRomSize << 1 ) );
	}

#else

  if( wAddr>=0x8065 && wAddr<=0x80A4 ) {
    /* Set ROM Banks */
    ROMBANK0 = ROMPAGE(((((wAddr-0x8065)&0x3)<<2)+0) % (NesHeader.byRomSize<<1));
    ROMBANK1 = ROMPAGE(((((wAddr-0x8065)&0x3)<<2)+1) % (NesHeader.byRomSize<<1));
    ROMBANK2 = ROMPAGE(((((wAddr-0x8065)&0x3)<<2)+2) % (NesHeader.byRomSize<<1));
    ROMBANK3 = ROMPAGE(((((wAddr-0x8065)&0x3)<<2)+3) % (NesHeader.byRomSize<<1));
  }
  
  if( wAddr>=0x80A5 && wAddr<=0x80E4 ) {
    /* Set ROM Banks */
    ROMBANK0 = ROMPAGE(((((wAddr-0x80A5)&0x7)<<2)+0) % (NesHeader.byRomSize<<1));
    ROMBANK1 = ROMPAGE(((((wAddr-0x80A5)&0x7)<<2)+1) % (NesHeader.byRomSize<<1));
    ROMBANK2 = ROMPAGE(((((wAddr-0x80A5)&0x7)<<2)+2) % (NesHeader.byRomSize<<1));
    ROMBANK3 = ROMPAGE(((((wAddr-0x80A5)&0x7)<<2)+3) % (NesHeader.byRomSize<<1));
  }

#endif
}
