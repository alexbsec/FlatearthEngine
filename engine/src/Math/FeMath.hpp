#ifndef _FLATEARTH_ENGINE_FE_MATH_HPP
#define _FLATEARTH_ENGINE_FE_MATH_HPP

#include "Definitions.hpp"

// ------------------------------------------
// General math functions
// ------------------------------------------
namespace flatearht {
namespace math {

FEAPI float32 Sin(float32 x);
FEAPI float32 Cos(float32 x);
FEAPI float32 Tan(float32 x);
FEAPI float32 Arccos(float32 x);
FEAPI float32 Arcsin(float32 x);
FEAPI float32 Arctan(float32 x);
FEAPI float32 Sqrt(float32 x);
FEAPI float32 Abs(float32 x);

FINLINE bool IsPowerOf2(uint64 val) {
  return (val != 0) && ((val & (val - 1)) == 0);
}

FEAPI sint32 GetRandomInt();
FEAPI sint32 GetRandomInt(float32 min, float32 max);

FEAPI float32 GetRandomFloat();
FEAPI float32 GetRandomFloat(float32 min, float32 max);

}
} // namespace flatearht

#endif // _FLATEARTH_ENGINE_FE_MATH_HPP
