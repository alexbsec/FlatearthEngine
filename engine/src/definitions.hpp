#ifndef _FLATEARTH_ENGINE_DEFINITIONS_HPP
#define _FLATEARTH_ENGINE_DEFINITIONS_HPP

#include <string>

// String alias
using string = std::string;

// Unsigned types
using uchar = unsigned char;
using ushort = unsigned short;
using uint32 = unsigned int;
using uint64 = unsigned long long;

// Signed types
using schar = signed char;
using sshort = signed short;
using sint32 = signed int;
using sint64 = signed long long;

// Floating point types
using float32 = float;
using float64 = double;

// Properly define static assertions
#if defined(__cplusplus) && __cplusplus >= 201703L
#define STATIC_ASSERT static_assert
#else
// Fallback to old compilers
#define STATIC_ASSERT(condition, message)
typedef char static_assertion_##message[(condition) ? 1 : -1]
#endif // STATIC_ASSERT

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

#endif // _FLATEARTH_ENGINE_DEFINITIONS_HP
