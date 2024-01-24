#include "NocturneGL.h"
#include <iostream>
/// <summary>
/// Creates a render given input rect file
/// </summary>
/// <returns></returns>
int Render_1(int width, int height, const char* input, const char* output)
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
				NtPutDisplay(displayPtr, i, j, r, g, b, 1);
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

//int main()
//{
//	int status = Render_1(512, 512, "rects", "output1.ppm");
//	std::cout << "\nRender Status: " << (status == NT_SUCCESS ? "Success" : "Failed");
//}