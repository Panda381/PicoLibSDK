/*===================================================================*/
/*                                                                   */
/*                   Mapper 16 (Bandai Mapper)                       */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map16_Regs[3];	// 0x00
	BYTE	md_Map16_IRQ_Enable;	// 0x03
	DWORD	md_Map16_IRQ_Cnt;	// 0x04
	DWORD	md_Map16_IRQ_Latch;	// 0x08
} sMap16_Data;

STATIC_ASSERT((sizeof(sMap16_Data) == 0x0C) && (sizeof(sMap16_Data) <= MAPDATA_SIZE), "Incorrect sMap16_Data!");

#define MAPDATA16		((sMap16_Data*)NES->mapdata)	// mapper data

#define Map16_Regs		MAPDATA16->md_Map16_Regs
#define Map16_IRQ_Enable	MAPDATA16->md_Map16_IRQ_Enable
#define Map16_IRQ_Cnt		MAPDATA16->md_Map16_IRQ_Cnt
#define Map16_IRQ_Latch		MAPDATA16->md_Map16_IRQ_Latch

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 16                                             */
/*-------------------------------------------------------------------*/
void Map16_Init()
{
  /* Initialize Mapper */
  MapperInit = Map16_Init;

  /* Write to Mapper */
  MapperWrite = Map16_Write;

  /* Write to SRAM */
  MapperSram = Map16_Write;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map16_HSync;

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

  /* Initialize State Flag */
  Map16_Regs[ 0 ] = 0;
  Map16_Regs[ 1 ] = 0;
  Map16_Regs[ 2 ] = 0;

  Map16_IRQ_Enable = 0;
  Map16_IRQ_Cnt = 0;
  Map16_IRQ_Latch = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 16 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map16_Write)( WORD wAddr, BYTE byData )
{
  switch ( wAddr & 0x000f )
  {
    case 0x0000:
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 0 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;

    case 0x0001:
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 1 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;

    case 0x0002:
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 2 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;

    case 0x0003:
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 3 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;

    case 0x0004:
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 4 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;

    case 0x0005:
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 5 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;

    case 0x0006:
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 6 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;

    case 0x0007:
      byData %= ( NesHeader.byVRomSize << 3 );
      PPUBANK[ 7 ] = VROMPAGE( byData );
      InfoNES_SetupChr();
      break;

    case 0x0008:
      byData <<= 1;
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK0 = ROMPAGE( byData );
      ROMBANK1 = ROMPAGE( byData + 1 );
      break;

    case 0x0009:
      switch ( byData & 0x03 )
      {
        case 0x00:
          InfoNES_Mirroring( MIRROR_V );
          break;

        case 0x01:
          InfoNES_Mirroring( MIRROR_H );
          break;    

        case 0x02:
          InfoNES_Mirroring( MIRROR_0 );
          break;

        case 0x03:
          InfoNES_Mirroring( MIRROR_1 );
          break; 
      }
      break;

      case 0x000a:
        Map16_IRQ_Enable = byData & 0x01;
        Map16_IRQ_Cnt = Map16_IRQ_Latch << STEP_PER_SCANLINE_SHIFT;
        break;

      case 0x000b:
        Map16_IRQ_Latch = ( Map16_IRQ_Latch & 0xff00 ) | byData;
        break;

      case 0x000c:
        Map16_IRQ_Latch = ( (DWORD)byData << 8 ) | ( Map16_IRQ_Latch & 0x00ff );
        break;

      case 0x000d:
        /* Write Protect */
        break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 16 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map16_HSync)()
{
  if ( Map16_IRQ_Enable )
  {
    /* Normal IRQ */
    if ( Map16_IRQ_Cnt <= STEP_PER_SCANLINE_H )
    {
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
      IRQ_REQ;
#endif
      Map16_IRQ_Cnt = 0;
      Map16_IRQ_Enable = 0;
    } else {
      Map16_IRQ_Cnt -= STEP_PER_SCANLINE_H;
    }
  }
}
