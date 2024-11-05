/*===================================================================*/
/*                                                                   */
/*                  Mapper 24 (Konami VRC6)                          */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map24_IRQ_Count;	// 0x00
	BYTE	md_Map24_IRQ_State;	// 0x01
	BYTE	md_Map24_IRQ_Latch;	// 0x02
	u8	md_res;			// 0x03
} sMap24_Data;

STATIC_ASSERT((sizeof(sMap24_Data) == 0x04) && (sizeof(sMap24_Data) <= MAPDATA_SIZE), "Incorrect sMap24_Data!");

#define MAPDATA24		((sMap24_Data*)NES->mapdata)	// mapper data

#define Map24_IRQ_Count		MAPDATA24->md_Map24_IRQ_Count
#define Map24_IRQ_State		MAPDATA24->md_Map24_IRQ_State
#define Map24_IRQ_Latch		MAPDATA24->md_Map24_IRQ_Latch

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 24                                             */
/*-------------------------------------------------------------------*/
void Map24_Init()
{
  /* Initialize Mapper */
  MapperInit = Map24_Init;

  /* Write to Mapper */
  MapperWrite = Map24_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map24_HSync;

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

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 24 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map24_Write)( WORD wAddr, BYTE byData )
{
  switch ( wAddr )
  {
    case 0x8000:
      /* Set ROM Banks */
      byData <<= 1;
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK0 = ROMPAGE( byData + 0 );
      ROMBANK1 = ROMPAGE( byData + 1 );
      break;

    case 0xb003:
      /* Name Table Mirroring */
      switch ( byData & 0x0c )
      {
        case 0x00:
          InfoNES_Mirroring( MIRROR_V );   /* Vertical */
          break;
        case 0x04:
          InfoNES_Mirroring( MIRROR_H );   /* Horizontal */
          break;
        case 0x08:
          InfoNES_Mirroring( MIRROR_0 );   /* One Screen 0x2000 */
          break;
        case 0x0c:
          InfoNES_Mirroring( MIRROR_1 );   /* One Screen 0x2400 */
          break;
      }
      break;

	  case 0xC000:
      ROMBANK2 = ROMPAGE( byData % ( NesHeader.byRomSize << 1) );
		  break;

	  case 0xD000:
      PPUBANK[ 0 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

	  case 0xD001:
      PPUBANK[ 1 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

	  case 0xD002:
      PPUBANK[ 2 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

	  case 0xD003:
      PPUBANK[ 3 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

	  case 0xE000:
      PPUBANK[ 4 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

	  case 0xE001:
      PPUBANK[ 5 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

	  case 0xE002:
      PPUBANK[ 6 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

	  case 0xE003:
      PPUBANK[ 7 ] = VROMPAGE( byData % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

	  case 0xF000:
			Map24_IRQ_Latch = byData;
	  	break;

	  case 0xF001:
			Map24_IRQ_State = byData & 0x03;
			if(Map24_IRQ_State & 0x02)
			{
				Map24_IRQ_Count = Map24_IRQ_Latch;
			}
		  break;

	  case 0xF002:
			if(Map24_IRQ_State & 0x01)
			{
				Map24_IRQ_State |= 0x02;
			}
			else
			{
				Map24_IRQ_State &= 0x01;
			}
		break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 24 H-Sync Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map24_HSync)()
{
/*
 *  Callback at HSync
 *
 */
	if(Map24_IRQ_State & 0x02)
	{
	  if(Map24_IRQ_Count == 0xFF)
		{
#ifndef NES_NOIRQ_HSYNC		// if defined, disable IRQ on H-Sync in mapper
			IRQ_REQ;
#endif
			Map24_IRQ_Count = Map24_IRQ_Latch;
		}
		else
		{
			Map24_IRQ_Count++;
		}
	}
}
