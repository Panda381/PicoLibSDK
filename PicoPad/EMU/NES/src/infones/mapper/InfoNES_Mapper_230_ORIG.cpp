/*===================================================================*/
/*                                                                   */
/*                       Mapper 230 : 22-in-1                        */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map230_RomSw;	// 0x00
	u8	md_res[3];		// 0x01
} sMap230_Data;

STATIC_ASSERT((sizeof(sMap230_Data) == 0x04) && (sizeof(sMap230_Data) <= MAPDATA_SIZE), "Incorrect sMap230_Data!");

#define MAPDATA230		((sMap230_Data*)NES->mapdata)	// mapper data

#define Map230_RomSw	MAPDATA230->md_Map230_RomSw

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 230                                            */
/*-------------------------------------------------------------------*/
void Map230_Init()
{
  /* Initialize Mapper */
  MapperInit = Map230_Init;

  /* Write to Mapper */
  MapperWrite = Map230_Write;

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

  /* Initialize Registers */
  if( Map230_RomSw ) {
    Map230_RomSw = 0;
  } else {
    Map230_RomSw = 1;
  }

  /* Set ROM Banks */
  if( Map230_RomSw ) {
    ROMBANK0 = ROMPAGE( 0 );
    ROMBANK1 = ROMPAGE( 1 );
    ROMBANK2 = ROMPAGE( 14 );
    ROMBANK3 = ROMPAGE( 15 );
  } else {
    ROMBANK0 = ROMPAGE( 16 );
    ROMBANK1 = ROMPAGE( 17 );
    ROMBANK2 = ROMLASTPAGE( 1 );
    ROMBANK3 = ROMLASTPAGE( 0 );
  }

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 230 Write Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map230_Write)( WORD wAddr, BYTE byData )
{
  if( Map230_RomSw ) {
    ROMBANK0 = ROMPAGE((((byData&0x07)<<1)+0) % (NesHeader.byRomSize<<1));
    ROMBANK1 = ROMPAGE((((byData&0x07)<<1)+1) % (NesHeader.byRomSize<<1));
  } else {
    if( byData & 0x20 ) {
      ROMBANK0 = ROMPAGE((((byData&0x1F)<<1)+16) % (NesHeader.byRomSize<<1));
      ROMBANK1 = ROMPAGE((((byData&0x1F)<<1)+17) % (NesHeader.byRomSize<<1));
      ROMBANK2 = ROMPAGE((((byData&0x1F)<<1)+16) % (NesHeader.byRomSize<<1));
      ROMBANK3 = ROMPAGE((((byData&0x1F)<<1)+17) % (NesHeader.byRomSize<<1));
    } else {
      ROMBANK0 = ROMPAGE((((byData&0x1E)<<1)+16) % (NesHeader.byRomSize<<1));
      ROMBANK1 = ROMPAGE((((byData&0x1E)<<1)+17) % (NesHeader.byRomSize<<1));
      ROMBANK2 = ROMPAGE((((byData&0x1E)<<1)+18) % (NesHeader.byRomSize<<1));
      ROMBANK3 = ROMPAGE((((byData&0x1E)<<1)+19) % (NesHeader.byRomSize<<1));
    }
    if( byData & 0x40 ) {
      InfoNES_Mirroring( MIRROR_V );
    } else {
      InfoNES_Mirroring( MIRROR_H );
    }
  }
}
