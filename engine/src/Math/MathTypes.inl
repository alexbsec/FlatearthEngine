#ifndef _FLATEARTH_ENGINE_MATH_TYPES_INL
#define _FLATEARTH_ENGINE_MATH_TYPES_INL

#include "Core/Asserts.hpp"
#include "Definitions.hpp"

namespace flatearth {
namespace math {

// NOTE: this is only needed for SIMD 3D
constexpr static uint32 ALIGNMENT = 16;

class Vec2 {
public:
  union {
    struct {
      union {
        float32 x, r, s, u;
      };
      union {
        float32 y, g, t, v;
      };
    };
    float elements[2];
  };

  // Constructors
  Vec2() : elements{0.0f, 0.0f} {}
  Vec2(float32 x, float32 y) : elements{x, y} {}

  // Zero creator
  FINLINE Vec2 Zero() { return Vec2(0.0f, 0.0f); }

  // One creator
  FINLINE Vec2 One() { return Vec2(1.0f, 1.0f); }

  // Up, Down, Left & Right
  FINLINE Vec2 Up() { return Vec2(0.0f, 1.0f); }
  FINLINE Vec2 Down() { return Vec2(0.0f, -1.0f); }
  FINLINE Vec2 Left() { return Vec2(-1.0f, 0.0f); }
  FINLINE Vec2 Right() { return Vec2(1.0f, 0.0f); }

  // Operator Overloads
  float &operator[](uint32 index) {
    FASSERT(index < SIZE);
    return elements[index];
  }

  const float32 &operator[](uint32 index) const {
    FASSERT(index < SIZE);
    return elements[index];
  }

  string GetVecStr() const {
    osstream out;
    out << "Vec2(" << x << ", " << y << ")";
    return out.str();
  }

private:
  constexpr static uint32 SIZE = 2;
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
  FINLINE Vec3 Zero() { return Vec3(0.0f, 0.0f, 0.0f); }

  // One creator
  FINLINE Vec3 One() { return Vec3(1.0f, 1.0f, 1.0f); }

  // Up, Down, Left, Right, Forward & Back
  FINLINE Vec3 Up() { return Vec3(0.0f, 1.0f, 0.0f); }
  FINLINE Vec3 Down() { return Vec3(0.0f, -1.0f, 0.0f); }
  FINLINE Vec3 Left() { return Vec3(-1.0f, 0.0f, 0.0f); }
  FINLINE Vec3 Right() { return Vec3(1.0f, 0.0f, 0.0f); }
  FINLINE Vec3 Forward() { return Vec3(0.0f, 0.0f, -1.0f); }
  FINLINE Vec3 Backward() { return Vec3(0.0f, 0.0f, 1.0f); }

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
    out << "Vec3(" << x << ", " << y << ", " << z << ")";
    return out.str();
  }

private:
  constexpr static uint32 SIZE = 3;
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
