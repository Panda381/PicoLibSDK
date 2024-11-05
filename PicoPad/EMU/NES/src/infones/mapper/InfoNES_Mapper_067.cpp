/*===================================================================*/
/*                                                                   */
/*               Mapper 67 (Sunsoft Mapper #3)                       */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map67_IRQ_Enable;	// 0x00
	BYTE	md_Map67_IRQ_Cnt;	// 0x01
	BYTE	md_Map67_IRQ_Latch;	// 0x02
	u8	md_res;			// 0x03
} sMap67_Data;

STATIC_ASSERT((sizeof(sMap67_Data) == 0x04) && (sizeof(sMap67_Data) <= MAPDATA_SIZE), "Incorrect sMap67_Data!");

#define MAPDATA67		((sMap67_Data*)NES->mapdata)	// mapper data

#define Map67_IRQ_Enable	MAPDATA67->md_Map67_IRQ_Enable
#define Map67_IRQ_Cnt		MAPDATA67->md_Map67_IRQ_Cnt
#define Map67_IRQ_Latch		MAPDATA67->md_Map67_IRQ_Latch

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 67                                             */
/*-------------------------------------------------------------------*/
void Map67_Init()
{
  /* Initialize Mapper */
  MapperInit = Map67_Init;

  /* Write to Mapper */
  MapperWrite = Map67_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map67_HSync;

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
  PPUBANK[ 0 ] = VROMPAGE( 0 );
  PPUBANK[ 1 ] = VROMPAGE( 1 );
  PPUBANK[ 2 ] = VROMPAGE( 2 );
  PPUBANK[ 3 ] = VROMPAGE( 3 );
  PPUBANK[ 4 ] = VROMPAGE( ( NesHeader.byVRomSize << 3 ) - 4 );
  PPUBANK[ 5 ] = VROMPAGE( ( NesHeader.byVRomSize << 3 ) - 3 );
  PPUBANK[ 6 ] = VROMPAGE( ( NesHeader.byVRomSize << 3 ) - 2 );
  PPUBANK[ 7 ] = VROMPAGE( ( NesHeader.byVRomSize << 3 ) - 1 );
  InfoNES_SetupChr();

  /* Initialize IRQ Registers */
  Map67_IRQ_Enable = 0;
  Map67_IRQ_Cnt = 0;
  Map67_IRQ_Latch = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 67 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map67_Write)( WORD wAddr, BYTE byData )
{
  switch ( wAddr & 0xf800 )
  {
    /* Set PPU Banks */
    case 0x8800:
      byData <<= 1;
      byData %= ( NesHeader.byVRomSize << 3 );

      PPUBANK[ 0 ] = VROMPAGE( byData + 0 );
      PPUBANK[ 1 ] = VROMPAGE( byData + 1 );
      InfoNES_SetupChr();
      break;

    case 0x9800:
      byData <<= 1;
      byData %= ( NesHeader.byVRomSize << 3 );

      PPUBANK[ 2 ] = VROMPAGE( byData + 0 );
      PPUBANK[ 3 ] = VROMPAGE( byData + 1 );
      InfoNES_SetupChr();
      break;

    case 0xa800:
      byData <<= 1;
      byData %= ( NesHeader.byVRomSize << 3 );

      PPUBANK[ 4 ] = VROMPAGE( byData + 0 );
      PPUBANK[ 5 ] = VROMPAGE( byData + 1 );
      InfoNES_SetupChr();
      break;

    case 0xb800:
      byData <<= 1;
      byData %= ( NesHeader.byVRomSize << 3 );

      PPUBANK[ 6 ] = VROMPAGE( byData + 0 );
      PPUBANK[ 7 ] = VROMPAGE( byData + 1 );
      InfoNES_SetupChr();
      break;

    case 0xc800:
      Map67_IRQ_Cnt = Map67_IRQ_Latch;
      Map67_IRQ_Latch = byData;
      break;

    case 0xd800:
      Map67_IRQ_Enable = byData & 0x10;
      break;

    case 0xe800:
      switch ( byData & 0x03 )
      {
        case 0:
          InfoNES_Mirroring( MIRROR_V );
          break;
        case 1:
          InfoNES_Mirroring( MIRROR_H );
          break;
        case 2:
          InfoNES_Mirroring( MIRROR_0 );
          break;
        case 3:
          InfoNES_Mirroring( MIRROR_1 );
          break;
      }
      break;

    /* Set ROM Banks */
    case 0xf800:
      byData <<= 1;
      byData %= ( NesHeader.byRomSize << 1 );

      ROMBANK0 = ROMPAGE( byData + 0 );
      ROMBANK1 = ROMPAGE( byData + 1 );
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 67 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map67_HSync)()
{
/*
 *  Callback at HSync
 *
 */
  if ( Map67_IRQ_Enable )
  {
    if ( 0 <= PPU_Scanline && PPU_Scanline <= 239 )
    {
      if ( PPU_R1 & R1_SHOW_SCR || PPU_R1 & R1_SHOW_SP )
      {
        if ( --Map67_IRQ_Cnt == 0xf6 )
        {
          Map67_IRQ_Cnt = Map67_IRQ_Latch;
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
          IRQ_REQ;
#endif
        }
      }
    }
  }
}
