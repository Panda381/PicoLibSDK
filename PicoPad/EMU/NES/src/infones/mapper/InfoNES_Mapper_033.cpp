/*===================================================================*/
/*                                                                   */
/*                 Mapper 33 (Taito TC0190/TC0350)                   */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map33_Switch;	// 0x00
	BYTE	md_Map33_IRQ_Enable;	// 0x01
	BYTE	md_Map33_IRQ_Cnt;	// 0x02
	u8	md_res;			// 0x03
	BYTE	md_Map33_Regs[ 8 ];	// 0x04
} sMap33_Data;

STATIC_ASSERT((sizeof(sMap33_Data) == 0x0C) && (sizeof(sMap33_Data) <= MAPDATA_SIZE), "Incorrect sMap33_Data!");

#define MAPDATA33		((sMap33_Data*)NES->mapdata)	// mapper data

#define Map33_Switch		MAPDATA33->md_Map33_Switch
#define Map33_IRQ_Enable	MAPDATA33->md_Map33_IRQ_Enable
#define Map33_IRQ_Cnt		MAPDATA33->md_Map33_IRQ_Cnt
#define Map33_Regs		MAPDATA33->md_Map33_Regs

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 33                                             */
/*-------------------------------------------------------------------*/
void Map33_Init()
{
  /* Initialize Mapper */
  MapperInit = Map33_Init;

  /* Write to Mapper */
  MapperWrite = Map33_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map0_HSync; // Map33_HSync;

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
      Map33_Regs[ nPage ] = nPage;
    }
    InfoNES_SetupChr();
  } 
  else 
  {
    for ( int nPage = 0; nPage < 8; ++nPage )
    {
      Map33_Regs[ nPage ] = 0;
    }
  }

  /* Initialize State Registers */
  Map33_Switch = 0;
  Map33_IRQ_Enable = 0;
  Map33_IRQ_Cnt = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 33 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map33_Write)( WORD wAddr, BYTE byData )
{
  /* Set ROM Banks */
  switch ( wAddr & 0xf003 )
  {
    case 0x8000:
      byData &= 0x3f;
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK0 = ROMPAGE( byData );

      if ( byData & 0x40 )
      {
        InfoNES_Mirroring( MIRROR_H );
      } else {
        InfoNES_Mirroring( MIRROR_V );
      }
      break;

    case 0x8001:
      byData &= 0x3f;
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK1 = ROMPAGE( byData );
      break;

    case 0x8002:
      Map33_Regs[ 0 ] = byData * 2;
      Map33_Regs[ 1 ] = byData * 2 + 1;

      PPUBANK[ 0 ] = VROMPAGE( Map33_Regs[ 0 ] % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 1 ] = VROMPAGE( Map33_Regs[ 1 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0x8003:
      Map33_Regs[ 2 ] = byData * 2;
      Map33_Regs[ 3 ] = byData * 2 + 1;

      PPUBANK[ 2 ] = VROMPAGE( Map33_Regs[ 2 ] % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 3 ] = VROMPAGE( Map33_Regs[ 3 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xa000:
      Map33_Regs[ 4 ] = byData;
      PPUBANK[ 4 ] = VROMPAGE( Map33_Regs[ 4 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xa001:
      Map33_Regs[ 5 ] = byData;
      PPUBANK[ 5 ] = VROMPAGE( Map33_Regs[ 5 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xa002:
      Map33_Regs[ 6 ] = byData;
      PPUBANK[ 6 ] = VROMPAGE( Map33_Regs[ 6 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xa003:
      Map33_Regs[ 7 ] = byData;
      PPUBANK[ 7 ] = VROMPAGE( Map33_Regs[ 7 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;
/*
    case 0xc000:
      Map33_IRQ_Cnt = byData;
      break;

    case 0xc001:
    case 0xc002:
    case 0xe001:
    case 0xe002:
      if ( Map33_IRQ_Cnt == byData )
      {
        Map33_IRQ_Enable = 0xff;
      } else {
        Map33_IRQ_Enable = byData;
      }
      break;
*/
/*
    case 0xe000:
      if ( byData & 0x40 )
      {
        InfoNES_Mirroring( MIRROR_H );
      } else {
        InfoNES_Mirroring( MIRROR_V );
      }
      break;
*/
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 33 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
#if 0
void FASTCODE NOFLASH(Map33_HSync)()
{
/*
 *  Callback at HSync
 *
 */
  if ( Map33_IRQ_Enable )
  {
    if ( Map33_IRQ_Enable == 0xff )
    {
      if ( PPU_Scanline == (WORD)( 0xff - Map33_IRQ_Cnt ) )
      {
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
        IRQ_REQ;
#endif
        Map33_IRQ_Cnt = 0;
        Map33_IRQ_Enable = 0;
      }
    } else {
      if ( Map33_IRQ_Cnt == 0xff )
      {
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
        IRQ_REQ;
#endif
        Map33_IRQ_Cnt = 0;
        Map33_IRQ_Enable = 0;
      } else {
        Map33_IRQ_Cnt++;
      }
    }
  }
}
#endif
