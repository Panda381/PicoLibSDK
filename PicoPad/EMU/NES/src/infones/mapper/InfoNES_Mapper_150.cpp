/*===================================================================*/
/*                                                                   */
/*                             Mapper 150                            */
/*                                                                   */
/*===================================================================*/

// Mapper data (max. size MAPDATA_SIZE)
typedef struct {
	BYTE	md_Map150_Latch[5];	// 0x00
	BYTE	md_Map150_Cmd;		// 0x05
	u8	md_res[2];		// 0x06
} sMap150_Data;

STATIC_ASSERT((sizeof(sMap150_Data) == 0x08) && (sizeof(sMap150_Data) <= MAPDATA_SIZE), "Incorrect sMap150_Data!");

#define MAPDATA150		((sMap150_Data*)NES->mapdata)	// mapper data

#define Map150_Latch		MAPDATA150->md_Map150_Latch
#define Map150_Cmd		MAPDATA150->md_Map150_Cmd

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 150                                            */
/*-------------------------------------------------------------------*/
void Map150_Init()
{
	/* Initialize Mapper */
	MapperInit = Map150_Init;

	/* Write to Mapper */
	MapperWrite = Map0_Write;

	/* Write to SRAM */
	MapperSram = Map150_Sram;

	/* Write to APU */
	MapperApu = Map0_Apu;

	/* Read from APU */
	MapperReadApu = Map150_ReadApu;

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
	if (NesHeader.byRomSize > 1)
	{
		ROMBANK0 = ROMPAGE(0);
		ROMBANK1 = ROMPAGE(1);
		ROMBANK2 = ROMPAGE(2);
		ROMBANK3 = ROMPAGE(3);
	}
	else if (NesHeader.byRomSize > 0)
	{
		ROMBANK0 = ROMPAGE(0);
		ROMBANK1 = ROMPAGE(1);
		ROMBANK2 = ROMPAGE(0);
		ROMBANK3 = ROMPAGE(1);
	}
	else
	{
		ROMBANK0 = ROMPAGE(0);
		ROMBANK1 = ROMPAGE(0);
		ROMBANK2 = ROMPAGE(0);
		ROMBANK3 = ROMPAGE(0);
	}

	/* Set PPU Banks */
	if (NesHeader.byVRomSize > 0)
	{
		for (int nPage = 0; nPage < 8; ++nPage)
			PPUBANK[nPage] = VROMPAGE(nPage);
		InfoNES_SetupChr();
	}

	Map150_Latch[0] = 0;
	Map150_Latch[1] = 0;
	Map150_Latch[2] = 0;
	Map150_Latch[3] = 0;
	Map150_Latch[4] = 0;
	Map150_Cmd = 0;

	/* Set up wiring of the interrupt pin */
	K6502_Set_Int_Wiring(1, 1);
}

/*-------------------------------------------------------------------*/
/*  Mapper 150 Write to Sram Function                                */
/*-------------------------------------------------------------------*/
void FASTCODE NOFLASH(Map150_Sram)(WORD wAddr, BYTE byData)
{
	if (wAddr < 0x4100) return;

	if ((wAddr & 0x4101) == 0x4100)
	{
		Map150_Cmd = byData & 7;
	}
	else
	{
		switch (Map150_Cmd)
		{
		case 2:
			Map150_Latch[0] = byData & 1;
			Map150_Latch[3] = (byData & 1) << 3;
			break;

		case 4:
			Map150_Latch[4] = (byData & 1) << 2;
			break;

		case 5:
			Map150_Latch[0] = byData & 7;
			break;

		case 6:
			Map150_Latch[1] = byData & 3;
			break;

		case 7:
			Map150_Latch[2] = byData >> 1;
			break;
		}

		u8 m = (NesHeader.byRomSize << 1) - 1;
		u8 d = Map150_Latch[0] << 2;
		ROMBANK0 = ROMPAGE((d + 0) & m);
		ROMBANK1 = ROMPAGE((d + 1) & m);
		ROMBANK2 = ROMPAGE((d + 2) & m);
		ROMBANK3 = ROMPAGE((d + 3) & m);

		m = (NesHeader.byVRomSize << 3) - 1;
		d = Map150_Latch[1] | Map150_Latch[3] | Map150_Latch[4];
		d <<= 3;
		PPUBANK[0] = VROMPAGE((d + 0) & m);
		PPUBANK[1] = VROMPAGE((d + 1) & m);
		PPUBANK[2] = VROMPAGE((d + 2) & m);
		PPUBANK[3] = VROMPAGE((d + 3) & m);
		PPUBANK[4] = VROMPAGE((d + 4) & m);
		PPUBANK[5] = VROMPAGE((d + 5) & m);
		PPUBANK[6] = VROMPAGE((d + 6) & m);
		PPUBANK[7] = VROMPAGE((d + 7) & m);
		InfoNES_SetupChr();

		switch (Map150_Latch[2] & 3)
		{
		case 0:
			InfoNES_Mirroring(MIRROR_V);
			break;

		case 1:
			InfoNES_Mirroring(MIRROR_H);
			break;

		case 2:
			InfoNES_Mirroring(MIRROR_0111);
			break;

		case 3:
			InfoNES_Mirroring(MIRROR_0);
			break;
		}
	}
}

/*-------------------------------------------------------------------*/
/*  Mapper 150 Read from APU Function                                */
/*-------------------------------------------------------------------*/
BYTE FASTCODE NOFLASH(Map150_ReadApu)(WORD wAddr)
{
	if ((wAddr & 0x4100) == 0x4100)
		return ((~Map150_Cmd) & 0x3f) ^ 1;
	else
		return NES->reg_x;
}
