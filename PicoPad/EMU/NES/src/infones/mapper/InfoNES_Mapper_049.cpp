/*===================================================================*/
/*                                                                   */
/*                       Mapper 49 (Nin1)                            */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map49_Regs[ 3 ];	// 0x00
	BYTE	md_Map49_IRQ_Enable;	// 0x03
	BYTE	md_Map49_IRQ_Cnt;	// 0x04
	BYTE	md_Map49_IRQ_Latch;	// 0x05
	u8	md_res[2];		// 0x06
	DWORD	md_Map49_Prg0;		// 0x08
	DWORD	md_Map49_Prg1;		// 0x0C
	DWORD	md_Map49_Chr01;		// 0x10
	DWORD	md_Map49_Chr23;		// 0x14
	DWORD	md_Map49_Chr4;		// 0x18
	DWORD	md_Map49_Chr5;		// 0x1C
	DWORD	md_Map49_Chr6;		// 0x20
	DWORD	md_Map49_Chr7;		// 0x24
} sMap49_Data;

STATIC_ASSERT((sizeof(sMap49_Data) == 0x28) && (sizeof(sMap49_Data) <= MAPDATA_SIZE), "Incorrect sMap49_Data!");

#define MAPDATA49		((sMap49_Data*)NES->mapdata)	// mapper data

#define Map49_Regs		MAPDATA49->md_Map49_Regs
#define Map49_IRQ_Enable	MAPDATA49->md_Map49_IRQ_Enable
#define Map49_IRQ_Cnt		MAPDATA49->md_Map49_IRQ_Cnt
#define Map49_IRQ_Latch		MAPDATA49->md_Map49_IRQ_Latch
#define Map49_Prg0		MAPDATA49->md_Map49_Prg0
#define Map49_Prg1		MAPDATA49->md_Map49_Prg1
#define Map49_Chr01		MAPDATA49->md_Map49_Chr01
#define Map49_Chr23		MAPDATA49->md_Map49_Chr23
#define Map49_Chr4		MAPDATA49->md_Map49_Chr4
#define Map49_Chr5		MAPDATA49->md_Map49_Chr5
#define Map49_Chr6		MAPDATA49->md_Map49_Chr6
#define Map49_Chr7		MAPDATA49->md_Map49_Chr7

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 49                                             */
/*-------------------------------------------------------------------*/
void Map49_Init()
{
  /* Initialize Mapper */
  MapperInit = Map49_Init;

  /* Write to Mapper */
  MapperWrite = Map49_Write;

  /* Write to SRAM */
  MapperSram = Map49_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map49_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  /* Set ROM Banks */
  Map49_Prg0 = 0;
  Map49_Prg1 = 1;
  ROMBANK0 = ROMPAGE( 0 );
  ROMBANK1 = ROMPAGE( 1 );
  ROMBANK2 = ROMPAGE( 2 );
  ROMBANK3 = ROMPAGE( 3 );
  
  /* Set PPU Banks */
  Map49_Chr01 = 0;
  Map49_Chr23 = 2;
  Map49_Chr4  = 4;
  Map49_Chr5  = 5;
  Map49_Chr6  = 6;
  Map49_Chr7  = 7;

  for ( int nPage = 0; nPage < 8; ++nPage )
  {
    PPUBANK[ nPage ] = VROMPAGE( nPage );
  }
  InfoNES_SetupChr();
  
  /* Initialize IRQ Registers */
  Map49_Regs[ 0 ] = Map49_Regs[ 1 ] = Map49_Regs[ 2 ] = 0;
  Map49_IRQ_Enable = 0;
  Map49_IRQ_Cnt = 0;
  Map49_IRQ_Latch = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 49 Write to Sram Function                                 */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map49_Sram)( WORD wAddr, BYTE byData )
{
  if ( Map49_Regs[ 2 ] & 0x80 )
  {
    Map49_Regs[ 1 ] = byData;
    Map49_Set_CPU_Banks();
    Map49_Set_PPU_Banks();
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 49 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map49_Write)( WORD wAddr, BYTE byData )
{
  switch ( wAddr & 0xe001 )
  {
    case 0x8000:
      if ( ( byData & 0x40 ) != ( Map49_Regs[ 0 ] & 0x40 ) )
      {
        Map49_Set_CPU_Banks();
      }
      if ( ( byData & 0x80 ) != ( Map49_Regs[ 0 ] & 0x80 ) )
      {
        Map49_Regs[ 0 ] = byData;
        Map49_Set_PPU_Banks();
      }
      Map49_Regs[ 0 ] = byData;
      break;

    case 0x8001:
      switch ( Map49_Regs[ 0 ] & 0x07 )
      {
        /* Set PPU Banks */
        case 0x00:
          Map49_Chr01 = byData & 0xfe;
          Map49_Set_PPU_Banks();
          break;

        case 0x01:
          Map49_Chr23 = byData & 0xfe;
          Map49_Set_PPU_Banks();
          break;

        case 0x02:
          Map49_Chr4 = byData;
          Map49_Set_PPU_Banks();
          break;

        case 0x03:
          Map49_Chr5 = byData;
          Map49_Set_PPU_Banks();
          break;

        case 0x04:
          Map49_Chr6 = byData;
          Map49_Set_PPU_Banks();
          break;

        case 0x05:
          Map49_Chr7 = byData;
          Map49_Set_PPU_Banks();
          break;

        /* Set ROM Banks */
        case 0x06:
          Map49_Prg0 = byData;
          Map49_Set_CPU_Banks();
          break;

        case 0x07:
          Map49_Prg1 = byData;
          Map49_Set_CPU_Banks();
          break;
      }
      break;

    case 0xa000:
      if ( !ROM_FourScr )
      {
        if ( byData & 0x01 )
        {
          InfoNES_Mirroring( MIRROR_H );
        } else {
          InfoNES_Mirroring( MIRROR_V );
        }
      }
      break;

    case 0xa001:
      Map49_Regs[ 2 ] = byData;
      break;

    case 0xc000:
      Map49_IRQ_Cnt = byData;
      break;

    case 0xc001:
      Map49_IRQ_Latch = byData;
      break;

    case 0xe000:
      Map49_IRQ_Enable = 0;
      break;

    case 0xe001:
      Map49_IRQ_Enable = 1;
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 49 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map49_HSync)()
{
/*
 *  Callback at HSync
 *
 */
  if ( Map49_IRQ_Enable )
  {
    if ( 0 <= PPU_Scanline && PPU_Scanline <= 239 )
    {
      if ( PPU_R1 & R1_SHOW_SCR || PPU_R1 & R1_SHOW_SP )
      {
        if ( !( Map49_IRQ_Cnt-- ) )
        {
          Map49_IRQ_Cnt = Map49_IRQ_Latch;
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
          IRQ_REQ;
#endif
        }
      }
    }
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 49 Set CPU Banks Function                                 */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map49_Set_CPU_Banks)()
{
  DWORD dwBank0, dwBank1, dwBank2, dwBank3;

  if ( Map49_Regs[ 1 ] & 0x01 )
  {
    if ( Map49_Regs[ 0 ] & 0x40 )
    {
      dwBank0 = ( ( ( NesHeader.byRomSize << 1 ) - 1 ) & 0x0e ) | ( ( Map49_Regs[ 1 ] & 0xc0 ) >> 2 );
      dwBank1 = ( Map49_Prg1 & 0x0f ) | ( ( Map49_Regs[ 1 ] & 0xc0 ) >> 2 );
      dwBank2 = ( Map49_Prg0 & 0x0f ) | ( ( Map49_Regs[ 1 ] & 0xc0 ) >> 2 );
      dwBank3 = ( ( ( NesHeader.byRomSize << 1 ) - 1 ) & 0x0f ) | ( ( Map49_Regs[ 1 ] & 0xc0 ) >> 2 );
    } else {
      dwBank0 = ( Map49_Prg0 & 0x0f ) | ( ( Map49_Regs[ 1 ] & 0xc0 ) >> 2 );
      dwBank1 = ( Map49_Prg1 & 0x0f ) | ( ( Map49_Regs[ 1 ] & 0xc0 ) >> 2 );
      dwBank2 = ( ( ( NesHeader.byRomSize << 1 ) - 1 ) & 0x0e ) | ( ( Map49_Regs[ 1 ] & 0xc0 ) >> 2 );
      dwBank3 = ( ( ( NesHeader.byRomSize << 1 ) - 1 ) & 0x0f ) | ( ( Map49_Regs[ 1 ] & 0xc0 ) >> 2 );
    }
  } else {
    dwBank0 = ( ( Map49_Regs[ 1 ] & 0x70 ) >> 2 ) | 0;
    dwBank1 = ( ( Map49_Regs[ 1 ] & 0x70 ) >> 2 ) | 1;
    dwBank2 = ( ( Map49_Regs[ 1 ] & 0x70 ) >> 2 ) | 2;
    dwBank3 = ( ( Map49_Regs[ 1 ] & 0x70 ) >> 2 ) | 3;
  }

  /* Set ROM Banks */ 
  ROMBANK0 = ROMPAGE( dwBank0 % ( NesHeader.byRomSize << 1) );
  ROMBANK1 = ROMPAGE( dwBank1 % ( NesHeader.byRomSize << 1) );
  ROMBANK2 = ROMPAGE( dwBank2 % ( NesHeader.byRomSize << 1) );
  ROMBANK3 = ROMPAGE( dwBank3 % ( NesHeader.byRomSize << 1) );
}

/*-------------------------------------------------------------------*/
/*  Mapper 49 Set PPU Banks Function                                 */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map49_Set_PPU_Banks)()
{
  Map49_Chr01 = ( Map49_Chr01 & 0x7f ) | ( ( Map49_Regs[ 1 ] & 0xc0 ) << 1 );
  Map49_Chr23 = ( Map49_Chr23 & 0x7f ) | ( ( Map49_Regs[ 1 ] & 0xc0 ) << 1 );
  Map49_Chr4 = ( Map49_Chr4 & 0x7f ) | ( ( Map49_Regs[ 1 ] & 0xc0 ) << 1 );
  Map49_Chr5 = ( Map49_Chr5 & 0x7f ) | ( ( Map49_Regs[ 1 ] & 0xc0 ) << 1 );
  Map49_Chr6 = ( Map49_Chr6 & 0x7f ) | ( ( Map49_Regs[ 1 ] & 0xc0 ) << 1 );
  Map49_Chr7 = ( Map49_Chr7 & 0x7f ) | ( ( Map49_Regs[ 1 ] & 0xc0 ) << 1 );

  /* Set PPU Banks */ 
  if ( Map49_Regs[ 0 ] & 0x80 )
  { 
    PPUBANK[ 0 ] = VROMPAGE( Map49_Chr4 % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 1 ] = VROMPAGE( Map49_Chr5 % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 2 ] = VROMPAGE( Map49_Chr6 % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 3 ] = VROMPAGE( Map49_Chr7 % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 4 ] = VROMPAGE( ( Map49_Chr01 + 0 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 5 ] = VROMPAGE( ( Map49_Chr01 + 1 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 6 ] = VROMPAGE( ( Map49_Chr23 + 0 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 7 ] = VROMPAGE( ( Map49_Chr23 + 1 ) % ( NesHeader.byVRomSize << 3 ) );
    InfoNES_SetupChr();
  } else {
    PPUBANK[ 0 ] = VROMPAGE( ( Map49_Chr01 + 0 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 1 ] = VROMPAGE( ( Map49_Chr01 + 1 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 2 ] = VROMPAGE( ( Map49_Chr23 + 0 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 3 ] = VROMPAGE( ( Map49_Chr23 + 1 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 4 ] = VROMPAGE( Map49_Chr4 % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 5 ] = VROMPAGE( Map49_Chr5 % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 6 ] = VROMPAGE( Map49_Chr6 % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 7 ] = VROMPAGE( Map49_Chr7 % ( NesHeader.byVRomSize << 3 ) );
    InfoNES_SetupChr();
  }
}
