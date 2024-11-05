/*===================================================================*/
/*                                                                   */
/*                       Mapper 118 (Others)                         */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map118_Regs[ 8 ];	// 0x00
	BYTE	md_Map118_IRQ_Enable;	// 0x08
	BYTE	md_Map118_IRQ_Cnt;	// 0x09
	BYTE	md_Map118_IRQ_Latch;	// 0x0A
	u8	md_res;			// 0x0B
	DWORD	md_Map118_Prg0;		// 0x0C
	DWORD	md_Map118_Prg1;		// 0x10
	DWORD	md_Map118_Chr0;		// 0x14
	DWORD	md_Map118_Chr1;		// 0x18
	DWORD	md_Map118_Chr2;		// 0x1C
	DWORD	md_Map118_Chr3;		// 0x20
	DWORD	md_Map118_Chr4;		// 0x24
	DWORD	md_Map118_Chr5;		// 0x28
	DWORD	md_Map118_Chr6;		// 0x2C
	DWORD	md_Map118_Chr7;		// 0x30
} sMap118_Data;

STATIC_ASSERT((sizeof(sMap118_Data) == 0x34) && (sizeof(sMap118_Data) <= MAPDATA_SIZE), "Incorrect sMap118_Data!");

#define MAPDATA118		((sMap118_Data*)NES->mapdata)	// mapper data

#define Map118_Regs		MAPDATA118->md_Map118_Regs
#define Map118_IRQ_Enable	MAPDATA118->md_Map118_IRQ_Enable
#define Map118_IRQ_Cnt		MAPDATA118->md_Map118_IRQ_Cnt
#define Map118_IRQ_Latch	MAPDATA118->md_Map118_IRQ_Latch
#define Map118_Prg0		MAPDATA118->md_Map118_Prg0
#define Map118_Prg1		MAPDATA118->md_Map118_Prg1
#define Map118_Chr0		MAPDATA118->md_Map118_Chr0
#define Map118_Chr1		MAPDATA118->md_Map118_Chr1
#define Map118_Chr2		MAPDATA118->md_Map118_Chr2
#define Map118_Chr3		MAPDATA118->md_Map118_Chr3
#define Map118_Chr4		MAPDATA118->md_Map118_Chr4
#define Map118_Chr5		MAPDATA118->md_Map118_Chr5
#define Map118_Chr6		MAPDATA118->md_Map118_Chr6
#define Map118_Chr7		MAPDATA118->md_Map118_Chr7

#define Map118_Chr_Swap()    ( Map118_Regs[ 0 ] & 0x80 )
#define Map118_Prg_Swap()    ( Map118_Regs[ 0 ] & 0x40 )

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 118                                            */
/*-------------------------------------------------------------------*/
void Map118_Init()
{
  /* Initialize Mapper */
  MapperInit = Map118_Init;

  /* Write to Mapper */
  MapperWrite = Map118_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map118_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  /* Initialize State Registers */
  for ( int nPage = 0; nPage < 8; nPage++)
  {
    Map118_Regs[ nPage ] = 0x00;
  }

  /* Set ROM Banks */
  Map118_Prg0 = 0;
  Map118_Prg1 = 1;
  Map118_Set_CPU_Banks();

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 )
  {
    Map118_Chr0 = 0;
    Map118_Chr1 = 1;
    Map118_Chr2 = 2;
    Map118_Chr3 = 3;
    Map118_Chr4 = 4;
    Map118_Chr5 = 5;
    Map118_Chr6 = 6;
    Map118_Chr7 = 7;
    Map118_Set_PPU_Banks();
  } else {
    Map118_Chr0 = Map118_Chr1 = Map118_Chr2 = Map118_Chr3 = 0;
    Map118_Chr4 = Map118_Chr5 = Map118_Chr6 = Map118_Chr7 = 0;
  }

  /* Initialize IRQ Registers */
  Map118_IRQ_Enable = 0;
  Map118_IRQ_Cnt = 0;
  Map118_IRQ_Latch = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 118 Write Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map118_Write)( WORD wAddr, BYTE byData )
{
  DWORD dwBankNum;

  switch ( wAddr & 0xe001 )
  {
    case 0x8000:
      Map118_Regs[ 0 ] = byData;
      Map118_Set_PPU_Banks();
      Map118_Set_CPU_Banks();
      break;

    case 0x8001:
      Map118_Regs[ 1 ] = byData;
      dwBankNum = Map118_Regs[ 1 ];

      /* Name Table Mirroring */
      if ( ( Map118_Regs[ 0 ] & 0x07 ) < 6 )
      {
        if ( byData & 0x80 )
        {
          InfoNES_Mirroring( MIRROR_0 );
        } else {
          InfoNES_Mirroring( MIRROR_1 );
        }
      }

      switch ( Map118_Regs[ 0 ] & 0x07 )
      {
        /* Set PPU Banks */
        case 0x00:
          if ( NesHeader.byVRomSize > 0 )
          {
            dwBankNum &= 0xfe;
            Map118_Chr0 = dwBankNum;
            Map118_Chr1 = dwBankNum + 1;
            Map118_Set_PPU_Banks();
          }
          break;

        case 0x01:
          if ( NesHeader.byVRomSize > 0 )
          {
            dwBankNum &= 0xfe;
            Map118_Chr2 = dwBankNum;
            Map118_Chr3 = dwBankNum + 1;
            Map118_Set_PPU_Banks();
          }
          break;

        case 0x02:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map118_Chr4 = dwBankNum;
            Map118_Set_PPU_Banks();
          }
          break;

        case 0x03:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map118_Chr5 = dwBankNum;
            Map118_Set_PPU_Banks();
          }
          break;

        case 0x04:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map118_Chr6 = dwBankNum;
            Map118_Set_PPU_Banks();
          }
          break;

        case 0x05:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map118_Chr7 = dwBankNum;
            Map118_Set_PPU_Banks();
          }
          break;

        /* Set ROM Banks */
        case 0x06:
          Map118_Prg0 = dwBankNum;
          Map118_Set_CPU_Banks();
          break;

        case 0x07:
          Map118_Prg1 = dwBankNum;
          Map118_Set_CPU_Banks();
          break;
      }
      break;

    case 0xc000:
      Map118_Regs[ 4 ] = byData;
      Map118_IRQ_Cnt = Map118_Regs[ 4 ];
      break;

    case 0xc001:
      Map118_Regs[ 5 ] = byData;
      Map118_IRQ_Latch = Map118_Regs[ 5 ];
      break;

    case 0xe000:
      Map118_Regs[ 6 ] = byData;
      Map118_IRQ_Enable = 0;
      break;

    case 0xe001:
      Map118_Regs[ 7 ] = byData;
      Map118_IRQ_Enable = 1;
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 118 H-Sync Function                                       */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map118_HSync)()
{
/*
 *  Callback at HSync
 *
 */
  if ( Map118_IRQ_Enable )
  {
    if ( 0 <= PPU_Scanline && PPU_Scanline <= 239 )
    {
      if ( PPU_R1 & R1_SHOW_SCR || PPU_R1 & R1_SHOW_SP )
      {
        if ( !( Map118_IRQ_Cnt-- ) )
        {
          Map118_IRQ_Cnt = Map118_IRQ_Latch;
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
          IRQ_REQ;
#endif
        }
      }
    }
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 118 Set CPU Banks Function                                */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map118_Set_CPU_Banks)()
{
  if ( Map118_Prg_Swap() )
  {
    ROMBANK0 = ROMLASTPAGE( 1 );
    ROMBANK1 = ROMPAGE( Map118_Prg1 % ( NesHeader.byRomSize << 1 ) );
    ROMBANK2 = ROMPAGE( Map118_Prg0 % ( NesHeader.byRomSize << 1 ) );
    ROMBANK3 = ROMLASTPAGE( 0 );
  } else {
    ROMBANK0 = ROMPAGE( Map118_Prg0 % ( NesHeader.byRomSize << 1 ) );
    ROMBANK1 = ROMPAGE( Map118_Prg1 % ( NesHeader.byRomSize << 1 ) );
    ROMBANK2 = ROMLASTPAGE( 1 );
    ROMBANK3 = ROMLASTPAGE( 0 );
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 118 Set PPU Banks Function                                */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map118_Set_PPU_Banks)()
{
  if ( NesHeader.byVRomSize > 0 )
  {
    if ( Map118_Chr_Swap() )
    { 
      PPUBANK[ 0 ] = VROMPAGE( Map118_Chr4 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 1 ] = VROMPAGE( Map118_Chr5 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 2 ] = VROMPAGE( Map118_Chr6 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 3 ] = VROMPAGE( Map118_Chr7 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 4 ] = VROMPAGE( Map118_Chr0 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 5 ] = VROMPAGE( Map118_Chr1 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 6 ] = VROMPAGE( Map118_Chr2 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 7 ] = VROMPAGE( Map118_Chr3 % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
    } else {
      PPUBANK[ 0 ] = VROMPAGE( Map118_Chr0 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 1 ] = VROMPAGE( Map118_Chr1 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 2 ] = VROMPAGE( Map118_Chr2 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 3 ] = VROMPAGE( Map118_Chr3 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 4 ] = VROMPAGE( Map118_Chr4 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 5 ] = VROMPAGE( Map118_Chr5 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 6 ] = VROMPAGE( Map118_Chr6 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 7 ] = VROMPAGE( Map118_Chr7 % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
    }
  }
}
