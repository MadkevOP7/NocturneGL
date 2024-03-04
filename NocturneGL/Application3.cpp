#include "NocturneGL.h"
#include <iostream>
#pragma warning(disable:4996)
Vector3 ComputeTriColor_3(Vector3 normal) {
	float dotp = 0.707f * normal.x + 0.5f * normal.y + 0.5f * normal.z;

	if (dotp < 0.0) {
		dotp = -dotp;
	}
	else if (dotp > 1.0) {
		dotp = 1.0;
	}
	Vector3 triColor;
	// "tint" the gray [for no good reason!]
	triColor.x = 0.95f * dotp;
	triColor.y = 0.65f * dotp;
	triColor.z = 0.88f * dotp;
	return triColor;
}

/// <summary>
/// Creates a render given input rect file
/// </summary>
/// <returns></returns>
int Render_3(int width, int height, const char* input, const char* output)
{
	NtCamera	camera;
	int		i, j;
	int		xRes, yRes, dispClass;	/* display parameters */
	int		status;
	status = 0;

	//Matrix
	/* Translation matrix */
	NtMatrix	scale =
	{
		3.25,	0.0,	0.0,	0.0,
		0.0,	3.25,	0.0,	-3.25,
		0.0,	0.0,	3.25,	3.5,
		0.0,	0.0,	0.0,	1.0
	};

	NtMatrix	rotateX =
	{
		1.0,	0.0,	0.0,	0.0,
		0.0,	.7071,	.7071,	0.0,
		0.0,	-.7071,	.7071,	0.0,
		0.0,	0.0,	0.0,	1.0
	};

	NtMatrix	rotateY =
	{
		.866,	0.0,	-0.5,	0.0,
		0.0,	1.0,	0.0,	0.0,
		0.5,	0.0,	.866,	0.0,
		0.0,	0.0,	0.0,	1.0
	};

	/*
	 * initialize the display and the renderer
	 */

	NtDisplay* displayPtr;
	status |= NtNewDisplay(&displayPtr, width, height);
	NtRender* renderPtr;
	status |= NtNewRender(&renderPtr, displayPtr);
	if (status) exit(NT_FAILURE);
	//Put camera and matrix
	status |= NtPutCamera(renderPtr, camera);
	status |= NtCalculateViewMatrix(camera, Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), Vector3(0, 0, 20));
	status |= NtCalculateProjectionMatrix(camera, 3, 10, 1, -1, -1, 1);
	/*NtPushMatrix(renderPtr, scale);
	NtPushMatrix(renderPtr, rotateY);
	NtPushMatrix(renderPtr, rotateX);*/
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

	char		dummy[256];
	Vector3		vertexList[3];	/* vertex position coordinates */
	Vector3		normalList[3];	/* vertex normals */
	Vector2	uvList[3];		/* vertex texture map indices */
	//Parse input to triangles
	while (fscanf(infile, "%s", dummy) == 1) {
		fscanf(infile, "%f %f %f %f %f %f %f %f",
			&(vertexList[0].v[0]), &(vertexList[0].v[1]),
			&(vertexList[0].v[2]),
			&(normalList[0].v[0]), &(normalList[0].v[1]),
			&(normalList[0].v[2]),
			&(uvList[0].v[0]), &(uvList[0].v[1]));
		fscanf(infile, "%f %f %f %f %f %f %f %f",
			&(vertexList[1].v[0]), &(vertexList[1].v[1]),
			&(vertexList[1].v[2]),
			&(normalList[1].v[0]), &(normalList[1].v[1]),
			&(normalList[1].v[2]),
			&(uvList[1].v[0]), &(uvList[1].v[1]));
		fscanf(infile, "%f %f %f %f %f %f %f %f",
			&(vertexList[2].v[0]), &(vertexList[2].v[1]),
			&(vertexList[2].v[2]),
			&(normalList[2].v[0]), &(normalList[2].v[1]),
			&(normalList[2].v[2]),
			&(uvList[2].v[0]), &(uvList[2].v[1]));
		//Shade and put triangle
		Vector3 col = ComputeTriColor_3(normalList[0]);
		//NtPutTriangle(renderPtr, vertexList, normalList, col);
	}



	NtFlushDisplayBufferPPM(outfile, displayPtr); 	/* write out or update display to file*/

	/*
	 * Clean up and exit
	 */

	if (fclose(infile))
		std::cout << "The input file was not closed\n";

	if (fclose(outfile))
		std::cout << "The output file was not closed\n";

	status |= NtFreeRender(renderPtr);
	status |= NtFreeDisplay(displayPtr);

	if (status)
		return(NT_FAILURE);
	else
		return(NT_SUCCESS);

}

int main_3()
{
	int status = Render_3(256, 256, "app3Pot.asc", "output3.ppm");
	std::cout << "\nRender Status: " << (status == NT_SUCCESS ? "Success" : "Failed");
	return 0;
}