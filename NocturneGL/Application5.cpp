#include "NocturneGL.h"
#include <iostream>
#include <chrono>

/// <summary>
/// Creates a render given input rect file
/// </summary>
/// <returns></returns>
int Render_5(int width, int height, const std::string output)
{
	auto start = std::chrono::high_resolution_clock::now();
	int status = 0;
	NtScene* scene = new NtScene();

	status |= NtLoadSceneJSON("scene5.json", scene);
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << "\nNocturne Renderer load scene completed in " << duration.count() << " milliseconds.\n";

	start = std::chrono::high_resolution_clock::now();
	status |= NtRenderScene(scene, output, NT_SHADE_PHONG);
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

	std::cout << "\nNtRenderScene completed in " << duration.count() << " milliseconds.\n";

	if (status)
		return(NT_FAILURE);
	else
		return(NT_SUCCESS);

}

int main()
{
	int status = Render_5(512, 512, "output5.ppm");
	std::cout << "\nRender Status: " << (status == NT_SUCCESS ? "Success" : "Failed");
	return 0;
}