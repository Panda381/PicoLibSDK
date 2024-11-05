/*===================================================================*/
/*                                                                   */
/*                    Mapper 117 (PC-Future)                         */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map117_IRQ_Line;	// 0x00
	BYTE	md_Map117_IRQ_Enable1;	// 0x01
	BYTE	md_Map117_IRQ_Enable2;	// 0x02
	u8	md_res;			// 0x03
} sMap117_Data;

STATIC_ASSERT((sizeof(sMap117_Data) == 0x04) && (sizeof(sMap117_Data) <= MAPDATA_SIZE), "Incorrect sMap117_Data!");

#define MAPDATA117		((sMap117_Data*)NES->mapdata)	// mapper data

#define Map117_IRQ_Line		MAPDATA117->md_Map117_IRQ_Line
#define Map117_IRQ_Enable1	MAPDATA117->md_Map117_IRQ_Enable1
#define Map117_IRQ_Enable2	MAPDATA117->md_Map117_IRQ_Enable2

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 117                                            */
/*-------------------------------------------------------------------*/
void Map117_Init()
{
  /* Initialize Mapper */
  MapperInit = Map117_Init;

  /* Write to Mapper */
  MapperWrite = Map117_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map117_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  /* Set ROM Banks */
  ROMBANK0 = ROMPAGE( 0 );
  ROMBANK1 = ROMPAGE( 1 );
  ROMBANK2 = ROMLASTPAGE( 1 );
  ROMBANK3 = ROMLASTPAGE( 0 );

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 )
  {
    for ( int nPage = 0; nPage < 8; ++nPage )
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    InfoNES_SetupChr();
  }

  /* Initialize IRQ Registers */
  Map117_IRQ_Line = 0;
  Map117_IRQ_Enable1 = 0;
  Map117_IRQ_Enable2 = 1;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 117 Write Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map117_Write)( WORD wAddr, BYTE byData )
{
  switch ( wAddr )
  {
  
    /* Set ROM Banks */
    case 0x8000:
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK0 = ROMPAGE( byData );
      break;

    case 0x8001:
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK1 = ROMPAGE( byData );
      break;

    case 0x8002:
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK2 = ROMPAGE( byData );
      break;

    /* Set PPU Banks */
    case 0xA000:
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 0 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;

    case 0xA001:
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 1 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;

    case 0xA002:
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 2 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;

    case 0xA003:
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 3 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;

    case 0xA004:
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 4 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;

    case 0xA005:
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 5 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;

    case 0xA006:
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 6 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;

    case 0xA007:
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 7 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;

    case 0xc001:
    case 0xc002:
    case 0xc003:
      Map117_IRQ_Enable1 = Map117_IRQ_Line = byData;
      break;

    case 0xe000:
      Map117_IRQ_Enable2 = byData & 0x01;
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 117 H-Sync Function                                       */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map117_HSync)()
{
  if ( Map117_IRQ_Enable1 && Map117_IRQ_Enable2 )
  {
    if ( Map117_IRQ_Line == PPU_Scanline )
    {
      Map117_IRQ_Enable1 = 0x00;
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
      IRQ_REQ;
#endif
    }
  }
}
