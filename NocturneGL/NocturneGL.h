#pragma once
#include<iostream>
#include<vector>
#include <unordered_map>
/*Pixel Data*/
typedef struct {
	unsigned short r, g, b, a;
} NtPixel;

typedef struct {
	float r, g, b, a;
} NtPixelf;

/// <summary>
/// Multiply a pixel by scalar
/// </summary>
/// <param name="pixel"></param>
/// <param name="scalar"></param>
/// <returns></returns>
static NtPixelf NtPixelMultiply(const NtPixelf& pixel, const float scalar) {
	NtPixelf result;
	result.r = pixel.r * scalar;
	result.g = pixel.g * scalar;
	result.b = pixel.b * scalar;
	result.a = pixel.a * scalar;
	return result;
}

/// <summary>
/// Returns a NtPixel result of a - b
/// </summary>
/// <param name="a"></param>
/// <param name="b"></param>
/// <returns></returns>
static NtPixelf NtPixelSubtraction(const NtPixelf& a, const NtPixelf& b) {
	NtPixelf result;
	result.r = a.r - b.r;
	result.g = a.g - b.g;
	result.b = a.b - b.b;
	result.a = a.a - b.a;
	return result;
}

/// <summary>
/// Returns a NtPixel result of a + b
/// </summary>
/// <param name="a"></param>
/// <param name="b"></param>
/// <returns></returns>
static NtPixelf NtPixelAddition(const NtPixelf& a, const NtPixelf& b) {
	NtPixelf result;
	result.r = a.r + b.r;
	result.g = a.g + b.g;
	result.b = a.b + b.b;
	result.a = a.a + b.a;
	return result;
}


class NtTexture {
private:
	int width;
	int height;
	std::vector<std::vector<NtPixelf>> textureData;
public:
	int GetWidth() const { return width; }
	int GetHeight() const { return height; }
	NtTexture(const std::string& filename);

	NtPixelf GetPixelf(int x, int y) const {
		if (x >= 0 && x < width && y >= 0 && y < height) {
			NtPixelf result;
			result.r = textureData[y][x].r;
			result.g = textureData[y][x].g;
			result.b = textureData[y][x].b;
			result.a = textureData[y][x].a;
			return result;
		}
		std::cerr << "NtTexture: Invalid pixel lookup parameters! x: " << x << " y: " << y << " width: " << width << " height: " << height << "\n";
		return { 0, 0, 0 ,0 };
	}
};
NtPixelf NtTextureLookUp(float u, float v, const NtTexture& texture);

/*Constants*/
#define RED     0               /* array indices for color vector */
#define GREEN   1
#define BLUE    2

#define X       0               /* array indices for position vector */
#define Y       1
#define Z       2

#define NT_SUCCESS      0
#define NT_FAILURE      1
#define NT_PI 3.1415926
#define EPSILON 1e-6
enum NT_SHADING_MODE {
	NT_SHADE_FLAT,
	NT_SHADE_PHONG,
	NT_SHADE_GOURAUD
};

enum NT_LIGHT_TYPE {
	NT_LIGHT_AMBIENT,
	NT_LIGHT_DIRECTIONAL
};

struct NtMatrix;
struct Vector3;
struct Vector4;

struct Vector4 {
	union {
		struct {
			float x, y, z, w;

		};
		float v[4];
	};

	Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}

	// Constructor for initializing Vector3 from individual x, y, and z values
	Vector4(float xVal, float yVal, float zVal, float wVal) : x(xVal), y(yVal), z(zVal), w(wVal) {}

	// Constructor for initializing Vector3 from an array of 3 values
	Vector4(const float values[4]) : x(values[0]), y(values[1]), z(values[2]), w(values[3]) {}

	inline Vector4 operator*(const NtMatrix& mat);

};

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

	inline Vector3 operator*(const NtMatrix& mat);

	Vector3 operator/(const float scalar) const {
		return { x / scalar, y / scalar, z / scalar };
	}

	Vector3 operator*(const float scalar) const {
		return { x * scalar, y * scalar, z * scalar };
	}

	//Component wise multiplication
	Vector3 operator*(const Vector3& other) const {
		return { x * other.x, y * other.y, z * other.z };
	}

	// Vector subtraction
	Vector3 operator-(const Vector3& other) const {
		return { x - other.x, y - other.y, z - other.z };
	}

	Vector3 operator+(const Vector3& other) const {
		return { x + other.x, y + other.y, z + other.z };
	}

	// Normalize the vector
	void normalize() {
		float length = std::sqrt(x * x + y * y + z * z);
		x /= length; y /= length; z /= length;
	}

	// Cross product
	static Vector3 cross(const Vector3& a, const Vector3& b) {
		return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
	}

	Vector3 cross(const Vector3& other) {
		return { y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x };
	}

	// Dot product
	static float dot(const Vector3& a, const Vector3& b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	float dot(const Vector3& other) {
		return x * other.x + y * other.y + z * other.z;
	}

	float dot(const Vector3& other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	static Vector3 reflect(const Vector3& I, const Vector3& N) {
		float IDotN = I.dot(N);
		IDotN *= 2;
		Vector3 _N = N * IDotN;
		return I - _N;
	}
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

typedef struct NtMatrix {
	float m[4][4];

	// Overload the subscript operator to allow direct access to internal array
	float* operator[](int index) {
		return m[index];
	}
	const float* operator[](int index) const {
		return m[index];
	}

	NtMatrix operator*(const NtMatrix& other) const {
		NtMatrix result;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				result.m[i][j] = 0;
				for (int k = 0; k < 4; ++k) {
					result.m[i][j] += this->m[i][k] * other.m[k][j];
				}
			}
		}
		return result;
	}

	NtMatrix operator+(const NtMatrix& other) const {
		NtMatrix result;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				result.m[i][j] = this->m[i][j] + other.m[i][j];
			}
		}
		return result;
	}

	NtMatrix operator-(const NtMatrix& other) const {
		NtMatrix result;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				result.m[i][j] = this->m[i][j] - other.m[i][j];
			}
		}
		return result;
	}

	NtMatrix transpose() const {
		NtMatrix result;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				result.m[i][j] = this->m[j][i];
			}
		}
		return result;
	}
} NtMatrix;

struct NtLight {
	NT_LIGHT_TYPE type;
	Vector3 color;
	float intensity;

	//For directional
	Vector3 direction;
};

/*Rendering*/
typedef struct {
	unsigned short	xRes;
	unsigned short	yRes;
	short			open;
	NtPixel* frameBuffer;		/* frame buffer array */
} NtDisplay;

typedef struct NtVertex {
	Vector3 vertexPos;
	Vector3 vertexNormal;
	Vector2 texture;
}NtVertex;

typedef struct NtTriangle {
	NtVertex v0;
	NtVertex v1;
	NtVertex v2;
} NtTriangle;

typedef struct NtMesh {
	std::vector<NtTriangle> triangles;
};

//Perspective & Camera
typedef struct NtCamera
{
	NtMatrix        viewMatrix;		/* world to image space */
	NtMatrix        projectMatrix;  /* perspective projection */
	Vector3 viewDirection;
	Vector3 from;
	Vector3 to;
	float near, far, right, left, top, bottom;
	int xRes;
	int yRes;
} NzCamera;
/*Renderer*/
typedef struct {
	NtDisplay* display;
	float** zBuffer;
	NtCamera* camera;
	std::vector<NtMatrix> matrixStack;
	NtMatrix worldMatrix; //Object to world
	NtMatrix worldMatrixInverseTransposed; //For normal
	NT_SHADING_MODE shadingMode;
	std::vector<NtLight> lights;
	NtLight directionalLight;
	NtLight ambientLight;
}  NtRender;


//User input (provide transformations)
typedef struct  NtInput
{
	Vector3         rotation;       /* object rotation */
	Vector3			translation;	/* object translation */
	Vector3			scale;			/* object scaling */
	NtCamera		camera;			/* camera */
} NtInput;

//Operator overloads
inline Vector3 Vector3::operator*(const NtMatrix& mat) {
	float x = mat.m[0][0] * this->x + mat.m[0][1] * this->y + mat.m[0][2] * this->z + mat.m[0][3];
	float y = mat.m[1][0] * this->x + mat.m[1][1] * this->y + mat.m[1][2] * this->z + mat.m[1][3];
	float z = mat.m[2][0] * this->x + mat.m[2][1] * this->y + mat.m[2][2] * this->z + mat.m[2][3];
	float w = mat.m[3][0] * this->x + mat.m[3][1] * this->y + mat.m[3][2] * this->z + mat.m[3][3];

	return Vector3(x, y, z);
}

inline Vector4 Vector4::operator*(const NtMatrix& mat) {
	float x = mat.m[0][0] * this->x + mat.m[0][1] * this->y + mat.m[0][2] * this->z + mat.m[0][3] * this->w;
	float y = mat.m[1][0] * this->x + mat.m[1][1] * this->y + mat.m[1][2] * this->z + mat.m[1][3] * this->w;
	float z = mat.m[2][0] * this->x + mat.m[2][1] * this->y + mat.m[2][2] * this->z + mat.m[2][3] * this->w;
	float w = mat.m[3][0] * this->x + mat.m[3][1] * this->y + mat.m[3][2] * this->z + mat.m[3][3] * this->w;

	return Vector4(x, y, z, w);
}

//Scene
typedef struct NtTransformation {
	Vector3 scale = Vector3(1, 1, 1);
	//Each x, y, z is rotation around that axis in degrees
	Vector3 rotation;
	Vector3 translation;
}NtTransformation;
typedef struct NtMaterial {
	Vector3 surfaceColor;
	float Ka;
	float Kd;
	float Ks;
	float Kt = 0.7f;
	float specularExponent;
	std::string textureId;
	NtTexture* texture;
} NtMaterial;
typedef struct NtShape
{
	std::string id;
	std::string geometryId;
	std::string notes;
	NtMaterial material;
	NtTransformation transforms;

} NtShape;

typedef struct  NtScene
{
	std::vector<NtShape> shapes;
	NtCamera camera;
	std::unordered_map<std::string, NtMesh*> meshMap;
	std::unordered_map<std::string, NtTexture*> textureMap;
	std::vector<NtLight> lights;
	NtLight directional;
	NtLight ambient;
} NtScene;

/*Core Functions*/
int NtSetShadingMode(NtRender* render, NT_SHADING_MODE mode);
int NtNewFrameBuffer(NtPixel** frameBuffer, int width, int height);
int NtNewDisplay(NtDisplay** display, int xRes, int yRes, Vector4 backgroundColor = { 0, 0, 0, 255 });
int NtFreeDisplay(NtDisplay* display);
int NtInitDisplay(NtDisplay* display, const Vector4& backgroundColor); //Default black
int NtFlushDisplayBufferPPM(FILE* outfile, NtDisplay* display);
int NtFlushDisplayBufferJPEG(FILE* outfile, NtDisplay* display);
int NtPutDisplay(NtDisplay* display, int i, int j, short r, short g, short b, short a);
int ClipInt(int input, int min, int max);
float Clipf(float input, int min, int max);
void ClipVec3(Vector3& vec);
int NtNewRender(NtRender** render, NtDisplay* display);
int NtFreeRender(NtRender* render);
int NtPutTriangle(NtRender* render, Vector3 vertexList[], Vector3 normalList[], Vector2 uvList[], const NtMaterial& material);
int NtPutTriangle(NtRender* render, NtTriangle& triangle, const NtMaterial& material);

//////Perspective, Matrix//////
void NtLoadIdentityMatrix(NtMatrix& mat);
void NtPushMatrix(NtRender* render, const NtMatrix& matrix);
void NtPopMatrix(NtRender* render);
int NtGetTopMatrix(NtRender* render, NtMatrix& mat);
int NtRotXMat(float degree, NtMatrix& mat);
int NtRotYMat(float degree, NtMatrix& mat);
int NtRotZMat(float degree, NtMatrix& mat);

int NtInvertRotMat(const NtMatrix& mat, NtMatrix& result);
int NtInvertScaleMat(const NtMatrix& mat, NtMatrix& result);
int NtInvertTranslateMat(const NtMatrix& mat, NtMatrix& result);

int NtTranslateMat(Vector3 translate, NtMatrix& mat);
int NtScaleMat(Vector3 scale, NtMatrix& mat);

//Camera
int NtCalculateViewMatrix(NtCamera& cameram, Vector3 u, Vector3 v, Vector3 n, Vector3 r);
int NtCalculateProjectionMatrix(NtCamera& camera, float near, float far, float top, float bottom, float left, float right);
int NtPutCamera(NtRender* render, NtCamera& camera);

int NtLoadSceneJSON(std::string scenePath, NtScene* scene, bool autoLoadMeshAndTexture = true);
int NtLoadTexture(NtMaterial& material, NtScene* scene);
int NtLoadMesh(std::string meshName, const std::string meshExtension, NtScene* scene);
int NtSetWorldMatrix(NtRender* render, NtMatrix& matrix, NtMatrix& matrixInverseTransposed);
int NtSetRenderAttributes(NtRender* render, NtScene* scene);
int NtRenderScene(NtScene* scene, const std::string outputName, NT_SHADING_MODE shadingMode = NT_SHADE_FLAT);

//Shading
Vector3 NtLightingPhong(const NtMaterial& material, const Vector3& normal, const NtLight& lightSource, const Vector3& viewDirection, const NtLight& ambientLight);
Vector3 NtAverageQuadNormals(const Vector3 normalList[]);
float NtInterpolate(const Vector3& vec, float alpha, float beta, float gamma);
Vector3 NtInterpolateVector3(const Vector3 vectors[], float alpha, float beta, float gamma, bool isNormal);
void NtTexturePixel(Vector3& color, const NtMaterial& material, Vector2 vertsUV[], Vector3 triVerts[], float alpha, float beta, float gamma);