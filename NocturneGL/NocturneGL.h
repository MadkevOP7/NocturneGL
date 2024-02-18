#pragma once
#include<iostream>
#include<vector>
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
#define NT_PI 3.1415926

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

	// Vector subtraction
	Vector3 operator-(const Vector3& other) const {
		return { x - other.x, y - other.y, z - other.z };
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

/*Rendering*/
typedef struct {
	unsigned short	xRes;
	unsigned short	yRes;
	short			open;
	NtPixel* frameBuffer;		/* frame buffer array */
} NtDisplay;

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
} NtMatrix;

//Perspective & Camera
typedef struct NtCamera
{
	NtMatrix        viewMatrix;		/* world to image space */
	NtMatrix        projectMatrix;  /* perspective projection */
} NzCamera;

/*Core Functions*/
int NtNewFrameBuffer(NtPixel** frameBuffer, int width, int height);
int NtNewDisplay(NtDisplay** display, int xRes, int yRes, Vector4 backgroundColor = { 0, 0, 0, 255 });
int NtFreeDisplay(NtDisplay* display);
int NtInitDisplay(NtDisplay* display, const Vector4& backgroundColor); //Default black
int NtFlushDisplayBufferPPM(FILE* outfile, NtDisplay* display);
int NtPutDisplay(NtDisplay* display, int i, int j, short r, short g, short b, short a);
int ClipInt(int input, int min, int max);
float Clipf(float input, int min, int max);

/*Renderer*/
typedef struct {
	NtDisplay* display;
	float** zBuffer;
	NtCamera* camera;
	std::vector<NtMatrix> matrixStack;
	NtMatrix mvp; //Model view perspective
}  NtRender;


//User input (provide transformations)
typedef struct  NtInput
{
	Vector3         rotation;       /* object rotation */
	Vector3			translation;	/* object translation */
	Vector3			scale;			/* object scaling */
	NtCamera		camera;			/* camera */
} NtInput;

int NtNewRender(NtRender** render, NtDisplay* display);
int NtFreeRender(NtRender* render);
int NtPutTriangle(NtRender* render, Vector3 vertexList[], Vector3 normalList[], Vector3 color);
//////Perspective, Matrix//////
void NtLoadIdentityMatrix(NtMatrix& mat);
void NtPushMatrix(NtRender* render, const NtMatrix& matrix);
void NtPopMatrix(NtRender* render);
int NtGetTopMatrix(NtRender* render, NtMatrix& mat);
int NtRotXMat(float degree, NtMatrix& mat);
int NtRotYMat(float degree, NtMatrix& mat);
int NtRotZMat(float degree, NtMatrix& mat);
int NtTranslateMat(Vector3 translate, NtMatrix& mat);
int NtScaleMat(Vector3 scale, NtMatrix& mat);

//Camera
int NtCalculateViewMatrix(NtCamera& cameram, Vector3 u, Vector3 v, Vector3 n, Vector3 r);
int NtCalculateProjectionMatrix(NtCamera& camera, float near, float far, float top, float bottom, float left, float right);
int NtPutCamera(NtRender* render, NtCamera& camera);

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