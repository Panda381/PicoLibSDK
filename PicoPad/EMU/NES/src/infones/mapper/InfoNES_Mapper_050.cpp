/*===================================================================*/
/*                                                                   */
/*                        Mapper 50 (Pirates)                        */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map50_IRQ_Enable;	// 0x00
	u8	res[3];			// 0x01
} sMap50_Data;

STATIC_ASSERT((sizeof(sMap50_Data) == 0x04) && (sizeof(sMap50_Data) <= MAPDATA_SIZE), "Incorrect sMap50_Data!");

#define MAPDATA50		((sMap50_Data*)NES->mapdata)	// mapper data

#define Map50_IRQ_Enable	MAPDATA50->md_Map50_IRQ_Enable

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 50                                             */
/*-------------------------------------------------------------------*/
void Map50_Init()
{
  /* Initialize Mapper */
  MapperInit = Map50_Init;

  /* Write to Mapper */
  MapperWrite = Map0_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map50_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map50_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = ROMPAGE( 15 % ( NesHeader.byRomSize << 1 ) );

  /* Set ROM Banks */
  ROMBANK0 = ROMPAGE( 8 % ( NesHeader.byRomSize << 1 ) );
  ROMBANK1 = ROMPAGE( 9 % ( NesHeader.byRomSize << 1 ) );
  ROMBANK2 = ROMPAGE( 0 % ( NesHeader.byRomSize << 1 ) );
  ROMBANK3 = ROMPAGE( 11 % ( NesHeader.byRomSize << 1 ) );

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 )
  {
    for ( int nPage = 0; nPage < 8; ++nPage )
    {
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    }    
    InfoNES_SetupChr();
  }

  /* Initialize IRQ Registers */
  Map50_IRQ_Enable = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 50 Write to Apu Function                                  */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map50_Apu)( WORD wAddr, BYTE byData )
{
  if ( ( wAddr & 0xE060 ) == 0x4020 )
  {
    if( wAddr & 0x0100 )
    {
      Map50_IRQ_Enable = byData & 0x01;
    }
    else
    {
      BYTE byDummy;

      byDummy = ( byData & 0x08 ) | ( ( byData & 0x01 ) << 2 ) | ( ( byData & 0x06 ) >> 1 );
      ROMBANK2 = ROMPAGE( byDummy % ( NesHeader.byRomSize << 1 ) );
    }
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 50 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map50_HSync)()
{
/*
 *  Callback at HSync
 *
 */
  if ( Map50_IRQ_Enable )
  {
    if ( PPU_Scanline == 21 )
    {
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
      IRQ_REQ;
#endif
    }
  }
}
