/*
nifly
C++ NIF library for the Gamebryo/NetImmerse File Format
See the included GPLv3 LICENSE file
*/

#pragma once

#include <cstdint>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <vector>

namespace nifly {
constexpr float EPSILON = 0.0001f;

constexpr float PI = 3.141592f;
constexpr float DEG2RAD = PI / 180.0f;

inline bool FloatsAreNearlyEqual(float a, float b) {
	float scale = std::max(std::max(std::fabs(a), std::fabs(b)), 1.0f);
	return std::fabs(a - b) <= EPSILON * scale;
}

float CalcMedianOfFloats(std::vector<float>& data);

template<class T>
static T Round(T a)
{
	static_assert(std::is_floating_point<T>::value, "Round<T>: T must be floating point");

	return (a > 0) ? ::floor(a + static_cast<T>(0.5)) : ::ceil(a - static_cast<T>(0.5));
}

template<class T>
static T Round(T a, int places)
{
	static_assert(std::is_floating_point<T>::value, "Round<T>: T must be floating point");

	const T shift = static_cast<T>(std::pow(static_cast<T>(10.0), places));

	return Round(a * shift) / shift;
}

// Vector with 2 float components (uv)
struct Vector2 {
	float u = 0.0f;
	float v = 0.0f;

	Vector2() = default;
	Vector2(float U, float V) {
		u = U;
		v = V;
	}

	bool operator==(const Vector2& other) {
		return u == other.u && v == other.v;
	}
	bool operator!=(const Vector2& other) { return !(*this == other); }

	Vector2& operator-=(const Vector2& other) {
		u -= other.u;
		v -= other.v;
		return (*this);
	}
	Vector2 operator-(const Vector2& other) const {
		Vector2 tmp = (*this);
		tmp -= other;
		return tmp;
	}

	Vector2& operator+=(const Vector2& other) {
		u += other.u;
		v += other.v;
		return (*this);
	}
	Vector2 operator+(const Vector2& other) const {
		Vector2 tmp = (*this);
		tmp += other;
		return tmp;
	}

	Vector2& operator*=(float val) {
		u *= val;
		v *= val;
		return (*this);
	}
	Vector2 operator*(float val) const {
		Vector2 tmp = (*this);
		tmp *= val;
		return tmp;
	}

	Vector2& operator/=(float val) {
		u /= val;
		v /= val;
		return (*this);
	}
	Vector2 operator/(float val) const {
		Vector2 tmp = (*this);
		tmp /= val;
		return tmp;
	}
};

// Vector with 3 float components (xyz)
struct Vector3 {
	float x;
	float y;
	float z;

	constexpr Vector3()
		: x(0.0f)
		, y(0.0f)
		, z(0.0f) {}

	constexpr Vector3(float X, float Y, float Z)
		: x(X)
		, y(Y)
		, z(Z) {}

	constexpr Vector3(const Vector3& rhs) {
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
	}

	constexpr float& operator[](int ind) { return ind ? (ind == 2 ? z : y) : x; }
	constexpr float operator[](int ind) const { return ind ? (ind == 2 ? z : y) : x; }

	void Zero() { x = y = z = 0.0f; }

	// With bUseEpsilon, uses nifly::EPSILON for a nearly zero comparison.
	constexpr bool IsZero(bool bUseEpsilon = false) const {
		if (bUseEpsilon) {
			if (std::fabs(x) < EPSILON && std::fabs(y) < EPSILON && std::fabs(z) < EPSILON)
				return true;
		}
		else {
			if (x == 0.0f && y == 0.0f && z == 0.0f)
				return true;
		}

		return false;
	}

	void Normalize() {
		float d = std::sqrt(x * x + y * y + z * z);
		if (d == 0.0f)
			d = 1.0f;

		x /= d;
		y /= d;
		z /= d;
	}

	void SetPrecision(unsigned int prec) {
		x = Round(x, prec);
		y = Round(y, prec);
		z = Round(z, prec);
	}

	uint32_t hash() const {
		static_assert(sizeof(float) == sizeof(uint32_t));
		const uint32_t* h = reinterpret_cast<const uint32_t*>(this);
		uint32_t f = (h[0] + h[1] * 11 - h[2] * 17) & 0x7fffffff;
		return (f >> 22) ^ (f >> 12) ^ (f);
	}

	constexpr bool operator==(const Vector3& other) const {
		return x == other.x && y == other.y && z == other.z;
	}
	constexpr bool operator!=(const Vector3& other) const { return !(*this == other); }

	Vector3& operator-=(const Vector3& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return (*this);
	}
	constexpr Vector3 operator-(const Vector3& other) const {
		return Vector3(x - other.x, y - other.y, z - other.z);
	}
	Vector3& operator+=(const Vector3& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return (*this);
	}
	constexpr Vector3 operator+(const Vector3& other) const {
		return Vector3(x + other.x, y + other.y, z + other.z);
	}
	[[deprecated("Replaced by ComponentMultiplyBy; should not have been an operator")]]
	Vector3& operator*=(const Vector3& other) {
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return (*this);
	}
	Vector3& ComponentMultiplyBy(const Vector3& other) {
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return (*this);
	}
	[[deprecated("Replaced by ComponentMultiply; should not have been an operator")]]
	constexpr Vector3 operator*(const Vector3& other) const {
		return Vector3(x * other.x, y * other.y, z * other.z);
	}
	constexpr Vector3 ComponentMultiply(const Vector3& other) const {
		return Vector3(x * other.x, y * other.y, z * other.z);
	}
	[[deprecated("Replaced by ComponentDivideBy; should not have been an operator")]]
	Vector3& operator/=(const Vector3& other) {
		x /= other.x;
		y /= other.y;
		z /= other.z;
		return (*this);
	}
	Vector3& ComponentDivideBy(const Vector3& other) {
		x /= other.x;
		y /= other.y;
		z /= other.z;
		return (*this);
	}
	[[deprecated("Replaced by ComponentDivide; should not have been an operator")]]
	constexpr Vector3 operator/(const Vector3& other) const {
		return Vector3(x / other.x, y / other.y, z / other.z);
	}
	constexpr Vector3 ComponentDivide(const Vector3& other) const {
		return Vector3(x / other.x, y / other.y, z / other.z);
	}

	Vector3& operator*=(float val) {
		x *= val;
		y *= val;
		z *= val;
		return (*this);
	}
	constexpr Vector3 operator*(float val) const {
		return Vector3(x * val, y * val, z * val);
	}
	Vector3& operator/=(float val) {
		x /= val;
		y /= val;
		z /= val;
		return (*this);
	}
	constexpr Vector3 operator/(float val) const {
		return Vector3(x / val, y / val, z / val);
	}

	Vector3& operator*=(int val) {
		auto v = static_cast<float>(val);
		x *= v;
		y *= v;
		z *= v;
		return (*this);
	}
	constexpr Vector3 operator*(int val) const {
		float v = static_cast<float>(val);
		return Vector3(x * v, y * v, z * v);
	}
	Vector3& operator/=(int val) {
		auto v = static_cast<float>(val);
		x /= v;
		y /= v;
		z /= v;
		return (*this);
	}
	constexpr Vector3 operator/(int val) const {
		float v = static_cast<float>(val);
		return Vector3(x / v, y / v, z / v);
	}

	Vector3& operator*=(uint32_t val) {
		auto v = static_cast<float>(val);
		x *= v;
		y *= v;
		z *= v;
		return (*this);
	}
	constexpr Vector3 operator*(uint32_t val) const {
		float v = static_cast<float>(val);
		return Vector3(x * v, y * v, z * v);
	}
	Vector3& operator/=(uint32_t val) {
		auto v = static_cast<float>(val);
		x /= v;
		y /= v;
		z /= v;
		return (*this);
	}
	constexpr Vector3 operator/(uint32_t val) const {
		float v = static_cast<float>(val);
		return Vector3(x / v, y / v, z / v);
	}

	Vector3& operator*=(uint64_t val) {
		auto v = static_cast<float>(val);
		x *= v;
		y *= v;
		z *= v;
		return (*this);
	}
	constexpr Vector3 operator*(uint64_t val) const {
		float v = static_cast<float>(val);
		return Vector3(x * v, y * v, z * v);
	}
	Vector3& operator/=(uint64_t val) {
		auto v = static_cast<float>(val);
		x /= v;
		y /= v;
		z /= v;
		return (*this);
	}
	constexpr Vector3 operator/(uint64_t val) const {
		float v = static_cast<float>(val);
		return Vector3(x / v, y / v, z / v);
	}

	constexpr Vector3 cross(const Vector3& other) const {
		Vector3 tmp;
		tmp.x = y * other.z - z * other.y;
		tmp.y = z * other.x - x * other.z;
		tmp.z = x * other.y - y * other.x;
		return tmp;
	}

	constexpr float dot(const Vector3& other) const { return x * other.x + y * other.y + z * other.z; }

	float DistanceTo(const Vector3& target) const {
		float dx = target.x - x;
		float dy = target.y - y;
		float dz = target.z - z;
		return static_cast<float>(std::sqrt(dx * dx + dy * dy + dz * dz));
	}

	constexpr float DistanceSquaredTo(const Vector3& target) const {
		float dx = target.x - x;
		float dy = target.y - y;
		float dz = target.z - z;
		return static_cast<float>(dx * dx + dy * dy + dz * dz);
	}

	float angle(const Vector3& other) const {
		Vector3 A(x, y, z);
		Vector3 B(other.x, other.y, other.z);
		A.Normalize();
		B.Normalize();

		float dot = A.dot(B);
		if (dot > 1.0f)
			return 0.0f;
		else if (dot < -1.0f)
			return PI;
		else if (dot == 0.0f)
			return PI / 2.0f;

		return std::acos(dot);
	}

	void clampEpsilon() {
		if (std::fabs(x) < EPSILON)
			x = 0.0f;
		if (std::fabs(y) < EPSILON)
			y = 0.0f;
		if (std::fabs(z) < EPSILON)
			z = 0.0f;
	}

	bool IsNearlyEqualTo(const Vector3& other) const {
		return FloatsAreNearlyEqual(x, other.x) && FloatsAreNearlyEqual(y, other.y)
			   && FloatsAreNearlyEqual(z, other.z);
	}

	constexpr float length2() const { return x * x + y * y + z * z; }

	float length() const { return std::sqrt(x * x + y * y + z * z); }

	float DistanceToSegment(const Vector3& p1, const Vector3& p2) const {
		Vector3 segvec(p2 - p1);
		Vector3 diffp1(*this - p1);
		float dp = segvec.dot(diffp1);
		if (dp <= 0)
			return diffp1.length();
		else if (dp >= segvec.length2())
			return (*this - p2).length();
		return segvec.cross(diffp1).length() / segvec.length();
	}
};

inline constexpr Vector3 operator*(float f, const Vector3& v) {
	return Vector3(f * v.x, f * v.y, f * v.z);
}

Vector3 CalcMedianOfVector3(const std::vector<Vector3>& data);

// Vector with 4 float components (xyzw)
struct Vector4 {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;

	constexpr Vector4() = default;
	constexpr Vector4(float X, float Y, float Z, float W)
		: x(X)
		, y(Y)
		, z(Z)
		, w(W) {}
};

// Color with 3 float components (rgb)
struct Color3 {
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;

	constexpr Color3() = default;
	constexpr Color3(float r_, float g_, float b_)
		: r(r_)
		, g(g_)
		, b(b_) {}

	constexpr bool operator==(const Color3& other) {
		return r == other.r && g == other.g && b == other.b;
	}
	constexpr bool operator!=(const Color3& other) { return !(*this == other); }

	Color3& operator*=(float val) {
		r *= val;
		g *= val;
		b *= val;
		return *this;
	}
	constexpr Color3 operator*(float val) const {
		return Color3(r * val, g * val, b * val);
	}

	Color3& operator/=(float val) {
		r /= val;
		g /= val;
		b /= val;
		return *this;
	}
	constexpr Color3 operator/(float val) const {
		return Color3(r / val, g / val, b / val);
	}
};

// Color with 4 float components (rgba)
struct Color4 {
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 0.0f;

	constexpr Color4() = default;
	constexpr Color4(float r_, float g_, float b_, float a_)
		: r(r_)
		, g(g_)
		, b(b_)
		, a(a_) {}

	constexpr bool operator==(const Color4& other) {
		return r == other.r && g == other.g && b == other.b && a == other.a;
	}
	constexpr bool operator!=(const Color4& other) { return !(*this == other); }

	Color4& operator*=(float val) {
		r *= val;
		g *= val;
		b *= val;
		a *= val;
		return *this;
	}
	constexpr Color4 operator*(float val) const {
		return Color4(r * val, g * val, b * val, a * val);
	}

	Color4& operator/=(float val) {
		r /= val;
		g /= val;
		b /= val;
		a /= val;
		return *this;
	}
	constexpr Color4 operator/(float val) const {
		return Color4(r / val, g / val, b / val, a / val);
	}
};

// Color with 3 byte components (rgb)
struct ByteColor3 {
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;

	bool operator==(const ByteColor3& other) { return (r == other.r && g == other.g && b == other.b); }
	bool operator!=(const ByteColor3& other) { return !(*this == other); }
};

// Color with 4 byte components (rgba)
struct ByteColor4 {
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	uint8_t a = 0;

	bool operator==(const ByteColor4& other) {
		return (r == other.r && g == other.g && b == other.b && a == other.a);
	}
	bool operator!=(const ByteColor4& other) { return !(*this == other); }
};

class Matrix3 {
	std::array<Vector3, 3> rows = {Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)};

public:
	constexpr Matrix3() {}
	constexpr Matrix3(const Vector3& r1, const Vector3& r2, const Vector3& r3)
		: rows{r1, r2, r3} {}
	constexpr Matrix3(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22)
		: rows{Vector3(m00, m01, m02), Vector3(m10, m11, m12), Vector3(m20, m21, m22)} {}

	constexpr Vector3& operator[](int index) { return rows[index]; }

	constexpr const Vector3& operator[](int index) const { return rows[index]; }

	constexpr bool operator==(const Matrix3& other) {
		return rows[0] == other[0] && rows[1] == other[1] && rows[2] == other[2];
	}

	constexpr bool IsIdentity() { return *this == Matrix3(); }

	Matrix3& Identity() {
		//1.0f, 0.0f, 0.0f
		//0.0f, 1.0f, 0.0f
		//0.0f, 0.0f, 1.0f

		rows[0].Zero();
		rows[1].Zero();
		rows[2].Zero();
		rows[0].x = 1.0f;
		rows[1].y = 1.0f;
		rows[2].z = 1.0f;
		return *this;
	}

	constexpr Matrix3 operator+(const Matrix3& other) const {
		return Matrix3(rows[0] + other[0], rows[1] + other[1], rows[2] + other[2]);
	}

	Matrix3& operator+=(const Matrix3& other) {
		*this = *this + other;
		return *this;
	}

	constexpr Matrix3 operator-(const Matrix3& other) const {
		return Matrix3(rows[0] - other[0], rows[1] - other[1], rows[2] - other[2]);
	}

	Matrix3& operator-=(const Matrix3& other) {
		*this = *this - other;
		return *this;
	}

	Matrix3& operator*=(const Matrix3& other) {
		*this = *this * other;
		return *this;
	}

	constexpr Matrix3 operator*(const Matrix3& o) const {
		Matrix3 res;
		res[0][0] = rows[0][0] * o[0][0] + rows[0][1] * o[1][0] + rows[0][2] * o[2][0];
		res[0][1] = rows[0][0] * o[0][1] + rows[0][1] * o[1][1] + rows[0][2] * o[2][1];
		res[0][2] = rows[0][0] * o[0][2] + rows[0][1] * o[1][2] + rows[0][2] * o[2][2];
		res[1][0] = rows[1][0] * o[0][0] + rows[1][1] * o[1][0] + rows[1][2] * o[2][0];
		res[1][1] = rows[1][0] * o[0][1] + rows[1][1] * o[1][1] + rows[1][2] * o[2][1];
		res[1][2] = rows[1][0] * o[0][2] + rows[1][1] * o[1][2] + rows[1][2] * o[2][2];
		res[2][0] = rows[2][0] * o[0][0] + rows[2][1] * o[1][0] + rows[2][2] * o[2][0];
		res[2][1] = rows[2][0] * o[0][1] + rows[2][1] * o[1][1] + rows[2][2] * o[2][1];
		res[2][2] = rows[2][0] * o[0][2] + rows[2][1] * o[1][2] + rows[2][2] * o[2][2];
		return res;
	}

	constexpr Vector3 operator*(const Vector3& v) const {
		return Vector3(rows[0][0] * v.x + rows[0][1] * v.y + rows[0][2] * v.z,
					   rows[1][0] * v.x + rows[1][1] * v.y + rows[1][2] * v.z,
					   rows[2][0] * v.x + rows[2][1] * v.y + rows[2][2] * v.z);
	}

	[[deprecated("Should not have been an operator; replace with multiplying by Vector3(f,f,f)")]]
	constexpr Vector3 operator*(const float f) const {
		return Vector3(rows[0][0] * f + rows[0][1] * f + rows[0][2] * f,
					   rows[1][0] * f + rows[1][1] * f + rows[1][2] * f,
					   rows[2][0] * f + rows[2][1] * f + rows[2][2] * f);
	}

	constexpr Matrix3 Transpose() const {
		Matrix3 res;
		res[0][0] = rows[0][0];
		res[0][1] = rows[1][0];
		res[0][2] = rows[2][0];
		res[1][0] = rows[0][1];
		res[1][1] = rows[1][1];
		res[1][2] = rows[2][1];
		res[2][0] = rows[0][2];
		res[2][1] = rows[1][2];
		res[2][2] = rows[2][2];
		return res;
	}

	void Zero() {
		for (auto& row : rows) {
			row.Zero();
		}
	}

	float Determinant() const;

	// Invert attempts to invert this matrix, returning the result in
	// inverse.  It returns false if the matrix is not invertible, in
	// which case inverse is not changed.
	bool Invert(Matrix3* inverse) const;

	// Inverse returns the inverse of this matrix if it's invertible.
	// If this matrix is not invertible, the identity matrix is returned.
	Matrix3 Inverse() const;

	// Generate rotation matrix from yaw, pitch and roll (in radians)
	// This is not the inverse of ToEulerAngles; though both functions
	// work with Euler angles, there are many conflicting definitions
	// of "Euler angles" (yaw, pitch, and roll), and these two functions
	// use different definitions.
	static Matrix3 MakeRotation(const float yaw, const float pitch, const float roll);

	// Convert rotation to euler degrees (Yaw, Pitch, Roll)
	// This function assumes that the matrix is a rotation matrix.
	// ToEulerAngles is not the inverse of MakeRotation; though both
	// functions work with Euler angles, there are many conflicting
	// definitions of "Euler angles", and these two functions use
	// different definitions.
	// The return result "canRot" apparently means roll is not zero.
	bool ToEulerAngles(float& y, float& p, float& r) const;
	bool ToEulerDegrees(float& y, float& p, float& r) const {
		bool canRot = ToEulerAngles(y, p, r);
		y *= 180.0f / PI;
		p *= 180.0f / PI;
		r *= 180.0f / PI;
		return canRot;
	}

	bool IsNearlyEqualTo(const Matrix3& other) const {
		return rows[0].IsNearlyEqualTo(other.rows[0]) && rows[1].IsNearlyEqualTo(other.rows[1])
			   && rows[2].IsNearlyEqualTo(other.rows[2]);
	}

	void SetPrecision(unsigned int prec) {
		for (auto& row : rows) {
			row.SetPrecision(prec);
		}
	}
};

// RotVecToMat: converts a rotation vector to a rotation matrix.
// (A rotation vector has direction the axis of the rotation
// and magnitude the angle of rotation.)
Matrix3 RotVecToMat(const Vector3& v);

// RotMatToVec: converts a rotation matrix into a rotation vector.
// (A rotation vector has direction the axis of the rotation
// and magnitude the angle of rotation.)
// Note that this function is unstable for angles near pi, but it
// should still work.
Vector3 RotMatToVec(const Matrix3& m);

Matrix3 CalcAverageRotation(const std::vector<Matrix3>& rots);
Matrix3 CalcMedianRotation(const std::vector<Matrix3>& rots);

// 4D Matrix class for calculating and applying transformations.
class Matrix4 {
	std::array<float, 16> m{};

public:
	constexpr Matrix4(): m{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f} {}

	Matrix4(const std::vector<Vector3>& mat33) { Set(mat33); }

	void Set(std::array<Vector3, 3> mat33) {
		m[0] = mat33[0].x;
		m[1] = mat33[0].y;
		m[2] = mat33[0].z;
		m[3] = 0;
		m[4] = mat33[1].x;
		m[5] = mat33[1].y;
		m[6] = mat33[1].z;
		m[7] = 0;
		m[8] = mat33[2].x;
		m[9] = mat33[2].y;
		m[10] = mat33[2].z;
		m[11] = 0;
		m[12] = 0;
		m[13] = 0;
		m[14] = 0;
		m[15] = 1;
	}

	void Set(const std::vector<Vector3>& mat33) {
		m[0] = mat33[0].x;
		m[1] = mat33[0].y;
		m[2] = mat33[0].z;
		m[3] = 0;
		m[4] = mat33[1].x;
		m[5] = mat33[1].y;
		m[6] = mat33[1].z;
		m[7] = 0;
		m[8] = mat33[2].x;
		m[9] = mat33[2].y;
		m[10] = mat33[2].z;
		m[11] = 0;
		m[12] = 0;
		m[13] = 0;
		m[14] = 0;
		m[15] = 1;
	}

	void SetRow(int row, const Vector3& inVec) {
		m[row * 4 + 0] = inVec.x;
		m[row * 4 + 1] = inVec.y;
		m[row * 4 + 2] = inVec.z;
	}

	constexpr float& operator[](int index) { return m[index]; }
	constexpr float operator[](int index) const { return m[index]; }

	bool operator==(const Matrix4& other) { return m == other.m; }

	bool IsIdentity() { return *this == Matrix4(); }

	Matrix4& Identity() {
		std::memset(&m, 0, sizeof(float) * 16);
		m[0] = m[5] = m[10] = m[15] = 1.0f;
		return *this;
	}

	void GetRow(int row, Vector3& outVec) {
		outVec.x = m[row * 4 + 0];
		outVec.y = m[row * 4 + 1];
		outVec.z = m[row * 4 + 2];
	}

	void Get33(float* o, int r = 3, int c = 3) {
		int p = 0;
		for (int i = 0; i < 4; i++) {
			if (i == r)
				continue;
			for (int j = 0; j < 4; j++) {
				if (j == c)
					continue;
				o[p++] = m[4 * i + j];
			}
		}
	}

	Matrix4 Inverse() {
		Matrix4 c;
		float det = Det();
		if (det == 0.0f) {
			c[0] = std::numeric_limits<float>::max();
			return c;
		}
		return (Adjoint() * (1.0f / det));
	}

	Matrix4 Cofactor() {
		Matrix4 c;
		float minor[9];
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				Get33(minor, i, j);
				c[4 * i + j] = Det33(minor);
			}
		}
		return c;
	}

	//  i&1   j&1	   xor
	//	0000 0101    0 1 0 1
	//	1111 0101    1 0 1 0
	//	0000 0101    0 1 0 1
	//	1111 0101    1 0 1 0
	// Adjoint is the transpose of the cofactor.
	Matrix4 Adjoint() {
		Matrix4 c;
		float minor[9];
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				Get33(minor, i, j);
				if ((i & 1) ^ (j & 1))
					c[i + j * 4] = -Det33(minor);
				else
					c[i + j * 4] = Det33(minor);
			}
		}
		return c;
	}

	float Det() {
		//  |a	b	c	d|	|0	1	2	3 |
		//	|e	f	g	h|	|4	5	6	7 |
		//	|i	j	k	l|	|8	9	10	11|
		//	|m	n	o	p|	|12 13	14	15|
		float A = m[0]
				  * ((m[5] * m[10] * m[15] + m[6] * m[11] * m[13] + m[7] * m[9] * m[14])
					 - (m[7] * m[10] * m[13] + m[6] * m[9] * m[15] + m[5] * m[11] * m[14]));
		float B = m[1]
				  * ((m[4] * m[10] * m[15] + m[6] * m[11] * m[12] + m[7] * m[8] * m[14])
					 - (m[7] * m[10] * m[12] + m[6] * m[8] * m[15] + m[4] * m[11] * m[14]));
		float C = m[2]
				  * ((m[4] * m[9] * m[15] + m[5] * m[11] * m[12] + m[7] * m[8] * m[13])
					 - (m[7] * m[9] * m[12] + m[5] * m[8] * m[15] + m[4] * m[11] * m[13]));
		float D = m[3]
				  * ((m[4] * m[9] * m[14] + m[5] * m[10] * m[12] + m[6] * m[8] * m[13])
					 - (m[6] * m[9] * m[12] + m[5] * m[8] * m[14] + m[4] * m[10] * m[13]));
		return A - B + C - D;
	}

	float Det33(float* t) {
		//  |a	b  c|	|0	1	2 |
		//	|d	e  f|	|3	4	5 |
		//	|g	h  i|	|6  7	8 |
		//  (aei + bfg + cdh) - (ceg + bdi + afh)
		//  (0*4*8 + 1*5*6 + 2*3*7) - (2*4*6 + 1*3*8 + 0*5*7)
		return ((t[0] * t[4] * t[8] + t[1] * t[5] * t[6] + t[2] * t[3] * t[7])
				- (t[2] * t[4] * t[6] + t[1] * t[3] * t[8] + t[0] * t[5] * t[7]));
	}

	constexpr Matrix4 operator+(const Matrix4& other) const {
		Matrix4 t(*this);
		for (int i = 0; i < 16; i++)
			t[i] += other[i];
		return t;
	}
	Matrix4& operator+=(const Matrix4& other) {
		for (int i = 0; i < 16; i++)
			m[i] += other.m[i];

		return (*this);
	}
	constexpr Matrix4 operator-(const Matrix4& other) const {
		Matrix4 t(*this);
		for (int i = 0; i < 16; i++)
			t[i] -= other[i];
		return t;
	}
	Matrix4& operator-=(const Matrix4& other) {
		for (int i = 0; i < 16; i++)
			m[i] -= other.m[i];

		return (*this);
	}
	constexpr Vector3 operator*(const Vector3& v) const {
		return Vector3(m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3],
					   m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7],
					   m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11]);
	}

	Matrix4& operator*=(const Matrix4& r) {
		float v1, v2, v3, v4;
		for (int n = 0; n < 16; n += 4) {
			v1 = m[n] * r.m[0] + m[n + 1] * r.m[4] + m[n + 2] * r.m[8] + m[n + 3] * r.m[12];
			v2 = m[n] * r.m[1] + m[n + 1] * r.m[5] + m[n + 2] * r.m[9] + m[n + 3] * r.m[13];
			v3 = m[n] * r.m[2] + m[n + 1] * r.m[6] + m[n + 2] * r.m[10] + m[n + 3] * r.m[14];
			v4 = m[n] * r.m[3] + m[n + 1] * r.m[7] + m[n + 2] * r.m[11] + m[n + 3] * r.m[15];
			m[n] = v1;
			m[n + 1] = v2;
			m[n + 2] = v3;
			m[n + 3] = v4;
		}
		return *this;
	}
	Matrix4 operator*(const Matrix4& other) {
		Matrix4 t(*this);
		t *= other;
		return t;
	}
	constexpr Matrix4 operator*(float val) {
		Matrix4 t(*this);
		for (int i = 0; i < 16; i++)
			t[i] *= val;

		return t;
	}

	void PushTranslate(const Vector3& byvec) {
		Matrix4 tmp;
		tmp.Translate(byvec);
		(*this) *= tmp;
	}

	Matrix4& Translate(const Vector3& byVec) { return Translate(byVec.x, byVec.y, byVec.z); }
	Matrix4& Translate(float x, float y, float z) {
		m[3] += x;
		m[7] += y;
		m[11] += z;
		return (*this);
	}

	void PushScale(float x, float y, float z) {
		Matrix4 tmp;
		tmp.Scale(x, y, z);
		(*this) *= tmp;
	}

	Matrix4& Scale(float x, float y, float z) {
		m[0] *= x;
		m[1] *= x;
		m[2] *= x;
		m[3] *= x;
		m[4] *= y;
		m[5] *= y;
		m[6] *= y;
		m[7] *= y;
		m[8] *= z;
		m[9] *= z;
		m[10] *= z;
		m[11] *= z;
		return (*this);
	}

	void PushRotate(float radAngle, const Vector3& axis) {
		Matrix4 tmp;
		tmp.Rotate(radAngle, axis);
		(*this) *= tmp;
	}

	Matrix4& Rotate(float radAngle, const Vector3& axis) { return Rotate(radAngle, axis.x, axis.y, axis.z); }

	Matrix4& Rotate(float radAngle, float x, float y, float z) {
		float c = std::cos(radAngle);
		float s = std::sin(radAngle);

		float xx = x * x;
		float xy = x * y;
		float xz = x * z;
		float yy = y * y;
		float yz = y * z;
		float zz = z * z;

		float ic = 1 - c;

		Matrix4 t;
		t.m[0] = xx * ic + c;
		t.m[1] = xy * ic - z * s;
		t.m[2] = xz * ic + y * s;
		t.m[3] = 0.0f;

		t.m[4] = xy * ic + z * s;
		t.m[5] = yy * ic + c;
		t.m[6] = yz * ic - x * s;
		t.m[7] = 0.0f;

		t.m[8] = xz * ic - y * s;
		t.m[9] = yz * ic + x * s;
		t.m[10] = zz * ic + c;

		t.m[11] = t.m[12] = t.m[13] = t.m[14] = 0.0f;
		t.m[15] = 1.0f;

		*this = t * (*this);

		return (*this);
	}

	Matrix4& Align(const Vector3& sourceVec, const Vector3& destVec) {
		Identity();
		float angle = sourceVec.angle(destVec);
		Vector3 axis = sourceVec.cross(destVec);
		axis.Normalize();

		return Rotate(angle, axis);
	}
};


struct BoundingSphere {
	Vector3 center;
	float radius = 0.0f;

	constexpr BoundingSphere() {}

	constexpr BoundingSphere(const Vector3& center_, const float radius_)
		: center(center_)
		, radius(radius_) {}

	// Miniball algorithm
	BoundingSphere(const std::vector<Vector3>& vertices);
};


// Quaternion using float components (wxyz)
struct Quaternion {
	float w;
	float x;
	float y;
	float z;

	constexpr Quaternion()
		: w(1.0f)
		, x(0.0f)
		, y(0.0f)
		, z(0.0f) {}

	constexpr Quaternion(float w_, float x_, float y_, float z_)
		: w(w_)
		, x(x_)
		, y(y_)
		, z(z_) {}
};

// Quaternion using float components (xyzw)
struct QuaternionXYZW {
	float x;
	float y;
	float z;
	float w;

	constexpr QuaternionXYZW()
		: x(0.0f)
		, y(0.0f)
		, z(0.0f)
		, w(1.0f) {}

	constexpr QuaternionXYZW(float x_, float y_, float z_, float w_)
		: x(x_)
		, y(y_)
		, z(z_)
		, w(w_) {}
};


struct QuatTransform {
	Vector3 translation;
	Quaternion rotation;
	float scale = 1.0f;
};

struct MatTransform {
	/* On MatTransform and coordinate-system (CS) transformations:

	A MatTransform can represent a "similarity transform", where
	it scales, rotates, and moves geometry; or it can represent a
	"coordinate-system transform", where the geometry itself does
	not change, but its representation changes from one CS to another.

	If CS1 is the source CS and CS2 is the target CS, then:
	ApplyTransform(v) converts a point v represented in CS1 to CS2.
	translation is CS1's origin represented in CS2.
	rotation has columns the basis vectors of CS1 represented in CS2.
	scale gives how much farther apart points appear to be in CS2 than in CS1.

	Note that we do not force "rotation" to actually be a rotation
	matrix.  A rotation matrix's inverse is its transpose.  Instead,
	we only assume "rotation" is invertible, which means its inverse
	must be calculated (using Matrix3::Invert).  Even though we always
	treat "rotation" as a general invertible matrix and not a rotation
	matrix, in practice it is always a rotation matrix.
	*/
	Vector3 translation;
	Matrix3 rotation;	// must be invertible
	float scale = 1.0f; // must be nonzero

	void Clear() {
		translation.Zero();
		rotation.Identity();
		scale = 1.0f;
	}

	// Rotation in euler degrees (Yaw, Pitch, Roll)
	bool ToEulerDegrees(float& y, float& p, float& r) const { return rotation.ToEulerDegrees(y, p, r); }

	// Full matrix of translation, rotation and scale
	constexpr Matrix4 ToMatrix() const {
		Matrix4 mat;
		mat[0] = rotation[0].x * scale;
		mat[1] = rotation[0].y * scale;
		mat[2] = rotation[0].z * scale;
		mat[3] = translation.x;
		mat[4] = rotation[1].x * scale;
		mat[5] = rotation[1].y * scale;
		mat[6] = rotation[1].z * scale;
		mat[7] = translation.y;
		mat[8] = rotation[2].x * scale;
		mat[9] = rotation[2].y * scale;
		mat[10] = rotation[2].z * scale;
		mat[11] = translation.z;
		return mat;
	}

	// ToGLMMatrix: turns this transform into a glm::mat4x4.  This is
	// basically the same as ToMatrix above, except glm::mat4x4 stores its
	// data in column-major form instead of row-major like everything else.
	// To call, do xform.ToGLMMatrix<glm::mat4x4>();
	template<typename Mat>
	constexpr Mat ToGLMMatrix() const {
		Mat m;
		m[0][0] = rotation[0][0] * scale;
		m[0][1] = rotation[1][0] * scale;
		m[0][2] = rotation[2][0] * scale;
		m[0][3] = 0.0f;
		m[1][0] = rotation[0][1] * scale;
		m[1][1] = rotation[1][1] * scale;
		m[1][2] = rotation[2][1] * scale;
		m[1][3] = 0.0f;
		m[2][0] = rotation[0][2] * scale;
		m[2][1] = rotation[1][2] * scale;
		m[2][2] = rotation[2][2] * scale;
		m[2][3] = 0.0f;
		m[3][0] = translation.x;
		m[3][1] = translation.y;
		m[3][2] = translation.z;
		m[3][3] = 1.0f;
		return m;
	}

	[[deprecated("Does something nonsensical")]]
	Vector3 GetVector() const { return translation + rotation * Vector3(scale, scale, scale); }

	// ApplyTransform applies this MatTransform to a position vector v by
	// first scaling v, then rotating the result of that, and then
	// translating the result of that.
	constexpr Vector3 ApplyTransform(const Vector3& pos) const {
		return translation + rotation * (pos * scale);
	}

	// ApplyTransformToDiff applies this transform to a position difference
	// (or offset) vector.
	constexpr Vector3 ApplyTransformToDiff(const Vector3& diff) const {
		return rotation * (diff * scale);
	}

	// ApplyTransformToDir applies this transform to a direction unit
	// vector or normal.
	constexpr Vector3 ApplyTransformToDir(const Vector3& dir) const {
		return rotation * dir;
	}

	// ApplyTransformToDist applies this transform to a distance.
	constexpr float ApplyTransformToDist(float d) const {
		return scale * d;
	}

	// Note that InverseTransform will return garbage if "rotation"
	// is not invertible or scale is 0.
	MatTransform InverseTransform() const;

	// ComposeTransforms returns the transform that is the composition
	// of this and other.  That is, if t3 = t1.ComposeTransforms(t2), then
	// t3.ApplyTransform(v) == t1.ApplyTransform(t2.ApplyTransform(v)).
	MatTransform ComposeTransforms(const MatTransform& other) const;

	bool IsNearlyEqualTo(const MatTransform& other) const {
		return translation.IsNearlyEqualTo(other.translation) && rotation.IsNearlyEqualTo(other.rotation)
			   && FloatsAreNearlyEqual(scale, other.scale);
	}
};

MatTransform CalcAverageMatTransform(const std::vector<MatTransform>& ts);
MatTransform CalcMedianMatTransform(const std::vector<MatTransform>& ts);


// Edge with uint16_t point indices
struct Edge {
	uint16_t p1;
	uint16_t p2;

	constexpr Edge(): p1(0), p2(0) {}
	constexpr Edge(uint16_t P1, uint16_t P2): p1(P1), p2(P2) {}

	constexpr bool CompareIndices(const Edge& o) { return (p1 == o.p1 && p2 == o.p2) || (p1 == o.p2 && p2 == o.p1); }
};

// Triangle with uint16_t point indices
struct Triangle {
	uint16_t p1;
	uint16_t p2;
	uint16_t p3;

	constexpr Triangle(): p1(0), p2(0), p3(0) {}
	constexpr Triangle(uint16_t P1, uint16_t P2, uint16_t P3): p1(P1), p2(P2), p3(P3) {}

	void set(uint16_t P1, uint16_t P2, uint16_t P3) {
		p1 = P1;
		p2 = P2;
		p3 = P3;
	}

	void trinormal(const Vector3* vertref, Vector3* outNormal) const {
		*outNormal = trinormal(vertref);
	}

	void trinormal(const std::vector<Vector3>& vertref, Vector3* outNormal) const {
		*outNormal = trinormal(&vertref[0]);
	}

	Vector3 trinormal(const Vector3* vertref) const {
		return (vertref[p2] - vertref[p1]).cross(vertref[p3] - vertref[p1]);
	}

	Vector3 trinormal(const std::vector<Vector3>& vertref) const {
		return trinormal(&vertref[0]);
	}

	void midpoint(const Vector3* vertref, Vector3& outPoint) {
		outPoint = vertref[p1];
		outPoint += vertref[p2];
		outPoint += vertref[p3];
		outPoint /= 3;
	}

	float AxisMidPointY(const Vector3* vertref) const { return (vertref[p1].y + vertref[p2].y + vertref[p3].y) / 3.0f; }

	float AxisMidPointX(const Vector3* vertref) const { return (vertref[p1].x + vertref[p2].x + vertref[p3].x) / 3.0f; }

	float AxisMidPointZ(const Vector3* vertref) const { return (vertref[p1].z + vertref[p2].z + vertref[p3].z) / 3.0f; }

	constexpr Edge GetEdge(int i) const {
		if (i == 0)
			return Edge(p1, p2);
		else if (i == 1)
			return Edge(p2, p3);
		else
			return Edge(p3, p1);
	}

	constexpr bool HasVertex(uint16_t p) const {
		return p == p1 || p == p2 || p == p3;
	}

	constexpr bool HasOrientedEdge(const Edge& e) const {
		return (e.p1 == p1 && e.p2 == p2) || (e.p1 == p2 && e.p2 == p3) || (e.p1 == p3 && e.p2 == p1);
	}

	Edge ClosestEdge(Vector3* vertref, const Vector3& p) const {
		float d1 = p.DistanceToSegment(vertref[p1], vertref[p2]);
		float d2 = p.DistanceToSegment(vertref[p2], vertref[p3]);
		float d3 = p.DistanceToSegment(vertref[p3], vertref[p1]);
		if (d1 <= d2 && d1 <= d3)
			return Edge(p1, p2);
		else if (d2 < d3)
			return Edge(p2, p3);
		return Edge(p3, p1);
	}

	uint16_t ClosestVertex(const Vector3* vertref, const Vector3& p) const {
		float d1 = p.DistanceTo(vertref[p1]);
		float d2 = p.DistanceTo(vertref[p2]);
		float d3 = p.DistanceTo(vertref[p3]);
		if (d1 <= d2 && d1 <= d3)
			return p1;
		else if (d2 <= d3)
			return p2;
		else
			return p3;
	}

	float DistanceToPoint(const Vector3* vertref, const Vector3& p) const {
		// Let pp be the projection of p onto the triangle's plane.
		// If pp is to the right of edge 1, then pp (and therefore p) is
		// closest to edge 1.  The same for edge 2 and edge 3.  Otherwise,
		// pp is inside the triangle.
		const Vector3& v1 = vertref[p1];
		const Vector3& v2 = vertref[p2];
		const Vector3& v3 = vertref[p3];
		Vector3 n = trinormal(vertref);
		if ((p - v1).dot((v2 - v1).cross(n)) >= 0)
			return p.DistanceToSegment(v1, v2);
		if ((p - v2).dot((v3 - v2).cross(n)) >= 0)
			return p.DistanceToSegment(v2, v3);
		if ((p - v3).dot((v1 - v3).cross(n)) >= 0)
			return p.DistanceToSegment(v3, v1);
		n.Normalize();
		return std::fabs((p - v1).dot(n));
	}

	bool IntersectRay(const Vector3* vertref,
					  const Vector3& origin,
					  const Vector3& direction,
					  float* outDistance = nullptr,
					  Vector3* worldPos = nullptr) {
		Vector3 c0(vertref[p1].x, vertref[p1].y, vertref[p1].z);
		Vector3 c1(vertref[p2].x, vertref[p2].y, vertref[p2].z);
		Vector3 c2(vertref[p3].x, vertref[p3].y, vertref[p3].z);

		Vector3 e1 = c1 - c0;
		Vector3 e2 = c2 - c0;
		float u, v;

		Vector3 pvec = direction.cross(e2);
		float det = e1.dot(pvec);

		if (det <= 0.0f)
			return false;

		Vector3 tvec = origin - c0;
		u = tvec.dot(pvec);
		if (u < 0 || u > det)
			return false;

		Vector3 qvec = tvec.cross(e1);
		v = direction.dot(qvec);
		if (v < 0 || u + v > det)
			return false;

		float dist = e2.dot(qvec);
		if (dist < 0)
			return false;

		dist *= (1.0f / det);

		if (outDistance)
			(*outDistance) = dist;
		if (worldPos)
			(*worldPos) = origin + (direction * dist);

		return true;
	}

	// Triangle/Sphere collision psuedocode by Christer Ericson: http://realtimecollisiondetection.net/blog/?p=103
	//   separating axis test on seven features --  3 points, 3 edges, and the tri plane.  For a sphere, this
	//   involves finding the minimum distance to each feature from the sphere origin and comparing it to the sphere radius.
	bool IntersectSphere(const Vector3* vertref, const Vector3& origin, float radius, float* outDistance = nullptr) {
		//A = A - P
		//B = B - P
		//C = C - P

		// Triangle points A,B,C.  translate them so the sphere's origin is their origin
		Vector3 A(vertref[p1].x, vertref[p1].y, vertref[p1].z);
		A = A - origin;
		Vector3 B(vertref[p2].x, vertref[p2].y, vertref[p2].z);
		B = B - origin;
		Vector3 C(vertref[p3].x, vertref[p3].y, vertref[p3].z);
		C = C - origin;

		//rr = r * r
		// Squared radius to avoid sqrts.
		float rr = radius * radius;
		//V = cross(B - A, C - A)

		// first test: tri plane.  Calculate the normal V
		Vector3 AB = B - A;
		Vector3 AC = C - A;
		Vector3 V = AB.cross(AC);
		//d = dot(A, V)
		//e = dot(V, V)
		// optimized distance test of the plane to the sphere -- removing sqrts and divides
		float d = A.dot(V);
		float e = V.dot(V); // e = squared normal vector length -- the normalization factor
		//sep1 = d * d > rr * e
		if (d * d > rr * e)
			return false;

		//aa = dot(A, A)
		//ab = dot(A, B)
		//ac = dot(A, C)
		//bb = dot(B, B)
		//bc = dot(B, C)
		//cc = dot(C, C)

		// second test: tri points.  A sparating axis exists if a point lies outside the sphere, and the other tri points aren't on the other side of the sphere.
		float aa = A.dot(A); // dist to point A
		float ab = A.dot(B);
		float ac = A.dot(C);
		float bb = B.dot(B); // dist to point B
		float bc = B.dot(C);
		float cc = C.dot(C); // dist to point C
		bool sep2 = (aa > rr) && (ab > aa) && (ac > aa);
		bool sep3 = (bb > rr) && (ab > bb) && (bc > bb);
		bool sep4 = (cc > rr) && (ac > cc) && (bc > cc);

		if (sep2 | sep3 | sep4)
			return false;

		//AB = B - A
		Vector3 BC = C - B;
		Vector3 CA = A - C;

		float d1 = ab - aa;
		d1 = A.dot(AB);
		float d2 = bc - bb;
		d2 = B.dot(BC);
		float d3 = ac - cc;
		d3 = C.dot(CA);

		//e1 = dot(AB, AB)
		//e2 = dot(BC, BC)
		//e3 = dot(CA, CA)
		float e1 = AB.dot(AB);
		float e2 = BC.dot(BC);
		float e3 = CA.dot(CA);

		//Q1 = A * e1 - d1 * AB
		//Q2 = B * e2 - d2 * BC
		//Q3 = C * e3 - d3 * CA
		//QC = C * e1 - Q1
		//QA = A * e2 - Q2
		//QB = B * e3 - Q3
		Vector3 Q1 = (A * e1) - (AB * d1);
		Vector3 Q2 = (B * e2) - (BC * d2);
		Vector3 Q3 = (C * e3) - (CA * d3);
		Vector3 QC = (C * e1) - Q1;
		Vector3 QA = (A * e2) - Q2;
		Vector3 QB = (B * e3) - Q3;

		//sep5 = [dot(Q1, Q1) > rr * e1 * e1] & [dot(Q1, QC) > 0]
		//sep6 = [dot(Q2, Q2) > rr * e2 * e2] & [dot(Q2, QA) > 0]
		//sep7 = [dot(Q3, Q3) > rr * e3 * e3] & [dot(Q3, QB) > 0]

		bool sep5 = (Q1.dot(Q1) > (rr * e1 * e1)) && (Q1.dot(QC) > 0);
		bool sep6 = (Q2.dot(Q2) > (rr * e2 * e2)) && (Q2.dot(QA) > 0);
		bool sep7 = (Q3.dot(Q3) > (rr * e3 * e3)) && (Q3.dot(QB) > 0);
		//separated = sep1 | sep2 | sep3 | sep4 | sep5 | sep6 | sep7
		if (sep5 | sep6 | sep7)
			return false;

		// Note that this calculation of outDistance does not give the
		// distance from the triangle to the point "origin"; it gives
		// the distance from "origin" to the nearest vertex of the triangle.
		// If you want the distance from the triangle to "origin",
		// Triangle::DistanceToPoint may be a better choice.
		if (outDistance)
			(*outDistance) = std::min({vertref[p1].DistanceTo(origin),
									   vertref[p2].DistanceTo(origin),
									   vertref[p3].DistanceTo(origin)});

		return true;
	}

	uint16_t& operator[](int ind) { return ind ? (ind == 2 ? p3 : p2) : p1; }
	const uint16_t& operator[](int ind) const { return ind ? (ind == 2 ? p3 : p2) : p1; }

	constexpr bool operator<(const Triangle& other) const {
		int d = 0;
		if (d == 0)
			d = p1 - other.p1;
		if (d == 0)
			d = p2 - other.p2;
		if (d == 0)
			d = p3 - other.p3;
		return d < 0;
	}

	constexpr bool operator==(const Triangle& other) const {
		return (p1 == other.p1 && p2 == other.p2 && p3 == other.p3);
	}

	constexpr bool CompareIndices(const Triangle& other) const {
		return ((p1 == other.p1 || p1 == other.p2 || p1 == other.p3)
				&& (p2 == other.p1 || p2 == other.p2 || p2 == other.p3)
				&& (p3 == other.p1 || p3 == other.p2 || p3 == other.p3));
	}

	void rot() {
		if (p2 < p1 && p2 < p3) {
			set(p2, p3, p1);
		}
		else if (p3 < p1) {
			set(p3, p1, p2);
		}
	}
};

inline constexpr bool operator==(const Edge& t1, const Edge& t2) {
	return ((t1.p1 == t2.p1) && (t1.p2 == t2.p2));
}

// Face with either 3 or 4 point and uv indices
struct Face {
	uint8_t nPoints = 0;
	uint16_t p1 = 0;
	uint16_t uv1 = 0;
	uint16_t p2 = 0;
	uint16_t uv2 = 0;
	uint16_t p3 = 0;
	uint16_t uv3 = 0;
	uint16_t p4 = 0;
	uint16_t uv4 = 0;

	Face(const uint8_t npts = 0, const uint16_t* points = nullptr, const uint16_t* tc = nullptr) {
		nPoints = npts;
		if (npts < 3)
			return;

		p1 = points[0];
		p2 = points[1];
		p3 = points[2];
		uv1 = tc[0];
		uv2 = tc[1];
		uv3 = tc[2];
		if (npts == 4) {
			p4 = points[3];
			uv4 = tc[3];
		}
	}
};

// Rectangle with float components (x1, y1, x2, y2)
struct Rect {
	float x1 = 0.0f;
	float y1 = 0.0f;
	float x2 = 0.0f;
	float y2 = 0.0f;

	Rect() {}

	Rect(float X1, float Y1, float X2, float Y2) {
		x1 = X1;
		y1 = Y1;
		x2 = X2;
		y2 = Y2;
	}

	float GetLeft() { return x1; }
	float GetTop() { return y1; }
	float GetRight() { return x2; }
	float GetBottom() { return y2; }

	Vector2 GetTopLeft() { return Vector2(x1, y1); }
	Vector2 GetBottomRight() { return Vector2(x2, y2); }
	Vector2 GetTopRight() { return Vector2(x2, y1); }
	Vector2 GetBottomLeft() { return Vector2(x1, y2); }

	Vector2 GetCenter() { return Vector2((x1 + x2) / 2, (y1 + y2) / 2); }

	float GetWidth() { return x2 - x1 + 1; }
	float GetHeight() { return y2 - y1 + 1; }
	Vector2 GetSize() { return Vector2(GetWidth(), GetHeight()); }

	void SetLeft(float pos) { x1 = pos; }
	void SetTop(float pos) { y1 = pos; }
	void SetRight(float pos) { x2 = pos; }
	void SetBottom(float pos) { y2 = pos; }

	void SetTopLeft(const Vector2& p) {
		x1 = p.u;
		y1 = p.v;
	}
	void SetBottomRight(const Vector2& p) {
		x2 = p.u;
		y2 = p.v;
	}
	void SetTopRight(const Vector2& p) {
		x2 = p.u;
		y1 = p.v;
	}
	void SetBottomLeft(const Vector2& p) {
		x1 = p.u;
		y2 = p.v;
	}

	void SetWidth(float w) { x2 = x1 + w - 1.0f; }
	void SetHeight(float h) { y2 = y1 + h - 1.0f; }

	Rect Normalized() {
		Rect r;

		if (x2 < x1) {
			r.x1 = x2;
			r.x2 = x1;
		}
		else {
			r.x1 = x1;
			r.x2 = x2;
		}

		if (y2 < y1) {
			r.y1 = y2;
			r.y2 = y1;
		}
		else {
			r.y1 = y1;
			r.y2 = y2;
		}

		return r;
	}

	bool Contains(const Vector2& p) {
		float l = 0.0f;
		float r = 0.0f;

		if (x2 < x1 - 1.0f) {
			l = x2;
			r = x1;
		}
		else {
			l = x1;
			r = x2;
		}

		if (p.u < l || p.u > r)
			return false;

		float t = 0.0f;
		float b = 0.0f;

		if (y2 < y1 - 1.0f) {
			t = y2;
			b = y1;
		}
		else {
			t = y1;
			b = y2;
		}

		if (p.v < t || p.v > b)
			return false;

		return true;
	}
};
} // namespace nifly

namespace std {
using namespace nifly;

template<>
struct hash<Edge> {
	std::size_t operator()(const Edge& t) const {
		return (static_cast<size_t>(t.p2) << 16) | (t.p1 & 0xFFFF);
	}
};

template<>
struct hash<Triangle> {
	std::size_t operator()(const Triangle& t) const {
		auto d = reinterpret_cast<const char*>(&t);
		std::size_t len = sizeof(Triangle);
		std::size_t hash, i;
		for (hash = i = 0; i < len; ++i) {
			hash += static_cast<size_t>(d[i]);
			hash += (hash << 10);
			hash ^= (hash >> 6);
		}
		hash += (hash << 3);
		hash ^= (hash >> 11);
		hash += (hash << 15);
		return hash;
	}
};
} // namespace std
