#ifndef _FLATEARTH_TESTS_EXPECT_HPP
#define _FLATEARTH_TESTS_EXPECT_HPP

#include <Core/Logger.hpp>
#include <Math/FeMath.hpp>

namespace flatearth {
namespace tests {

inline void* ToVoidPtr(std::nullptr_t) { return nullptr; }

template <typename T>
inline void* ToVoidPtr(T* ptr) { return static_cast<void*>(ptr); }

// Inteiros
#define ASSERT_EQ_INT(expected, actual)                                        \
  do {                                                                         \
    auto _expected = (expected);                                               \
    auto _actual = (actual);                                                   \
    if (_actual != _expected) {                                                \
      FERROR("--> Expected %lld, but got: %lld. File: %s:%d.",                 \
             static_cast<long long>(_expected),                                \
             static_cast<long long>(_actual), __FILE__, __LINE__);             \
      return FeFalse;                                                          \
    }                                                                          \
  } while (0)

#define ASSERT_NEQ_INT(expected, actual)                                       \
  do {                                                                         \
    auto _expected = (expected);                                               \
    auto _actual = (actual);                                                   \
    if (_actual == _expected) {                                                \
      FERROR(                                                                  \
          "--> Expected values to differ, but both were %lld. File: %s:%d.",   \
          static_cast<long long>(_actual), __FILE__, __LINE__);                \
      return FeFalse;                                                          \
    }                                                                          \
  } while (0)

// Ponteiros
#define ASSERT_EQ_PTR(expected, actual)                                        \
  do {                                                                         \
    auto _expected = (expected);                                               \
    auto _actual = (actual);                                                  \
    if (_actual != _expected) {                                                \
      FERROR("--> Expected %p, but got: %p. File: %s:%d.",                     \
             ToVoidPtr(_expected), ToVoidPtr(_actual), __FILE__, __LINE__);   \
      return FeFalse;                                                          \
    }                                                                          \
  } while (0)

#define ASSERT_NEQ_PTR(expected, actual)                                       \
  do {                                                                         \
    auto _expected = (expected);                                               \
    auto _actual = (actual);                                                   \
    if (_actual == _expected) {                                                \
      FERROR(                                                                  \
          "--> Expected different pointers, but both were %p. File: %s:%d.",   \
          reinterpret_cast<void *>(_actual), __FILE__, __LINE__);              \
      return FeFalse;                                                          \
    }                                                                          \
  } while (0)

// Floats
#define ASSERT_EQ_FLOAT(expected, actual)                                      \
  do {                                                                         \
    float _expected = (expected);                                              \
    float _actual = (actual);                                                  \
    if (flatearth::math::Abs(_actual - _expected) > 0.001f) {                  \
      FERROR("--> Expected %.5f, but got: %.5f. File: %s:%d.", _expected,      \
             _actual, __FILE__, __LINE__);                                     \
      return FeFalse;                                                          \
    }                                                                          \
  } while (0)

// Booleanos
#define ASSERT_TRUE(actual)                                                    \
  do {                                                                         \
    if ((actual) != FeTrue) {                                                  \
      FERROR("--> Expected true, but got false. File: %s:%d.", __FILE__,       \
             __LINE__);                                                        \
      return FeFalse;                                                          \
    }                                                                          \
  } while (0)

#define ASSERT_FALSE(actual)                                                   \
  do {                                                                         \
    if ((actual) != FeFalse) {                                                 \
      FERROR("--> Expected false, but got true. File: %s:%d.", __FILE__,       \
             __LINE__);                                                        \
      return FeFalse;                                                          \
    }                                                                          \
  } while (0)

/**
 * @brief Expects 'exected' to be equal to 'actual'
 */
#define ASSERT_EQ(expected, actual)                                            \
  if (actual != expected) {                                                    \
    FERROR("--> Expected %lld, but got: %lld. File: %s:%d.", expected, actual, \
           __FILE__, __LINE__);                                                \
    return FeFalse;                                                            \
  }

} // namespace tests
} // namespace flatearth

#endif // _FLATEARTH_TESTS_EXPECT_HPP
