/*===================================================================*/
/*                                                                   */
/*                       Mapper 230 : 22-in-1                        */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
//typedef struct {
//	BYTE	md_Map230_RomSw;	// 0x00
//	u8	md_res[3];		// 0x01
//} sMap230_Data;

//STATIC_ASSERT((sizeof(sMap230_Data) == 0x04) && (sizeof(sMap230_Data) <= MAPDATA_SIZE), "Incorrect sMap230_Data!");

//#define MAPDATA230	((sMap230_Data*)NES->mapdata)	// mapper data

//#define Map230_RomSw	MAPDATA230->md_Map230_RomSw

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 230                                            */
/*-------------------------------------------------------------------*/
void Map230_Init()
{
  /* Initialize Mapper */
  MapperInit = Map230_Init;

  /* Write to Mapper */
  MapperWrite = Map230_Write;

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

  /* Initialize Registers */
//  Map230_RomSw ^= 1;

  /* Set ROM Banks */
//  Map230_Write(0x8000, 0);

    ROMBANK0 = ROMPAGE( 0 );
    ROMBANK1 = ROMPAGE( 1 );
    ROMBANK2 = ROMPAGE( 14 );
    ROMBANK3 = ROMPAGE( 15 );

    InfoNES_Mirroring( MIRROR_H );

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
/*  Mapper 230 Write Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map230_Write)( WORD wAddr, BYTE byData )
{
  u8 m = (NesHeader.byRomSize << 1) - 1;
  u8 d;

#if 0
//  if( Map230_RomSw )
  {
    d = (byData & 7) << 1;
    ROMBANK0 = ROMPAGE((d + 0) & m);
    ROMBANK1 = ROMPAGE((d + 1) & m);
    d = 7 << 1;
    ROMBANK2 = ROMPAGE((d + 0) & m);
    ROMBANK3 = ROMPAGE((d + 1) & m);
    InfoNES_Mirroring( MIRROR_V );
  }
//  else

#else
  {
    d = (byData & 0x1f) + 8;
    if( byData & 0x20 )
    {
      d <<= 1;
      ROMBANK0 = ROMPAGE((d + 0) & m);
      ROMBANK1 = ROMPAGE((d + 1) & m);
      ROMBANK2 = ROMPAGE((d + 0) & m);
      ROMBANK3 = ROMPAGE((d + 1) & m);
    }
    else
    {
      d >>= 1;
      d <<= 2;
      ROMBANK0 = ROMPAGE((d + 0) & m);
      ROMBANK1 = ROMPAGE((d + 1) & m);
      ROMBANK2 = ROMPAGE((d + 2) & m);
      ROMBANK3 = ROMPAGE((d + 3) & m);
    }

    if( byData & 0x40 )
    {
      InfoNES_Mirroring( MIRROR_V );
    }
    else
    {
      InfoNES_Mirroring( MIRROR_H );
    }
  }
#endif
}
