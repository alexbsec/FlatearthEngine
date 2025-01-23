#ifndef _FLATEARHT_ENGINE_ASSERTS_HPP
#define _FLATEARHT_ENGINE_ASSERTS_HPP

#include "Definitions.hpp"

// Enable assertions by defining FASSERTIONS_ENABLE.
#define FASSERTIONS_ENABLE

#ifdef FASSERTIONS_ENABLE

#if defined(_MSC_VER)
#include <intrin.h>
#define __debug_break() __debugBreak()
#else
#include <signal.h>
#define __debug_break() raise(SIGTRAP)
#endif

// Declaration for the assertion failure reporter.
FEAPI void ReportAssertionFailure(const string &expression,
                                  const string &message, const string file,
                                  sint32 line);

// FASSERT: if expr is false, report and break.
#define FASSERT(expr)                                                          \
  {                                                                            \
    if (!(expr)) {                                                             \
      ReportAssertionFailure(#expr, "", __FILE__, __LINE__);                   \
      __debug_break();                                                         \
    }                                                                          \
  }

// FASSERT_MSG: similar to FASSERT, but with a custom message.
#define FASSERT_MSG(expr, message)                                             \
  {                                                                            \
    if (!(expr)) {                                                             \
      ReportAssertionFailure(#expr, message, __FILE__, __LINE__);              \
      __debug_break();                                                         \
    }                                                                          \
  }

// FASSERT_DEBUG: active only in debug builds.
#ifdef _DEBUG
#define FASSERT_DEBUG(expr)                                                    \
  {                                                                            \
    if (!(expr)) {                                                             \
      ReportAssertionFailure(#expr, "", __FILE__, __LINE__);                   \
      __debug_break();                                                         \
    }                                                                          \
  }
#else
#define FASSERT_DEBUG(expr)
#endif

#else

// If assertions are disabled, expand macros to nothing.
#define FASSERT(expr)
#define FASSERT_MSG(expr, message)
#define FASSERT_DEBUG(expr)
#endif

#endif // _FLATEARHT_ENGINE_ASSERTS_HPP
