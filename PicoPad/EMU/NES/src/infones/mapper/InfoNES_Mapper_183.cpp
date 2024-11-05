/*===================================================================*/
/*                                                                   */
/*                 Mapper 183 : Gimmick (Bootleg)                    */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map183_Reg[8];	// 0x00
	BYTE	md_Map183_IRQ_Enable;	// 0x08
	u8	md_res[3];		// 0x09
	s32	md_Map183_IRQ_Counter;	// 0x0C
} sMap183_Data;

STATIC_ASSERT((sizeof(sMap183_Data) == 0x10) && (sizeof(sMap183_Data) <= MAPDATA_SIZE), "Incorrect sMap183_Data!");

#define MAPDATA183		((sMap183_Data*)NES->mapdata)	// mapper data

#define Map183_Reg		MAPDATA183->md_Map183_Reg
#define Map183_IRQ_Enable	MAPDATA183->md_Map183_IRQ_Enable
#define Map183_IRQ_Counter	MAPDATA183->md_Map183_IRQ_Counter

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 183                                            */
/*-------------------------------------------------------------------*/
void Map183_Init()
{
  /* Initialize Mapper */
  MapperInit = Map183_Init;

  /* Write to Mapper */
  MapperWrite = Map183_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map183_HSync;

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
  if ( NesHeader.byVRomSize > 0 ) {
    for ( int nPage = 0; nPage < 8; ++nPage )
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    InfoNES_SetupChr();
  }

  /* Initialize Registers */
  for( int i = 0; i < 8; i++ ) {
    Map183_Reg[i] = i;
  }
  Map183_IRQ_Enable = 0;
  Map183_IRQ_Counter = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 183 Write Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map183_Write)( WORD wAddr, BYTE byData )
{
  switch( wAddr ) {
  case	0x8800:
    ROMBANK0 = ROMPAGE( byData % (NesHeader.byRomSize << 1) );
    break;
  case	0xA800:
    ROMBANK1 = ROMPAGE( byData % (NesHeader.byRomSize << 1) );
    break;
  case	0xA000:
    ROMBANK2 = ROMPAGE( byData % (NesHeader.byRomSize << 1) );
    break;
    
  case	0xB000:
    Map183_Reg[0] = (Map183_Reg[0]&0xF0)|(byData&0x0F);
    PPUBANK[ 0 ] = VROMPAGE( Map183_Reg[0] % (NesHeader.byVRomSize << 3) );
    InfoNES_SetupChr();
    break;
  case	0xB004:
    Map183_Reg[0] = (Map183_Reg[0]&0x0F)|((byData&0x0F)<<4);
    PPUBANK[ 0 ] = VROMPAGE( Map183_Reg[0] % (NesHeader.byVRomSize << 3) );
    InfoNES_SetupChr();
    break;
  case	0xB008:
    Map183_Reg[1] = (Map183_Reg[1]&0xF0)|(byData&0x0F);
    PPUBANK[ 1 ] = VROMPAGE( Map183_Reg[1] % (NesHeader.byVRomSize << 3) );
    InfoNES_SetupChr();
    break;
  case	0xB00C:
    Map183_Reg[1] = (Map183_Reg[1]&0x0F)|((byData&0x0F)<<4);
    PPUBANK[ 1 ] = VROMPAGE( Map183_Reg[1] % (NesHeader.byVRomSize << 3) );
    InfoNES_SetupChr();
    break;
    
  case	0xC000:
    Map183_Reg[2] = (Map183_Reg[2]&0xF0)|(byData&0x0F);
    PPUBANK[ 2 ] = VROMPAGE( Map183_Reg[2] % (NesHeader.byVRomSize << 3) );
    InfoNES_SetupChr();
    break;
  case	0xC004:
    Map183_Reg[2] = (Map183_Reg[2]&0x0F)|((byData&0x0F)<<4);
    PPUBANK[ 2 ] = VROMPAGE( Map183_Reg[2] % (NesHeader.byVRomSize << 3) );
    InfoNES_SetupChr();
    break;
  case	0xC008:
    Map183_Reg[3] = (Map183_Reg[3]&0xF0)|(byData&0x0F);
    PPUBANK[ 3 ] = VROMPAGE( Map183_Reg[3] % (NesHeader.byVRomSize << 3) );
    InfoNES_SetupChr();
    break;
  case	0xC00C:
    Map183_Reg[3] = (Map183_Reg[3]&0x0F)|((byData&0x0F)<<4);
    PPUBANK[ 3 ] = VROMPAGE( Map183_Reg[3] % (NesHeader.byVRomSize << 3) );
    InfoNES_SetupChr();
    break;
    
  case	0xD000:
    Map183_Reg[4] = (Map183_Reg[4]&0xF0)|(byData&0x0F);
    PPUBANK[ 4 ] = VROMPAGE( Map183_Reg[4] % (NesHeader.byVRomSize << 3) );
    InfoNES_SetupChr();
    break;
  case	0xD004:
    Map183_Reg[4] = (Map183_Reg[4]&0x0F)|((byData&0x0F)<<4);
    PPUBANK[ 4 ] = VROMPAGE( Map183_Reg[4] % (NesHeader.byVRomSize << 3) );
    InfoNES_SetupChr();
    break;
  case	0xD008:
    Map183_Reg[5] = (Map183_Reg[5]&0xF0)|(byData&0x0F);
    PPUBANK[ 5 ] = VROMPAGE( Map183_Reg[5] % (NesHeader.byVRomSize << 3) );
    InfoNES_SetupChr();
    break;
  case	0xD00C:
    Map183_Reg[5] = (Map183_Reg[5]&0x0F)|((byData&0x0F)<<4);
    PPUBANK[ 5 ] = VROMPAGE( Map183_Reg[5] % (NesHeader.byVRomSize << 3) );
    InfoNES_SetupChr();
    break;
    
  case	0xE000:
    Map183_Reg[6] = (Map183_Reg[6]&0xF0)|(byData&0x0F);
    PPUBANK[ 6 ] = VROMPAGE( Map183_Reg[6] % (NesHeader.byVRomSize << 3) );
    InfoNES_SetupChr();
    break;
  case	0xE004:
    Map183_Reg[6] = (Map183_Reg[6]&0x0F)|((byData&0x0F)<<4);
    PPUBANK[ 6 ] = VROMPAGE( Map183_Reg[6] % (NesHeader.byVRomSize << 3) );
    InfoNES_SetupChr();
    break;
  case	0xE008:
    Map183_Reg[7] = (Map183_Reg[3]&0xF0)|(byData&0x0F);
    PPUBANK[ 7 ] = VROMPAGE( Map183_Reg[7] % (NesHeader.byVRomSize << 3) );
    InfoNES_SetupChr();
    break;
  case	0xE00C:
    Map183_Reg[7] = (Map183_Reg[3]&0x0F)|((byData&0x0F)<<4);
    PPUBANK[ 7 ] = VROMPAGE( Map183_Reg[7] % (NesHeader.byVRomSize << 3) );
    InfoNES_SetupChr();
    break;
    
  case	0x9008:
    if( byData == 1 ) {
      for( int i = 0; i < 8; i++ ) {
	Map183_Reg[i] = i;
      }
      /* Set ROM Banks */
      ROMBANK0 = ROMPAGE( 0 );
      ROMBANK1 = ROMPAGE( 1 );
      ROMBANK2 = ROMLASTPAGE( 1 );
      ROMBANK3 = ROMLASTPAGE( 0 );

      /* Set PPU Banks */
      if ( NesHeader.byVRomSize > 0 ) {
	PPUBANK[ 0 ] = VROMPAGE( 0 );
	PPUBANK[ 1 ] = VROMPAGE( 1 );
	PPUBANK[ 2 ] = VROMPAGE( 2 );
	PPUBANK[ 3 ] = VROMPAGE( 3 );
	PPUBANK[ 4 ] = VROMPAGE( 4 );
	PPUBANK[ 5 ] = VROMPAGE( 5 );
	PPUBANK[ 6 ] = VROMPAGE( 6 );
	PPUBANK[ 7 ] = VROMPAGE( 7 );
	InfoNES_SetupChr();
      }
    }
    break;
    
  case	0x9800:
    if( byData == 0 )      InfoNES_Mirroring( MIRROR_V );
    else if( byData == 1 ) InfoNES_Mirroring( MIRROR_H );
    else if( byData == 2 ) InfoNES_Mirroring( MIRROR_0 );
    else if( byData == 3 ) InfoNES_Mirroring( MIRROR_1 );
    break;
    
  case	0xF000:
    Map183_IRQ_Counter = (((Map183_IRQ_Counter >> STEP_PER_SCANLINE_SHIFT) & 0xFF00) | byData) << STEP_PER_SCANLINE_SHIFT;
    break;
  case	0xF004:
    Map183_IRQ_Counter = (((Map183_IRQ_Counter >> STEP_PER_SCANLINE_SHIFT) & 0x00FF) | (byData << 8)) << STEP_PER_SCANLINE_SHIFT;
    break;
  case	0xF008:
    Map183_IRQ_Enable = byData;
    break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 183 H-Sync Function                                       */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map183_HSync)()
{
  if( Map183_IRQ_Enable & 0x02 ) {
    if( Map183_IRQ_Counter <= STEP_PER_SCANLINE_H ) {
      Map183_IRQ_Counter = 0;
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
      IRQ_REQ;
#endif
    } else {
      Map183_IRQ_Counter -= STEP_PER_SCANLINE_H;
    }
  }
}
