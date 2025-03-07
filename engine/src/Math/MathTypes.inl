#ifndef _FLATEARTH_ENGINE_MATH_TYPES_INL
#define _FLATEARTH_ENGINE_MATH_TYPES_INL

#include "Definitions.hpp"
#include "Core/Asserts.hpp"

namespace flatearth {
namespace math {

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

  // Operator Overloads
  float &operator[](uint32 index) { 
    FASSERT(index < 2);
    return elements[index]; 
  }

  const float32 &operator[](uint32 index) const { 
    FASSERT(index < 2);
    return elements[index]; 
  }

  string GetVecStr() const {
    osstream out;
    out << "Vec2(" << x << ", " << y << ")";
    return out.str();
  }
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
  
  // Operator Overloads
  float &operator[](uint32 index) {
    FASSERT(index < 3);
    return elements[index]; 
  }

  const float32 &operator[](uint32 index) const { 
    FASSERT(index < 3);
    return elements[index]; 
  }

  // Print for debugging
  string GetVecStr() const {
    osstream out;
    out << "Vec3(" << x << ", " << y << ", " << z << ")";
    return out.str();
  }
};

} // namespace math
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_MATH_TYPES_INL
