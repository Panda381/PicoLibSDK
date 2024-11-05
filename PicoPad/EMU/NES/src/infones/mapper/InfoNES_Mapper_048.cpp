/*===================================================================*/
/*                                                                   */
/*                   Mapper 48 (Taito TC0190V)                       */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map48_IRQ_Enable;	// 0x00
	BYTE	md_Map48_IRQ_Cnt;	// 0x01
	BYTE	md_Map48_IRQ_Latch;	// 0x02
	u8	res;			// 0x03
} sMap48_Data;

STATIC_ASSERT((sizeof(sMap48_Data) == 0x04) && (sizeof(sMap48_Data) <= MAPDATA_SIZE), "Incorrect sMap48_Data!");

#define MAPDATA48		((sMap48_Data*)NES->mapdata)	// mapper data

#define Map48_IRQ_Enable	MAPDATA48->md_Map48_IRQ_Enable
#define Map48_IRQ_Cnt		MAPDATA48->md_Map48_IRQ_Cnt
#define Map48_IRQ_Latch		MAPDATA48->md_Map48_IRQ_Latch

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 48                                             */
/*-------------------------------------------------------------------*/
void Map48_Init()
{
  /* Initialize Mapper */
  MapperInit = Map48_Init;

  /* Write to Mapper */
  MapperWrite = Map48_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map48_HSync;

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
  Map48_IRQ_Enable = 0;
  Map48_IRQ_Cnt = 0;
  Map48_IRQ_Latch = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 48 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map48_Write)( WORD wAddr, BYTE byData )
{
  switch ( wAddr & 0xf003)
  {
    case 0x8000:
      /* Set ROM Banks */
      ROMBANK0 = ROMPAGE( (byData & 0x3f) % ( NesHeader.byRomSize << 1 ) );
      break;

    case 0x8001:
      /* Set ROM Banks */
      ROMBANK1 = ROMPAGE( (byData & 0x3f) % ( NesHeader.byRomSize << 1 ) );
      break;  
 
    /* Set PPU Banks */
    case 0x8002:
      PPUBANK[ 0 ] = VROMPAGE( ( ( byData << 1 ) + 0 ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 1 ] = VROMPAGE( ( ( byData << 1 ) + 1 ) % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0x8003:
      PPUBANK[ 2 ] = VROMPAGE( ( ( byData << 1 ) + 0 ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 3 ] = VROMPAGE( ( ( byData << 1 ) + 1 ) % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xa000:
      PPUBANK[ 4 ] = VROMPAGE( ( byData + 0 ) % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xa001:
      PPUBANK[ 5 ] = VROMPAGE( ( byData + 0 ) % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xa002:
      PPUBANK[ 6 ] = VROMPAGE( ( byData + 0 ) % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xa003:
      PPUBANK[ 7 ] = VROMPAGE( ( byData + 0 ) % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xc000:
      Map48_IRQ_Latch = byData;
      break;

    case 0xc001:
      Map48_IRQ_Cnt = Map48_IRQ_Latch;
      break;

    case 0xc002:
      Map48_IRQ_Enable = 1;
      break;

    case 0xc003:
      Map48_IRQ_Enable = 0;
      break;

    case 0xe000:
      /* Name Table Mirroring */ 
      if ( byData & 0x40 )
      {
        InfoNES_Mirroring( MIRROR_H );
      } else {
        InfoNES_Mirroring( MIRROR_V );
      }
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 48 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map48_HSync)()
{
/*
 *  Callback at HSync
 *
 */
  if ( Map48_IRQ_Enable )
  {
    if ( 0 <= PPU_Scanline && PPU_Scanline <= 239 )
    {
      if ( PPU_R1 & R1_SHOW_SCR || PPU_R1 & R1_SHOW_SP )
      {
        if ( Map48_IRQ_Cnt == 0xff )
        {
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
          IRQ_REQ;
#endif
          Map48_IRQ_Enable = 0;
        }
        Map48_IRQ_Cnt++;
      }
    }
  }
}
