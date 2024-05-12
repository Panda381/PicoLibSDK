
// ****************************************************************************
//
//                                  Points
//
// ****************************************************************************

#define POINT_MAXLEN	8	// max. size of point buffer
#define POINT_NUM	20	// number of points
#define POINT_SPEEDY	2	// speed in Y direction
#define POINT_FRAMES	6	// frames to live

// points descriptor
typedef struct {
	char	buf[POINT_MAXLEN];	// point text buffer
	int	len;			// length of point text (0 = entry is not used)
	int	x, y;			// coordinates
	int	frames;			// frame counter
} sPoint;

// points
extern sPoint Point[POINT_NUM];

// initialize points on start of level
void InitPoint();

// add points
void AddPoint(int val, int x, int y);

// shift points
void PointShift();

// display points
void PointDisp();
