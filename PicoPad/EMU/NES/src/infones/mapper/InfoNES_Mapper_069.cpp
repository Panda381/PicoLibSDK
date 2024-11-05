/*===================================================================*/
/*                                                                   */
/*                  Mapper 69 (Sunsoft FME-7)                        */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map69_IRQ_Enable;	// 0x00
	BYTE	md_Map69_Regs[ 1 ];	// 0x01
	u8	md_res[2];		// 0x02
	DWORD	md_Map69_IRQ_Cnt;	// 0x04
} sMap69_Data;

STATIC_ASSERT((sizeof(sMap69_Data) == 0x08) && (sizeof(sMap69_Data) <= MAPDATA_SIZE), "Incorrect sMap69_Data!");

#define MAPDATA69		((sMap69_Data*)NES->mapdata)	// mapper data

#define Map69_IRQ_Enable	MAPDATA69->md_Map69_IRQ_Enable
#define Map69_Regs		MAPDATA69->md_Map69_Regs
#define Map69_IRQ_Cnt		MAPDATA69->md_Map69_IRQ_Cnt

// BYTE Map69_Prg_Ram[0x2000];
//#define Map69_Prg_Ram		NES->xram

//#define MAP69_XRAM_SIZE	0x2000
//STATIC_ASSERT(MAP69_XRAM_SIZE <= XRAM_SIZE, "Incorrect MAP69_XRAM_SIZE!");

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 69                                             */
/*-------------------------------------------------------------------*/
void Map69_Init()
{
  /* Initialize Mapper */
  MapperInit = Map69_Init;

  /* Write to Mapper */
  MapperWrite = Map69_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map69_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;
//  memset(Map69_Prg_Ram, 0xff, MAP69_XRAM_SIZE);

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

  /* Initialize IRQ Reg */
  Map69_IRQ_Enable = 0;
  Map69_IRQ_Cnt    = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 69 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map69_Write)( WORD wAddr, BYTE byData )
{
  if (wAddr >= 0x8000)
  {
    if (wAddr < 0xA000)
      Map69_Regs[ 0 ] = byData & 0x0f;
    else if (wAddr < 0xC000)
    {
      switch ( Map69_Regs[ 0 ] )
      {
        /* Set PPU Banks */
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
          byData %= ( NesHeader.byVRomSize << 3 );
          PPUBANK[ Map69_Regs[ 0 ] ] = VROMPAGE( byData );
          InfoNES_SetupChr();
          break;

        /* Set ROM Banks */
        case 0x08:
          if (SubMapperNo != 1)
          {
            if ( !( byData & 0x40 ) )
            {
              byData %= ( NesHeader.byRomSize << 1 );
              SRAMBANK = ROMPAGE( byData );
            }
//            else
//              SRAMBANK = Map69_Prg_Ram;
          }
          break;

        case 0x09:
          byData %= ( NesHeader.byRomSize << 1 );
          ROMBANK0 = ROMPAGE( byData );
          break;

        case 0x0a:
          byData %= ( NesHeader.byRomSize << 1 );
          ROMBANK1 = ROMPAGE( byData );
          break;

        case 0x0b:
          byData %= ( NesHeader.byRomSize << 1 );
          ROMBANK2 = ROMPAGE( byData );
          break;

        /* Name Table Mirroring */
        case 0x0c:  
          switch ( byData & 0x03 )
          {
            case 0:
              InfoNES_Mirroring( MIRROR_V );   /* Vertical */
              break;
            case 1:
              InfoNES_Mirroring( MIRROR_H );   /* Horizontal */
              break;
            case 2:
              InfoNES_Mirroring( MIRROR_0 );   /* One Screen 0x2000 */
              break;
            case 3:
              InfoNES_Mirroring( MIRROR_1 );   /* One Screen 0x2400 */
              break;
          }
          break;

        case 0x0d:
          Map69_IRQ_Enable = byData;
          break;

        case 0x0e:
          Map69_IRQ_Cnt = (( (Map69_IRQ_Cnt >> STEP_PER_SCANLINE_SHIFT) & 0xff00) | (DWORD)byData) << STEP_PER_SCANLINE_SHIFT;
          break;

        case 0x0f:
          Map69_IRQ_Cnt = (( (Map69_IRQ_Cnt >> STEP_PER_SCANLINE_SHIFT) & 0x00ff) | ( (DWORD)byData << 8 )) << STEP_PER_SCANLINE_SHIFT;
          break;
      }
    } // if (wAddr < 0xC000)
  } // if (wAddr >= 0x8000)
}

/*-------------------------------------------------------------------*/
/*  Mapper 69 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map69_HSync)()
{
/*
 *  Callback at HSync
 *
 */
  if ( Map69_IRQ_Enable )
  {
    if ( Map69_IRQ_Cnt <= STEP_PER_SCANLINE_H + STEP_PER_SCANLINE_H/16)
    {
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
      IRQ_REQ;
#endif
      Map69_IRQ_Cnt = 0;
      Map69_IRQ_Enable = 0;
    } else {
      Map69_IRQ_Cnt -= STEP_PER_SCANLINE_H;
    }
  }
}
