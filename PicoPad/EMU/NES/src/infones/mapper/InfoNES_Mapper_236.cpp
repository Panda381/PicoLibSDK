/*===================================================================*/
/*                                                                   */
/*                   Mapper 236 : 800-in-1                           */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map236_Bank;		// 0x00
	BYTE	md_Map236_Mode;		// 0x01
	u8	md_res[2];		// 0x02
} sMap236_Data;

STATIC_ASSERT((sizeof(sMap236_Data) == 0x04) && (sizeof(sMap236_Data) <= MAPDATA_SIZE), "Incorrect sMap236_Data!");

#define MAPDATA236		((sMap236_Data*)NES->mapdata)	// mapper data

#define Map236_Bank	MAPDATA236->md_Map236_Bank
#define Map236_Mode	MAPDATA236->md_Map236_Mode

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 236                                            */
/*-------------------------------------------------------------------*/
void Map236_Init()
{
  /* Initialize Mapper */
  MapperInit = Map236_Init;

  /* Write to Mapper */
  MapperWrite = Map236_Write;

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

  /* Set Registers */
  Map236_Bank = Map236_Mode = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 236 Write Function                                        */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map236_Write)( WORD wAddr, BYTE byData )
{
  if( wAddr >= 0x8000 && wAddr <= 0xBFFF ) {
    Map236_Bank = ((wAddr&0x03)<<4)|(Map236_Bank&0x07);
  } else {
    Map236_Bank = (wAddr&0x07)|(Map236_Bank&0x30);
    Map236_Mode = wAddr&0x30;
  }

  if( wAddr & 0x20 ) {
    InfoNES_Mirroring( MIRROR_H );
  } else {
    InfoNES_Mirroring( MIRROR_V );
  }

  switch( Map236_Mode ) {
  case	0x00:
    Map236_Bank |= 0x08;
    ROMBANK0 = ROMPAGE(((Map236_Bank<<1)+0) % (NesHeader.byRomSize<<1));
    ROMBANK1 = ROMPAGE(((Map236_Bank<<1)+1) % (NesHeader.byRomSize<<1));
    ROMBANK2 = ROMPAGE((((Map236_Bank|0x07)<<1)+0) % (NesHeader.byRomSize<<1));
    ROMBANK3 = ROMPAGE((((Map236_Bank|0x07)<<1)+1) % (NesHeader.byRomSize<<1));
    break;
  case	0x10:
    Map236_Bank |= 0x37;
    ROMBANK0 = ROMPAGE(((Map236_Bank<<1)+0) % (NesHeader.byRomSize<<1));
    ROMBANK1 = ROMPAGE(((Map236_Bank<<1)+1) % (NesHeader.byRomSize<<1));
    ROMBANK2 = ROMPAGE((((Map236_Bank|0x07)<<1)+0) % (NesHeader.byRomSize<<1));
    ROMBANK3 = ROMPAGE((((Map236_Bank|0x07)<<1)+1) % (NesHeader.byRomSize<<1));
    break;
  case	0x20:
    Map236_Bank |= 0x08;
    ROMBANK0 = ROMPAGE((((Map236_Bank&0xFE)<<1)+0) % (NesHeader.byRomSize<<1));
    ROMBANK1 = ROMPAGE((((Map236_Bank&0xFE)<<1)+1) % (NesHeader.byRomSize<<1));
    ROMBANK2 = ROMPAGE((((Map236_Bank&0xFE)<<1)+2) % (NesHeader.byRomSize<<1));
    ROMBANK3 = ROMPAGE((((Map236_Bank&0xFE)<<1)+3) % (NesHeader.byRomSize<<1));
    break;
  case	0x30:
    Map236_Bank |= 0x08;
    ROMBANK0 = ROMPAGE(((Map236_Bank<<1)+0) % (NesHeader.byRomSize<<1));
    ROMBANK1 = ROMPAGE(((Map236_Bank<<1)+1) % (NesHeader.byRomSize<<1));
    ROMBANK2 = ROMPAGE(((Map236_Bank<<1)+0) % (NesHeader.byRomSize<<1));
    ROMBANK3 = ROMPAGE(((Map236_Bank<<1)+1) % (NesHeader.byRomSize<<1));
    break;
  }
}
