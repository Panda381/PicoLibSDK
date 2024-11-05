/*===================================================================*/
/*                                                                   */
/*                      Mapper 65 (Irem H3001)                       */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map65_IRQ_Enable;	// 0x00
	u8	res[3];			// 0x01
	s32	md_Map65_IRQ_Cnt;	// 0x04
	s32	md_Map65_IRQ_Latch;	// 0x08
} sMap65_Data;

STATIC_ASSERT((sizeof(sMap65_Data) == 0x0C) && (sizeof(sMap65_Data) <= MAPDATA_SIZE), "Incorrect sMap65_Data!");

#define MAPDATA65		((sMap65_Data*)NES->mapdata)	// mapper data

#define Map65_IRQ_Enable	MAPDATA65->md_Map65_IRQ_Enable
#define Map65_IRQ_Cnt		MAPDATA65->md_Map65_IRQ_Cnt
#define Map65_IRQ_Latch		MAPDATA65->md_Map65_IRQ_Latch

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 65                                             */
/*-------------------------------------------------------------------*/
void Map65_Init()
{
  /* Initialize Mapper */
  MapperInit = Map65_Init;

  /* Write to Mapper */
  MapperWrite = Map65_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map65_HSync;

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

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 65 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map65_Write)( WORD wAddr, BYTE byData )
{
  switch ( wAddr )
  {
    /* Set ROM Banks */
    case 0x8000:
      ROMBANK0 = ROMPAGE( byData % ( NesHeader.byRomSize << 1) );
      break;

    case 0xa000:
      ROMBANK1 = ROMPAGE( byData % ( NesHeader.byRomSize << 1) );
      break;

    case 0xc000:
      ROMBANK2 = ROMPAGE( byData % ( NesHeader.byRomSize << 1) );
      break;

    // set mirror
    case 0x9001:
      if ( byData & 0x80 )
      {
        InfoNES_Mirroring( MIRROR_H );
      } else {
        InfoNES_Mirroring( MIRROR_V );
      }
      break;

    // set HSYNC IRQ
    case 0x9003:
      Map65_IRQ_Enable = byData & 0x80;
      break;

    case 0x9004:
      Map65_IRQ_Cnt = Map65_IRQ_Latch << STEP_PER_SCANLINE_SHIFT;
      break;

    case 0x9005:
      Map65_IRQ_Latch = ( Map65_IRQ_Latch & 0x00ff ) | ((DWORD)byData << 8 );
      break;

    case 0x9006:
      Map65_IRQ_Latch = ( Map65_IRQ_Latch & 0xff00 ) | (DWORD)byData;
      break;

    /* Set PPU Banks */
    case 0xb000:
      PPUBANK[ 0 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xb001:
      PPUBANK[ 1 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xb002:
      PPUBANK[ 2 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xb003:
      PPUBANK[ 3 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xb004:
      PPUBANK[ 4 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xb005:
      PPUBANK[ 5 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xb006:
      PPUBANK[ 6 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xb007:
      PPUBANK[ 7 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 65 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map65_HSync)()
{
/*
 *  Callback at HSync
 *
 */
  if ( Map65_IRQ_Enable )
  {
    if ( Map65_IRQ_Cnt < (STEP_PER_SCANLINE_H - (4 << STEP_PER_SCANLINE_SHIFT)))
    {
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
      IRQ_REQ;
#endif
      Map65_IRQ_Enable = 0;
      Map65_IRQ_Cnt = 0xffff << STEP_PER_SCANLINE_SHIFT;
    } else {
      Map65_IRQ_Cnt -= STEP_PER_SCANLINE_H;
    }
  }
}
