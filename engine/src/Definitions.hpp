#ifndef _FLATEARTH_ENGINE_DEFINITIONS_HPP
#define _FLATEARTH_ENGINE_DEFINITIONS_HPP

#include <memory>
#include <sstream>
#include <string>

using unique_void_ptr = std::unique_ptr<void, void (*)(void const *)>;

namespace flatearth {
namespace core {
namespace memory {} // namespace memory
} // namespace core
} // namespace flatearth

// String alias
using string = std::string;
using vstring = std::string_view;
using osstream = std::ostringstream;

// Unsigned types
using uchar = unsigned char;
using ushort = unsigned short;
using uint32 = unsigned int;
using ulong = unsigned long;
using uint64 = unsigned long long;

// Signed types
using schar = signed char;
using sshort = signed short;
using sint32 = signed int;
using slong = signed long;
using sint64 = signed long long;

// Floating point types
using float32 = float;
using float64 = double;

#if defined(_MSVC_LANG)
#define STATIC_ASSERT static_assert
#elif defined(__cplusplus) && __cplusplus >= 201703L
#define STATIC_ASSERT static_assert
#else
#error "C++17 or newer needed to proceed"
#endif

// Ensure all types are of the correct size
STATIC_ASSERT(sizeof(uchar) == 1, "Expected uchar to be 1 byte");
STATIC_ASSERT(sizeof(ushort) == 2, "Expected ushort to be 2 bytes");
STATIC_ASSERT(sizeof(uint32) == 4, "Expected uint32 to be 4 bytes");
STATIC_ASSERT(sizeof(uint64) == 8, "Expected uint64 to be 8 bytes");

STATIC_ASSERT(sizeof(schar) == 1, "Expected schar to be 1 byte");
STATIC_ASSERT(sizeof(sshort) == 2, "Expected sshort to be 2 bytes");
STATIC_ASSERT(sizeof(sint32) == 4, "Expected sint32 to be 4 bytes");
STATIC_ASSERT(sizeof(sint64) == 8, "Expected sint64 to be 8 bytes");

STATIC_ASSERT(sizeof(float32) == 4, "Expected float32 to be 4 bytes");
STATIC_ASSERT(sizeof(float64) == 8, "Expected float64 to be 8 bytes");

#define FeTrue true
#define FeFalse false

// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define FEPLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required for windows"
#endif // _WIN64
#elif defined(__linux__) || defined(__gnu_linux__)
#define FEPLATFORM_LINUX 1
#if defined(__ANDROID__)
#define FEPLATFORM_ANDROID 1
#endif
#elif defined(__unix__)
#define FEPLATFORM_UNIX 1
#elif define(_POSIX_VERSION)
#define FEPLATFORM_POSIX 1
#else
#error "Unknown platform"
#endif

#ifdef FEXPORT
// Exports
#ifdef _MSC_VER
#define FEAPI __declspec(dllexport)
#else
#define FEAPI __attribute__((visibility("default")))
#endif
#else
// Imports
#ifdef _MSC_VER
#define FEAPI __declspec(dllimport)
#else
#define FEAPI
#endif
#endif

#ifndef _DEBUG
// Set to false if in release
#define _DEBUG FeTrue
#endif

#define FCLAMP(value, min, max)                                                \
  (value <= min) ? min : (value >= max) ? max : value;

// Inlining
#ifdef _MSC_VER
#define FINLINE __forceinline
#define FNOINLINE __declspec(noinline)
#else
#define FINLINE static inline
#define FNOINLINE
#endif

constexpr static float64 FE_PI = 3.14159265358979323846f;
constexpr static float64 FE_2_PI = 2 * FE_PI;
constexpr static float64 FE_HALF_PI = FE_PI / 2.0f;
constexpr static float64 FE_QUARTER_PI = FE_PI / 4.0f;
constexpr static float64 FE_1_OVER_PI = 1 / FE_PI;
constexpr static float64 FE_1_OVER_2_PI = 1 / FE_2_PI;
constexpr static float64 FE_SQRT_2 = 1.41421356237309504880f;
constexpr static float64 FE_SQRT_3 = 1.73205080756887729352f;
constexpr static float64 FE_1_OVER_SQRT_2 = 0.70710678118654752440f;
constexpr static float64 FE_1_OVER_SQRT_3 = 0.57735026918962576450f;
constexpr static float64 FE_DEG_TO_RAD_MUL = FE_PI / 180.0f;
constexpr static float64 FE_RAD_TO_DEG_MUL = 180.0f / FE_PI;

constexpr static float64 FE_MS_TO_SEC_MUL = 1 / 1000.0f;
constexpr static float64 FE_SEC_TO_MS_MUL = 1000.0f;

constexpr static float64 FE_F64MAX = 1e30f;
constexpr static float64 FE_F64EPS = 1.192092896e-7f;

#endif // _FLATEARTH_ENGINE_DEFINITIONS_HP
