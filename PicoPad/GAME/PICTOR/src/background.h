
// ****************************************************************************
//
//                               Background
//
// ****************************************************************************

extern int BackInx;		// index of background
extern int BackPhase1, BackPhase2, BackPhase3; // background phases

// background template descriptors
typedef struct {
	const char*	name;		// background name
	const u16*	pal1;		// palettes of layer 1
	const u8*	img1;		// image of layer 1
	const u16*	pal2;		// palettes of layer 2
	const u8*	img2;		// image of layer 2
	int		dy;		// Y correction of layer 2
	const u16*	pal3;		// palettes of layer 3
	const u8*	img3;		// image of layer 3
} sBackTemp;

extern const sBackTemp	BackTemp[BG_NUM]; // background templates
extern sBackTemp	Back;		// current background template

// background speed
#define BG_SPEED1	1		// layer 1 speed
#define BG_SPEED2	2		// layer 2 speed
#define BG_SPEED3	3		// layer 3 speed

// === Backgrounds, layers 1

#define BG_Y_MIN	TIT_HEIGHT	// background minimal Y coordinate

// background layer 1 (Y = 20..179)
#define BG_Y		BG_Y_MIN	// background Y coordinate (= 20)
#define BG_WIDTH	640		// width of background image
#define BG_HEIGHT	160		// height of background image

// format: 8-bit paletted pixel graphics
// image width: 320 pixels
// image height: 80 lines
// image pitch: 320 bytes
extern const u16 MeadowImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 MeadowImg[25600] __attribute__ ((aligned(4)));

extern const u16 JungleImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 JungleImg[25600] __attribute__ ((aligned(4)));

extern const u16 SandbeachImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 SandbeachImg[25600] __attribute__ ((aligned(4)));

extern const u16 UnderwaterImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 UnderwaterImg[25600] __attribute__ ((aligned(4)));

extern const u16 IcelandImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 IcelandImg[25600] __attribute__ ((aligned(4)));

extern const u16 FirelandImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 FirelandImg[25600] __attribute__ ((aligned(4)));

extern const u16 HauntedImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 HauntedImg[25600] __attribute__ ((aligned(4)));

extern const u16 SpacecraftImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 SpacecraftImg[25600] __attribute__ ((aligned(4)));

extern const u16 GalaxyImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 GalaxyImg[25600] __attribute__ ((aligned(4)));

extern const u16 PlanetImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 PlanetImg[25600] __attribute__ ((aligned(4)));

extern const u16 CandylandImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 CandylandImg[25600] __attribute__ ((aligned(4)));

extern const u16 SurrealImg_Pal[256] __attribute__ ((aligned(4)));
extern const u8 SurrealImg[25600] __attribute__ ((aligned(4)));

// - Overlap layer 1 and layer 2 by 70 lines
// - Layer 2 must add 10 lines

// === Backgrounds, layers 2

// background layer 2 (Y = 150..189)
#define BG2_Y		(BG_Y+BG_HEIGHT-70) // background Y coordinate (= 110)
#define BG2_WIDTH	640		// width of background image
#define BG2_HEIGHT	80		// max. height of background image
#define BG2_TRANST	COL_WHITE	// transparent color

// format: 8-bit paletted pixel graphics
// image width: 640 pixels
// image height: 64 lines
// image pitch: 640 bytes
#define BG2_DY_MEADOW (BG2_HEIGHT - 64)	// Y correction
extern const u16 Meadow2Img_Pal[256] __attribute__ ((aligned(4)));
extern const u8 Meadow2Img[40960] __attribute__ ((aligned(4)));

// format: 8-bit paletted pixel graphics
// image width: 640 pixels
// image height: 38 lines
// image pitch: 640 bytes
#define BG2_DY_JUNGLE (BG2_HEIGHT - 38)	// Y correction
extern const u16 Jungle2Img_Pal[256] __attribute__ ((aligned(4)));
extern const u8 Jungle2Img[24320] __attribute__ ((aligned(4)));

// format: 8-bit paletted pixel graphics
// image width: 640 pixels
// image height: 40 lines
// image pitch: 640 bytes
#define BG2_DY_SANDBEACH (BG2_HEIGHT - 40) // Y correction
extern const u16 Sandbeach2Img_Pal[256] __attribute__ ((aligned(4)));
extern const u8 Sandbeach2Img[25600] __attribute__ ((aligned(4)));

// format: 8-bit paletted pixel graphics
// image width: 640 pixels
// image height: 80 lines
// image pitch: 640 bytes
#define BG2_DY_UNDERWATER (BG2_HEIGHT-80)  // Y correction
extern const u16 Underwater2Img_Pal[256] __attribute__ ((aligned(4)));
extern const u8 Underwater2Img[51200] __attribute__ ((aligned(4)));

// format: 8-bit paletted pixel graphics
// image width: 640 pixels
// image height: 38 lines
// image pitch: 640 bytes
#define BG2_DY_ICELAND (BG2_HEIGHT-38)  // Y correction
extern const u16 Iceland2Img_Pal[256] __attribute__ ((aligned(4)));
extern const u8 Iceland2Img[24320] __attribute__ ((aligned(4)));

// format: 8-bit paletted pixel graphics
// image width: 640 pixels
// image height: 48 lines
// image pitch: 640 bytes
#define BG2_DY_FIRELAND (BG2_HEIGHT - 48) // Y correction
extern const u16 Fireland2Img_Pal[256] __attribute__ ((aligned(4)));
extern const u8 Fireland2Img[30720] __attribute__ ((aligned(4)));

// format: 8-bit paletted pixel graphics
// image width: 640 pixels
// image height: 64 lines
// image pitch: 640 bytes
#define BG2_DY_HAUNTED (BG2_HEIGHT - 64) // Y correction
extern const u16 Haunted2Img_Pal[256] __attribute__ ((aligned(4)));
extern const u8 Haunted2Img[40960] __attribute__ ((aligned(4)));

// format: 8-bit paletted pixel graphics
// image width: 640 pixels
// image height: 56 lines
// image pitch: 640 bytes
#define BG2_DY_SPACECRAFT (BG2_HEIGHT - 56) // Y correction
extern const u16 Spacecraft2Img_Pal[256] __attribute__ ((aligned(4)));
extern const u8 Spacecraft2Img[35840] __attribute__ ((aligned(4)));

// format: 8-bit paletted pixel graphics
// image width: 640 pixels
// image height: 66 lines
// image pitch: 640 bytes
#define BG2_DY_GALAXY (BG2_HEIGHT - 66) // Y correction
extern const u16 Galaxy2Img_Pal[256] __attribute__ ((aligned(4)));
extern const u8 Galaxy2Img[42240] __attribute__ ((aligned(4)));

// format: 8-bit paletted pixel graphics
// image width: 640 pixels
// image height: 47 lines
// image pitch: 640 bytes
#define BG2_DY_PLANET (BG2_HEIGHT - 47) // Y correction
extern const u16 Planet2Img_Pal[256] __attribute__ ((aligned(4)));
extern const u8 Planet2Img[30080] __attribute__ ((aligned(4)));

// format: 8-bit paletted pixel graphics
// image width: 640 pixels
// image height: 35 lines
// image pitch: 640 bytes
#define BG2_DY_CANDYLAND (BG2_HEIGHT - 35) // Y correction
extern const u16 Candyland2Img_Pal[256] __attribute__ ((aligned(4)));
extern const u8 Candyland2Img[22400] __attribute__ ((aligned(4)));

// format: 8-bit paletted pixel graphics
// image width: 640 pixels
// image height: 58 lines
// image pitch: 640 bytes
#define BG2_DY_SURREAL (BG2_HEIGHT - 58) // Y correction
extern const u16 Surreal2Img_Pal[249] __attribute__ ((aligned(4)));
extern const u8 Surreal2Img[37120] __attribute__ ((aligned(4)));

// - Overlap layer 2 and layer 3 by 10 lines
// - Layer 3 must add 30 lines

// === Backgrounds, layers 3

// background layer 3 (Y = 180..219)
#define BG3_Y		(BG2_Y+BG2_HEIGHT-10) // background Y coordinate (= 180)
#define BG3_WIDTH	640		// width of background image
#define BG3_HEIGHT	40		// height of background image
#define BG3_TRANST	COL_WHITE	// transparent color

// format: 8-bit paletted pixel graphics
// image width: 640 pixels
// image height: 40 lines
// image pitch: 640 bytes
extern const u8 Meadow3Img[25600] __attribute__ ((aligned(4)));
extern const u8 Spacecraft3Img[25600] __attribute__ ((aligned(4)));

extern const u16 Meadow3Img_Pal[256] __attribute__ ((aligned(4)));
extern const u16 Jungle3Img_Pal[256] __attribute__ ((aligned(4)));
extern const u16 Underwater3Img_Pal[256] __attribute__ ((aligned(4)));
extern const u16 Iceland3Img_Pal[256] __attribute__ ((aligned(4)));
extern const u16 Haunted3Img_Pal[256] __attribute__ ((aligned(4)));
extern const u16 Galaxy3Img_Pal[256] __attribute__ ((aligned(4)));
extern const u16 Fireland3Img_Pal[256] __attribute__ ((aligned(4)));
extern const u16 Candyland3Img_Pal[256] __attribute__ ((aligned(4)));
extern const u16 Spacecraft3Img_Pal[256] __attribute__ ((aligned(4)));
extern const u16 Sandbeach3Img_Pal[256] __attribute__ ((aligned(4)));
extern const u16 Planet3Img_Pal[256] __attribute__ ((aligned(4)));
extern const u16 Surreal3Img_Pal[256] __attribute__ ((aligned(4)));

// initialize backgrounds on new level
void InitBack();

// display backgrounds
void DispBack();

// shift backgrounds
void ShiftBack();
