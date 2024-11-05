/*===================================================================*/
/*                                                                   */
/*                   Mapper 96 : Bandai 74161                        */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map96_Reg[2];	// 0x00
	u8	res[2];			// 0x02
} sMap96_Data;

STATIC_ASSERT((sizeof(sMap96_Data) == 0x04) && (sizeof(sMap96_Data) <= MAPDATA_SIZE), "Incorrect sMap96_Data!");

#define MAPDATA96		((sMap96_Data*)NES->mapdata)	// mapper data

#define Map96_Reg	MAPDATA96->md_Map96_Reg

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 96                                             */
/*-------------------------------------------------------------------*/
void Map96_Init()
{
  /* Initialize Mapper */
  MapperInit = Map96_Init;

  /* Write to Mapper */
  MapperWrite = Map96_Write;

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
  MapperPPU = Map96_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  /* Set Registers */
  Map96_Reg[0] = Map96_Reg[1] = 0;

  /* Set ROM Banks */
  ROMBANK0 = ROMPAGE( 0 );
  ROMBANK1 = ROMPAGE( 1 );
  ROMBANK2 = ROMPAGE( 2 );
  ROMBANK3 = ROMPAGE( 3 );

  /* Set PPU Banks */
  Map96_Set_Banks();

#ifndef NES_MIRRORING	// use custom mirroring: 0=horiz, 1=vert, 2=2400, 3=2000, 4=four, 5=0001, 6=0111
  InfoNES_Mirroring( MIRROR_0 );
#endif

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 96 Write Function                                         */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map96_Write)( WORD wAddr, BYTE byData )
{
  ROMBANK0 = ROMPAGE((((byData & 0x03)<<2)+0) % (NesHeader.byRomSize<<1));
  ROMBANK1 = ROMPAGE((((byData & 0x03)<<2)+1) % (NesHeader.byRomSize<<1));
  ROMBANK2 = ROMPAGE((((byData & 0x03)<<2)+2) % (NesHeader.byRomSize<<1));
  ROMBANK3 = ROMPAGE((((byData & 0x03)<<2)+3) % (NesHeader.byRomSize<<1));
  
  Map96_Reg[0] = (byData & 0x04) >> 2;
  Map96_Set_Banks();
}

/*-------------------------------------------------------------------*/
/*  Mapper 96 PPU Function                                           */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map96_PPU)( WORD wAddr )
{
  if( (wAddr & 0xF000) == 0x2000 ) {
    Map96_Reg[1] = (wAddr>>8)&0x03;
    Map96_Set_Banks();
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 96 Set Banks Function                                     */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map96_Set_Banks)()
{
  PPUBANK[ 0 ] = CRAMPAGE(((Map96_Reg[0]*4+Map96_Reg[1])<<2)+0);
  PPUBANK[ 1 ] = CRAMPAGE(((Map96_Reg[0]*4+Map96_Reg[1])<<2)+1);
  PPUBANK[ 2 ] = CRAMPAGE(((Map96_Reg[0]*4+Map96_Reg[1])<<2)+2);
  PPUBANK[ 3 ] = CRAMPAGE(((Map96_Reg[0]*4+Map96_Reg[1])<<2)+3);
  PPUBANK[ 4 ] = CRAMPAGE(((Map96_Reg[0]*4+0x03)<<2)+0);
  PPUBANK[ 5 ] = CRAMPAGE(((Map96_Reg[0]*4+0x03)<<2)+1);
  PPUBANK[ 6 ] = CRAMPAGE(((Map96_Reg[0]*4+0x03)<<2)+2);
  PPUBANK[ 7 ] = CRAMPAGE(((Map96_Reg[0]*4+0x03)<<2)+3);
  InfoNES_SetupChr();
}

