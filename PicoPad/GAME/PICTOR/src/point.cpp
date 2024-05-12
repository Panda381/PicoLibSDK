
// ****************************************************************************
//
//                                  Points
//
// ****************************************************************************

#include "../include.h"

// points
sPoint Point[POINT_NUM];

// initialize points on start of level
void InitPoint()
{
	int i;
	for (i = 0; i < POINT_NUM; i++) Point[i].len = 0;
}

// add points
void AddPoint(int val, int x, int y)
{
	int i;
	sPoint* p = Point;
	for (i = 0; i < POINT_NUM; i++)
	{
		if (p->len == 0)
		{
			p->len = DecNum(p->buf, val, 0);
			p->x = x;
			p->y = y;
			p->frames = POINT_FRAMES;
			break;
		}
		p++;
	}
}

// shift points
void PointShift()
{
	int i;
	sPoint* p = Point;
	for (i = 0; i < POINT_NUM; i++)
	{
		if (p->len != 0)
		{
			p->y -= POINT_SPEEDY;
			p->frames--;
			if (p->frames <= 0) p->len = 0;
		}
		p++;
	}
}

// display points
void PointDisp()
{
	int i;
	sPoint* p = Point;
	for (i = 0; i < POINT_NUM; i++)
	{
		if (p->len != 0)
		{
			DrawText(p->buf, p->x - p->len*4, p->y - 8, COL_AZURE);
		}
		p++;
	}
}
