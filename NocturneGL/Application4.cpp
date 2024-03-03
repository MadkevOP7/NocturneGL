#include "NocturneGL.h"
#include <iostream>
#pragma warning(disable:4996)
Vector3 ComputeTriColor_4(Vector3 normal) {
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
int Render_4(int width, int height, const std::string output)
{
	int status = 0;
	NtScene* scene = new NtScene();

	status |= NtLoadSceneJSON("scene.json", scene);
	status |= NtLoadMesh("teapot", ".json", scene);

	NtRenderScene(scene, output);
	//NtFlushDisplayBufferPPM(outfile, displayPtr); 	/* write out or update display to file*/

	if (status)
		return(NT_FAILURE);
	else
		return(NT_SUCCESS);

}

int main()
{
	int status = Render_4(512, 512, "output4.ppm");
	std::cout << "\nRender Status: " << (status == NT_SUCCESS ? "Success" : "Failed");
	return 0;
}