/*===================================================================*/
/*                                                                   */
/*   Mapper 115 : CartSaint : Yuu Yuu Hakusho Final JusticePao(?)    */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map115_Reg[8];	// 0x00
	BYTE	md_Map115_Prg0;		// 0x08
	BYTE	md_Map115_Prg1;		// 0x09
	BYTE	md_Map115_Prg2;		// 0x0A
	BYTE	md_Map115_Prg3;		// 0x0B
	BYTE	md_Map115_Prg0L;	// 0x0C
	BYTE	md_Map115_Prg1L;	// 0x0D
	BYTE	md_Map115_Chr0;		// 0x0E
	BYTE	md_Map115_Chr1;		// 0x0F
	BYTE	md_Map115_Chr2;		// 0x10
	BYTE	md_Map115_Chr3;		// 0x11
	BYTE    md_Map115_Chr4;		// 0x12
	BYTE	md_Map115_Chr5;		// 0x13
	BYTE	md_Map115_Chr6;		// 0x14
	BYTE	md_Map115_Chr7;		// 0x15
	BYTE	md_Map115_IRQ_Enable;	// 0x16
	BYTE	md_Map115_IRQ_Counter;	// 0x17
	BYTE	md_Map115_IRQ_Latch;	// 0x18
	BYTE	md_Map115_ExPrgSwitch;	// 0x19
	BYTE	md_Map115_ExChrSwitch;	// 0x1A
	u8	md_res;			// 0x1B
} sMap115_Data;

STATIC_ASSERT((sizeof(sMap115_Data) == 0x1C) && (sizeof(sMap115_Data) <= MAPDATA_SIZE), "Incorrect sMap115_Data!");

#define MAPDATA115		((sMap115_Data*)NES->mapdata)	// mapper data

#define Map115_Reg		MAPDATA115->md_Map115_Reg
#define Map115_Prg0		MAPDATA115->md_Map115_Prg0
#define Map115_Prg1		MAPDATA115->md_Map115_Prg1
#define Map115_Prg2		MAPDATA115->md_Map115_Prg2
#define Map115_Prg3		MAPDATA115->md_Map115_Prg3
#define Map115_Prg0L		MAPDATA115->md_Map115_Prg0L
#define Map115_Prg1L		MAPDATA115->md_Map115_Prg1L
#define Map115_Chr0		MAPDATA115->md_Map115_Chr0
#define Map115_Chr1		MAPDATA115->md_Map115_Chr1
#define Map115_Chr2		MAPDATA115->md_Map115_Chr2
#define Map115_Chr3		MAPDATA115->md_Map115_Chr3
#define Map115_Chr4		MAPDATA115->md_Map115_Chr4
#define Map115_Chr5		MAPDATA115->md_Map115_Chr5
#define Map115_Chr6		MAPDATA115->md_Map115_Chr6
#define Map115_Chr7		MAPDATA115->md_Map115_Chr7
#define Map115_IRQ_Enable	MAPDATA115->md_Map115_IRQ_Enable
#define Map115_IRQ_Counter	MAPDATA115->md_Map115_IRQ_Counter
#define Map115_IRQ_Latch	MAPDATA115->md_Map115_IRQ_Latch
#define Map115_ExPrgSwitch	MAPDATA115->md_Map115_ExPrgSwitch
#define Map115_ExChrSwitch	MAPDATA115->md_Map115_ExChrSwitch

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 115                                            */
/*-------------------------------------------------------------------*/
void Map115_Init()
{
  /* Initialize Mapper */
  MapperInit = Map115_Init;

  /* Write to Mapper */
  MapperWrite = Map115_Write;

  /* Write to SRAM */
  MapperSram = Map115_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map115_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  /* Initialize Registers */
  for( int i = 0; i < 8; i++ ) {
    Map115_Reg[i] = 0x00;
  }

  Map115_Prg0 = Map115_Prg0L = 0;
  Map115_Prg1 = Map115_Prg1L = 1;
  Map115_Prg2 = ( NesHeader.byRomSize << 1 ) - 2;
  Map115_Prg3 = ( NesHeader.byRomSize << 1 ) - 1;

  Map115_ExPrgSwitch = 0;
  Map115_ExChrSwitch = 0;

  /* Set ROM Banks */
  Map115_Set_CPU_Banks();

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 ) {
    Map115_Chr0 = 0;
    Map115_Chr1 = 1;
    Map115_Chr2 = 2;
    Map115_Chr3 = 3;
    Map115_Chr4 = 4;
    Map115_Chr5 = 5;
    Map115_Chr6 = 6;
    Map115_Chr7 = 7;
    Map115_Set_PPU_Banks();
  } else {
    Map115_Chr0 = Map115_Chr2 = Map115_Chr4 = Map115_Chr5 = Map115_Chr6 = Map115_Chr7 = 0;
    Map115_Chr1 = Map115_Chr3 = 1;
  }

  Map115_IRQ_Enable = 0;	/* Disable */
  Map115_IRQ_Counter = 0;
  Map115_IRQ_Latch = 0;
  
  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 115 Write Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map115_Write)( WORD wAddr, BYTE byData )
{
  switch( wAddr & 0xE001 ) {
  case	0x8000:
    Map115_Reg[0] = byData;
    Map115_Set_CPU_Banks();
    Map115_Set_PPU_Banks();
    break;
  case	0x8001:
    Map115_Reg[1] = byData;
    switch( Map115_Reg[0] & 0x07 ) {
    case	0x00:
      Map115_Chr0 = byData & 0xFE;
      Map115_Chr1 = Map115_Chr0+1;
      Map115_Set_PPU_Banks();
      break;
    case	0x01:
      Map115_Chr2 = byData & 0xFE;
      Map115_Chr3 = Map115_Chr2+1;
      Map115_Set_PPU_Banks();
      break;
    case	0x02:
      Map115_Chr4 = byData;
      Map115_Set_PPU_Banks();
      break;
    case	0x03:
      Map115_Chr5 = byData;
      Map115_Set_PPU_Banks();
      break;
    case	0x04:
      Map115_Chr6 = byData;
      Map115_Set_PPU_Banks();
      break;
    case	0x05:
      Map115_Chr7 = byData;
      Map115_Set_PPU_Banks();
      break;
    case	0x06:
      Map115_Prg0 = Map115_Prg0L = byData;
      Map115_Set_CPU_Banks();
      break;
    case	0x07:
      Map115_Prg1 = Map115_Prg1L = byData;
      Map115_Set_CPU_Banks();
      break;
    }
    break;
  case	0xA000:
    Map115_Reg[2] = byData;
    if ( !ROM_FourScr ) {
      if( byData & 0x01 ) InfoNES_Mirroring( MIRROR_H );
      else		  InfoNES_Mirroring( MIRROR_V );
    }
    break;
  case	0xA001:
    Map115_Reg[3] = byData;
    break;
  case	0xC000:
    Map115_Reg[4] = byData;
    Map115_IRQ_Counter = byData;
    Map115_IRQ_Enable = 0xFF;
    break;
  case	0xC001:
    Map115_Reg[5] = byData;
    Map115_IRQ_Latch = byData;
    break;
  case	0xE000:
    Map115_Reg[6] = byData;
    Map115_IRQ_Enable = 0;
    break;
  case	0xE001:
    Map115_Reg[7] = byData;
    Map115_IRQ_Enable = 0xFF;
    break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 115 Write to SRAM Function                                */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map115_Sram)( WORD wAddr, BYTE byData )
{
  switch ( wAddr ) {
  case	0x6000:
    Map115_ExPrgSwitch = byData;
    Map115_Set_CPU_Banks();
    break;
  case	0x6001:
    Map115_ExChrSwitch = byData&0x1;
    Map115_Set_PPU_Banks();
    break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 115 H-Sync Function                                       */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map115_HSync)()
{
  if( (PPU_Scanline >= 0 && PPU_Scanline <= 239) ) {
    if( PPU_R1 & R1_SHOW_SCR || PPU_R1 & R1_SHOW_SP ) {
      if( Map115_IRQ_Enable ) {
	if( !(Map115_IRQ_Counter--) ) {
	  Map115_IRQ_Counter = Map115_IRQ_Latch;
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
          IRQ_REQ;
#endif
	}
      }
    }
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 115 Set CPU Banks Function                                */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map115_Set_CPU_Banks)()
{
  if( Map115_ExPrgSwitch & 0x80 ) {
    Map115_Prg0 = ((Map115_ExPrgSwitch<<1)&0x1e);
    Map115_Prg1 = Map115_Prg0+1;

    ROMBANK0 = ROMPAGE( Map115_Prg0 % ( NesHeader.byRomSize << 1 ) );
    ROMBANK1 = ROMPAGE( Map115_Prg1 % ( NesHeader.byRomSize << 1 ) );
    ROMBANK2 = ROMPAGE( ( Map115_Prg0+2 ) % ( NesHeader.byRomSize << 1 ) );
    ROMBANK3 = ROMPAGE( ( Map115_Prg1+2 ) % ( NesHeader.byRomSize << 1 ) );
  } else {
    Map115_Prg0 = Map115_Prg0L;
    Map115_Prg1 = Map115_Prg1L;
    if( Map115_Reg[0] & 0x40 ) {
      ROMBANK0 = ROMPAGE( ( NesHeader.byRomSize << 1 ) - 2 );
      ROMBANK1 = ROMPAGE( Map115_Prg1 % ( NesHeader.byRomSize << 1 ) );
      ROMBANK2 = ROMPAGE( Map115_Prg0 % ( NesHeader.byRomSize << 1 ) );
      ROMBANK3 = ROMPAGE( ( NesHeader.byRomSize << 1 ) - 1 );
    } else {
      ROMBANK0 = ROMPAGE( Map115_Prg0 % ( NesHeader.byRomSize << 1 ) );
      ROMBANK1 = ROMPAGE( Map115_Prg1 % ( NesHeader.byRomSize << 1 ) );
      ROMBANK2 = ROMPAGE( ( NesHeader.byRomSize << 1 ) - 2 );
      ROMBANK3 = ROMPAGE( ( NesHeader.byRomSize << 1 ) - 1 );
    }
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 115 Set PPU Banks Function                                */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map115_Set_PPU_Banks)()
{
  if ( NesHeader.byVRomSize > 0 ) {
    if( Map115_Reg[0] & 0x80 ) {
      PPUBANK[ 0 ] = VROMPAGE( ((Map115_ExChrSwitch<<8)+Map115_Chr4) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 1 ] = VROMPAGE( ((Map115_ExChrSwitch<<8)+Map115_Chr5) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 2 ] = VROMPAGE( ((Map115_ExChrSwitch<<8)+Map115_Chr6) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 3 ] = VROMPAGE( ((Map115_ExChrSwitch<<8)+Map115_Chr7) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 4 ] = VROMPAGE( ((Map115_ExChrSwitch<<8)+Map115_Chr0) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 5 ] = VROMPAGE( ((Map115_ExChrSwitch<<8)+Map115_Chr1) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 6 ] = VROMPAGE( ((Map115_ExChrSwitch<<8)+Map115_Chr2) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 7 ] = VROMPAGE( ((Map115_ExChrSwitch<<8)+Map115_Chr3) % (NesHeader.byVRomSize<<3));
      InfoNES_SetupChr();
    } else {
      PPUBANK[ 0 ] = VROMPAGE( ((Map115_ExChrSwitch<<8)+Map115_Chr0) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 1 ] = VROMPAGE( ((Map115_ExChrSwitch<<8)+Map115_Chr1) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 2 ] = VROMPAGE( ((Map115_ExChrSwitch<<8)+Map115_Chr2) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 3 ] = VROMPAGE( ((Map115_ExChrSwitch<<8)+Map115_Chr3) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 4 ] = VROMPAGE( ((Map115_ExChrSwitch<<8)+Map115_Chr4) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 5 ] = VROMPAGE( ((Map115_ExChrSwitch<<8)+Map115_Chr5) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 6 ] = VROMPAGE( ((Map115_ExChrSwitch<<8)+Map115_Chr6) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 7 ] = VROMPAGE( ((Map115_ExChrSwitch<<8)+Map115_Chr7) % (NesHeader.byVRomSize<<3));
      InfoNES_SetupChr();
    }
  }
}

