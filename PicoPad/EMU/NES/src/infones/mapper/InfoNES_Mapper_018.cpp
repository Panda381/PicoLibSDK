/*===================================================================*/
/*                                                                   */
/*                   Mapper 18 (Jaleco SS8806)                       */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map18_Regs[11];	// 0x00
	BYTE	md_Map18_IRQ_Enable;	// 0x0B
	u32	md_Map18_IRQ_Latch;	// 0x0C
	u32	md_Map18_IRQ_Cnt;	// 0x10
} sMap18_Data;

STATIC_ASSERT((sizeof(sMap18_Data) == 0x14) && (sizeof(sMap18_Data) <= MAPDATA_SIZE), "Incorrect sMap18_Data!");

#define MAPDATA18		((sMap18_Data*)NES->mapdata)	// mapper data

#define Map18_Regs		MAPDATA18->md_Map18_Regs
#define Map18_IRQ_Enable	MAPDATA18->md_Map18_IRQ_Enable
#define Map18_IRQ_Latch		MAPDATA18->md_Map18_IRQ_Latch
#define Map18_IRQ_Cnt		MAPDATA18->md_Map18_IRQ_Cnt

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 18                                             */
/*-------------------------------------------------------------------*/
void Map18_Init()
{
  /* Initialize Mapper */
  MapperInit = Map18_Init;

  /* Write to Mapper */
  MapperWrite = Map18_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map18_HSync;

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
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    InfoNES_SetupChr();
  }

  /* Initialize Regs */
  for ( int i = 0; i < sizeof( Map18_Regs ); i++ )
  {
    Map18_Regs[ i ] = 0;
  }
  Map18_IRQ_Enable = 0;
  Map18_IRQ_Latch = 0;
  Map18_IRQ_Cnt = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 18 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map18_Write)( WORD wAddr, BYTE byData )
{
  switch( wAddr & 0xf003)
  {
    /* Set ROM Banks */
    case 0x8000:
      Map18_Regs[ 0 ] = ( Map18_Regs[ 0 ] & 0xf0 ) | ( byData & 0x0f );
      ROMBANK0 = ROMPAGE( Map18_Regs[ 0 ] % ( NesHeader.byRomSize << 1 ) );
      break;

    case 0x8001:
      Map18_Regs[ 0 ] = ( Map18_Regs[ 0 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      ROMBANK0 = ROMPAGE( Map18_Regs[ 0 ] % ( NesHeader.byRomSize << 1 ) );
      break;

    case 0x8002:
      Map18_Regs[ 1 ] = ( Map18_Regs[ 1 ] & 0xf0 ) | ( byData & 0x0f );
      ROMBANK1 = ROMPAGE( Map18_Regs[ 1 ] % ( NesHeader.byRomSize << 1 ) );
      break;

    case 0x8003:
      Map18_Regs[ 1 ] = ( Map18_Regs[ 1 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      ROMBANK1 = ROMPAGE( Map18_Regs[ 1 ] % ( NesHeader.byRomSize << 1 ) );
      break;

    case 0x9000:
      Map18_Regs[ 2 ] = ( Map18_Regs[ 2 ] & 0xf0 ) | ( byData & 0x0f );
      ROMBANK2 = ROMPAGE( Map18_Regs[ 2 ] % ( NesHeader.byRomSize << 1 ) );
      break;

    case 0x9001:
      Map18_Regs[ 2 ] = ( Map18_Regs[ 2 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      ROMBANK2 = ROMPAGE( Map18_Regs[ 2 ] % ( NesHeader.byRomSize << 1 ) );
      break;

    /* Set PPU Banks */
    case 0xA000:
      Map18_Regs[ 3 ]  = ( Map18_Regs[ 3 ] & 0xf0 ) | ( byData & 0x0f );
      PPUBANK[ 0 ] = VROMPAGE( Map18_Regs[ 3 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();      
      break;

    case 0xA001:
      Map18_Regs[ 3 ] = ( Map18_Regs[ 3 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      PPUBANK[ 0 ] = VROMPAGE( Map18_Regs[ 3 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();        
      break;

    case 0xA002:
      Map18_Regs[ 4 ]  = ( Map18_Regs[ 4 ] & 0xf0 ) | ( byData & 0x0f );
      PPUBANK[ 1 ] = VROMPAGE( Map18_Regs[ 4 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();         
      break;

    case 0xA003:
      Map18_Regs[ 4 ] = ( Map18_Regs[ 4 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      PPUBANK[ 1 ] = VROMPAGE( Map18_Regs[ 4 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr(); 
      break;

    case 0xB000:
      Map18_Regs[ 5 ]  = ( Map18_Regs[ 5 ] & 0xf0 ) | ( byData & 0x0f );
      PPUBANK[ 2 ] = VROMPAGE( Map18_Regs[ 5 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();      
      break;

    case 0xB001:
      Map18_Regs[ 5 ] = ( Map18_Regs[ 5 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      PPUBANK[ 2 ] = VROMPAGE( Map18_Regs[ 5 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();        
      break;

    case 0xB002:
      Map18_Regs[ 6 ]  = ( Map18_Regs[ 6 ] & 0xf0 ) | ( byData & 0x0f );
      PPUBANK[ 3 ] = VROMPAGE( Map18_Regs[ 6 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();         
      break;

    case 0xB003:
      Map18_Regs[ 6 ] = ( Map18_Regs[ 6 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      PPUBANK[ 3 ] = VROMPAGE( Map18_Regs[ 6 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr(); 
      break;

    case 0xC000:
      Map18_Regs[ 7 ]  = ( Map18_Regs[ 7 ] & 0xf0 ) | ( byData & 0x0f );
      PPUBANK[ 4 ] = VROMPAGE( Map18_Regs[ 7 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();      
      break;

    case 0xC001:
      Map18_Regs[ 7 ] = ( Map18_Regs[ 7 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      PPUBANK[ 4 ] = VROMPAGE( Map18_Regs[ 7 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();        
      break;

    case 0xC002:
      Map18_Regs[ 8 ]  = ( Map18_Regs[ 8 ] & 0xf0 ) | ( byData & 0x0f );
      PPUBANK[ 5 ] = VROMPAGE( Map18_Regs[ 8 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();         
      break;

    case 0xC003:
      Map18_Regs[ 8 ] = ( Map18_Regs[ 8 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      PPUBANK[ 5 ] = VROMPAGE( Map18_Regs[ 8 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr(); 
      break;

    case 0xD000:
      Map18_Regs[ 9 ]  = ( Map18_Regs[ 9 ] & 0xf0 ) | ( byData & 0x0f );
      PPUBANK[ 6 ] = VROMPAGE( Map18_Regs[ 9 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();      
      break;

    case 0xD001:
      Map18_Regs[ 9 ] = ( Map18_Regs[ 9 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      PPUBANK[ 6 ] = VROMPAGE( Map18_Regs[ 9 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();        
      break;

    case 0xD002:
      Map18_Regs[ 10 ]  = ( Map18_Regs[ 10 ] & 0xf0 ) | ( byData & 0x0f );
      PPUBANK[ 7 ] = VROMPAGE( Map18_Regs[ 10 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();         
      break;

    case 0xD003:
      Map18_Regs[ 10 ] = ( Map18_Regs[ 10 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      PPUBANK[ 7 ] = VROMPAGE( Map18_Regs[ 10 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr(); 
      break;

    // IRQ
    case 0xE000:
      Map18_IRQ_Latch = ( Map18_IRQ_Latch & 0xfff0 ) | ( byData & 0x0f );
      break;

    case 0xE001:
      Map18_IRQ_Latch = ( Map18_IRQ_Latch & 0xff0f ) | ( ( byData & 0x0f ) << 4 );
      break;

    case 0xE002:
      Map18_IRQ_Latch = ( Map18_IRQ_Latch & 0xf0ff ) | ( (WORD)( byData & 0x0f ) << 8 );
      break;

    case 0xE003:
      Map18_IRQ_Latch = ( Map18_IRQ_Latch & 0x0fff ) | ( (WORD)( byData & 0x0f ) << 12 );
      break;

    case 0xF000:
      Map18_IRQ_Cnt = Map18_IRQ_Latch << STEP_PER_SCANLINE_SHIFT;
      break;

    case 0xF001:
      Map18_IRQ_Enable = byData & 0x01;
      break;

    /* Name Table Mirroring */
    case 0xF002:
      switch ( byData & 0x03 )
      {
        case 0:
          InfoNES_Mirroring( MIRROR_H );   /* Horizontal */
          break;

        case 1:
          InfoNES_Mirroring( MIRROR_V );   /* Vertical */            
          break;

        case 2:
	case 3:
          InfoNES_Mirroring( MIRROR_0 );   /* One Screen 0x2000 */
          break;
      }    
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 18 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map18_HSync)()
{
/*
 *  Callback at HSync
 *
 */
  if ( Map18_IRQ_Enable )
  {
    if ( Map18_IRQ_Cnt <= STEP_PER_SCANLINE_H )
    {
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
      IRQ_REQ;
#endif
      Map18_IRQ_Cnt = 0;
      Map18_IRQ_Enable = 0;
    } else {
      Map18_IRQ_Cnt -= STEP_PER_SCANLINE_H;
    }
  }
}
