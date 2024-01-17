#include <iostream>
#include "NocturneGL.h"

int main()
{
	int status = Render(512, 512, "rects", "output1.ppm");
	std::cout << "\nRender Status: " << status ? "Failed" : "Success";
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
		display->frameBuffer[i].z = 0;
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
	int index = ClipInt(i) + ClipInt(j) * display->xRes;
	display->frameBuffer[index].r = r;
	display->frameBuffer[index].g = g;
	display->frameBuffer[index].b = b;
	display->frameBuffer[index].a = a;
	display->frameBuffer[index].z = z;

	return NT_SUCCESS;
}

/// <summary>
/// Placeholder clip function as negative rect position isn't supported yet
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
int ClipInt(int input) {
	if (input < 0) return 0;
	if (input > 511) return 511;
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

/// <summary>
/// Creates a render given input rect file
/// </summary>
/// <returns></returns>
int Render(int width, int height, const char* input, const char* output)
{
	int		i, j;
	int		xRes, yRes;
	int		status;

	status = 0;

	/*
	 * initialize the display and the renderer
	 */

	NtDisplay* displayPtr;
	status |= NtNewDisplay(&displayPtr, width, height);

	if (status) exit(NT_FAILURE);

	// I/O File open
	FILE* infile = NULL;
	errno_t errInfile = fopen_s(&infile, input, "r");
	if (errInfile != 0 || infile == NULL)
	{
		std::cout << "Failed to open input file\n" << "Error code : " << errInfile << "\n";
		return NT_FAILURE;
	}

	FILE* outfile = NULL;
	errno_t errOutfile = fopen_s(&outfile, output, "wb");
	if (errOutfile != 0 || outfile == NULL) {
		std::cout << "Failed to open output file: " << output << "\n";
		return NT_FAILURE;
	}

	// Parsing input
	int ulx, uly, lrx, lry, r, g, b;
	while (fscanf_s(infile, "%d %d %d %d %d %d %d",
		&ulx, &uly, &lrx, &lry, &r, &g, &b) == 7) {
		for (j = uly; j <= lry; j++) {
			for (i = ulx; i <= lrx; i++) {
				NtPutDisplay(displayPtr, i, j, r, g, b, 1, 0);
			}
		}
	}


	NtFlushDisplayBufferPPM(outfile, displayPtr); 	/* write out or update display to file*/

	/*
	 * Clean up and exit
	 */

	if (fclose(infile))
		std::cout << "The input file was not closed\n";

	if (fclose(outfile))
		std::cout << "The output file was not closed\n";

	status |= NtFreeDisplay(displayPtr);

	if (status)
		return(NT_FAILURE);
	else
		return(NT_SUCCESS);

}

