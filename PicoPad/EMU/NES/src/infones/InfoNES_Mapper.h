/*===================================================================*/
/*                                                                   */
/*  InfoNES_Mapper.h : InfoNES Mapper Function                       */
/*                                                                   */
/*  2000/05/16  InfoNES Project ( based on NesterJ and pNesX )       */
/*                                                                   */
/*===================================================================*/

#ifndef InfoNES_MAPPER_H_INCLUDED
#define InfoNES_MAPPER_H_INCLUDED

/*-------------------------------------------------------------------*/
/*  Macros                                                           */
/*-------------------------------------------------------------------*/

/* The address of 8Kbytes unit of the ROM */
#define ROMPAGE(a) &ROM[(a)*0x2000]
/* From behind the ROM, the address of 8kbytes unit */
#define ROMLASTPAGE(a) &ROM[NesHeader.byRomSize * 0x4000 - ((a) + 1) * 0x2000]
/* The address of 1Kbytes unit of the VROM */
#define VROMPAGE(a) &VROM[(a)*0x400]
/* The address of 1Kbytes unit of the CRAM */
#define CRAMPAGE(a) &PPURAM[0x0000 + ((a)&0x1F) * 0x400]
/* The address of 1Kbytes unit of the VRAM */
#define VRAMPAGE(a) &PPURAM[0x2000 + (a)*0x400]
/* Translate the pointer to ChrBuf into the address of Pattern Table */
#define PATTBL(a) (((a)-ChrBuf) >> 2)

/*-------------------------------------------------------------------*/
/*  Table of Mapper initialize function                              */
/*-------------------------------------------------------------------*/

struct MapperTable_tag
{
  int nMapperNo;
  void (*pMapperInit)();
};

extern const struct MapperTable_tag MapperTable[];

/*-------------------------------------------------------------------*/
/*  Function prototypes                                              */
/*-------------------------------------------------------------------*/

void Map0_Init();
void FASTCODE NOFLASH(Map0_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map0_Sram)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map0_Apu)(WORD wAddr, BYTE byData);
BYTE FASTCODE NOFLASH(Map0_ReadApu)(WORD wAddr);
void FASTCODE NOFLASH(Map0_VSync)();
void FASTCODE NOFLASH(Map0_HSync)();
void FASTCODE NOFLASH(Map0_PPU)(WORD wAddr);
void FASTCODE NOFLASH(Map0_RenderScreen)(BYTE byMode);

void Map1_Init();
void FASTCODE NOFLASH(Map1_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map1_set_ROM_banks)();

void Map2_Init();
void FASTCODE NOFLASH(Map2_Write)(WORD wAddr, BYTE byData);

void Map3_Init();
void FASTCODE NOFLASH(Map3_Write)(WORD wAddr, BYTE byData);

void Map4_Init();
void FASTCODE NOFLASH(Map4_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map4_HSync)();
void FASTCODE NOFLASH(Map4_Set_CPU_Banks)();
void FASTCODE NOFLASH(Map4_Set_PPU_Banks)();

void Map5_Init();
void FASTCODE NOFLASH(Map5_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map5_Apu)(WORD wAddr, BYTE byData);
BYTE FASTCODE NOFLASH(Map5_ReadApu)(WORD wAddr);
void FASTCODE NOFLASH(Map5_HSync)();
void FASTCODE NOFLASH(Map5_RenderScreen)(BYTE byMode);
void FASTCODE NOFLASH(Map5_Sync_Prg_Banks)(void);

void Map6_Init();
void FASTCODE NOFLASH(Map6_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map6_Apu)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map6_HSync)();

void Map7_Init();
void FASTCODE NOFLASH(Map7_Write)(WORD wAddr, BYTE byData);

void Map8_Init();
void FASTCODE NOFLASH(Map8_Write)(WORD wAddr, BYTE byData);

void Map9_Init();
void FASTCODE NOFLASH(Map9_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map9_PPU)(WORD wAddr);

void Map10_Init();
void FASTCODE NOFLASH(Map10_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map10_PPU)(WORD wAddr);

void Map11_Init();
void FASTCODE NOFLASH(Map11_Write)(WORD wAddr, BYTE byData);

void Map13_Init();
void FASTCODE NOFLASH(Map13_Write)(WORD wAddr, BYTE byData);

void Map15_Init();
void FASTCODE NOFLASH(Map15_Write)(WORD wAddr, BYTE byData);

void Map16_Init();
void FASTCODE NOFLASH(Map16_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map16_HSync)();

void Map17_Init();
void FASTCODE NOFLASH(Map17_Apu)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map17_HSync)();

void Map18_Init();
void FASTCODE NOFLASH(Map18_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map18_HSync)();

void Map19_Init();
void FASTCODE NOFLASH(Map19_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map19_Apu)(WORD wAddr, BYTE byData);
BYTE FASTCODE NOFLASH(Map19_ReadApu)(WORD wAddr);
void FASTCODE NOFLASH(Map19_HSync)();

void Map21_Init();
void FASTCODE NOFLASH(Map21_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map21_HSync)();

void Map22_Init();
void FASTCODE NOFLASH(Map22_Write)(WORD wAddr, BYTE byData);

void Map23_Init();
void FASTCODE NOFLASH(Map23_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map23_HSync)();

void Map24_Init();
void FASTCODE NOFLASH(Map24_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map24_HSync)();

void Map25_Init();
void FASTCODE NOFLASH(Map25_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map25_Sync_Vrom)(int nBank);
void FASTCODE NOFLASH(Map25_HSync)();

void Map26_Init();
void FASTCODE NOFLASH(Map26_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map26_HSync)();

void Map32_Init();
void FASTCODE NOFLASH(Map32_Write)(WORD wAddr, BYTE byData);

void Map33_Init();
void FASTCODE NOFLASH(Map33_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map33_HSync)();

void Map34_Init();
void FASTCODE NOFLASH(Map34_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map34_Sram)(WORD wAddr, BYTE byData);

void Map40_Init();
void FASTCODE NOFLASH(Map40_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map40_HSync)();

void Map41_Init();
void FASTCODE NOFLASH(Map41_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map41_Sram)(WORD wAddr, BYTE byData);

void Map42_Init();
void FASTCODE NOFLASH(Map42_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map42_HSync)();

void Map43_Init();
void FASTCODE NOFLASH(Map43_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map43_Apu)(WORD wAddr, BYTE byData);
BYTE FASTCODE NOFLASH(Map43_ReadApu)(WORD wAddr);
void FASTCODE NOFLASH(Map43_HSync)();

void Map44_Init();
void FASTCODE NOFLASH(Map44_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map44_HSync)();
void FASTCODE NOFLASH(Map44_Set_CPU_Banks)();
void FASTCODE NOFLASH(Map44_Set_PPU_Banks)();

void Map45_Init();
void FASTCODE NOFLASH(Map45_Sram)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map45_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map45_HSync)();
void FASTCODE NOFLASH(Map45_Set_CPU_Bank4)(BYTE byData);
void FASTCODE NOFLASH(Map45_Set_CPU_Bank5)(BYTE byData);
void FASTCODE NOFLASH(Map45_Set_CPU_Bank6)(BYTE byData);
void FASTCODE NOFLASH(Map45_Set_CPU_Bank7)(BYTE byData);
void FASTCODE NOFLASH(Map45_Set_PPU_Banks)();

void Map46_Init();
void FASTCODE NOFLASH(Map46_Sram)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map46_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map46_Set_ROM_Banks)();

void Map47_Init();
void FASTCODE NOFLASH(Map47_Sram)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map47_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map47_HSync)();
void FASTCODE NOFLASH(Map47_Set_CPU_Banks)();
void FASTCODE NOFLASH(Map47_Set_PPU_Banks)();

void Map48_Init();
void FASTCODE NOFLASH(Map48_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map48_HSync)();

void Map49_Init();
void FASTCODE NOFLASH(Map49_Sram)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map49_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map49_HSync)();
void FASTCODE NOFLASH(Map49_Set_CPU_Banks)();
void FASTCODE NOFLASH(Map49_Set_PPU_Banks)();

void Map50_Init();
void FASTCODE NOFLASH(Map50_Apu)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map50_HSync)();

void Map51_Init();
void FASTCODE NOFLASH(Map51_Sram)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map51_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map51_Set_CPU_Banks)();

void Map57_Init();
void FASTCODE NOFLASH(Map57_Write)(WORD wAddr, BYTE byData);

void Map58_Init();
void FASTCODE NOFLASH(Map58_Write)(WORD wAddr, BYTE byData);

void Map60_Init();
void FASTCODE NOFLASH(Map60_Write)(WORD wAddr, BYTE byData);

void Map61_Init();
void FASTCODE NOFLASH(Map61_Write)(WORD wAddr, BYTE byData);

void Map62_Init();
void FASTCODE NOFLASH(Map62_Write)(WORD wAddr, BYTE byData);

void Map64_Init();
void FASTCODE NOFLASH(Map64_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map64_HSync)();

void Map65_Init();
void FASTCODE NOFLASH(Map65_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map65_HSync)();

void Map66_Init();
void FASTCODE NOFLASH(Map66_Write)(WORD wAddr, BYTE byData);

void Map67_Init();
void FASTCODE NOFLASH(Map67_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map67_HSync)();

void Map68_Init();
void FASTCODE NOFLASH(Map68_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map68_SyncMirror)();

void Map69_Init();
void FASTCODE NOFLASH(Map69_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map69_HSync)();

void Map70_Init();
void FASTCODE NOFLASH(Map70_Write)(WORD wAddr, BYTE byData);

void Map71_Init();
void FASTCODE NOFLASH(Map71_Write)(WORD wAddr, BYTE byData);

void Map72_Init();
void FASTCODE NOFLASH(Map72_Write)(WORD wAddr, BYTE byData);

void Map73_Init();
void FASTCODE NOFLASH(Map73_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map73_HSync)();

void Map74_Init();
void FASTCODE NOFLASH(Map74_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map74_HSync)();
void FASTCODE NOFLASH(Map74_Set_CPU_Banks)();
void FASTCODE NOFLASH(Map74_Set_PPU_Banks)();

void Map75_Init();
void FASTCODE NOFLASH(Map75_Write)(WORD wAddr, BYTE byData);

void Map76_Init();
void FASTCODE NOFLASH(Map76_Write)(WORD wAddr, BYTE byData);

void Map77_Init();
void FASTCODE NOFLASH(Map77_Write)(WORD wAddr, BYTE byData);

void Map78_Init();
void FASTCODE NOFLASH(Map78_Write)(WORD wAddr, BYTE byData);

void Map79_Init();
void FASTCODE NOFLASH(Map79_Apu)(WORD wAddr, BYTE byData);

void Map80_Init();
void FASTCODE NOFLASH(Map80_Sram)(WORD wAddr, BYTE byData);

void Map82_Init();
void FASTCODE NOFLASH(Map82_Sram)(WORD wAddr, BYTE byData);

void Map83_Init();
void FASTCODE NOFLASH(Map83_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map83_Apu)(WORD wAddr, BYTE byData);
BYTE FASTCODE NOFLASH(Map83_ReadApu)(WORD wAddr);
void FASTCODE NOFLASH(Map83_HSync)();

void Map85_Init();
void FASTCODE NOFLASH(Map85_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map85_HSync)();

void Map86_Init();
void FASTCODE NOFLASH(Map86_Sram)(WORD wAddr, BYTE byData);

void Map87_Init();
void FASTCODE NOFLASH(Map87_Sram)(WORD wAddr, BYTE byData);

void Map88_Init();
void FASTCODE NOFLASH(Map88_Write)(WORD wAddr, BYTE byData);

void Map89_Init();
void FASTCODE NOFLASH(Map89_Write)(WORD wAddr, BYTE byData);

void Map90_Init();
void FASTCODE NOFLASH(Map90_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map90_Apu)(WORD wAddr, BYTE byData);
BYTE FASTCODE NOFLASH(Map90_ReadApu)(WORD wAddr);
void FASTCODE NOFLASH(Map90_HSync)();
void FASTCODE NOFLASH(Map90_Sync_Mirror)(void);
void FASTCODE NOFLASH(Map90_Sync_Prg_Banks)(void);
void FASTCODE NOFLASH(Map90_Sync_Chr_Banks)(void);

void Map91_Init();
void FASTCODE NOFLASH(Map91_Sram)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map91_HSync)();

void Map92_Init();
void FASTCODE NOFLASH(Map92_Write)(WORD wAddr, BYTE byData);

void Map93_Init();
void FASTCODE NOFLASH(Map93_Sram)(WORD wAddr, BYTE byData);

void Map94_Init();
void FASTCODE NOFLASH(Map94_Write)(WORD wAddr, BYTE byData);

void Map95_Init();
void FASTCODE NOFLASH(Map95_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map95_Set_CPU_Banks)();
void FASTCODE NOFLASH(Map95_Set_PPU_Banks)();

void Map96_Init();
void FASTCODE NOFLASH(Map96_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map96_PPU)(WORD wAddr);
void FASTCODE NOFLASH(Map96_Set_Banks)();

void Map97_Init();
void FASTCODE NOFLASH(Map97_Write)(WORD wAddr, BYTE byData);

void Map99_Init();
void FASTCODE NOFLASH(Map99_HSync)();

void Map100_Init();
void FASTCODE NOFLASH(Map100_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map100_HSync)();
void FASTCODE NOFLASH(Map100_Set_CPU_Banks)();
void FASTCODE NOFLASH(Map100_Set_PPU_Banks)();

void Map101_Init();
void FASTCODE NOFLASH(Map101_Write)(WORD wAddr, BYTE byData);

void Map105_Init();
void FASTCODE NOFLASH(Map105_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map105_HSync)();

void Map107_Init();
void FASTCODE NOFLASH(Map107_Write)(WORD wAddr, BYTE byData);

void Map108_Init();
void FASTCODE NOFLASH(Map108_Write)(WORD wAddr, BYTE byData);

void Map109_Init();
void FASTCODE NOFLASH(Map109_Apu)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map109_Set_PPU_Banks)();

void Map110_Init();
void FASTCODE NOFLASH(Map110_Apu)(WORD wAddr, BYTE byData);

void Map112_Init();
void FASTCODE NOFLASH(Map112_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map112_HSync)();
void FASTCODE NOFLASH(Map112_Set_CPU_Banks)();
void FASTCODE NOFLASH(Map112_Set_PPU_Banks)();

void Map113_Init();
void FASTCODE NOFLASH(Map113_Apu)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map113_Write)(WORD wAddr, BYTE byData);

void Map114_Init();
void FASTCODE NOFLASH(Map114_Sram)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map114_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map114_HSync)();
void FASTCODE NOFLASH(Map114_Set_CPU_Banks)();
void FASTCODE NOFLASH(Map114_Set_PPU_Banks)();

void Map115_Init();
void FASTCODE NOFLASH(Map115_Sram)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map115_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map115_HSync)();
void FASTCODE NOFLASH(Map115_Set_CPU_Banks)();
void FASTCODE NOFLASH(Map115_Set_PPU_Banks)();

void Map116_Init();
void FASTCODE NOFLASH(Map116_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map116_HSync)();
void FASTCODE NOFLASH(Map116_Set_CPU_Banks)();
void FASTCODE NOFLASH(Map116_Set_PPU_Banks)();

void Map117_Init();
void FASTCODE NOFLASH(Map117_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map117_HSync)();

void Map118_Init();
void FASTCODE NOFLASH(Map118_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map118_HSync)();
void FASTCODE NOFLASH(Map118_Set_CPU_Banks)();
void FASTCODE NOFLASH(Map118_Set_PPU_Banks)();

void Map119_Init();
void FASTCODE NOFLASH(Map119_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map119_HSync)();
void FASTCODE NOFLASH(Map119_Set_CPU_Banks)();
void FASTCODE NOFLASH(Map119_Set_PPU_Banks)();

void Map122_Init();
void FASTCODE NOFLASH(Map122_Sram)(WORD wAddr, BYTE byData);

void Map133_Init();
void FASTCODE NOFLASH(Map133_Apu)(WORD wAddr, BYTE byData);

void Map134_Init();
void FASTCODE NOFLASH(Map134_Apu)(WORD wAddr, BYTE byData);

void Map135_Init();
void FASTCODE NOFLASH(Map135_Apu)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map135_Set_PPU_Banks)();

void Map140_Init();
void FASTCODE NOFLASH(Map140_Sram)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map140_Apu)(WORD wAddr, BYTE byData);

void Map144_Init();
void FASTCODE NOFLASH(Map144_Write)(WORD wAddr, BYTE byData);

void Map149_Init();
void FASTCODE NOFLASH(Map149_Write)(WORD wAddr, BYTE byData);

void Map150_Init();
void FASTCODE NOFLASH(Map150_Sram)(WORD wAddr, BYTE byData);
BYTE FASTCODE NOFLASH(Map150_ReadApu)(WORD wAddr);

void Map151_Init();
void FASTCODE NOFLASH(Map151_Write)(WORD wAddr, BYTE byData);

void Map154_Init();
void FASTCODE NOFLASH(Map154_Write)(WORD wAddr, BYTE byData);

void Map156_Init();
void FASTCODE NOFLASH(Map156_Write)( WORD wAddr, BYTE byData);

void Map160_Init();
void FASTCODE NOFLASH(Map160_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map160_HSync)();

void Map180_Init();
void FASTCODE NOFLASH(Map180_Write)(WORD wAddr, BYTE byData);

void Map181_Init();
void FASTCODE NOFLASH(Map181_Apu)(WORD wAddr, BYTE byData);

void Map182_Init();
void FASTCODE NOFLASH(Map182_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map182_HSync)();

void Map183_Init();
void FASTCODE NOFLASH(Map183_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map183_HSync)();

void Map184_Init();
void FASTCODE NOFLASH(Map184_Sram)(WORD wAddr, BYTE byData);

void Map185_Init();
void FASTCODE NOFLASH(Map185_Write)(WORD wAddr, BYTE byData);

void Map187_Init();
void FASTCODE NOFLASH(Map187_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map187_Apu)(WORD wAddr, BYTE byData);
BYTE FASTCODE NOFLASH(Map187_ReadApu)(WORD wAddr);
void FASTCODE NOFLASH(Map187_HSync)();
void FASTCODE NOFLASH(Map187_Set_CPU_Banks)();
void FASTCODE NOFLASH(Map187_Set_PPU_Banks)();

void Map188_Init();
void FASTCODE NOFLASH(Map188_Write)(WORD wAddr, BYTE byData);

void Map189_Init();
void FASTCODE NOFLASH(Map189_Apu)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map189_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map189_HSync)();

void Map191_Init();
void FASTCODE NOFLASH(Map191_Apu)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map191_Set_CPU_Banks)();
void FASTCODE NOFLASH(Map191_Set_PPU_Banks)();

void Map193_Init();
void FASTCODE NOFLASH(Map193_Sram)(WORD wAddr, BYTE byData);

void Map194_Init();
void FASTCODE NOFLASH(Map194_Write)(WORD wAddr, BYTE byData);

void Map200_Init();
void FASTCODE NOFLASH(Map200_Write)(WORD wAddr, BYTE byData);

void Map201_Init();
void FASTCODE NOFLASH(Map201_Write)(WORD wAddr, BYTE byData);

void Map202_Init();
void FASTCODE NOFLASH(Map202_Apu)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map202_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map202_WriteSub)(WORD wAddr, BYTE byData);

void Map206_Init();
void FASTCODE NOFLASH(Map206_Write)(WORD wAddr, BYTE byData);

void Map207_Init();
void FASTCODE NOFLASH(Map207_Sram)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map207_PPU_hook)(u16 addr);

void Map210_Init();
void FASTCODE NOFLASH(Map210_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map210_Apu)(WORD wAddr, BYTE byData);
BYTE FASTCODE NOFLASH(Map210_ReadApu)(WORD wAddr);
void FASTCODE NOFLASH(Map210_HSync)();

void Map212_Init();
void FASTCODE NOFLASH(Map212_Write)(WORD wAddr, BYTE byData);

void Map216_Init();
BYTE FASTCODE NOFLASH(Map216_ReadApu)(WORD wAddr);
void FASTCODE NOFLASH(Map216_Write)(WORD wAddr, BYTE byData);

void Map222_Init();
void FASTCODE NOFLASH(Map222_Write)(WORD wAddr, BYTE byData);

void Map225_Init();
void FASTCODE NOFLASH(Map225_Write)(WORD wAddr, BYTE byData);

void Map226_Init();
void FASTCODE NOFLASH(Map226_Write)(WORD wAddr, BYTE byData);

void Map227_Init();
void FASTCODE NOFLASH(Map227_Write)(WORD wAddr, BYTE byData);

void Map228_Init();
void FASTCODE NOFLASH(Map228_Write)(WORD wAddr, BYTE byData);

void Map229_Init();
void FASTCODE NOFLASH(Map229_Write)(WORD wAddr, BYTE byData);

void Map230_Init();
void FASTCODE NOFLASH(Map230_Write)(WORD wAddr, BYTE byData);

void Map231_Init();
void FASTCODE NOFLASH(Map231_Write)(WORD wAddr, BYTE byData);

void Map232_Init();
void FASTCODE NOFLASH(Map232_Write)(WORD wAddr, BYTE byData);

void Map233_Init();
void FASTCODE NOFLASH(Map233_Write)(WORD wAddr, BYTE byData);

void Map234_Init();
void FASTCODE NOFLASH(Map234_Read)( WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map234_Set_Banks)();

void Map235_Init();
void FASTCODE NOFLASH(Map235_Write)(WORD wAddr, BYTE byData);

void Map236_Init();
void FASTCODE NOFLASH(Map236_Write)(WORD wAddr, BYTE byData);

void Map240_Init();
void FASTCODE NOFLASH(Map240_Apu)(WORD wAddr, BYTE byData);

void Map241_Init();
void FASTCODE NOFLASH(Map241_Write)(WORD wAddr, BYTE byData);

void Map242_Init();
void FASTCODE NOFLASH(Map242_Write)(WORD wAddr, BYTE byData);

void Map243_Init();
void FASTCODE NOFLASH(Map243_Apu)(WORD wAddr, BYTE byData);

void Map244_Init();
void FASTCODE NOFLASH(Map244_Write)(WORD wAddr, BYTE byData);

void Map245_Init();
void FASTCODE NOFLASH(Map245_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map245_HSync)();

#if 0
void FASTCODE NOFLASH(Map245_Set_CPU_Banks)();
void FASTCODE NOFLASH(Map245_Set_PPU_Banks)();
#endif

void Map246_Init();
void FASTCODE NOFLASH(Map246_Sram)(WORD wAddr, BYTE byData);

void Map248_Init();
void FASTCODE NOFLASH(Map248_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map248_Apu)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map248_Sram)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map248_HSync)();
void FASTCODE NOFLASH(Map248_Set_CPU_Banks)();
void FASTCODE NOFLASH(Map248_Set_PPU_Banks)();

void Map249_Init();
void FASTCODE NOFLASH(Map249_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map249_Apu)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map249_HSync)();

void Map251_Init();
void FASTCODE NOFLASH(Map251_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map251_Sram)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map251_Set_Banks)();

void Map252_Init();
void FASTCODE NOFLASH(Map252_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map252_HSync)();

void Map255_Init();
void FASTCODE NOFLASH(Map255_Write)(WORD wAddr, BYTE byData);
void FASTCODE NOFLASH(Map255_Apu)(WORD wAddr, BYTE byData);
BYTE FASTCODE NOFLASH(Map255_ReadApu)(WORD wAddr);

#endif /* !InfoNES_MAPPER_H_INCLUDED */
