#ifndef _FLATEARTH_ENGINE_MATH_TYPES_INL
#define _FLATEARTH_ENGINE_MATH_TYPES_INL

#include "Definitions.hpp"

namespace flatearth {
namespace math {

class Vec2 {
public:
  union {
    struct {
      union {
        float x, r, s, u;
      };
      union {
        float y, g, t, v;
      };
    };
    float elements[2];
  };

  // Constructors
  Vec2() : x(0.0f), y(0.0f) {}
  Vec2(float x, float y) : x(x), y(y) {}

  // Operator Overloads
  float &operator[](size_t index) { return elements[index]; }
  const float &operator[](size_t index) const { return elements[index]; }

  // Print for debugging
  const char *Print() const {
    static char outPrintBuffer[32];
    std::snprintf(outPrintBuffer, sizeof(outPrintBuffer), "Vec2(%.2f, %.2f)", x,
                  y);
    return outPrintBuffer;
  }
};

} // namespace math
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_MATH_TYPES_INL
