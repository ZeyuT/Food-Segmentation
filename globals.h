#include "stdafx.h"
#define SQR(x) ((x)*(x))	/* macro for square */
#ifndef M_PI			/* in case M_PI not found in math.h */
#define M_PI 3.1415927
#endif
#ifndef M_E
#define M_E 2.718282
#endif

#define MAX_FILENAME_CHARS	320

/* desired height and width of resized images*/
#ifndef DES_ROWS
#define DES_ROWS	480
#endif
#ifndef DES_COLS
#define DES_COLS	600
#endif

char	filename[MAX_FILENAME_CHARS];

HWND	MainWnd;

		// Display flags
int		ShowPixelCoords;
int		PlayMode;
int		StepMode;
int		Green, Blue, Red;
int		grow_flag;

		// Color of Growing Region
COLORREF		RegionColor;	/* Default color is blue */

		// Image data
unsigned char	*OriginalImage,*GrayImage,*ColorImage;
unsigned char	*BlankImage;
int				ROWS;		/* actual height and width of resized images*/
int				COLS;
int				rows, cols;		/*height and width of original images*/
int				PRE_ROWS;	/*height and width of the previous images*/
int				PRE_COLS; 

struct pixel {
	unsigned char R;
	unsigned char G;
	unsigned char B;
};
struct pixel *currentPixel;

#ifndef TIMER_SECOND
#define TIMER_SECOND	1			/* ID of timer used for animation */
#endif

		// Drawing flags
int		TimerRow,TimerCol;
int		ThreadRow,ThreadCol;
int		ThreadRunning, RegionGrow_ThreadRunning;
int		r, c, step_flag;

		// Function prototypes
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);

void PaintImage(unsigned char*, int, int);
void AnimationThread(HWND);		/* passes address of window */
void RegionGrow(HWND);
void bilinear_interpolation(unsigned char*, int, int,
	unsigned char*, int, int);
int is_in_input(int, int, int, int);
void PaintDot(HDC, int, int, COLORREF);
void ActiveContour(HWND);

#ifndef MAX_QUEUE
#define MAX_QUEUE 10000	/* max perimeter size (pixels) of border wavefront */
#endif

int Intensity_difference;
int Distance_to_centroid; /* Predicates */
int  prev_Intensity_difference;
int  prev_Distance_to_centroid;
int  *indices;
unsigned char	*labels;

/* Used for active contour algortithm*/
int contour_flag, contour_finish_flag, balloon_flag, PointNum;
int *Xcoordinates, *Ycoordinates;
int left_click_flag, shift_flag;
int radius;
int grab_idx;
int iterations;
int ThreadRunning_AC;