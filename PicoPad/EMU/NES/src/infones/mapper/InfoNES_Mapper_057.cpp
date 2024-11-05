/*===================================================================*/
/*                                                                   */
/*                           Mapper 57                               */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map57_Reg;	// 0x00
	u8	md_res[3];	// 0x01
} sMap57_Data;

STATIC_ASSERT((sizeof(sMap57_Data) == 0x04) && (sizeof(sMap57_Data) <= MAPDATA_SIZE), "Incorrect sMap57_Data!");

#define MAPDATA57		((sMap57_Data*)NES->mapdata)	// mapper data

#define Map57_Reg	MAPDATA57->md_Map57_Reg

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 57                                             */
/*-------------------------------------------------------------------*/
void Map57_Init()
{
  /* Initialize Mapper */
  MapperInit = Map57_Init;

  /* Write to Mapper */
  MapperWrite = Map57_Write;

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
  ROMBANK2 = ROMPAGE( 0 );
  ROMBANK3 = ROMPAGE( 1 );

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 ) {
    for ( int nPage = 0; nPage < 8; ++nPage )
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    InfoNES_SetupChr();
  }

  /* Set Registers */
  Map57_Reg = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 57 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map57_Write)( WORD wAddr, BYTE byData )
{
  BYTE byChr;

  switch( wAddr ) {
  case	0x8000:
  case	0x8001:
  case	0x8002:
  case	0x8003:
    if( byData & 0x40 ) {
      byChr = (byData&0x03)+((Map57_Reg&0x10)>>1)+(Map57_Reg&0x07);

      PPUBANK[ 0 ] = VROMPAGE(((byChr<<3)+0) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 1 ] = VROMPAGE(((byChr<<3)+1) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 2 ] = VROMPAGE(((byChr<<3)+2) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 3 ] = VROMPAGE(((byChr<<3)+3) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 4 ] = VROMPAGE(((byChr<<3)+4) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 5 ] = VROMPAGE(((byChr<<3)+5) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 6 ] = VROMPAGE(((byChr<<3)+6) % (NesHeader.byVRomSize<<3));
      PPUBANK[ 7 ] = VROMPAGE(((byChr<<3)+7) % (NesHeader.byVRomSize<<3));
      InfoNES_SetupChr();
    }
    break;
  case	0x8800:
    Map57_Reg = byData;
    
    if( byData & 0x80 ) {
      ROMBANK0 = ROMPAGE((((byData & 0x40)>>6)*4+8+0) % (NesHeader.byRomSize<<1));
      ROMBANK1 = ROMPAGE((((byData & 0x40)>>6)*4+8+1) % (NesHeader.byRomSize<<1));
      ROMBANK2 = ROMPAGE((((byData & 0x40)>>6)*4+8+2) % (NesHeader.byRomSize<<1));
      ROMBANK3 = ROMPAGE((((byData & 0x40)>>6)*4+8+3) % (NesHeader.byRomSize<<1));
    } else {
      ROMBANK0 = ROMPAGE((((byData & 0x60)>>5)*2+0) % (NesHeader.byRomSize<<1));
      ROMBANK1 = ROMPAGE((((byData & 0x60)>>5)*2+1) % (NesHeader.byRomSize<<1));
      ROMBANK2 = ROMPAGE((((byData & 0x60)>>5)*2+0) % (NesHeader.byRomSize<<1));
      ROMBANK3 = ROMPAGE((((byData & 0x60)>>5)*2+1) % (NesHeader.byRomSize<<1));
    }
    
    byChr = (byData&0x07)+((byData&0x10)>>1);
    
    PPUBANK[ 0 ] = VROMPAGE(((byChr<<3)+0) % (NesHeader.byVRomSize<<3));
    PPUBANK[ 1 ] = VROMPAGE(((byChr<<3)+1) % (NesHeader.byVRomSize<<3));
    PPUBANK[ 2 ] = VROMPAGE(((byChr<<3)+2) % (NesHeader.byVRomSize<<3));
    PPUBANK[ 3 ] = VROMPAGE(((byChr<<3)+3) % (NesHeader.byVRomSize<<3));
    PPUBANK[ 4 ] = VROMPAGE(((byChr<<3)+4) % (NesHeader.byVRomSize<<3));
    PPUBANK[ 5 ] = VROMPAGE(((byChr<<3)+5) % (NesHeader.byVRomSize<<3));
    PPUBANK[ 6 ] = VROMPAGE(((byChr<<3)+6) % (NesHeader.byVRomSize<<3));
    PPUBANK[ 7 ] = VROMPAGE(((byChr<<3)+7) % (NesHeader.byVRomSize<<3));
    InfoNES_SetupChr();

    if( byData & 0x08 ) InfoNES_Mirroring( MIRROR_H );
    else		InfoNES_Mirroring( MIRROR_V );
    
    break;
  }
}

