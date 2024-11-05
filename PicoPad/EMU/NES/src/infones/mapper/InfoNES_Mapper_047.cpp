/*===================================================================*/
/*                                                                   */
/*                       Mapper 47 (MMC)                             */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map47_Regs[ 8 ];	// 0x00
	BYTE	md_Map47_IRQ_Enable;	// 0x08
	BYTE	md_Map47_IRQ_Cnt;	// 0x09
	BYTE	md_Map47_IRQ_Latch;	// 0x0A
	u8	md_res;			// 0x0B
	DWORD	Map47_Rom_Bank;		// 0x0C
	DWORD	md_Map47_Prg0;		// 0x10
	DWORD	md_Map47_Prg1;		// 0x14
	DWORD	md_Map47_Chr01;		// 0x18
	DWORD	md_Map47_Chr23;		// 0x1C
	DWORD	md_Map47_Chr4;		// 0x20
	DWORD	md_Map47_Chr5;		// 0x24
	DWORD	md_Map47_Chr6;		// 0x28
	DWORD	md_Map47_Chr7;		// 0x2C
} sMap47_Data;

STATIC_ASSERT((sizeof(sMap47_Data) == 0x30) && (sizeof(sMap47_Data) <= MAPDATA_SIZE), "Incorrect sMap47_Data!");

#define MAPDATA47		((sMap47_Data*)NES->mapdata)	// mapper data

#define	Map47_Regs		MAPDATA47->md_Map47_Regs
#define Map47_IRQ_Enable	MAPDATA47->md_Map47_IRQ_Enable
#define Map47_IRQ_Cnt		MAPDATA47->md_Map47_IRQ_Cnt
#define Map47_IRQ_Latch		MAPDATA47->md_Map47_IRQ_Latch
#define Map47_Rom_Bank		MAPDATA47->Map47_Rom_Bank
#define Map47_Prg0		MAPDATA47->md_Map47_Prg0
#define Map47_Prg1		MAPDATA47->md_Map47_Prg1
#define Map47_Chr01		MAPDATA47->md_Map47_Chr01
#define Map47_Chr23		MAPDATA47->md_Map47_Chr23
#define Map47_Chr4		MAPDATA47->md_Map47_Chr4
#define Map47_Chr5		MAPDATA47->md_Map47_Chr5
#define Map47_Chr6		MAPDATA47->md_Map47_Chr6
#define Map47_Chr7		MAPDATA47->md_Map47_Chr7

#define Map47_Chr_Swap()    ( Map47_Regs[ 0 ] & 0x80 )
#define Map47_Prg_Swap()    ( Map47_Regs[ 0 ] & 0x40 )

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 47                                             */
/*-------------------------------------------------------------------*/
void Map47_Init()
{
  /* Initialize Mapper */
  MapperInit = Map47_Init;

  /* Write to Mapper */
  MapperWrite = Map47_Write;

  /* Write to SRAM */
  MapperSram = Map47_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map47_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  /* Initialize State Registers */
  for ( int nPage = 0; nPage < 8; nPage++ )
  {
    Map47_Regs[ nPage ] = 0x00;
  }

  /* Set ROM Banks */
  Map47_Rom_Bank = 0;
  Map47_Prg0 = 0;
  Map47_Prg1 = 1;
  Map47_Set_CPU_Banks();

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 )
  {
    Map47_Chr01 = 0;
    Map47_Chr23 = 2;
    Map47_Chr4  = 4;
    Map47_Chr5  = 5;
    Map47_Chr6  = 6;
    Map47_Chr7  = 7;
    Map47_Set_PPU_Banks();
  } else {
    Map47_Chr01 = Map47_Chr23 = 0;
    Map47_Chr4 = Map47_Chr5 = Map47_Chr6 = Map47_Chr7 = 0;
  }

  /* Initialize IRQ Registers */
  Map47_IRQ_Enable = 0;
  Map47_IRQ_Cnt = 0;
  Map47_IRQ_Latch = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 47 Write to Sram Function                                 */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map47_Sram)( WORD wAddr, BYTE byData )
{
  switch ( wAddr )
  {
    case 0x6000:
      Map47_Rom_Bank = byData & 0x01;
      Map47_Set_CPU_Banks();
      Map47_Set_PPU_Banks();
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 47 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map47_Write)( WORD wAddr, BYTE byData )
{
  DWORD dwBankNum;

  switch ( wAddr & 0xe001 )
  {
    case 0x8000:
      Map47_Regs[ 0 ] = byData;
      Map47_Set_PPU_Banks();
      Map47_Set_CPU_Banks();
      break;

    case 0x8001:
      Map47_Regs[ 1 ] = byData;
      dwBankNum = Map47_Regs[ 1 ];

      switch ( Map47_Regs[ 0 ] & 0x07 )
      {
        /* Set PPU Banks */
        case 0x00:
          if ( NesHeader.byVRomSize > 0 )
          {
            dwBankNum &= 0xfe;
            Map47_Chr01 = dwBankNum;
            Map47_Set_PPU_Banks();
          }
          break;

        case 0x01:
          if ( NesHeader.byVRomSize > 0 )
          {
            dwBankNum &= 0xfe;
            Map47_Chr23 = dwBankNum;
            Map47_Set_PPU_Banks();
          }
          break;

        case 0x02:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map47_Chr4 = dwBankNum;
            Map47_Set_PPU_Banks();
          }
          break;

        case 0x03:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map47_Chr5 = dwBankNum;
            Map47_Set_PPU_Banks();
          }
          break;

        case 0x04:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map47_Chr6 = dwBankNum;
            Map47_Set_PPU_Banks();
          }
          break;

        case 0x05:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map47_Chr7 = dwBankNum;
            Map47_Set_PPU_Banks();
          }
          break;

        /* Set ROM Banks */
        case 0x06:
          Map47_Prg0 = dwBankNum;
          Map47_Set_CPU_Banks();
          break;

        case 0x07:
          Map47_Prg1 = dwBankNum;
          Map47_Set_CPU_Banks();
          break;
      }
      break;

    case 0xa000:
      Map47_Regs[ 3 ] = byData;
      break;

    case 0xc000:
      Map47_Regs[ 4 ] = byData;
      Map47_IRQ_Cnt = Map47_Regs[ 4 ];
      break;

    case 0xc001:
      Map47_Regs[ 5 ] = byData;
      Map47_IRQ_Latch = Map47_Regs[ 5 ];
      break;

    case 0xe000:
      Map47_Regs[ 6 ] = byData;
      Map47_IRQ_Enable = 0;
      break;

    case 0xe001:
      Map47_Regs[ 7 ] = byData;
      Map47_IRQ_Enable = 1;
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 47 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map47_HSync)()
{
/*
 *  Callback at HSync
 *
 */
  if ( Map47_IRQ_Enable )
  {
    if ( 0 <= PPU_Scanline && PPU_Scanline <= 239 )
    {
      if ( PPU_R1 & R1_SHOW_SCR || PPU_R1 & R1_SHOW_SP )
      {
        if ( !( --Map47_IRQ_Cnt ) )
        {
          Map47_IRQ_Cnt = Map47_IRQ_Latch;
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
          IRQ_REQ;
#endif
        }
      }
    }
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 47 Set CPU Banks Function                                 */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map47_Set_CPU_Banks)()
{
  if ( Map47_Prg_Swap() )
  {
    ROMBANK0 = ROMPAGE( ( ( Map47_Rom_Bank << 4 ) + 14 ) % ( NesHeader.byRomSize << 1 ) );
    ROMBANK1 = ROMPAGE( ( ( Map47_Rom_Bank << 4 ) + Map47_Prg1 ) % ( NesHeader.byRomSize << 1 ) );
    ROMBANK2 = ROMPAGE( ( ( Map47_Rom_Bank << 4 ) + Map47_Prg0 ) % ( NesHeader.byRomSize << 1 ) );
    ROMBANK3 = ROMPAGE( ( ( Map47_Rom_Bank << 4 ) + 15 ) % ( NesHeader.byRomSize << 1 ) );
  } else {
    ROMBANK0 = ROMPAGE( ( ( Map47_Rom_Bank << 4 ) + Map47_Prg0 ) % ( NesHeader.byRomSize << 1 ) );
    ROMBANK1 = ROMPAGE( ( ( Map47_Rom_Bank << 4 ) + Map47_Prg1 ) % ( NesHeader.byRomSize << 1 ) );
    ROMBANK2 = ROMPAGE( ( ( Map47_Rom_Bank << 4 ) + 14 ) % ( NesHeader.byRomSize << 1 ) );
    ROMBANK3 = ROMPAGE( ( ( Map47_Rom_Bank << 4 ) + 15 ) % ( NesHeader.byRomSize << 1 ) );
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 47 Set PPU Banks Function                                 */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map47_Set_PPU_Banks)()
{
  if ( NesHeader.byVRomSize > 0 )
  {
    if ( Map47_Chr_Swap() )
    { 
      PPUBANK[ 0 ] = VROMPAGE( ( ( Map47_Rom_Bank << 7 ) + Map47_Chr4 ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 1 ] = VROMPAGE( ( ( Map47_Rom_Bank << 7 ) + Map47_Chr5 ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 2 ] = VROMPAGE( ( ( Map47_Rom_Bank << 7 ) + Map47_Chr6 ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 3 ] = VROMPAGE( ( ( Map47_Rom_Bank << 7 ) + Map47_Chr7 ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 4 ] = VROMPAGE( ( ( Map47_Rom_Bank << 7 ) + ( Map47_Chr01 + 0 ) ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 5 ] = VROMPAGE( ( ( Map47_Rom_Bank << 7 ) + ( Map47_Chr01 + 1 ) ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 6 ] = VROMPAGE( ( ( Map47_Rom_Bank << 7 ) + ( Map47_Chr23 + 0 ) ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 7 ] = VROMPAGE( ( ( Map47_Rom_Bank << 7 ) + ( Map47_Chr23 + 1 ) ) % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
    } else {
      PPUBANK[ 0 ] = VROMPAGE( ( ( Map47_Rom_Bank << 7 ) + ( Map47_Chr01 + 0 ) ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 1 ] = VROMPAGE( ( ( Map47_Rom_Bank << 7 ) + ( Map47_Chr01 + 1 ) ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 2 ] = VROMPAGE( ( ( Map47_Rom_Bank << 7 ) + ( Map47_Chr23 + 0 ) ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 3 ] = VROMPAGE( ( ( Map47_Rom_Bank << 7 ) + ( Map47_Chr23 + 1 ) ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 4 ] = VROMPAGE( ( ( Map47_Rom_Bank << 7 ) + Map47_Chr4 ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 5 ] = VROMPAGE( ( ( Map47_Rom_Bank << 7 ) + Map47_Chr5 ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 6 ] = VROMPAGE( ( ( Map47_Rom_Bank << 7 ) + Map47_Chr6 ) % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 7 ] = VROMPAGE( ( ( Map47_Rom_Bank << 7 ) + Map47_Chr7 ) % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
    }
  }
}
