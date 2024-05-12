
// ****************************************************************************
//
//                                 Enemy bullets
//
// ****************************************************************************

#define BULLET_SPEED	5		// bullet speed

// enemy bullet template
typedef struct {
	const u16*	pal;		// palettes
	const u8*	img;		// image
	int		w;		// image width
	int		wall;		// total width
	int		h;		// image height
	COLTYPE		trans;		// transparent color
} sBulletTemp;

// enemy bullet templates
#define BULLET_TEMP_NUM	3
extern const sBulletTemp BulletTemp[BULLET_TEMP_NUM];

// enemy bullet descriptor
typedef struct {
	const sBulletTemp*	temp;	// bullet template (NULL = not used)
	int		x, y;		// bullet coordinate
} sBullet;

// enemy bullet list
#define BULLET_MAX	30	// max. bullets
extern sBullet Bullet[BULLET_MAX];

// format: 4-bit paletted pixel graphics
// image width: 10 pixels
// image height: 10 lines
// image pitch: 5 bytes
extern const u16 Bullet1Img_Pal[7] __attribute__ ((aligned(4)));
extern const u8 Bullet1Img[50] __attribute__ ((aligned(4)));

extern const u16 Bullet2Img_Pal[7] __attribute__ ((aligned(4)));
extern const u8 Bullet2Img[50] __attribute__ ((aligned(4)));

extern const u16 Bullet3Img_Pal[7] __attribute__ ((aligned(4)));
extern const u8 Bullet3Img[50] __attribute__ ((aligned(4)));

#define BULLET_W	10		// bullet width
#define BULLET_WALL	10		// bullet total width
#define BULLET_H	10		// bullet height
#define BULLET_TRANS	COL_MAGENTA	// bullet transparent color

// initialize enemy bullets on new level
void InitBullet();

// add new bullet
void AddBullet(const sBulletTemp* temp, int x, int y);

// shift bullets
void BulletShift();

// display bullets
void BulletDisp();
