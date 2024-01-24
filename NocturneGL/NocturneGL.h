#pragma once
#include<iostream>
/*Pixel Data*/
typedef struct {
	short r, g, b, a;
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
int NtFlushDisplayBufferPPM(FILE* outfile, NtDisplay* display);
int NtPutDisplay(NtDisplay* display, int i, int j, short r, short g, short b, short a);
int ClipInt(int input, int min, int max);
float Clipf(float input, int min, int max);

/*Renderer*/
typedef struct {
	NtDisplay* display;
	float** zBuffer;
}  NtRender;

struct Vector3 {
	union {
		struct {
			float x, y, z;

		};
		float v[3];
	};

	Vector3() : x(0.0f), y(0.0f), z(0.0f) {}

	// Constructor for initializing Vector3 from individual x, y, and z values
	Vector3(float xVal, float yVal, float zVal) : x(xVal), y(yVal), z(zVal) {}

	// Constructor for initializing Vector3 from an array of 3 values
	Vector3(const float values[3]) : x(values[0]), y(values[1]), z(values[2]) {}
};

struct Vector2 {
	union {
		struct {
			float x, y;
		};
		float v[2];
	};
	//Default constructor
	Vector2() : x(0.0f), y(0.0f) {}

	// Constructor for initializing Vector2 from individual x and y values
	Vector2(float xVal, float yVal) : x(xVal), y(yVal) {}

	// Constructor for initializing Vector2 from an array of 2 values
	Vector2(const float values[2]) : x(values[0]), y(values[1]) {}
};

int NtNewRender(NtRender** render, NtDisplay* display);
int NtFreeRender(NtRender* render);
int NtPutTriangle(NtRender* render, Vector3 vertexList[], Vector3 normalList[], Vector3 color);
