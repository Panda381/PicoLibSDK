/*===================================================================*/
/*                                                                   */
/*                       Mapper 114 (PC-SuperGames)                  */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map114_Regs[ 8 ];	// 0x00
	BYTE	md_Map114_IRQ_Enable;	// 0x08
	BYTE	md_Map114_IRQ_Cnt;	// 0x09
	BYTE	md_Map114_IRQ_Latch;	// 0x0A
	u8	md_res;			// 0x0B
	DWORD	md_Map114_Prg0;		// 0x0C
	DWORD	md_Map114_Prg1;		// 0x10
	DWORD	md_Map114_Chr01;	// 0x14
	DWORD	md_Map114_Chr23;	// 0x18
	DWORD	md_Map114_Chr4;		// 0x1C
	DWORD	md_Map114_Chr5;		// 0x20
	DWORD	md_Map114_Chr6;		// 0x24
	DWORD	md_Map114_Chr7;		// 0x28
} sMap114_Data;

STATIC_ASSERT((sizeof(sMap114_Data) == 0x2C) && (sizeof(sMap114_Data) <= MAPDATA_SIZE), "Incorrect sMap114_Data!");

#define MAPDATA114		((sMap114_Data*)NES->mapdata)	// mapper data

#define Map114_Regs		MAPDATA114->md_Map114_Regs
#define Map114_IRQ_Enable	MAPDATA114->md_Map114_IRQ_Enable
#define Map114_IRQ_Cnt		MAPDATA114->md_Map114_IRQ_Cnt
#define Map114_IRQ_Latch	MAPDATA114->md_Map114_IRQ_Latch
#define Map114_Prg0		MAPDATA114->md_Map114_Prg0
#define Map114_Prg1		MAPDATA114->md_Map114_Prg1
#define Map114_Chr01		MAPDATA114->md_Map114_Chr01
#define Map114_Chr23		MAPDATA114->md_Map114_Chr23
#define Map114_Chr4		MAPDATA114->md_Map114_Chr4
#define Map114_Chr5		MAPDATA114->md_Map114_Chr5
#define Map114_Chr6		MAPDATA114->md_Map114_Chr6
#define Map114_Chr7		MAPDATA114->md_Map114_Chr7

#define Map114_Chr_Swap()    ( Map114_Regs[ 0 ] & 0x80 )
#define Map114_Prg_Swap()    ( Map114_Regs[ 0 ] & 0x40 )

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 114                                            */
/*-------------------------------------------------------------------*/
void Map114_Init()
{
  /* Initialize Mapper */
  MapperInit = Map114_Init;

  /* Write to Mapper */
  MapperWrite = Map114_Write;

  /* Write to SRAM */
  MapperSram = Map114_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map114_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  /* Initialize State Registers */
  for ( int nPage = 0; nPage < 8; nPage++)
  {
    Map114_Regs[ nPage ] = 0x00;
  }

  /* Set ROM Banks */
  Map114_Prg0 = 0;
  Map114_Prg1 = 1;
  Map114_Set_CPU_Banks();

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 )
  {
    Map114_Chr01 = 0;
    Map114_Chr23 = 2;
    Map114_Chr4  = 4;
    Map114_Chr5  = 5;
    Map114_Chr6  = 6;
    Map114_Chr7  = 7;
    Map114_Set_PPU_Banks();
  } else {
    Map114_Chr01 = Map114_Chr23 = 0;
    Map114_Chr4 = Map114_Chr5 = Map114_Chr6 = Map114_Chr7 = 0;
  }

  /* Initialize IRQ Registers */
  Map114_IRQ_Enable = 0;
  Map114_IRQ_Cnt = 0;
  Map114_IRQ_Latch = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 114 Write to Sram Function                                */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map114_Sram)( WORD wAddr, BYTE byData )
{
  if ( ( wAddr == 0x6000 ) && ( byData == 0x00 ) )
  {
    /* Initialize State Registers */
    for ( int nPage = 0; nPage < 8; nPage++)
    {
      Map114_Regs[ nPage ] = 0x00;
    }

    /* Set ROM Banks */
    Map114_Prg0 = 0;
    Map114_Prg1 = 1;
    Map114_Set_CPU_Banks();

    /* Set PPU Banks */
    if ( NesHeader.byVRomSize > 0 )
    { 
      Map114_Chr01 = 0;
      Map114_Chr23 = 2;
      Map114_Chr4  = 4;
      Map114_Chr5  = 5;
      Map114_Chr6  = 6;
      Map114_Chr7  = 7;
      Map114_Set_PPU_Banks();
    } else {
      Map114_Chr01 = Map114_Chr23 = 0;
      Map114_Chr4 = Map114_Chr5 = Map114_Chr6 = Map114_Chr7 = 0;
    }

    /* Initialize IRQ Registers */
    Map114_IRQ_Enable = 0;
    Map114_IRQ_Cnt = 0;
    Map114_IRQ_Latch = 0;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 114 Write Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map114_Write)( WORD wAddr, BYTE byData )
{
  DWORD dwBankNum;

  switch ( wAddr & 0xe001 )
  {
    case 0x8000:
      Map114_Regs[ 0 ] = byData;
      Map114_Set_PPU_Banks();
      Map114_Set_CPU_Banks();
      break;

    case 0x8001:
      Map114_Regs[ 1 ] = byData;
      dwBankNum = Map114_Regs[ 1 ];

      switch ( Map114_Regs[ 0 ] & 0x07 )
      {
        /* Set PPU Banks */
        case 0x00:
          if ( NesHeader.byVRomSize > 0 )
          {
            dwBankNum &= 0xfe;
            Map114_Chr01 = dwBankNum;
            Map114_Set_PPU_Banks();
          }
          break;

        case 0x01:
          if ( NesHeader.byVRomSize > 0 )
          {
            dwBankNum &= 0xfe;
            Map114_Chr23 = dwBankNum;
            Map114_Set_PPU_Banks();
          }
          break;

        case 0x02:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map114_Chr4 = dwBankNum;
            Map114_Set_PPU_Banks();
          }
          break;

        case 0x03:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map114_Chr5 = dwBankNum;
            Map114_Set_PPU_Banks();
          }
          break;

        case 0x04:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map114_Chr6 = dwBankNum;
            Map114_Set_PPU_Banks();
          }
          break;

        case 0x05:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map114_Chr7 = dwBankNum;
            Map114_Set_PPU_Banks();
          }
          break;

        /* Set ROM Banks */
        case 0x06:
          Map114_Prg0 = dwBankNum;
          Map114_Set_CPU_Banks();
          break;

        case 0x07:
          Map114_Prg1 = dwBankNum;
          Map114_Set_CPU_Banks();
          break;
      }
      break;

    case 0xa000:
      Map114_Regs[ 2 ] = byData;

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
      Map114_Regs[ 3 ] = byData;

      if ( byData & 0x80 )
      {
        /* Enable Save RAM $6000-$7fff */
      } else {
        /* Disable Save RAM $6000-$7fff */
      }
      break;

    case 0xc000:
      Map114_Regs[ 4 ] = byData;
      Map114_IRQ_Cnt = Map114_Regs[ 4 ];
      break;

    case 0xc001:
      Map114_Regs[ 5 ] = byData;
      Map114_IRQ_Latch = Map114_Regs[ 5 ];
      break;

    case 0xe000:
      Map114_Regs[ 6 ] = byData;
      Map114_IRQ_Enable = 0;
      break;

    case 0xe001:
      Map114_Regs[ 7 ] = byData;
      Map114_IRQ_Enable = 1;
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 114 H-Sync Function                                       */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map114_HSync)()
{
/*
 *  Callback at HSync
 *
 */
  if ( Map114_IRQ_Enable )
  {
    if ( 0 <= PPU_Scanline && PPU_Scanline <= 239 )
    {
      if ( PPU_R1 & R1_SHOW_SCR || PPU_R1 & R1_SHOW_SP )
      {
        if ( !( Map114_IRQ_Cnt-- ) )
        {
          Map114_IRQ_Cnt = Map114_IRQ_Latch;
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
          IRQ_REQ;
#endif
        }
      }
    }
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 114 Set CPU Banks Function                                */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map114_Set_CPU_Banks)()
{
  if ( Map114_Prg_Swap() )
  {
    ROMBANK0 = ROMLASTPAGE( 1 );
    ROMBANK1 = ROMPAGE( Map114_Prg1 % ( NesHeader.byRomSize << 1 ) );
    ROMBANK2 = ROMPAGE( Map114_Prg0 % ( NesHeader.byRomSize << 1 ) );
    ROMBANK3 = ROMLASTPAGE( 0 );
  } else {
    ROMBANK0 = ROMPAGE( Map114_Prg0 % ( NesHeader.byRomSize << 1 ) );
    ROMBANK1 = ROMPAGE( Map114_Prg1 % ( NesHeader.byRomSize << 1 ) );
    ROMBANK2 = ROMLASTPAGE( 1 );
    ROMBANK3 = ROMLASTPAGE( 0 );
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 114 Set PPU Banks Function                                */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map114_Set_PPU_Banks)()
{
  if ( NesHeader.byVRomSize > 0 )
  {
    if ( Map114_Chr_Swap() )
    { 
      PPUBANK[ 0 ] = VROMPAGE( Map114_Chr4 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 1 ] = VROMPAGE( Map114_Chr5 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 2 ] = VROMPAGE( Map114_Chr6 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 3 ] = VROMPAGE( Map114_Chr7 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 4 ] = VROMPAGE( ( Map114_Chr01 + 0 ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 5 ] = VROMPAGE( ( Map114_Chr01 + 1 ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 6 ] = VROMPAGE( ( Map114_Chr23 + 0 ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 7 ] = VROMPAGE( ( Map114_Chr23 + 1 ) % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
    } else {
      PPUBANK[ 0 ] = VROMPAGE( ( Map114_Chr01 + 0 ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 1 ] = VROMPAGE( ( Map114_Chr01 + 1 ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 2 ] = VROMPAGE( ( Map114_Chr23 + 0 ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 3 ] = VROMPAGE( ( Map114_Chr23 + 1 ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 4 ] = VROMPAGE( Map114_Chr4 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 5 ] = VROMPAGE( Map114_Chr5 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 6 ] = VROMPAGE( Map114_Chr6 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 7 ] = VROMPAGE( Map114_Chr7 % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
    }
  }
}
