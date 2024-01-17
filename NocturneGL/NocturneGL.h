#pragma once

/*Pixel Data*/
typedef struct {
	short r, g, b, a;
	int z;
} NtPixel;

/*Constants*/
#define RED     0               /* array indicies for color vector */
#define GREEN   1
#define BLUE    2

#define X       0               /* array indicies for position vector */
#define Y       1
#define Z       2

#define NT_SUCCESS      0
#define NT_FAILURE      1

/*Rendering*/
typedef struct {
	unsigned short	xRes;
	unsigned short	yRes;
	short			open;
	NtPixel* frameBuffer;		/* frame buffer array */
} NtDisplay;

/*Core Functions*/
int NtNewFrameBuffer(NtPixel** frameBuffer, int width, int height);
int NtNewDisplay(NtDisplay** display, int xRes, int yRes);
int NtFreeDisplay(NtDisplay* display);
int NtInitDisplay(NtDisplay* display);
