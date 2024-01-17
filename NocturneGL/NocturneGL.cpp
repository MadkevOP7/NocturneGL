#include <iostream>
#include "NocturneGL.h"

int main()
{
	NtDisplay* displayPtr;
	NtNewDisplay(&displayPtr, 520, 520);
}

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
int NtPutDisplay(NtDisplay* display, int i, int j, short r, short g, short b, short a, short z)
{
	if (display == nullptr || display->frameBuffer == nullptr) return NT_FAILURE;
	int index = i + j * display->xRes;
	display->frameBuffer[index].r = r;
	display->frameBuffer[index].g = g;
	display->frameBuffer[index].b = b;
	display->frameBuffer[index].a = a;
	display->frameBuffer[index].z = z;
}
