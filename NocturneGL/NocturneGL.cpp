#include <iostream>
#include <fstream>
#include "NocturneGL.h"
#include <cmath>
#include <limits>
#include "externalPlugins/json.hpp"
#include <math.h>
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

	/*Degree to radians*/
	static float dtr(float degrees)
	{
		return degrees * static_cast<float>(NT_PI) / 180.0f;
	}
};

//Texture//
NtTexture::NtTexture(const std::string& filename) {
	width = 0;
	height = 0;
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "NtTexture: Error opening file " << filename << "\n";
		return;
	}

	std::string header;
	file >> header;
	if (header != "P6") {
		std::cerr << "NtTexture: Unsupported file format " << header << "\n";
		return;
	}

	file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	//Skip comments and whitespace
	char ch;
	while (file.peek() == '#' || std::isspace(file.peek())) {
		if (file.peek() == '#') {
			file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		else {
			file.get(ch);
		}
	}

	file >> width >> height;
	if (width <= 0 || height <= 0) {
		std::cerr << "Texture loading error, invalid size result!\n";
		return;
	}

	int maxVal;
	file >> maxVal;
	file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	if (maxVal != 255) {
		std::cerr << "Unsupported maxVal in PPM: " << maxVal << "\n";
		return;
	}

	textureData.resize(height, std::vector<NtPixelf>(width));
	unsigned char rgb[3];
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			if (!file.read(reinterpret_cast<char*>(rgb), 3)) {
				std::cerr << "Error reading pixel data at (" << x << ", " << y << ").\n";
				std::cerr << "Read error: " << (file.eof() ? "End of file reached unexpectedly." : "Unknown error.") << "\n";
				return;
			}

			// Normalize the 8-bit values to the range [0, 1] for floating-point
			textureData[y][x] = {
				rgb[0] / 255.0f,
				rgb[1] / 255.0f,
				rgb[2] / 255.0f,
				1.0f  // Assuming full opacity for alpha
			};
		}
	}


	if (width == 0 || height == 0) {
		std::cerr << "Texture loading error, invalid size result!\n";
	}
	else {
		std::cout << "Texture read: " << filename << " width: " << width << " height: " << height << "\n";
	}
}

NtPixelf NtTextureLookUp(float u, float v, const NtTexture& texture) {
	float xLocation = u * (texture.GetWidth() - 1);
	float yLocation = v * (texture.GetHeight() - 1);
	int x0 = static_cast<int>(xLocation);
	int y0 = static_cast<int>(yLocation);
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	float xFrac = xLocation - x0;
	float yFrac = yLocation - y0;

	NtPixelf p00 = texture.GetPixelf(x0, y0);
	NtPixelf p10 = texture.GetPixelf(x1, y0);
	NtPixelf p01 = texture.GetPixelf(x0, y1);
	NtPixelf p11 = texture.GetPixelf(x1, y1);

	// Directly interpolate along x for both y0 and y1 lines
	NtPixelf p0010 = NtPixelAddition(NtPixelMultiply(p00, 1 - xFrac), NtPixelMultiply(p10, xFrac));
	NtPixelf p0111 = NtPixelAddition(NtPixelMultiply(p01, 1 - xFrac), NtPixelMultiply(p11, xFrac));

	// Final interpolation along y
	NtPixelf output = NtPixelAddition(NtPixelMultiply(p0010, 1 - yFrac), NtPixelMultiply(p0111, yFrac));

	return output;
}

/// <summary>
/// Sets the renderer's shading mode, return NT_FAILURE if render pointer is null
/// </summary>
/// <param name="render"></param>
/// <param name="mode"></param>
/// <returns></returns>
int NtSetShadingMode(NtRender* render, NT_SHADING_MODE mode) {
	if (render == nullptr)
		return NT_FAILURE;

	render->shadingMode = mode;
	return NT_SUCCESS;
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
int NtNewDisplay(NtDisplay** display, int xRes, int yRes, Vector4 backgroundColor)
{
	*display = new NtDisplay();
	(*display)->xRes = xRes;
	(*display)->yRes = yRes;
	int status = 0;

	status |= NtNewFrameBuffer(&((*display)->frameBuffer), xRes, yRes);
	status |= NtInitDisplay(*display, backgroundColor);

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
/// Initializes an empty display with all pixels set to background color
/// </summary>
/// <param name="display"></param>
/// <returns></returns>
int NtInitDisplay(NtDisplay* display, const Vector4& backgroundColor)
{
	if (display == nullptr || display->frameBuffer == nullptr) return NT_FAILURE;

	//Todo: we could cache buffer size
	int bufferSize = display->xRes * display->yRes * sizeof(NtPixel);
	for (int i = 0; i < bufferSize; i++) {
		display->frameBuffer[i].r = NTMath::fts(backgroundColor.x);
		display->frameBuffer[i].g = NTMath::fts(backgroundColor.y);
		display->frameBuffer[i].b = NTMath::fts(backgroundColor.z);
		display->frameBuffer[i].a = NTMath::fts(backgroundColor.w);
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

float Min(float a, float b) {
	if (a < b)return a;
	return b;
}

float Max(float a, float b) {
	if (a > b)return a;
	return b;
}

float Clipf(float input, int min, int max) {
	if (input < min) return min;
	if (input > max) return max;
	return input;
}

void ClipVec3(Vector3& vec) {
	vec.x = Clipf(vec.x, 0, 1);
	vec.y = Clipf(vec.y, 0, 1);
	vec.z = Clipf(vec.z, 0, 1);
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

// Callback function for stb_image_write to write to FILE*
void write_func(void* context, void* data, int size) {
	fwrite(data, 1, size, (FILE*)context);
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
	(*render)->zBuffer = new float* [display->xRes + 1];
	if (!(*render)->zBuffer) {
		delete render;
		return NT_FAILURE;
	}

	for (int i = 0; i <= display->xRes; i++) {
		(*render)->zBuffer[i] = new float[display->yRes + 1];

		// Initialize Z-buffer to infinity
		for (int j = 0; j <= display->yRes; j++) {
			(*render)->zBuffer[i][j] = INFINITY;
		}
	}
	NtLoadIdentityMatrix((*render)->worldMatrix);
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
int NtPutTriangle(NtRender* render, Vector3 vertexList[], Vector3 normalList[], Vector2 uvList[], const NtMaterial& material) {
	if (render == nullptr) return NT_FAILURE;

	//Transform vertex and normals
	for (int i = 0; i < 3; i++) {
		Vector4 vec4(vertexList[i].x, vertexList[i].y, vertexList[i].z, 1);
		Vector4 vec4Normal(normalList[i].x, normalList[i].y, normalList[i].z, 0);
		Vector4 worldResult = vec4 * render->worldMatrix;
		Vector4 worldResultNormal = vec4Normal * render->worldMatrixInverseTransposed;
		Vector4 camResult = worldResult * render->camera->viewMatrix;
		Vector4 camResultNormal = worldResultNormal * render->camera->viewMatrix;
		Vector4 ndcResult = camResult * render->camera->projectMatrix;
		//Write result back to vector3 
		vertexList[i].x = ndcResult.x / ndcResult.w;
		vertexList[i].y = ndcResult.y / ndcResult.w;
		vertexList[i].z = ndcResult.z / ndcResult.w;

		//Scale NDC cords to image
		vertexList[i].x = (vertexList[i].x + 1) * ((render->display->xRes - 1) / 2);
		vertexList[i].y = (1 - vertexList[i].y) * ((render->display->yRes - 1) / 2);

		//Write normal result back
		normalList[i].x = camResultNormal.x;
		normalList[i].y = camResultNormal.y;
		normalList[i].z = camResultNormal.z;
		normalList[i].normalize();
	}

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

	//Lighting pre-compute for flat and gouraud
	//Compute Color - Shading
	Vector3 finalColor;
	Vector3 vertexColors[3];
	switch (render->shadingMode) {
	case NT_SHADE_FLAT:
		finalColor = NtLightingPhong(material, NtAverageQuadNormals(normalList), render->directionalLight, render->camera->viewDirection, render->ambientLight);
		break;

	case NT_SHADE_GOURAUD:
		for (int i = 0; i < 3; i++) {
			vertexColors[i] = NtLightingPhong(material, normalList[i], render->directionalLight, render->camera->viewDirection, render->ambientLight);
		}
		break;
	}
	//Rasterization
	for (int y = yMin; y <= yMax; y++) {
		for (int x = xMin; x <= xMax; x++) {
			Vector3 v0 = vertexList[0];
			Vector3 v1 = vertexList[1];
			Vector3 v2 = vertexList[2];

			float f12 = NTMath::f12(v0.x, v0.y, v1, v2);
			float f20 = NTMath::f20(v1.x, v1.y, v2, v0);
			float f01 = NTMath::f01(v2.x, v2.y, v0, v1);

			float alpha = NTMath::f12(x, y, v1, v2) / f12;
			float beta = NTMath::f20(x, y, v2, v0) / f20;
			float gamma = NTMath::f01(x, y, v0, v1) / f01;
			if ((alpha >= 0) && (beta >= 0) && (gamma >= 0)) {
				//Z-Buffer to determine if current pixel should be put
				//Interpolate z from alpha beta gamma
				float currZ = alpha * v0.z + beta * v1.z + gamma * v2.z;
				if (currZ < render->zBuffer[x][y]) {
					// Update the Z-buffer
					render->zBuffer[x][y] = currZ;

					//Compute Color - Phong (interpolate normals and light compute per pixel)
					if (render->shadingMode == NT_SHADE_PHONG) {
						Vector3 interpolatedNormal = NtInterpolateVector3(normalList, alpha, beta, gamma, true);
						finalColor = NtLightingPhong(material, interpolatedNormal, render->directionalLight, render->camera->viewDirection, render->ambientLight);
					}
					else if (render->shadingMode == NT_SHADE_GOURAUD) {
						finalColor = NtInterpolateVector3(vertexColors, alpha, beta, gamma, false);
					}

					NtTexturePixel(finalColor, material, uvList, vertexList, alpha, beta, gamma);
					NtPutDisplay(render->display, x, y, NTMath::fts(finalColor.x), NTMath::fts(finalColor.y), NTMath::fts(finalColor.z), 255);
				}
			}
		}
	}
}

int NtPutTriangle(NtRender* render, NtTriangle& triangle, const NtMaterial& material) {
	if (render == nullptr) return NT_FAILURE;
	Vector3 vertexList[3];
	vertexList[0] = triangle.v0.vertexPos;
	vertexList[1] = triangle.v1.vertexPos;
	vertexList[2] = triangle.v2.vertexPos;

	Vector3 normalList[3];
	normalList[0] = triangle.v0.vertexNormal;
	normalList[1] = triangle.v1.vertexNormal;
	normalList[2] = triangle.v2.vertexNormal;

	Vector2 uvList[3];
	uvList[0] = triangle.v0.texture;
	uvList[1] = triangle.v1.texture;
	uvList[2] = triangle.v2.texture;

	return NtPutTriangle(render, vertexList, normalList, uvList, material);
}
//////Transormations//////

/// <summary>
/// Sets the input matrix to identity matrix
/// </summary>
/// <param name="mat"></param>
void NtLoadIdentityMatrix(NtMatrix& mat) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			mat[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
}

/// <summary>
/// Sets the render's world matrix
/// </summary>
/// <param name="matrix"></param>
/// <returns></returns>
int NtSetWorldMatrix(NtRender* render, NtMatrix& matrix, NtMatrix& matrixInverseTransposed) {
	render->worldMatrix = matrix;
	render->worldMatrixInverseTransposed = matrixInverseTransposed;
	return NT_SUCCESS;
}

/// <summary>
///	Create rotate matrix : rotate along x axis.
/// Pass back the matrix using mat value
/// </summary>
/// <param name="degree"></param>
/// <param name="mat"></param>
/// <returns></returns>
int NtRotXMat(float degree, NtMatrix& mat)
{
	NtLoadIdentityMatrix(mat);
	float rad = NTMath::dtr(degree);

	mat[1][1] = cos(rad);
	mat[1][2] = -sin(rad);
	mat[2][1] = sin(rad);
	mat[2][2] = cos(rad);
	return NT_SUCCESS;
}

/// <summary>
/// Create rotate matrix : rotate along y axis. Pass back the matrix using mat value
/// </summary>
/// <param name="degree"></param>
/// <param name="mat"></param>
/// <returns></returns>
int NtRotYMat(float degree, NtMatrix& mat)
{
	NtLoadIdentityMatrix(mat);
	float rad = NTMath::dtr(degree);

	mat[0][0] = cos(rad);
	mat[0][2] = sin(rad);
	mat[2][0] = -sin(rad);
	mat[2][2] = cos(rad);
	return NT_SUCCESS;
}

/// <summary>
/// Create rotate matrix : rotate along z axis. Pass back the matrix using mat value
/// </summary>
/// <param name="degree"></param>
/// <param name="mat"></param>
/// <returns></returns>
int NtRotZMat(float degree, NtMatrix& mat)
{
	NtLoadIdentityMatrix(mat);
	float rad = NTMath::dtr(degree);

	mat[0][0] = cos(rad);
	mat[0][1] = -sin(rad);
	mat[1][0] = sin(rad);
	mat[1][1] = cos(rad);
	return NT_SUCCESS;
}


/// <summary>
/// Create translation matrix. Pass back the matrix using mat value.
/// </summary>
/// <param name="translate"></param>
/// <param name="mat"></param>
/// <returns></returns>
int NtTranslateMat(Vector3 translate, NtMatrix& mat)
{
	NtLoadIdentityMatrix(mat);

	mat[0][3] = translate.x;
	mat[1][3] = translate.y;
	mat[2][3] = translate.z;

	return NT_SUCCESS;
}


/// <summary>
/// Create scaling matrix. Pass back the matrix using mat value.
/// </summary>
/// <param name="scale"></param>
/// <param name="mat"></param>
/// <returns></returns>
int NtScaleMat(Vector3 scale, NtMatrix& mat)
{
	NtLoadIdentityMatrix(mat);

	mat[0][0] = scale.x;
	mat[1][1] = scale.y;
	mat[2][2] = scale.z;

	return NT_SUCCESS;
}

/// <summary>
/// Pushes a matrix down the render matrix stack
/// </summary>
/// <param name="render"></param>
/// <param name="matrix"></param>
void NtPushMatrix(NtRender* render, const NtMatrix& matrix) {
	render->matrixStack.push_back(matrix);
}
/// <summary>
/// Pops a matrix from the render matrix stack
/// </summary>
/// <param name="render"></param>
/// <param name="matrix"></param>
void NtPopMatrix(NtRender* render) {
	render->matrixStack.pop_back();
}

/// <summary>
/// Gets the top matrix of the stack on render, returns value through mat. Returns NT_FAILURE if stack is empty
/// </summary>
/// <param name="mat"></param>
/// <returns></returns>
int NtGetTopMatrix(NtRender* render, NtMatrix& mat) {
	if (render->matrixStack.size() == 0) return NT_FAILURE;
	mat = render->matrixStack.back();
	return NT_SUCCESS;
}

/// <summary>
/// Inverts a rotation matrix, essentially the transpose of it
/// </summary>
/// <param name="mat"></param>
/// <returns></returns>
int NtInvertRotMat(const NtMatrix& mat, NtMatrix& result) {
	result = mat.transpose();
	return NT_SUCCESS;
}

/// <summary>
/// Inverts the scale matrix, where the diagonal scale are the reciprocal of the original
/// </summary>
/// <param name="mat"></param>
/// <param name="result"></param>
/// <returns></returns>
int NtInvertScaleMat(const NtMatrix& mat, NtMatrix& result) {
	NtLoadIdentityMatrix(result);
	//Ignores w component
	for (int i = 0; i < 3; ++i) {

		//Check for division by zero
		if (mat[i][i] == 0)
			return NT_FAILURE;

		result[i][i] = 1.0f / mat[i][i];
	}

	return NT_SUCCESS;
}

/// <summary>
/// Inverts the translation, where the translation is negated from the original
/// </summary>
/// <param name="mat"></param>
/// <param name="result"></param>
/// <returns></returns>
int NtInvertTranslateMat(const NtMatrix& mat, NtMatrix& result) {
	result = mat;
	result[0][3] = -mat[0][3];
	result[1][3] = -mat[1][3];
	result[2][3] = -mat[2][3];
	return NT_SUCCESS;
}

/// <summary>
/// Sets the camera of that render
/// </summary>
/// <param name="render"></param>
/// <param name="camera"></param>
/// <returns></returns>
int NtPutCamera(NtRender* render, NtCamera& camera) {
	if (render == nullptr) return NT_FAILURE;
	render->camera = &camera;
	return NT_SUCCESS;
}

/// <summary>
/// Calculates the view matrix based on current camera definition
/// viewer's 'up' is v, view ('look') direction is n, cross(v,n) is u, and r is the translation (eg. "eye point", camera location).
/// </summary>
/// <param name="render"></param>
/// <returns></returns>
int NtCalculateViewMatrix(NtCamera& camera, Vector3 u, Vector3 v, Vector3 n, Vector3 r) {
	NtMatrix view;
	view[0][0] = u.x; view[0][1] = u.y; view[0][2] = u.z; view[0][3] = -r.dot(u);
	view[1][0] = v.x; view[1][1] = v.y; view[1][2] = v.z; view[1][3] = -r.dot(v);
	view[2][0] = n.x; view[2][1] = n.y; view[2][2] = n.z; view[2][3] = -r.dot(n);
	view[3][0] = 0; view[3][1] = 0; view[3][2] = 0; view[3][3] = 1;

	camera.viewMatrix = view;
	return NT_SUCCESS;
}

/// <summary>
/// Computes the projection matrix from camera specifications
/// </summary>
/// <param name="camera"></param>
/// <returns></returns>
int NtCalculateProjectionMatrix(NtCamera& camera, float near, float far, float top, float bottom, float left, float right) {
	NtMatrix proj;
	NtLoadIdentityMatrix(proj);
	proj[0][0] = (2 * near) / (right - left);
	proj[1][1] = (2 * near) / (top - bottom);
	proj[0][2] = (right + left) / (right - left);
	proj[1][2] = (top + bottom) / (top - bottom);
	proj[2][2] = -(far + near) / (far - near);
	proj[3][2] = -1;
	proj[2][3] = -2 * far * near / (far - near);

	camera.projectMatrix = proj;
	return NT_SUCCESS;
}


///////Scene///////

/// <summary>
/// Loads a scene description in JSON format to current scene. If autoLoadMeshAndTexture = false, user must manually specify to load mesh and textures.
/// </summary>
/// <param name="scenePath"></param>
int NtLoadSceneJSON(std::string scenePath, NtScene* scene, bool autoLoadMeshAndTexture) {
	using json = nlohmann::json;
	int status = 0;
	std::ifstream file(scenePath); // Assuming the JSON is stored in a file named "scene.json"

	if (!file.is_open()) {
		std::cerr << "Failed to open JSON file\n";
		return NT_FAILURE;
	}

	// Parse the JSON
	json jsonData;
	try {
		file >> jsonData;
	}
	catch (const std::exception& e) {
		std::cout << "Error parsing JSON" << "\n";
		return NT_FAILURE;
	}

	try {
		// Parse shapes
		if (jsonData["scene"].find("shapes") != jsonData["scene"].end()) {
			for (const auto& shapeValue : jsonData["scene"]["shapes"]) {
				NtShape shape;
				shape.id = shapeValue["id"];
				shape.geometryId = shapeValue["geometry"];
				shape.notes = shapeValue["notes"];

				//Write material
				const auto& material = shapeValue["material"];
				shape.material.surfaceColor.x = material["Cs"][0];
				shape.material.surfaceColor.y = material["Cs"][1];
				shape.material.surfaceColor.z = material["Cs"][2];
				shape.material.Ka = material["Ka"];
				shape.material.Kd = material["Kd"];
				shape.material.Ks = material["Ks"];
				shape.material.Kt = material["Kt"];
				shape.material.specularExponent = material["n"];
				shape.material.textureId = material["texture"];

				//Auto load into mesh map
				if (autoLoadMeshAndTexture) {
					status |= NtLoadMesh(shape.geometryId, ".json", scene);
					status |= NtLoadTexture(shape.material, scene);
				}

				//Write transformations
				const auto& transforms = shapeValue["transforms"];
				shape.transforms.scale.x = transforms[1]["S"][0];
				shape.transforms.scale.y = transforms[1]["S"][1];
				shape.transforms.scale.z = transforms[1]["S"][2];

				//Write rotations
				if (transforms[0].find("Ry") != transforms[0].end()) {
					shape.transforms.rotation.y = transforms[0]["Ry"];
				}
				if (transforms[0].find("Rx") != transforms[0].end()) {
					shape.transforms.rotation.x = transforms[0]["Rx"];
				}
				if (transforms[0].find("Rz") != transforms[0].end()) {
					shape.transforms.rotation.z = transforms[0]["Rz"];
				}

				shape.transforms.translation.x = transforms[2]["T"][0];
				shape.transforms.translation.y = transforms[2]["T"][1];
				shape.transforms.translation.z = transforms[2]["T"][2];

				scene->shapes.push_back(shape);
			}
		}

		// Parse camera
		if (jsonData["scene"].find("camera") != jsonData["scene"].end()) {
			const auto& cameraValue = jsonData["scene"]["camera"];
			scene->camera.from.x = cameraValue["from"][0];
			scene->camera.from.y = cameraValue["from"][1];
			scene->camera.from.z = cameraValue["from"][2];
			scene->camera.to.x = cameraValue["to"][0];
			scene->camera.to.y = cameraValue["to"][1];
			scene->camera.to.z = cameraValue["to"][2];
			scene->camera.near = cameraValue["bounds"][0];
			scene->camera.far = cameraValue["bounds"][1];
			scene->camera.right = cameraValue["bounds"][2];
			scene->camera.left = cameraValue["bounds"][3];
			scene->camera.top = cameraValue["bounds"][4];
			scene->camera.bottom = cameraValue["bounds"][5];

			scene->camera.xRes = cameraValue["resolution"][0];
			scene->camera.yRes = cameraValue["resolution"][1];
		}

		//Parse lights
		if (jsonData["scene"].find("lights") != jsonData["scene"].end()) {
			for (const auto& lightValue : jsonData["scene"]["lights"]) {
				NtLight light;

				// Common properties
				light.color = Vector3(lightValue["color"][0], lightValue["color"][1], lightValue["color"][2]);
				light.intensity = lightValue["intensity"];

				std::string typeStr = lightValue["type"];
				if (typeStr == "ambient") {
					light.type = NT_LIGHT_AMBIENT;
					scene->ambient = light;
				}
				else if (typeStr == "directional") {
					light.type = NT_LIGHT_DIRECTIONAL;
					Vector3 from = Vector3(lightValue["from"][0], lightValue["from"][1], lightValue["from"][2]);
					Vector3 to = Vector3(lightValue["to"][0], lightValue["to"][1], lightValue["to"][2]);

					scene->directional = light;
					scene->directional.direction = to - from;
					scene->directional.direction.normalize();
				}
				scene->lights.push_back(light);
			}
		}
		std::cout << "Scene parsing completed!\n";
		return status;
	}
	catch (const std::exception& e) {
		std::cout << "Error parsing JSON " << e.what() << "\n";
		return NT_FAILURE;
	}
}

/// <summary>
/// Loads a texture into given scene's texture map
/// </summary>
/// <param name="textureName"></param>
/// <param name="scene"></param>
/// <returns></returns>
int NtLoadTexture(NtMaterial& material, NtScene* scene) {
	std::string textureName = material.textureId;
	auto it = scene->textureMap.find(textureName);
	if (it != scene->textureMap.end()) {
		std::cout << "Texture map already contains " << textureName << ". Skipped loading" << std::endl;
		return NT_SUCCESS;
	}

	NtTexture* texture = new NtTexture(textureName);
	if (texture->GetHeight() == 0 || texture->GetWidth() == 0) {
		std::cerr << "Failed to load texture: " << textureName << "\n";
		return NT_FAILURE;
	}

	scene->textureMap[textureName] = texture;
	material.texture = texture;
	return NT_SUCCESS;
}

int NtLoadMesh(const std::string meshName, const std::string meshExtension, NtScene* scene) {
	auto it = scene->meshMap.find(meshName);
	if (it != scene->meshMap.end()) {
		std::cout << "Mesh map already contains " << meshName << ". Skipped loading" << std::endl;
		return NT_SUCCESS;
	}

	std::ifstream file(meshName + meshExtension);
	if (!file.is_open()) {
		std::cout << "File with name " << meshName << meshExtension << " could not be found";
		return NT_FAILURE;
	}

	nlohmann::json jsonData;
	file >> jsonData;

	NtMesh* mesh = new NtMesh();
	for (const auto& item : jsonData["data"]) {
		NtTriangle triangle;
		// Parse vertices
		for (int i = 0; i < 3; ++i) {
			std::string vertexKey = "v" + std::to_string(i);
			Vector3 pos = { item[vertexKey]["v"][0], item[vertexKey]["v"][1], item[vertexKey]["v"][2] };
			Vector3 norm = { item[vertexKey]["n"][0], item[vertexKey]["n"][1], item[vertexKey]["n"][2] };
			Vector2 tex = { item[vertexKey]["t"][0], item[vertexKey]["t"][1] };

			NtVertex vertex;
			vertex.vertexPos = pos;
			vertex.vertexNormal = norm;
			vertex.texture = tex;

			switch (i) {
			case 0: triangle.v0 = vertex; break;
			case 1: triangle.v1 = vertex; break;
			case 2: triangle.v2 = vertex; break;
			}
		}

		mesh->triangles.push_back(triangle);
	}

	scene->meshMap[meshName] = mesh;
	return NT_SUCCESS;
}

/// <summary>
/// Renders a scene, handles the other stuff automatically
/// </summary>
/// <param name="scene"></param>
/// <param name="outputName"></param>
/// <returns></returns>
int NtRenderScene(NtScene* scene, const std::string outputName, NT_SHADING_MODE shadingMode) {
	int status = 0;
	NtDisplay* displayPtr;
	status |= NtNewDisplay(&displayPtr, scene->camera.xRes, scene->camera.yRes);
	NtRender* renderPtr;
	status |= NtNewRender(&renderPtr, displayPtr);
	status |= NtSetRenderAttributes(renderPtr, scene);
	status |= NtSetShadingMode(renderPtr, shadingMode);
	if (status) return NT_FAILURE;


	//Put camera and matrix
	status |= NtPutCamera(renderPtr, scene->camera);

	//Calculate camerae matrix, we need to calculate u, v, n, r
	Vector3 n = (scene->camera.from - scene->camera.to);
	n.normalize();
	scene->camera.viewDirection = n;
	//Assume world up
	Vector3 worldUp = { 0, 1, 0 };

	Vector3 u = worldUp.cross(n);
	u.normalize();

	Vector3 v = n.cross(u);

	Vector3 r = scene->camera.from;

	status |= NtCalculateViewMatrix(scene->camera, u, v, n, r);
	status |= NtCalculateProjectionMatrix(scene->camera, scene->camera.near, scene->camera.far, scene->camera.top, scene->camera.bottom, scene->camera.left, scene->camera.right);

	//Render each shape, each time computing the new transformation (world matrix) and put triangle
	for (auto& shape : scene->shapes) {
		//Load transformation matrix
		NtMatrix zRot;
		NtRotZMat(shape.transforms.rotation.z, zRot);
		NtMatrix yRot;
		NtRotYMat(shape.transforms.rotation.y, yRot);
		NtMatrix xRot;
		NtRotXMat(shape.transforms.rotation.x, xRot);

		NtMatrix combinedRotation = zRot * yRot * xRot;
		NtMatrix combinedRotationInversed;
		NtInvertRotMat(combinedRotation, combinedRotationInversed);

		NtMatrix scale;
		NtScaleMat(shape.transforms.scale, scale);
		NtMatrix scaleInversed;
		NtInvertScaleMat(scale, scaleInversed);

		NtMatrix translation;
		NtTranslateMat(shape.transforms.translation, translation);
		NtMatrix translationInversed;
		NtInvertTranslateMat(translation, translationInversed);

		//Forward transformation order is scale -> rotation -> translation
		//But matrix multiplication first multiplied is last applied

		//For inverse transformation, the order is reversed forward
		NtMatrix combinedTransformation = translation * combinedRotation * scale;
		NtMatrix combinedTransformationInversed = scaleInversed * combinedRotationInversed * translationInversed;
		combinedRotationInversed.transpose();
		NtSetWorldMatrix(renderPtr, combinedTransformation, combinedTransformationInversed);
		//Render faces of that model
		NtMesh* mesh = scene->meshMap[shape.geometryId];
		for (NtTriangle& triangle : mesh->triangles) {
			NtPutTriangle(renderPtr, triangle, shape.material);
		}
	}

	//Flush to ppm
	FILE* outfile = NULL;
	errno_t errOutfile = fopen_s(&outfile, outputName.c_str(), "wb");
	if (errOutfile != 0 || outfile == NULL) {
		std::cout << "Failed to open output file: " << outputName << "\n";
		return NT_FAILURE;
	}
	NtFlushDisplayBufferPPM(outfile, displayPtr);
	return NT_SUCCESS;
}

/// <summary>
/// Sets the render attribute based on scene data
/// </summary>
/// <param name="render"></param>
/// <param name="scene"></param>
/// <returns></returns>
int NtSetRenderAttributes(NtRender* render, NtScene* scene) {
	if (render == nullptr || scene == nullptr)
		return NT_FAILURE;

	render->lights = scene->lights;
	render->directionalLight = scene->directional;
	render->ambientLight = scene->ambient;
	return NT_SUCCESS;
}

///////Shading//////////
Vector3 NtAverageQuadNormals(const Vector3 normalList[]) {
	Vector3 additionResult = normalList[0] + normalList[1] + normalList[2];
	additionResult = additionResult / 3.0f;
	additionResult.normalize();
	return additionResult;
}

Vector3 NtInterpolateVector3(const Vector3 vectors[], float alpha, float beta, float gamma, bool isNormal) {
	Vector3 result = vectors[0] * alpha + vectors[1] * beta + vectors[2] * gamma;
	if (isNormal)
		result.normalize();
	return result;
}

float NtInterpolate(const Vector3& vec, float alpha, float beta, float gamma) {
	return alpha * vec.x + beta * vec.y + gamma * vec.z;
}

Vector3 NtLightingPhong(const NtMaterial& material, const Vector3& normal, const NtLight& lightSource, const Vector3& viewDirection, const NtLight& ambientLight) {
	//Lighting = ambient + diffuse + specular
	Vector3 lighting;

	Vector3 _ambient = ambientLight.color * ambientLight.intensity;

	//Diffuse
	Vector3 _normal = normal;
	_normal.normalize();
	Vector3 lightVector = lightSource.direction * -1;
	lightVector.normalize();

	float diffuseStrength = Max(lightVector.dot(_normal), 0);
	Vector3 _diffuse = lightSource.color * diffuseStrength * lightSource.intensity;

	//Specular
	Vector3 reflection = Vector3::reflect(lightVector, _normal);
	reflection.normalize();

	Vector3 viewVector = viewDirection * -1;
	viewVector.normalize();

	float specularStrength = Max(Vector3::dot(viewVector, reflection), 0);
	specularStrength = std::powf(specularStrength, material.specularExponent);
	Vector3 _specular = lightSource.color * specularStrength * lightSource.intensity;

	lighting = _ambient * material.Ka + _diffuse * material.Kd + _specular * material.Ks;

	Vector3 color = material.surfaceColor * lighting;

	color.x = Clipf(color.x, 0, 1);
	color.y = Clipf(color.y, 0, 1);
	color.z = Clipf(color.z, 0, 1);

	return color;
}

void NtTexturePixel(Vector3& color, const NtMaterial& material, Vector2 vertsUV[], Vector3 triVerts[], float alpha, float beta, float gamma) {
	//Divide U, V by z
	Vector3 vertsU = { vertsUV[0].x / triVerts[0].z, vertsUV[1].x / triVerts[1].z, vertsUV[2].x / triVerts[2].z };
	Vector3 vertsV = { vertsUV[0].y / triVerts[0].z, vertsUV[1].y / triVerts[1].z, vertsUV[2].y / triVerts[2].z };

	float s = NtInterpolate(vertsU, alpha, beta, gamma);
	float t = NtInterpolate(vertsV, alpha, beta, gamma);

	if (nullptr == material.texture) {
		std::cerr << "Error Getting Texture, Material has null texture ptr!\n";
		return;
	}

	Vector3 vertsZ = { 1 / triVerts[0].z, 1 / triVerts[1].z ,1 / triVerts[2].z };
	float z = 1 / NtInterpolate(vertsZ, alpha, beta, gamma);

	s *= z;
	t *= z;

	//Blend pixel texture map color with the original color, here alpha from pixel is disposed
	//To make sure the final color is still in [0, 1] we clip all components of it
	NtPixelf textureResult = NtTextureLookUp(s, t, *material.texture);
	color = color + (Vector3(textureResult.r, textureResult.g, textureResult.b) * material.Kt);
	ClipVec3(color);
}