/*===================================================================*/
/*                                                                   */
/*                       Mapper 189 (Pirates)                        */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map189_Regs[ 1 ];	// 0x00
	BYTE	md_Map189_IRQ_Cnt;	// 0x01
	BYTE	md_Map189_IRQ_Latch;	// 0x02
	BYTE	md_Map189_IRQ_Enable;	// 0x03
} sMap189_Data;

STATIC_ASSERT((sizeof(sMap189_Data) == 0x04) && (sizeof(sMap189_Data) <= MAPDATA_SIZE), "Incorrect sMap189_Data!");

#define MAPDATA189		((sMap189_Data*)NES->mapdata)	// mapper data

#define Map189_Regs		MAPDATA189->md_Map189_Regs
#define Map189_IRQ_Cnt		MAPDATA189->md_Map189_IRQ_Cnt
#define Map189_IRQ_Latch	MAPDATA189->md_Map189_IRQ_Latch
#define Map189_IRQ_Enable	MAPDATA189->md_Map189_IRQ_Enable

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 189                                            */
/*-------------------------------------------------------------------*/
void Map189_Init()
{
  /* Initialize Mapper */
  MapperInit = Map189_Init;

  /* Write to Mapper */
  MapperWrite = Map189_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map189_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map189_HSync;

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
    {
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    }
    InfoNES_SetupChr();
  }

  /* Initialize IRQ registers */
  Map189_IRQ_Cnt = 0;
  Map189_IRQ_Latch = 0;
  Map189_IRQ_Enable = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 189 Write to Apu Function                                 */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map189_Apu)( WORD wAddr, BYTE byData )
{
  if ( wAddr >= 0x4100 && wAddr <= 0x41FF )
  {
    byData = ( byData & 0x30 ) >> 4;
    ROMBANK0 = ROMPAGE( ( byData * 4 + 0 ) % ( NesHeader.byRomSize << 1 ) );
    ROMBANK1 = ROMPAGE( ( byData * 4 + 1 ) % ( NesHeader.byRomSize << 1 ) );
    ROMBANK2 = ROMPAGE( ( byData * 4 + 2 ) % ( NesHeader.byRomSize << 1 ) );
    ROMBANK3 = ROMPAGE( ( byData * 4 + 3 ) % ( NesHeader.byRomSize << 1 ) );
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 189 Write Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map189_Write)( WORD wAddr, BYTE byData )
{
  /* Set ROM Banks */
  switch( wAddr )
  {
    case 0x8000:
      Map189_Regs[0] = byData;
      break;

    case 0x8001:
      switch( Map189_Regs[0] )
      {
        case 0x40:
	  PPUBANK[ 0 ] = VROMPAGE( ( byData + 0 ) % ( NesHeader.byVRomSize << 3 ) );
	  PPUBANK[ 1 ] = VROMPAGE( ( byData + 1 ) % ( NesHeader.byVRomSize << 3 ) );
	  InfoNES_SetupChr();
	  break;

        case 0x41:
	  PPUBANK[ 2 ] = VROMPAGE( ( byData + 0 ) % ( NesHeader.byVRomSize << 3 ) );
	  PPUBANK[ 3 ] = VROMPAGE( ( byData + 1 ) % ( NesHeader.byVRomSize << 3 ) );
	  InfoNES_SetupChr();
	  break;

        case 0x42:
	  PPUBANK[ 4 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
	  InfoNES_SetupChr();
	  break;

        case 0x43:
	  PPUBANK[ 5 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
	  InfoNES_SetupChr();
	  break;

        case 0x44:
	  PPUBANK[ 6 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
	  InfoNES_SetupChr();
	  break;

        case 0x45:
	  PPUBANK[ 7 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
	  InfoNES_SetupChr();
	  break;

        case 0x46:
	  ROMBANK2 = ROMPAGE( byData % ( NesHeader.byRomSize << 1 ) );
	  break;  

        case 0x47:
	  ROMBANK1 = ROMPAGE( byData % ( NesHeader.byRomSize << 1 ) );
	  break;  
      }
      break;

    case 0xC000:
      Map189_IRQ_Cnt = byData;
      break;

    case 0xC001:
      Map189_IRQ_Latch = byData;
      break;

    case 0xE000:
      Map189_IRQ_Enable = 0;
      break;

    case 0xE001:
      Map189_IRQ_Enable = 1;
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 189 H-Sync Function                                       */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map189_HSync)()
{
/*
 *  Callback at HSync
 *
 */
  if ( Map189_IRQ_Enable )
  {
    if ( 0 <= PPU_Scanline && PPU_Scanline <= 239 )
    {
      if ( PPU_R1 & R1_SHOW_SCR || PPU_R1 & R1_SHOW_SP )
      {
        if ( !( --Map189_IRQ_Cnt ) )
        {
          Map189_IRQ_Cnt = Map189_IRQ_Latch;
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
          IRQ_REQ;
#endif
        }
      }
    }
  }
}
