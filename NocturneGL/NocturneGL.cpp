#include <iostream>
#include "NocturneGL.h"
#include <cmath>
#include <limits>
class NTMath {
public:
	//Barycentric Coordinates
	static float f01(float x, float y, Vector3 v0, Vector3 v1) {
		return (v0.y - v1.y) * x + (v1.x - v0.x) * y + v0.x * v1.y - v1.x * v0.y;
	}

	static float f12(float x, float y, Vector3 v1, Vector3 v2) {
		return (v1.y - v2.y) * x + (v2.x - v1.x) * y + v1.x * v2.y - v2.x * v1.y;
	}

	static float f20(float x, float y, Vector3 v2, Vector3 v0) {
		return (v2.y - v0.y) * x + (v0.x - v2.x) * y + v2.x * v0.y - v0.x * v2.y;
	}

	static short fts(float color)		/* convert float color to short */
	{
		return(short)((int)(color * ((1 << 12) - 1)));
	}
};
/// <summary>
/// Creates a frame buffer and allocates memory of size NtPixel x width x height and passes back pointer
/// </summary>
/// <param name="framebuffer"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <returns></returns>
int NtNewFrameBuffer(NtPixel** frameBuffer, int width, int height)
{

	if (width <= 0 || height <= 0) return NT_FAILURE;
	size_t pixelSize = sizeof(NtPixel);

	// Allocate memory for the framebuffer as a row-major 2D array
	*frameBuffer = new NtPixel[width * height * pixelSize];
	return NT_SUCCESS;
}

/// <summary>
/// Allocates memory for display with resolution, passes back pointer to NtDisplay.
/// Initializes a new frame buffer and inits display by drawing black
/// </summary>
/// <param name="display"></param>
/// <param name="dispClass"></param>
/// <param name="xRes"></param>
/// <param name="yRes"></param>
/// <returns></returns>
int NtNewDisplay(NtDisplay** display, int xRes, int yRes)
{
	*display = new NtDisplay();
	(*display)->xRes = xRes;
	(*display)->yRes = yRes;
	int status = 0;

	status |= NtNewFrameBuffer(&((*display)->frameBuffer), xRes, yRes);
	status |= NtInitDisplay(*display);

	return status ? NT_FAILURE : NT_SUCCESS;
}

/// <summary>
/// Frees display memory and clean up
/// </summary>
/// <param name="display"></param>
/// <returns></returns>
int NtFreeDisplay(NtDisplay* display)
{
	delete display;
	/* clean up, free memory */
	return NT_SUCCESS;
}

/// <summary>
/// Initializes an empty display with all pixels set to black
/// </summary>
/// <param name="display"></param>
/// <returns></returns>
int NtInitDisplay(NtDisplay* display)
{
	if (display == nullptr || display->frameBuffer == nullptr) return NT_FAILURE;

	//Todo: we could cache buffer size
	int bufferSize = display->xRes * display->yRes * sizeof(NtPixel);

	for (int i = 0; i < bufferSize; i++) {
		display->frameBuffer[i].r = 0;
		display->frameBuffer[i].g = 0;
		display->frameBuffer[i].b = 0;
		display->frameBuffer[i].a = 255;
	}

	return NT_SUCCESS;
}

/// <summary>
/// Write a pixel into the display buffer
/// </summary>
/// <param name="display"></param>
/// <param name="i"></param>
/// <param name="j"></param>
/// <param name="r"></param>
/// <param name="g"></param>
/// <param name="b"></param>
/// <param name="a"></param>
/// <param name="z"></param>
/// <returns></returns>
int NtPutDisplay(NtDisplay* display, int i, int j, short r, short g, short b, short a)
{
	if (display == nullptr || display->frameBuffer == nullptr) return NT_FAILURE;
	int index = ClipInt(i, 0, display->xRes) + ClipInt(j, 0, display->yRes) * display->xRes;
	display->frameBuffer[index].r = r;
	display->frameBuffer[index].g = g;
	display->frameBuffer[index].b = b;
	display->frameBuffer[index].a = a;
	return NT_SUCCESS;
}

/// <summary>
/// Placeholder clip function as negative rect position isn't supported yet
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
int ClipInt(int input, int min, int max) {
	if (input < min) return min;
	if (input > max) return max;
	return input;
}

float Clipf(float input, int min, int max) {
	if (input < min) return min;
	if (input > max) return max;
	return input;
}
/// <summary>
/// Flushes display buffer to a ppm file
/// </summary>
/// <param name="outfile"></param>
/// <param name="display"></param>
/// <returns></returns>
int NtFlushDisplayBufferPPM(FILE* outfile, NtDisplay* display)
{

	/* write pixels to ppm file based on display class -- "P6 %d %d 255\r" */
	if (display == nullptr || display->frameBuffer == nullptr) return NT_FAILURE;

	// Write the PPM header
	fprintf(outfile, "P3\n%d %d\n%d\n", display->xRes, display->yRes, 5333);

	for (int y = 0; y < display->yRes; y++) {
		for (int x = 0; x < display->xRes; x++) {
			// Accessing the pixel at (x, y)
			NtPixel pixel = display->frameBuffer[y * display->xRes + x];

			// Write the RGB values to the file
			fprintf(outfile, "%d %d %d ", pixel.r, pixel.g, pixel.b);
		}
		fprintf(outfile, "\n"); // Newline after each row of pixels
	}
	return NT_SUCCESS;
}

/*Renderer*/

/// <summary>
/// Creates a new render, display must be already initialized here.
/// We grab params from display (ie. xRes, yRes)
/// </summary>
/// <param name="render"></param>
/// <param name="display"></param>
/// <returns></returns>
int NtNewRender(NtRender** render, NtDisplay* display) {
	if (display == nullptr) return NT_FAILURE;
	*render = new NtRender();
	(*render)->display = display;
	(*render)->zBuffer = new float* [display->xRes];
	if (!(*render)->zBuffer) {
		delete render;
		return NT_FAILURE;
	}

	for (int i = 0; i < display->xRes; i++) {
		(*render)->zBuffer[i] = new float[display->yRes];

		// Initialize Z-buffer to infinity
		for (int j = 0; j < display->yRes; j++) {
			(*render)->zBuffer[i][j] = INFINITY;
		}
	}

	return NT_SUCCESS;
}

/// <summary>
/// Deallocates memory
/// </summary>
/// <param name="render"></param>
/// <returns></returns>
int NtFreeRender(NtRender* render) {
	//Free zbuffer
	for (int i = 0; i < render->display->xRes; i++) {
		delete[] render->zBuffer[i];
	}
	delete[] render->zBuffer;
	delete render;
	return NT_SUCCESS;
}

/// <summary>
/// Process a single triangle with z-buffer
/// </summary>
/// <param name="render"></param>
/// <param name="vertexList"></param>
/// <param name="normalList"></param>
/// <param name="color"></param>
/// <returns></returns>
int NtPutTriangle(NtRender* render, Vector3 vertexList[], Vector3 normalList[], Vector3 color) {
	if (render == nullptr) return NT_FAILURE;

	//Clip x,y,z to display bounds
	float xRes = render->display->xRes;
	float yRes = render->display->yRes;

	for (int i = 0; i < 3; i++) {
		vertexList[i].x = Clipf(vertexList[i].x, 0, xRes);
		vertexList[i].y = Clipf(vertexList[i].y, 0, yRes);
	}

	//Calculate bounding box
	float xMinf = vertexList[0].x, xMaxf = vertexList[0].x, yMinf = vertexList[0].y, yMaxf = vertexList[0].y;
	for (int i = 0; i < 3; i++) {
		float currX = vertexList[i].x;
		float currY = vertexList[i].y;
		xMinf = std::fminf(xMinf, currX);
		xMaxf = std::fmaxf(xMaxf, currX);
		yMinf = std::fminf(yMinf, currY);
		yMaxf = std::fmaxf(yMaxf, currY);
	}
	int xMin = std::floor(xMinf);
	int yMin = std::floor(yMinf);
	int xMax = std::ceil(xMaxf);
	int yMax = std::ceil(yMaxf);
	for (int y = yMin; y <= yMax; y++) {
		for (int x = xMin; x <= xMax; x++) {
			Vector3 v0 = vertexList[0];
			Vector3 v1 = vertexList[1];
			Vector3 v2 = vertexList[2];
			float alpha = NTMath::f12(x, y, v1, v2) / NTMath::f12(v0.x, v0.y, v1, v2);
			float beta = NTMath::f20(x, y, v2, v0) / NTMath::f20(v1.x, v1.y, v2, v0);
			float gamma = NTMath::f01(x, y, v0, v1) / NTMath::f01(v2.x, v2.y, v0, v1);

			if ((alpha >= 0) && (beta >= 0) && (gamma >= 0)) {
				//Z-Buffer to determine if current pixel should be put
				//Interpolate z from alpha beta gamma
				float currZ = alpha * v0.z + beta * v1.z + gamma * v2.z;
				if (currZ < render->zBuffer[x][y]) {
					// Update the Z-buffer
					render->zBuffer[x][y] = currZ;
					NtPutDisplay(render->display, x, y, NTMath::fts(color.x), NTMath::fts(color.y), NTMath::fts(color.z), 255);
				}
			}
		}
	}
}


