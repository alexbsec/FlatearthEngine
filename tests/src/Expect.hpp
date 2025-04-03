#ifndef _FLATEARTH_TESTS_EXPECT_HPP
#define _FLATEARTH_TESTS_EXPECT_HPP

#include <Core/Logger.hpp>
#include <Math/FeMath.hpp>

/**
 * @brief Expects 'exected' to be equal to 'actual'
 */
#define ASSERT_EQ(expected, actual)                                            \
  if (actual != expected) {                                                    \
    FERROR("--> Expected %lld, but got: %lld. File: %s:%d.", expected, actual, \
           __FILE__, __LINE__);                                                \
    return FeFalse;                                                            \
  }

/**
 * @brief Expects 'expected' to not be equal to 'actual'
 */
#define ASSERT_NEQ(expected, actual)                                           \
  if (actual == expected) {                                                    \
    FERROR("--> Expected %d != %d, but they are equal. File: %s:%d.",          \
           expected, actual, __FILE__, __LINE__);                              \
    return FeFalse;                                                            \
  }

/**
 * @brief Expects float 'expected' to be equal float 'actual'
 */
#define ASSERT_FLOAT_EQ(expected, actual)                                      \
  if (flatearth::math::Abs(actual == expected) > 0.001f) {                     \
    FERROR("--> Expected %d != %d, but they are equal. File: %s:%d.",          \
           expected, actual, __FILE__, __LINE__);                              \
    return FeFalse;                                                            \
  }

#define ASSERT_TRUE(actual)                                                    \
  if (actual != FeTrue) {                                                      \
    FERROR("--> Expected true, but got: false. File: %s:%d.", __FILE__,        \
           __LINE__);                                                          \
    return FeFalse;                                                            \
  }

#define ASSERT_FALSE(actual)                                                   \
  if (actual != FeFalse) {                                                     \
    FERROR("--> Expected false, but got: true. File: %s:%d.", __FILE__,        \
           __LINE__);                                                          \
    return FeFalse;                                                            \
  }

#endif // _FLATEARTH_TESTS_EXPECT_HPP
