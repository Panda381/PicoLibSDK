/*===================================================================*/
/*                                                                   */
/*                             Mapper 156                            */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map156_ChrLo[ 8 ];	// 0x00
	BYTE	md_Map156_ChrHi[ 8 ];	// 0x08
} sMap156_Data;

STATIC_ASSERT((sizeof(sMap156_Data) == 0x10) && (sizeof(sMap156_Data) <= MAPDATA_SIZE), "Incorrect sMap156_Data!");

#define MAPDATA156	((sMap156_Data*)NES->mapdata)	// mapper data

#define Map156_ChrLo	MAPDATA156->md_Map156_ChrLo
#define Map156_ChrHi	MAPDATA156->md_Map156_ChrHi

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 156                                             */
/*-------------------------------------------------------------------*/
void Map156_Init()
{
  /* Initialize Mapper */
  MapperInit = Map156_Init;

  /* Write to Mapper */
  MapperWrite = Map156_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

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
  InfoNES_Mirroring( MIRROR_0 );
#endif

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 156 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map156_Write)( WORD wAddr, BYTE byData )
{
	switch ( wAddr )
	{
	case 0xc000:
		Map156_ChrLo[0] = byData;
		PPUBANK[0] = VROMPAGE((Map156_ChrLo[0] | ((u16)Map156_ChrHi[0] << 8)) & ((NesHeader.byVRomSize << 3) - 1));
		InfoNES_SetupChr();
		break;

	case 0xc001:
		Map156_ChrLo[1] = byData;
		PPUBANK[1] = VROMPAGE((Map156_ChrLo[1] | ((u16)Map156_ChrHi[1] << 8)) & ((NesHeader.byVRomSize << 3) - 1));
		InfoNES_SetupChr();
		break;

	case 0xc002:
		Map156_ChrLo[2] = byData;
		PPUBANK[2] = VROMPAGE((Map156_ChrLo[2] | ((u16)Map156_ChrHi[2] << 8)) & ((NesHeader.byVRomSize << 3) - 1));
		InfoNES_SetupChr();
		break;

	case 0xc003:
		Map156_ChrLo[3] = byData;
		PPUBANK[3] = VROMPAGE((Map156_ChrLo[3] | ((u16)Map156_ChrHi[3] << 8)) & ((NesHeader.byVRomSize << 3) - 1));
		InfoNES_SetupChr();
		break;

	case 0xc004:
		Map156_ChrHi[0] = byData;
		PPUBANK[0] = VROMPAGE((Map156_ChrLo[0] | ((u16)Map156_ChrHi[0] << 8)) & ((NesHeader.byVRomSize << 3) - 1));
		InfoNES_SetupChr();
		break;

	case 0xc005:
		Map156_ChrHi[1] = byData;
		PPUBANK[1] = VROMPAGE((Map156_ChrLo[1] | ((u16)Map156_ChrHi[1] << 8)) & ((NesHeader.byVRomSize << 3) - 1));
		InfoNES_SetupChr();
		break;

	case 0xc006:
		Map156_ChrHi[2] = byData;
		PPUBANK[2] = VROMPAGE((Map156_ChrLo[2] | ((u16)Map156_ChrHi[2] << 8)) & ((NesHeader.byVRomSize << 3) - 1));
		InfoNES_SetupChr();
		break;

	case 0xc007:
		Map156_ChrHi[3] = byData;
		PPUBANK[3] = VROMPAGE((Map156_ChrLo[3] | ((u16)Map156_ChrHi[3] << 8)) & ((NesHeader.byVRomSize << 3) - 1));
		InfoNES_SetupChr();
		break;

	case 0xc008:
		Map156_ChrLo[4] = byData;
		PPUBANK[4] = VROMPAGE((Map156_ChrLo[4] | ((u16)Map156_ChrHi[4] << 8)) & ((NesHeader.byVRomSize << 3) - 1));
		InfoNES_SetupChr();
		break;

	case 0xc009:
		Map156_ChrLo[5] = byData;
		PPUBANK[5] = VROMPAGE((Map156_ChrLo[5] | ((u16)Map156_ChrHi[5] << 8)) & ((NesHeader.byVRomSize << 3) - 1));
		InfoNES_SetupChr();
		break;

	case 0xc00a:
		Map156_ChrLo[6] = byData;
		PPUBANK[6] = VROMPAGE((Map156_ChrLo[6] | ((u16)Map156_ChrHi[6] << 8)) & ((NesHeader.byVRomSize << 3) - 1));
		InfoNES_SetupChr();
		break;

	case 0xc00b:
		Map156_ChrLo[7] = byData;
		PPUBANK[7] = VROMPAGE((Map156_ChrLo[7] | ((u16)Map156_ChrHi[7] << 8)) & ((NesHeader.byVRomSize << 3) - 1));
		InfoNES_SetupChr();
		break;

	case 0xc00c:
		Map156_ChrHi[4] = byData;
		PPUBANK[4] = VROMPAGE((Map156_ChrLo[4] | ((u16)Map156_ChrHi[4] << 8)) & ((NesHeader.byVRomSize << 3) - 1));
		InfoNES_SetupChr();
		break;

	case 0xc00d:
		Map156_ChrHi[5] = byData;
		PPUBANK[5] = VROMPAGE((Map156_ChrLo[5] | ((u16)Map156_ChrHi[5] << 8)) & ((NesHeader.byVRomSize << 3) - 1));
		InfoNES_SetupChr();
		break;

	case 0xc00e:
		Map156_ChrHi[6] = byData;
		PPUBANK[6] = VROMPAGE((Map156_ChrLo[6] | ((u16)Map156_ChrHi[6] << 8)) & ((NesHeader.byVRomSize << 3) - 1));
		InfoNES_SetupChr();
		break;

	case 0xc00f:
		Map156_ChrHi[7] = byData;
		PPUBANK[7] = VROMPAGE((Map156_ChrLo[7] | ((u16)Map156_ChrHi[7] << 8)) & ((NesHeader.byVRomSize << 3) - 1));
		InfoNES_SetupChr();
		break;

	case 0xc010:
		byData <<= 1;
		ROMBANK0 = ROMPAGE( (byData + 0) & ((NesHeader.byRomSize << 1) - 1));
		ROMBANK1 = ROMPAGE( (byData + 1) & ((NesHeader.byRomSize << 1) - 1));
		break;

	case 0xc014:
		InfoNES_Mirroring( (byData & 1) ? MIRROR_H : MIRROR_V );
		break;
	}
}
