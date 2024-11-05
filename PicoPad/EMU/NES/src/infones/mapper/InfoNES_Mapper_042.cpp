/*===================================================================*/
/*                                                                   */
/*                     Mapper 42 (Pirates)                           */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map42_IRQ_Cnt;	// 0x00
	BYTE	md_Map42_IRQ_Enable;	// 0x01
	u8	md_res[2];		// 0x02
} sMap42_Data;

STATIC_ASSERT((sizeof(sMap42_Data) == 0x04) && (sizeof(sMap42_Data) <= MAPDATA_SIZE), "Incorrect sMap42_Data!");

#define MAPDATA42		((sMap42_Data*)NES->mapdata)	// mapper data

#define Map42_IRQ_Cnt		MAPDATA42->md_Map42_IRQ_Cnt
#define Map42_IRQ_Enable	MAPDATA42->md_Map42_IRQ_Enable

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 42                                             */
/*-------------------------------------------------------------------*/
void Map42_Init()
{
  /* Initialize Mapper */
  MapperInit = Map42_Init;

  /* Write to Mapper */
  MapperWrite = Map42_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map42_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = ROMPAGE( 0 );

  /* Set ROM Banks */
  ROMBANK0 = ROMLASTPAGE( 3 );
  ROMBANK1 = ROMLASTPAGE( 2 );
  ROMBANK2 = ROMLASTPAGE( 1 );
  ROMBANK3 = ROMLASTPAGE( 0 );

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 )
  {
    for ( int nPage = 0; nPage < 8; ++nPage )
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    InfoNES_SetupChr();
  }

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 42 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map42_Write)( WORD wAddr, BYTE byData )
{
  switch ( wAddr & 0xe003 )
  {
    /* Set ROM Banks */
    case 0xe000:
      SRAMBANK = ROMPAGE( ( byData & 0x0f ) % ( NesHeader.byRomSize << 1 ) );
      break;

    case 0xe001:
      if ( byData & 0x08 )
      {
        InfoNES_Mirroring( MIRROR_H );
      } else {
        InfoNES_Mirroring( MIRROR_V );
      }
      break;

    case 0xe002:
      if ( byData & 0x02 )
      {
        Map42_IRQ_Enable = 1;
      } else {
        Map42_IRQ_Enable = 0;
        Map42_IRQ_Cnt = 0;
      }
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 42 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map42_HSync)()
{
/*
 *  Callback at HSync
 *
 */
  if ( Map42_IRQ_Enable )
  {
    if ( Map42_IRQ_Cnt < 215 )
    {
      Map42_IRQ_Cnt++;
    }
    if ( Map42_IRQ_Cnt == 215 )
    {
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
      IRQ_REQ;
#endif
      Map42_IRQ_Enable = 0;
    }
  }
}
