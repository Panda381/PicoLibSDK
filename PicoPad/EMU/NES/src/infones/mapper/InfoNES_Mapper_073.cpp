/*===================================================================*/
/*                                                                   */
/*                     Mapper 73 (Konami VRC 3)                      */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map73_IRQ_Enable;	// 0x00
	u8	md_res[3];		// 0x01
	DWORD	md_Map73_IRQ_Cnt;	// 0x04
} sMap73_Data;

STATIC_ASSERT((sizeof(sMap73_Data) == 0x08) && (sizeof(sMap73_Data) <= MAPDATA_SIZE), "Incorrect sMap73_Data!");

#define MAPDATA73		((sMap73_Data*)NES->mapdata)	// mapper data

#define Map73_IRQ_Enable	MAPDATA73->md_Map73_IRQ_Enable
#define Map73_IRQ_Cnt		MAPDATA73->md_Map73_IRQ_Cnt

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 73                                             */
/*-------------------------------------------------------------------*/
void Map73_Init()
{
  /* Initialize Mapper */
  MapperInit = Map73_Init;

  /* Write to Mapper */
  MapperWrite = Map73_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map73_HSync;

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

  /* Initialize IRQ Registers */
  Map73_IRQ_Enable = 0;
  Map73_IRQ_Cnt = 0;  

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 73 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map73_Write)( WORD wAddr, BYTE byData )
{
  switch ( wAddr )
  {
    case 0x8000:
      Map73_IRQ_Cnt = (( (Map73_IRQ_Cnt >> STEP_PER_SCANLINE_SHIFT) & 0xfff0 ) | ( byData & 0x0f )) << STEP_PER_SCANLINE_SHIFT;
      break;

    case 0x9000:
      Map73_IRQ_Cnt = (( (Map73_IRQ_Cnt >> STEP_PER_SCANLINE_SHIFT) & 0xff0f ) | ( ( byData & 0x0f ) << 4 )) << STEP_PER_SCANLINE_SHIFT;
      break;

    case 0xa000:
      Map73_IRQ_Cnt = (( (Map73_IRQ_Cnt >> STEP_PER_SCANLINE_SHIFT) & 0xf0ff ) | ( ( byData & 0x0f ) << 8 )) << STEP_PER_SCANLINE_SHIFT;
      break;

    case 0xb000:
      Map73_IRQ_Cnt = (( (Map73_IRQ_Cnt >> STEP_PER_SCANLINE_SHIFT) & 0x0fff ) | ( ( byData & 0x0f ) << 12 )) << STEP_PER_SCANLINE_SHIFT;
      break;

    case 0xc000:
      Map73_IRQ_Enable = byData;
      break;

    /* Set ROM Banks */
    case 0xf000:
      byData <<= 1;
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK0 = ROMPAGE( byData );
      ROMBANK1 = ROMPAGE( byData + 1 );
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 73 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map73_HSync)()
{
/*
 *  Callback at HSync
 *
 */
#if 1
  if ( Map73_IRQ_Enable & 0x02 )
  {
    if ( ( Map73_IRQ_Cnt += STEP_PER_SCANLINE_H ) > (0xffff << STEP_PER_SCANLINE_SHIFT) )
    {
      Map73_IRQ_Cnt = ((Map73_IRQ_Cnt >> STEP_PER_SCANLINE_SHIFT) & 0xffff) << STEP_PER_SCANLINE_SHIFT;
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
      IRQ_REQ;
#endif
      Map73_IRQ_Enable = 0;
    }
  }
#else
  if ( Map73_IRQ_Enable )
  {
    if ( Map73_IRQ_Cnt > (0xffff << STEP_PER_SCANLINE_SHIFT) - STEP_PER_SCANLINE_H )
    {
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
      IRQ_REQ;
#endif
      Map73_IRQ_Enable = 0;
    } else {
      Map73_IRQ_Cnt += STEP_PER_SCANLINE_H;
    }
  }
#endif
}
