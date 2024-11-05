/*===================================================================*/
/*                                                                   */
/*                         Mapper 43 (SMB2J)                         */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	DWORD	md_Map43_IRQ_Cnt;	// 0x00
	BYTE	md_Map43_IRQ_Enable;	// 0x04
	u8	md_res[3];		// 0x05
} sMap43_Data;

STATIC_ASSERT((sizeof(sMap43_Data) == 0x08) && (sizeof(sMap43_Data) <= MAPDATA_SIZE), "Incorrect sMap43_Data!");

#define MAPDATA43		((sMap43_Data*)NES->mapdata)	// mapper data

#define Map43_IRQ_Cnt		MAPDATA43->md_Map43_IRQ_Cnt
#define Map43_IRQ_Enable	MAPDATA43->md_Map43_IRQ_Enable

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 43                                             */
/*-------------------------------------------------------------------*/
void Map43_Init()
{
  /* Initialize Mapper */
  MapperInit = Map43_Init;

  /* Write to Mapper */
  MapperWrite = Map43_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map43_Apu;

  /* Read from APU */
  MapperReadApu = Map43_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map43_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = ROMPAGE( 2 );

  /* Set ROM Banks */
  ROMBANK0 = ROMPAGE( 1 );
  ROMBANK1 = ROMPAGE( 0 );
  ROMBANK2 = ROMPAGE( 4 );
  ROMBANK3 = ROMPAGE( 9 );

  /* Initialize State Registers */
	Map43_IRQ_Enable = 1;
	Map43_IRQ_Cnt = 0;

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
/*  Mapper 43 Read from APU Function                                 */
/*-------------------------------------------------------------------*/
BYTE FASTCODE NOFLASH(Map43_ReadApu)( WORD wAddr )
{
  if ( 0x5000 <= wAddr && wAddr < 0x6000 ) 
  {
    return ROM[ 0x2000*8 + 0x1000 + (wAddr - 0x5000) ];
  }
  return (BYTE)(wAddr >> 8);
}

/*-------------------------------------------------------------------*/
/*  Mapper 43 Write to APU Function                                  */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map43_Apu)( WORD wAddr, BYTE byData )
{
	if( (wAddr&0xF0FF) == 0x4022 ) 
  {
		switch( byData&0x07 ) 
    {
			case	0x00:
			case	0x02:
			case	0x03:
			case	0x04:
        ROMBANK2 = ROMPAGE( 4 );
				break;
			case	0x01:
        ROMBANK2 = ROMPAGE( 3 );
				break;
			case	0x05:
        ROMBANK2 = ROMPAGE( 7 );
				break;
			case	0x06:
        ROMBANK2 = ROMPAGE( 5 );
				break;
			case	0x07:
        ROMBANK2 = ROMPAGE( 6 );
				break;
		}
	}
}

/*-------------------------------------------------------------------*/
/*  Mapper 43 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map43_Write)( WORD wAddr, BYTE byData )
{
	if( wAddr == 0x8122 ) {
		if( byData&0x03 ) {
			Map43_IRQ_Enable = 1;
		} else {
			Map43_IRQ_Cnt = 0;
			Map43_IRQ_Enable = 0;
		}
	}
}

/*-------------------------------------------------------------------*/
/*  Mapper 43 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map43_HSync)()
{
	if( Map43_IRQ_Enable ) 
  {
		Map43_IRQ_Cnt += STEP_PER_SCANLINE_H;
		if( Map43_IRQ_Cnt >= (4096 << STEP_PER_SCANLINE_SHIFT)) {
			Map43_IRQ_Cnt -= (4096 << STEP_PER_SCANLINE_SHIFT);
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
			IRQ_REQ;
#endif
		}
	}
}
