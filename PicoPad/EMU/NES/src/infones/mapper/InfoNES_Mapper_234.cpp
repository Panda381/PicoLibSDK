/*===================================================================*/
/*                                                                   */
/*                     Mapper 234 : Maxi-15                          */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map234_Reg[2];	// 0x00
	u8	md_res[2];		// 0x02
} sMap234_Data;

STATIC_ASSERT((sizeof(sMap234_Data) == 0x04) && (sizeof(sMap234_Data) <= MAPDATA_SIZE), "Incorrect sMap234_Data!");

#define MAPDATA234	((sMap234_Data*)NES->mapdata)	// mapper data

#define Map234_Reg	MAPDATA234->md_Map234_Reg

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 234                                            */
/*-------------------------------------------------------------------*/
void Map234_Init()
{
  /* Initialize Mapper */
  MapperInit = Map234_Init;

  /* Write to Mapper */
  MapperWrite = Map0_Write;

  /* Read from Mapper */
  MapperRead = Map234_Read;

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

  /* Set Registers */
  Map234_Reg[0] = 0;
  Map234_Reg[1] = 0;

  /* Set ROM Banks */
  Map234_Set_Banks();

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 234 Read Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map234_Read)( WORD wAddr, BYTE byData )
{
  if( wAddr >= 0xFF80 && wAddr <= 0xFF9F )
  {
    if( !Map234_Reg[0] )
    {
      Map234_Reg[0] = byData;
      Map234_Set_Banks();
    }
  }
  
  if( wAddr >= 0xFFE8 && wAddr <= 0xFFF7 )
  {
    Map234_Reg[1] = byData;
    Map234_Set_Banks();
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 234 Set Banks Function                                    */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map234_Set_Banks)()
{
  BYTE byPrg, byChr;

  if( Map234_Reg[0] & 0x80 ) {
    InfoNES_Mirroring( MIRROR_H );
  } else {
    InfoNES_Mirroring( MIRROR_V );
  }

  if( Map234_Reg[0] & 0x40 ) {
    byPrg = (Map234_Reg[0] & 0x0E) | (Map234_Reg[1] & 0x01);
    byChr = ((Map234_Reg[0] & 0x0E) << 2) | ((Map234_Reg[1] >> 4) & 0x07);
  } else {
    byPrg = Map234_Reg[0] & 0x0F;
    byChr = ((Map234_Reg[0] & 0x0F) << 2) | ((Map234_Reg[1] >> 4) & 0x03);
  }
  
  /* Set ROM Banks */
  byPrg <<= 2;
  u8 m = (NesHeader.byRomSize << 1) - 1;
  ROMBANK0 = ROMPAGE((byPrg + 0) & m);
  ROMBANK1 = ROMPAGE((byPrg + 1) & m);
  ROMBANK2 = ROMPAGE((byPrg + 2) & m);
  ROMBANK3 = ROMPAGE((byPrg + 3) & m);

  /* Set PPU Banks */
  byChr <<= 3;
  m = (NesHeader.byVRomSize << 3) - 1;
  PPUBANK[ 0 ] = VROMPAGE((byChr + 0) & m);
  PPUBANK[ 1 ] = VROMPAGE((byChr + 1) & m);
  PPUBANK[ 2 ] = VROMPAGE((byChr + 2) & m);
  PPUBANK[ 3 ] = VROMPAGE((byChr + 3) & m);
  PPUBANK[ 4 ] = VROMPAGE((byChr + 4) & m);
  PPUBANK[ 5 ] = VROMPAGE((byChr + 5) & m);
  PPUBANK[ 6 ] = VROMPAGE((byChr + 6) & m);
  PPUBANK[ 7 ] = VROMPAGE((byChr + 7) & m);
  InfoNES_SetupChr();
}
