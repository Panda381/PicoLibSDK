/*===================================================================*/
/*                                                                   */
/*                   Mapper 100 : Nestile MMC 3                      */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map100_Reg[8];	// 0x00
	BYTE	md_Map100_Prg0;		// 0x08
	BYTE	md_Map100_Prg1;		// 0x09
	BYTE	md_Map100_Prg2;		// 0x0A
	BYTE	md_Map100_Prg3;		// 0x0B
	BYTE	md_Map100_Chr0;		// 0x0C
	BYTE	md_Map100_Chr1;		// 0x0D
	BYTE	md_Map100_Chr2;		// 0x0E
	BYTE	md_Map100_Chr3; 	// 0x0F
	BYTE	md_Map100_Chr4;		// 0x10
	BYTE	md_Map100_Chr5;		// 0x11
	BYTE	md_Map100_Chr6;		// 0x12
	BYTE	md_Map100_Chr7;		// 0x13
	BYTE	md_Map100_IRQ_Enable;	// 0x14
	BYTE	md_Map100_IRQ_Cnt;	// 0x15
	BYTE	md_Map100_IRQ_Latch;	// 0x16
	u8	md_res;			// 0x17
} sMap100_Data;

STATIC_ASSERT((sizeof(sMap100_Data) == 0x18) && (sizeof(sMap100_Data) <= MAPDATA_SIZE), "Incorrect sMap100_Data!");

#define MAPDATA100		((sMap100_Data*)NES->mapdata)	// mapper data

#define Map100_Reg		MAPDATA100->md_Map100_Reg
#define Map100_Prg0		MAPDATA100->md_Map100_Prg0
#define Map100_Prg1		MAPDATA100->md_Map100_Prg1
#define Map100_Prg2		MAPDATA100->md_Map100_Prg2
#define Map100_Prg3		MAPDATA100->md_Map100_Prg3
#define Map100_Chr0		MAPDATA100->md_Map100_Chr0
#define Map100_Chr1		MAPDATA100->md_Map100_Chr1
#define Map100_Chr2		MAPDATA100->md_Map100_Chr2
#define Map100_Chr3		MAPDATA100->md_Map100_Chr3
#define Map100_Chr4		MAPDATA100->md_Map100_Chr4
#define Map100_Chr5		MAPDATA100->md_Map100_Chr5
#define Map100_Chr6		MAPDATA100->md_Map100_Chr6
#define Map100_Chr7		MAPDATA100->md_Map100_Chr7
#define Map100_IRQ_Enable	MAPDATA100->md_Map100_IRQ_Enable
#define Map100_IRQ_Cnt		MAPDATA100->md_Map100_IRQ_Cnt
#define Map100_IRQ_Latch	MAPDATA100->md_Map100_IRQ_Latch

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 100                                            */
/*-------------------------------------------------------------------*/
void Map100_Init()
{
  /* Initialize Mapper */
  MapperInit = Map100_Init;

  /* Write to Mapper */
  MapperWrite = Map100_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map100_HSync;

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
    Map100_Chr0 = 0;
    Map100_Chr1 = 1;
    Map100_Chr2 = 2;
    Map100_Chr3 = 3;
    Map100_Chr4 = 4;
    Map100_Chr5 = 5;
    Map100_Chr6 = 6;
    Map100_Chr7 = 7;
    Map100_Set_PPU_Banks();
  } else {
    Map100_Chr0 = Map100_Chr2 = Map100_Chr4 = Map100_Chr5 = Map100_Chr6 = Map100_Chr7 = 0;
    Map100_Chr1 = Map100_Chr3 = 1;
  }

  /* Set IRQ Registers */
  Map100_IRQ_Enable = 0;
  Map100_IRQ_Cnt = 0;
  Map100_IRQ_Latch = 0;
  for( int i = 0; i < 8; i++ ) { Map100_Reg[ i ] = 0x00; }

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 100 Write Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map100_Write)( WORD wAddr, BYTE byData )
{
  switch ( wAddr & 0xE001 )
  {
    case 0x8000:
      Map100_Reg[0] = byData;
      break;

    case 0x8001:
      Map100_Reg[1] = byData;

      switch ( Map100_Reg[0] & 0xC7 ) {
        case 0x00:
	  if ( NesHeader.byVRomSize > 0 ) {
	    Map100_Chr0 = byData&0xFE;
	    Map100_Chr1 = Map100_Chr0+1;
	    Map100_Set_PPU_Banks();
	  }
	  break;
	  
        case 0x01:
	  if ( NesHeader.byVRomSize > 0 ) {
	    Map100_Chr2 = byData&0xFE;
	    Map100_Chr3 = Map100_Chr2+1;
	    Map100_Set_PPU_Banks();
	  }
	  break;

        case 0x02:
	  if ( NesHeader.byVRomSize > 0 ) {
	    Map100_Chr4 = byData;
	    Map100_Set_PPU_Banks();
	  }
	  break;

        case 0x03:
	  if ( NesHeader.byVRomSize > 0 ) {
	    Map100_Chr5 = byData;
	    Map100_Set_PPU_Banks();
	  }
	  break;

        case 0x04:
	  if ( NesHeader.byVRomSize > 0 ) {
	    Map100_Chr6 = byData;
	    Map100_Set_PPU_Banks();
	  }
	  break;

        case 0x05:
	  if ( NesHeader.byVRomSize > 0 ) {
	    Map100_Chr7 = byData;
	    Map100_Set_PPU_Banks();
	  }
	  break;

        case 0x06:
	  Map100_Prg0 = byData;
	  Map100_Set_CPU_Banks();
	  break;

        case 0x07:
	  Map100_Prg1 = byData;
	  Map100_Set_CPU_Banks();
	  break;

        case 0x46:
	  Map100_Prg2 = byData;
	  Map100_Set_CPU_Banks();
	  break;

        case 0x47:
	  Map100_Prg3 = byData;
	  Map100_Set_CPU_Banks();
	  break;

        case 0x80:
	  if ( NesHeader.byVRomSize > 0 ) {
	    Map100_Chr4 = byData&0xFE;
	    Map100_Chr5 = Map100_Chr4+1;
	    Map100_Set_PPU_Banks();
	  }
	  break;

        case 0x81:
	  if ( NesHeader.byVRomSize > 0 ) {
	    Map100_Chr6 = byData&0xFE;
	    Map100_Chr7 = Map100_Chr6+1;
	    Map100_Set_PPU_Banks();
	  }
	  break;

        case 0x82:
	  if ( NesHeader.byVRomSize > 0 ) {
	    Map100_Chr0 = byData;
	    Map100_Set_PPU_Banks();
	  }
	  break;

        case 0x83:
	  if ( NesHeader.byVRomSize > 0 ) {
	    Map100_Chr1 = byData;
	    Map100_Set_PPU_Banks();
	  }
	  break;

        case 0x84:
	  if ( NesHeader.byVRomSize > 0 ) {
	    Map100_Chr2 = byData;
	    Map100_Set_PPU_Banks();
	  }
	  break;

        case 0x85:
	  if( NesHeader.byVRomSize > 0 ) {
	    Map100_Chr3 = byData;
	    Map100_Set_PPU_Banks();
	  }
	  break;

      }
      break;

    case 0xA000:
      Map100_Reg[2] = byData;
      if ( !ROM_FourScr )
      {
	if( byData & 0x01 ) InfoNES_Mirroring( MIRROR_H );
	else		    InfoNES_Mirroring( MIRROR_V );
      }
      break;

    case 0xA001:
      Map100_Reg[3] = byData;
      break;

    case 0xC000:
      Map100_Reg[4] = byData;
      Map100_IRQ_Cnt = byData;
      break;

    case 0xC001:
      Map100_Reg[5] = byData;
      Map100_IRQ_Latch = byData;
      break;

    case 0xE000:
      Map100_Reg[6] = byData;
      Map100_IRQ_Enable = 0;
      break;

    case 0xE001:
      Map100_Reg[7] = byData;
      Map100_IRQ_Enable = 0xFF;
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 100 H-Sync Function                                       */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map100_HSync)()
{
  if( (PPU_Scanline >= 0 && PPU_Scanline <= 239) ) {
    if ( PPU_R1 & R1_SHOW_SCR || PPU_R1 & R1_SHOW_SP ) {
      if( Map100_IRQ_Enable ) {
	if( !(Map100_IRQ_Cnt--) ) {
	  Map100_IRQ_Cnt = Map100_IRQ_Latch;
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
          IRQ_REQ;
#endif
	}
      }
    }
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 100 Set CPU Banks Function                                */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map100_Set_CPU_Banks)()
{
  ROMBANK0 = ROMPAGE( Map100_Prg0 % ( NesHeader.byRomSize << 1 ) );
  ROMBANK1 = ROMPAGE( Map100_Prg1 % ( NesHeader.byRomSize << 1 ) );
  ROMBANK2 = ROMPAGE( Map100_Prg2 % ( NesHeader.byRomSize << 1 ) );
  ROMBANK3 = ROMPAGE( Map100_Prg3 % ( NesHeader.byRomSize << 1 ) );
}

/*-------------------------------------------------------------------*/
/*  Mapper 100 Set PPU Banks Function                                */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map100_Set_PPU_Banks)()
{
  if ( NesHeader.byVRomSize > 0 )
  {
      PPUBANK[ 0 ] = VROMPAGE( Map100_Chr0 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 1 ] = VROMPAGE( Map100_Chr1 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 2 ] = VROMPAGE( Map100_Chr2 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 3 ] = VROMPAGE( Map100_Chr3 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 4 ] = VROMPAGE( Map100_Chr4 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 5 ] = VROMPAGE( Map100_Chr5 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 6 ] = VROMPAGE( Map100_Chr6 % ( NesHeader.byVRomSize << 3 ) );
      PPUBANK[ 7 ] = VROMPAGE( Map100_Chr7 % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
  }
}

