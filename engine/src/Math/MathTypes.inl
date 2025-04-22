#ifndef _FLATEARTH_ENGINE_MATH_TYPES_INL
#define _FLATEARTH_ENGINE_MATH_TYPES_INL

#include "Core/Asserts.hpp"
#include "Core/Logger.hpp"
#include "Definitions.hpp"
#include "FeMath.hpp"

namespace flatearth {
namespace math {

// NOTE: this is only needed for SIMD 3D
constexpr static uint32 ALIGNMENT = 16;

enum AngleUnits {
  ANGLE_RADIANS,
  ANGLE_DEGREES,
};

class Vec3 {
public:
  union {
    struct {
      union {
        float32 x, r, s, u;
      };
      union {
        float32 y, g, t, v;
      };
      union {
        float32 z, b, p, w;
      };
    };
    float32 elements[3];
  };

  // Constructors
  Vec3() : elements{0.0f, 0.0f, 0.0f} {}
  Vec3(float32 x, float32 y, float32 z) : elements{x, y, z} {}

  // Zero creator
  FINLINE constexpr Vec3 Zero() { return Vec3(0.0f, 0.0f, 0.0f); }

  // One creator
  FINLINE constexpr Vec3 One() { return Vec3(1.0f, 1.0f, 1.0f); }

  // Up, Down, Left, Right, Forward & Back
  FINLINE constexpr Vec3 Up() { return Vec3(0.0f, 1.0f, 0.0f); }
  FINLINE constexpr Vec3 Down() { return Vec3(0.0f, -1.0f, 0.0f); }
  FINLINE constexpr Vec3 Left() { return Vec3(-1.0f, 0.0f, 0.0f); }
  FINLINE constexpr Vec3 Right() { return Vec3(1.0f, 0.0f, 0.0f); }
  FINLINE constexpr Vec3 Forward() { return Vec3(0.0f, 0.0f, -1.0f); }
  FINLINE constexpr Vec3 Backward() { return Vec3(0.0f, 0.0f, 1.0f); }

  // Operations
  FINLINE constexpr Vec3 Add(const Vec3 &firstVec, const Vec3 &secondVec) {
    return Vec3(firstVec.x + secondVec.x, firstVec.y + secondVec.y,
                firstVec.z + secondVec.z);
  }

  FINLINE constexpr Vec3 Subtract(const Vec3 &firstVec, const Vec3 &secondVec) {
    return Vec3(firstVec.x - secondVec.x, firstVec.y - secondVec.y,
                firstVec.z - secondVec.z);
  }

  FINLINE constexpr Vec3 Multiply(const Vec3 &firstVec, const Vec3 &secondVec) {
    return Vec3(firstVec.x * secondVec.x, firstVec.y * secondVec.y,
                firstVec.z * secondVec.z);
  }

  FINLINE constexpr Vec3 Divide(const Vec3 &firstVec, const Vec3 &secondVec) {
    if (secondVec.x == 0 || secondVec.y == 0 || secondVec.z == 0) {
      FWARN("Vec3::Divide(): Divison by zero");
      schar mulX = firstVec.x >= 0 ? 1 : -1;
      schar mulY = firstVec.y >= 0 ? 1 : -1;
      schar mulZ = firstVec.z >= 0 ? 1 : -1;
      return Vec3(mulX * FE_F64MAX, mulY * FE_F64MAX, mulZ * FE_F64MAX);
    }

    return Vec3(firstVec.x / secondVec.x, firstVec.y / secondVec.y,
                firstVec.z / secondVec.z);
  }

  FINLINE constexpr Vec3 ScalarMultiply(const Vec3 &vec, float32 scalar) {
    return Vec3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
  }

  FINLINE constexpr float32 SizeSquared(const Vec3 &vec) {
    return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
  }

  FINLINE constexpr float32 Size(const Vec3 &vec) {
    return Sqrt(SizeSquared(vec));
  }

  FINLINE constexpr void Normalize(Vec3 *vec) {
    const float32 LENGTH = Size(*vec);
    if (LENGTH > 0.0f) {
      vec->x /= LENGTH;
      vec->y /= LENGTH;
      vec->z /= LENGTH;
    } else {
      FWARN("Vec3::Normalize(): FINLINE attempt to normalize zero vector");
    }
  }

  FINLINE constexpr Vec3 Normalized(Vec3 vec) {
    Normalize(&vec);
    return vec;
  }

  FINLINE constexpr float32 DotProduct(const Vec3 &firstVec,
                                       const Vec3 &secondVec) {
    float32 p = 0;
    p += firstVec.x * secondVec.x;
    p += firstVec.y * secondVec.y;
    p += firstVec.z * secondVec.z;
    return p;
  }

  FINLINE constexpr Vec3 CurlProduct(const Vec3 &firstVec,
                                     const Vec3 &secondVec) {
    return Vec3(firstVec.y * secondVec.z - firstVec.z * secondVec.y,
                firstVec.z * secondVec.x - firstVec.x * secondVec.z,
                firstVec.x * secondVec.y - firstVec.y * secondVec.x);
  }

  FINLINE constexpr float32 DistanceOf(const Vec3 &firstVec,
                                       const Vec3 &secondVec) {
    Vec3 dist(firstVec.x - secondVec.x, firstVec.y - secondVec.y,
              firstVec.z - secondVec.z);

    return Size(dist);
  }

  FINLINE constexpr bool Equals(const Vec3 &firstVec, const Vec3 &secondVec,
                                float32 tolerance = FE_F64EPS) {
    return (Abs(firstVec.x - secondVec.x) <= tolerance &&
            Abs(firstVec.y - secondVec.y) <= tolerance &&
            Abs(firstVec.z - secondVec.z) <= tolerance);
  }

  inline constexpr float32 DotProduct(const Vec3 &other) {
    float p = 0;
    p += x * other.x;
    p += y * other.y;
    p += z * other.z;
    return p;
  }

  inline constexpr Vec3 CurlProduct(const Vec3 &other) {
    return Vec3(y * other.z - z * other.y, z * other.x - x * other.z,
                x * other.y - y * other.x);
  }

  // Operator Overloads
  inline constexpr Vec3 operator+(const Vec3 &other) const {
    return Vec3(x + other.x, y + other.y, z + other.z);
  }

  inline constexpr Vec3 operator-(const Vec3 &other) const {
    return Vec3(x - other.x, y - other.y, z - other.z);
  }

  inline constexpr Vec3 operator*(const Vec3 &other) const {
    return Vec3(x * other.x, y * other.y, z * other.z);
  }

  inline constexpr Vec3 operator*(float32 scalar) const {
    return Vec3(x * scalar, y * scalar, z * scalar);
  }

  inline constexpr Vec3 operator/(const Vec3 &other) const {
    if (other.x == 0.0f || other.y == 0.0f || other.z == 0.0f) {
      FWARN("Vec2::operator/: Division by zero");
      schar mulX = x >= 0 ? 1 : -1;
      schar mulY = y >= 0 ? 1 : -1;
      schar mulZ = z >= 0 ? 1 : -1;
      return Vec3(mulX * FE_F64MAX, mulY * FE_F64MAX, mulZ * FE_F64MAX);
    }

    return Vec3(x / other.x, y / other.y, z / other.z);
  }

  inline constexpr bool operator==(const Vec3 &other) const {
    return (Abs(x - other.x) <= FE_F64EPS && Abs(y - other.y) <= FE_F64EPS &&
            Abs(z - other.z) <= FE_F64EPS);
  }

  inline constexpr float &operator[](uint32 index) {
    FASSERT(index < _SIZE);
    return elements[index];
  }

  inline const float32 &operator[](uint32 index) const {
    FASSERT(index < _SIZE);
    return elements[index];
  }

  // Print for debugging
  inline constexpr string GetVecStr() const {
    osstream out;
    out << "Vec3(" << std::to_string(x) << ", " << std::to_string(y) << ", "
        << z << ")";
    return out.str();
  }

private:
  constexpr static uint32 _SIZE = 3;
};

class Vec2 {
public:
  union {
    struct {
      float32 x;
      float32 y;
    };
    struct {
      float32 r;
      float32 g;
    };
    struct {
      float32 s;
      float32 t;
    };
    struct {
      float32 u;
      float32 v;
    };
    float32 elements[2];
  };

  // Constructors
  Vec2() : elements{0.0f, 0.0f} {}
  Vec2(float32 x, float32 y) : elements{x, y} {}

  // Zero creator
  FINLINE constexpr Vec2 Zero() { return Vec2(0.0f, 0.0f); }

  // One creator
  FINLINE constexpr Vec2 One() { return Vec2(1.0f, 1.0f); }

  // Up, Down, Left & Right
  FINLINE constexpr Vec2 Up() { return Vec2(0.0f, 1.0f); }
  FINLINE constexpr Vec2 Down() { return Vec2(0.0f, -1.0f); }
  FINLINE constexpr Vec2 Left() { return Vec2(-1.0f, 0.0f); }
  FINLINE constexpr Vec2 Right() { return Vec2(1.0f, 0.0f); }

  // Operations
  FINLINE constexpr Vec2 Add(const Vec2 &firstVec, const Vec2 &secondVec) {
    return Vec2(firstVec.x + secondVec.x, firstVec.y + secondVec.y);
  }

  FINLINE constexpr Vec2 Subtract(const Vec2 &firstVec, const Vec2 &secondVec) {
    return Vec2(firstVec.x - secondVec.x, firstVec.y - secondVec.y);
  }

  FINLINE constexpr Vec2 Multiply(const Vec2 &firstVec, const Vec2 &secondVec) {
    return Vec2(firstVec.x * secondVec.x, firstVec.y * secondVec.y);
  }

  FINLINE constexpr Vec2 Divide(const Vec2 &firstVec, const Vec2 &secondVec) {
    if (secondVec.x == 0.0f || secondVec.y == 0.0f) {
      FWARN("Vec2::Divide(): Division by zero");
      schar mulX = firstVec.x >= 0 ? 1 : -1;
      schar mulY = firstVec.y >= 0 ? 1 : -1;
      return Vec2(mulX * FE_F64MAX, mulY * FE_F64MAX);
    }

    return Vec2(firstVec.x / secondVec.x, firstVec.y / secondVec.y);
  }

  FINLINE constexpr Vec2 ScalarMultiply(const Vec2 &vec, float32 scalar) {
    return Vec2(vec.x * scalar, vec.y * scalar);
  }

  FINLINE constexpr float32 SizeSquared(const Vec2 &vec) {
    return vec.x * vec.x + vec.y * vec.y;
  }

  FINLINE constexpr float32 Size(const Vec2 &vec) {
    return Sqrt(SizeSquared(vec));
  }

  FINLINE constexpr void Normalize(Vec2 *vec) {
    const float32 LENGTH = Size(*vec);
    if (LENGTH > 0.0f) {
      vec->x /= LENGTH;
      vec->y /= LENGTH;
    } else {
      FWARN("Vec2::Normalize(): FINLINE attempt to normalize a zero vector");
    }
  }

  FINLINE constexpr Vec2 Normalized(Vec2 vec) {
    Normalize(&vec);
    return vec;
  }

  FINLINE constexpr float32 DistanceOf(const Vec2 &firstVec,
                                       const Vec2 &secondVec) {
    Vec2 dist(firstVec.x - secondVec.x, firstVec.y - secondVec.y);
    return Size(dist);
  }

  FINLINE constexpr bool Equals(const Vec2 &firstVec, const Vec2 &secondVec,
                                float32 tolerance = FE_F64EPS) {
    return (Abs(firstVec.x - secondVec.x) <= tolerance &&
            Abs(firstVec.y - secondVec.y) <= tolerance);
  }

  FINLINE constexpr float32 DotProduct(const Vec2 &firstVec,
                                       const Vec2 &secondVec) {
    float32 p = 0;
    p += firstVec.x * secondVec.x;
    p += firstVec.y * secondVec.y;
    return p;
  }

  FINLINE constexpr Vec3 CurlProduct(const Vec2 &firstVec,
                                     const Vec2 &secondVec) {
    Vec3 firstVec3(firstVec.x, firstVec.y, 0.0f);
    Vec3 secondVec3(secondVec.x, secondVec.y, 0.0f);
    return Vec3::CurlProduct(firstVec3, secondVec3);
  }

  inline constexpr float32 SizeSquared() const { return x * x + y * y; }

  inline constexpr float32 Size() const { return Sqrt(SizeSquared()); }

  inline constexpr void Normalize() {
    const float32 LENGTH = Sqrt(x * x + y * y);
    if (LENGTH > 0.0f) {
      x /= LENGTH;
      y /= LENGTH;
    } else {
      FWARN("Vec2::Normalize(): attempt to normalize a zero vector");
    }
  }

  // Operator Overloads
  inline constexpr Vec2 operator+(const Vec2 &other) const {
    return Vec2(x + other.x, y + other.y);
  }

  inline constexpr Vec2 operator-(const Vec2 &other) const {
    return Vec2(x - other.x, y - other.y);
  }

  inline constexpr Vec2 operator-() const { return Vec2(-x, -y); }

  inline constexpr Vec2 operator*(const Vec2 &other) const {
    return Vec2(x * other.x, y * other.y);
  }

  inline constexpr Vec2 operator*(float32 scalar) const {
    return Vec2(x * scalar, y * scalar);
  }

  inline constexpr Vec2 operator/(const Vec2 &other) const {
    if (other.x == 0.0f || other.y == 0.0f) {
      FWARN("Vec2::operator/: Division by zero");
      schar mulX = x >= 0 ? 1 : -1;
      schar mulY = y >= 0 ? 1 : -1;
      return Vec2(mulX * FE_F64MAX, mulY * FE_F64MAX);
    }

    return Vec2(x / other.x, y / other.y);
  }

  inline constexpr bool operator==(const Vec2 &other) const {
    return (Abs(x - other.x) <= FE_F64EPS && Abs(y - other.y) <= FE_F64EPS);
  }

  inline constexpr float &operator[](uint32 index) {
    FASSERT(index < _SIZE);
    return elements[index];
  }

  inline const float32 &operator[](uint32 index) const {
    FASSERT(index < _SIZE);
    return elements[index];
  }

  inline constexpr string GetVecStr() const {
    osstream out;
    out << "Vec2(" << std::to_string(x) << ", " << std::to_string(y) << ")";
    return out.str();
  }

private:
  constexpr static uint32 _SIZE = 2;
};

class Vec2i {
public:
  float32 x, y;

  Vec2i() : x(0.0f), y(0.0f), _value(0.0f, 0.0f) {}
  Vec2i(float32 real, float32 imag) : x(real), y(imag), _value(real, imag) {}
  Vec2i(const Vec2 &base) : x(base.x), y(base.y), _value(base) {}

  FINLINE constexpr Vec2i Zero() { return Vec2i(0.0f, 0.0f); }

  FINLINE constexpr Vec2 ToVec2(const Vec2i &vec) {
    return Vec2(vec._value.x, vec._value.y);
  }

  inline constexpr Vec2 ToVec2() const { return Vec2(_value.x, _value.y); }

  // Accessors
  inline constexpr float32 Real() const { return _value.x; }

  inline constexpr float32 Imag() const { return _value.y; }

  inline constexpr Vec2i Conjugate() const { return Vec2i(_value.x, -_value.y); }

  inline constexpr Vec2i operator+(const Vec2i &other) const {
    return Vec2i(_value.x + other._value.x, _value.y + other._value.y);
  }

  inline constexpr Vec2i operator-(const Vec2i &other) const {
    return Vec2i(_value.x - other._value.x, _value.y - other._value.y);
  }

  inline constexpr Vec2i operator*(const Vec2i &other) const {
    return Vec2i(_value.x * other._value.x - _value.y * other._value.y,
                 _value.x * other._value.y + _value.y * other._value.x);
  }

  inline constexpr Vec2i operator*(float32 scalar) const {
    return Vec2i(scalar * _value.x, scalar * _value.y);
  }

  inline constexpr Vec2i operator/(const Vec2i &other) const {
    float32 denom =
        other._value.x * other._value.x + other._value.y * other._value.y;
    if (denom == 0.0f) {
      FWARN("Vec2i::operator/: Division by zero in complex division");
      return Vec2i(FE_F64MAX, FE_F64MAX);
    }

    return Vec2i((_value.x * other._value.x + _value.y * other._value.y) / denom,
                 (_value.y * other._value.x - _value.x * other._value.y) / denom);
  }

  inline constexpr bool operator==(const Vec2i &other) const {
    return _value.x == other._value.x && _value.y == other._value.y;
  }

  inline constexpr bool operator!=(const Vec2i &other) const {
    return _value.x != other._value.x || _value.y != other._value.y;
  }

  FINLINE constexpr float32 DotProduct(const Vec2i &vec1, const Vec2i &vec2) {
    return vec1._value.x * vec2._value.x + vec1._value.y * vec2._value.y;
  }

  inline constexpr float32 DotProduct(const Vec2i &other) const {
    return _value.x * other._value.x + _value.y * other._value.y;
  }

  inline constexpr Vec2i operator-() const { return Vec2i(-_value.x, -_value.y); }

  inline constexpr void Normalize() {
    float32 mag = Magnitude();
    if (mag > 0.0f) {
      _value.x /= mag;
      _value.y /= mag;
    } else {
      FWARN("Vec2i::Normalize(): attempt to normalize a zero complex number");
    }
  }

  inline constexpr Vec2i Normalized() const {
    float32 mag = Magnitude();
    if (mag > 0.0f) {
      return Vec2i(_value.x / mag, _value.y / mag);
    } else {
      FWARN("Vec2i::Normalized(): attempt to normalize a zero complex number");
      return *this;
    }
  }

  inline constexpr float32 Magnitude() const {
    return Sqrt(_value.x * _value.x + _value.y * _value.y);
  }

  inline constexpr float32 MaginitudeSquared() const {
    return _value.x * _value.x + _value.y * _value.y;
  }

  FINLINE Vec2i FromAngle(float32 angle, AngleUnits unit = ANGLE_RADIANS) {
    if (unit == ANGLE_DEGREES) {
      angle *= FE_PI / 180;
    }

    return Vec2i(Cos(angle), Sin(angle));
  }

  FINLINE constexpr Vec2 Rotate(const Vec2 &vec, float32 angle,
                                AngleUnits unit = ANGLE_RADIANS) {
    Vec2i rot = FromAngle(angle, unit);
    Vec2i vi(vec);
    return (rot * vi).ToVec2();
  }

  inline constexpr Vec2i Rotated(float32 angle,
                                 AngleUnits unit = ANGLE_RADIANS) const {
    return *this * FromAngle(angle, unit);
  }

  inline constexpr bool IsUnit(float32 eps = FE_F64EPS) const {
    return Abs(Magnitude() - 1.0f) <= eps;
  }

  string GetComplexStr() const {
    osstream out;
    out << "Vec2i(" << std::to_string(_value.x) << " + "
        << std::to_string(_value.y) << "i)";
    return out.str();
  }

private:
  Vec2 _value;
};

// NOTE: This might be unnecessary for a 2D engine, but I'll defined it
// anyways
class alignas(ALIGNMENT) Vec4 {
public:
  union {
#if defined(FUSE_SIMD)
    // Used for SIMD operations
    alignas(ALIGNMENT) __m128 data;
#endif
    struct {
      union {
        float32 x, r, s;
      };
      union {
        float32 y, g, t;
      };
      union {
        float32 z, b, p;
      };
      union {
        float32 w, a, q;
      };
    };
    float32 elements[4];
  };

  // Constructors
  Vec4() : elements{0.0f, 0.0f, 0.0f, 0.0f} {}
  Vec4(float32 x, float32 y, float32 z, float32 w) : elements{x, y, z, w} {}

  // Operator Overloads
  float &operator[](uint32 index) {
    FASSERT(index < SIZE);
    return elements[index];
  }

  const float32 &operator[](uint32 index) const {
    FASSERT(index < SIZE);
    return elements[index];
  }

  // Print for debugging
  string GetVecStr() const {
    osstream out;
    out << "Vec4(" << x << ", " << y << ", " << z << ", " << w << ")";
    return out.str();
  }

private:
  constexpr static uint32 SIZE = 4;
};

} // namespace math
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_MATH_TYPES_INL
