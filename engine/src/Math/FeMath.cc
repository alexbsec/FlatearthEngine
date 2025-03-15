#include "FeMath.hpp"
#include "Definitions.hpp"
#include "Platform/Platform.hpp"
#include <cmath>
#include <cstdlib>

namespace flatearth {
namespace math {

static bool randSeeded = FeFalse;

float32 Sin(float32 x) {
  return std::sinf(x);
}

float32 Cos(float32 x) {
  return std::cosf(x);
}

float32 Tan(float32 x) {
  return std::tanf(x);
}

float32 Arccos(float32 x) {
  return std::acosf(x);
}

float32 Arcsin(float32 x) {
  return std::asinf(x);
}

float32 Arctan(float32 x) {
  return std::atanf(x);
}

float32 Sqrt(float32 x) {
  return std::sqrtf(x);
}

float32 Abs(float32 x) {
  return std::fabsf(x);
}

sint32 GetRandomInt() {
  if (!randSeeded) {
    srand((uint32)platform::Platform::GetAbsoluteTime());
    randSeeded = FeTrue;
  }

  return rand();
}

sint32 GetRandomInt(sint32 min, sint32 max) {
  if (!randSeeded) {
    srand((uint32)platform::Platform::GetAbsoluteTime());
    randSeeded = FeTrue;
  }

  return (rand() % (max - min + 1)) + min;
}

float32 GetRandomFloat() {
  return (float32)GetRandomInt() / (float32)RAND_MAX;
}

float32 GetRandomFloat(float32 min, float32 max) {
  return min + ((float32)GetRandomInt() / ((float32)RAND_MAX / (max - min)));
}

} // namespace math
} // namespace flatearth
