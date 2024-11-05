/*===================================================================*/
/*                                                                   */
/*                        Mapper 91 (Pirates)                        */
/*                                                                   */
/*===================================================================*/


// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map91_IRQ_Cnt;	// 0x00
	BYTE	md_Map91_IRQ_Enable;	// 0x01
	WORD	md_Map91_IRQ_CntW;	// 0x02 (sub-mapper 1)
} sMap91_Data;

STATIC_ASSERT((sizeof(sMap91_Data) == 0x04) && (sizeof(sMap91_Data) <= MAPDATA_SIZE), "Incorrect sMap91_Data!");

#define MAPDATA91		((sMap91_Data*)NES->mapdata)	// mapper data

#define Map91_IRQ_Cnt		MAPDATA91->md_Map91_IRQ_Cnt
#define Map91_IRQ_Enable	MAPDATA91->md_Map91_IRQ_Enable
//#define Map91_IRQ_CntW		MAPDATA91->md_Map91_IRQ_CntW

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 91                                             */
/*-------------------------------------------------------------------*/
void Map91_Init()
{
  /* Initialize Mapper */
  MapperInit = Map91_Init;

  /* Write to Mapper */
  MapperWrite = Map0_Write;

  /* Write to SRAM */
  MapperSram = Map91_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map91_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  /* Set ROM Banks */
  ROMBANK0 = ROMLASTPAGE( 1 );
  ROMBANK1 = ROMLASTPAGE( 0 );
  ROMBANK2 = ROMLASTPAGE( 1 );
  ROMBANK3 = ROMLASTPAGE( 0 );

  Map91_IRQ_Cnt = 0;
  Map91_IRQ_Enable = 0;
//  Map91_IRQ_CntW = 0;

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
/*  Mapper 91 Write to Sram Function                                 */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map91_Sram)( WORD wAddr, BYTE byData )
{
  switch( wAddr & 0xF003)
  {
    /* Set PPU Banks */
    case 0x6000:
      PPUBANK[ 0 ] = VROMPAGE( (byData*2+0) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 1 ] = VROMPAGE( (byData*2+1) % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0x6001:
      PPUBANK[ 2 ] = VROMPAGE( (byData*2+0) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 3 ] = VROMPAGE( (byData*2+1) % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0x6002:
      PPUBANK[ 4 ] = VROMPAGE( (byData*2+0) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 5 ] = VROMPAGE( (byData*2+1) % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0x6003:
      PPUBANK[ 6 ] = VROMPAGE( (byData*2+0) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 7 ] = VROMPAGE( (byData*2+1) % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    /* Set CPU Banks */
    case 0x7000:
      ROMBANK0 = ROMPAGE( byData % ( NesHeader.byRomSize << 1 ) ); 
      break;

    case 0x7001:
      ROMBANK1 = ROMPAGE( byData % ( NesHeader.byRomSize << 1 ) ); 
      break;

    case 0x7002:
      Map91_IRQ_Enable = 0;
      Map91_IRQ_Cnt = 0;
      break;

    case 0x7003:
      Map91_IRQ_Enable = 1;
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 91 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map91_HSync)()
{
  if ( ( 0 <= PPU_Scanline && PPU_Scanline <= 239 ) && 
       ( PPU_R1 & R1_SHOW_SCR || PPU_R1 & R1_SHOW_SP ) )
  {
    if (Map91_IRQ_Enable)
    {
      Map91_IRQ_Cnt++;
      if (Map91_IRQ_Cnt >= 8)
      {
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
        IRQ_REQ;
#endif
        Map91_IRQ_Enable = 0;
      }
    }
  }
}
