/*===================================================================*/
/*                                                                   */
/*                        Mapper 82 (Taito X1-17)                    */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map82_Regs[ 1 ];	// 0x00
	u8	md_res[3];		// 0x01
} sMap82_Data;

STATIC_ASSERT((sizeof(sMap82_Data) == 0x04) && (sizeof(sMap82_Data) <= MAPDATA_SIZE), "Incorrect sMap82_Data!");

#define MAPDATA82		((sMap82_Data*)NES->mapdata)	// mapper data

#define Map82_Regs	MAPDATA82->md_Map82_Regs

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 82                                             */
/*-------------------------------------------------------------------*/
void Map82_Init()
{
  /* Initialize Mapper */
  MapperInit = Map82_Init;

  /* Write to Mapper */
  MapperWrite = Map0_Write;

  /* Write to SRAM */
  MapperSram = Map82_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map0_HSync;

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

#ifndef NES_MIRRORING	// use custom mirroring: 0=horiz, 1=vert, 2=2400, 3=2000, 4=four, 5=0001, 6=0111
  /* Name Table Mirroring */
  InfoNES_Mirroring( MIRROR_V );
#endif

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 82 Write to SRAM Function                                 */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map82_Sram)( WORD wAddr, BYTE byData )
{
  switch ( wAddr )
  {
    /* Set PPU Banks */
    case 0x7ef0:
      byData &= 0xfe;
      byData %= ( NesHeader.byVRomSize << 3 );

      if ( Map82_Regs[ 0 ] )
      {
        PPUBANK[ 4 ] = VROMPAGE( byData );
        PPUBANK[ 5 ] = VROMPAGE( byData + 1 );
      } else {
        PPUBANK[ 0 ] = VROMPAGE( byData );
        PPUBANK[ 1 ] = VROMPAGE( byData + 1 );
      }
      InfoNES_SetupChr();
      break;

    case 0x7ef1:
      byData &= 0xfe;
      byData %= ( NesHeader.byVRomSize << 3 );

      if ( Map82_Regs[ 0 ] )
      {
        PPUBANK[ 6 ] = VROMPAGE( byData );
        PPUBANK[ 7 ] = VROMPAGE( byData + 1 );
      } else {
        PPUBANK[ 2 ] = VROMPAGE( byData );
        PPUBANK[ 3 ] = VROMPAGE( byData + 1 );
      }
      InfoNES_SetupChr();
      break;
  
    case 0x7ef2:
      byData %= ( NesHeader.byVRomSize << 3 );
      
      if ( !Map82_Regs[ 0 ] )
      {
        PPUBANK[ 4 ] = VROMPAGE( byData );
      } else {
        PPUBANK[ 0 ] = VROMPAGE( byData );
      }
      InfoNES_SetupChr();
      break;    
      
    case 0x7ef3:
      byData %= ( NesHeader.byVRomSize << 3 );
      
      if ( !Map82_Regs[ 0 ] )
      {
        PPUBANK[ 5 ] = VROMPAGE( byData );
      } else {
        PPUBANK[ 1 ] = VROMPAGE( byData );
      }
      InfoNES_SetupChr();
      break;  

    case 0x7ef4:
      byData %= ( NesHeader.byVRomSize << 3 );
      
      if ( !Map82_Regs[ 0 ] )
      {
        PPUBANK[ 6 ] = VROMPAGE( byData );
      } else {
        PPUBANK[ 2 ] = VROMPAGE( byData );
      }
      InfoNES_SetupChr();
      break;  

    case 0x7ef5:
      byData %= ( NesHeader.byVRomSize << 3 );
      
      if ( !Map82_Regs[ 0 ] )
      {
        PPUBANK[ 7 ] = VROMPAGE( byData );
      } else {
        PPUBANK[ 3 ] = VROMPAGE( byData );
      }
      InfoNES_SetupChr();
      break;  

    /* Name Table Mirroring */
    case 0x7ef6:
      Map82_Regs[ 0 ] = byData & 0x02;
      
      if ( byData & 0x01 )
      {
        InfoNES_Mirroring( MIRROR_V );
      } else {
        InfoNES_Mirroring( MIRROR_H );
      }

    /* Set ROM Banks */
    case 0x7efa:
      byData >>= 2;
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK0 = ROMPAGE( byData );
      break;

    case 0x7efb:
      byData >>= 2;
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK1 = ROMPAGE( byData );
      break;

    case 0x7efc:
      byData >>= 2;
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK2 = ROMPAGE( byData );
      break;
  }
}
