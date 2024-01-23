#pragma once
#include<iostream>
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
	float* zBuffer;
} NtDisplay;

/*Core Functions*/
int NtNewFrameBuffer(NtPixel** frameBuffer, int width, int height);
int NtNewDisplay(NtDisplay** display, int xRes, int yRes);
int NtFreeDisplay(NtDisplay* display);
int NtInitDisplay(NtDisplay* display);
int NtFlushDisplayBufferPPM(FILE* outfile, NtDisplay* display);
int NtPutDisplay(NtDisplay* display, int i, int j, short r, short g, short b, short a, short z);
int ClipInt(int input);

/*Renderer*/
typedef struct {
	NtDisplay* display;
	//Todo
}  NtRender;

typedef struct {
	int x, y, z;
} Vertex3;

typedef struct {
	float x, y, z;
} Vertex3f;

typedef struct {
	int x, y;
} Vertex2;

typedef struct {
	float x, y;
} Vertex2f;

int NtNewRender(NtRender** render, NtDisplay* display);
int NtFreeRender(NtRender* render);
int NtBeginRender(NtRender* render);

int NtPutTriangle(NtRender* render, Vertex3f* vertices, Vertex3f* normals);
