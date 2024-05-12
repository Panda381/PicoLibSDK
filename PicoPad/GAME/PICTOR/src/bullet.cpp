
// ****************************************************************************
//
//                                 Enemy bullets
//
// ****************************************************************************

#include "../include.h"

// enemy bullet templates
const sBulletTemp BulletTemp[BULLET_TEMP_NUM] = {
	// pal			img		w		wall		h		trans
	{ Bullet1Img_Pal,	Bullet1Img,	BULLET_W,	BULLET_WALL,	BULLET_H,	BULLET_TRANS,	},
	{ Bullet2Img_Pal,	Bullet2Img,	BULLET_W,	BULLET_WALL,	BULLET_H,	BULLET_TRANS,	},
	{ Bullet3Img_Pal,	Bullet3Img,	BULLET_W,	BULLET_WALL,	BULLET_H,	BULLET_TRANS,	},
};

// enemy bullet list
sBullet Bullet[BULLET_MAX];

// initialize enemy bullets on new level
void InitBullet()
{
	// clear bullets
	int i;
	for (i = 0; i < BULLET_MAX; i++) Bullet[i].temp = NULL;
}

// add new bullet
void AddBullet(const sBulletTemp* temp, int x, int y)
{
	int i;
	sBullet* b = Bullet;
	for (i = 0; i < BULLET_MAX; i++)
	{
		if (b->temp == NULL)
		{
			b->temp = temp;
			b->x = x;
			b->y = y;
			break;
		}
		b++;
	}
}

// shift bullets
void BulletShift()
{
	int i, a;
	const sBulletTemp* temp;
	sBullet* b = Bullet;
	for (i = 0; i < BULLET_MAX; i++)
	{
		temp = b->temp;
		if (temp != NULL)
		{
			// shift bullet
			b->x -= BULLET_SPEED;

			// delete bullet
			if (b->x + temp->w/2 <= 0) b->temp = NULL;
		}
		b++;
	}
}

// display bullets
void BulletDisp()
{
	int i;
	const sBulletTemp* temp;
	sBullet* b = Bullet;
	for (i = 0; i < BULLET_MAX; i++)
	{
		temp = b->temp;
		if (temp != NULL)
		{
			DrawBlit4Pal(temp->img, temp->pal,		// image data and palettes
				0, 0,					// source X, Y
				b->x - temp->w/2, b->y - temp->h/2, // destination X, Y
				temp->w, temp->h,		// width, height
				temp->wall, temp->trans);	// source total width
		}
		b++;
	}
}
