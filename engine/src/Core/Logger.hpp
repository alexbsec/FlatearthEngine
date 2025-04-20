#ifndef _FLATEARTH_ENGINE_LOGGER_HPP
#define _FLATEARTH_ENGINE_LOGGER_HPP

#include "Definitions.hpp"

#define LOG_WARN_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_TRACE_ENABLED 1

// Disable debug and trace logging for releases and build
#if FERELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

namespace flatearth {
namespace core {
namespace logger {

typedef enum LogLevel {
  LOG_LEVEL_FATAL = 0,
  LOG_LEVEL_ERROR = 1,
  LOG_LEVEL_WARN = 2,
  LOG_LEVEL_INFO = 3,
  LOG_LEVEL_DEBUG = 4,
  LOG_LEVEL_TRACE = 5,
} LogLevel;

struct LoggerSystemState {
  bool initialized; 
};

class Logger {
public:
  Logger();
  ~Logger();

  static constexpr uint64 SizeOfLoggerSystem();
  static constexpr uint64 AlignOfLoggerSystem();
  bool Init(uint64 *memoryRequirement, void *state);

private:
  LoggerSystemState *_loggerState = nullptr;
  bool _ownsMemory = false;
};

FEAPI void LogOutput(LogLevel level, const char *message, ...);

} // namespace logger
} // namespace core
} // namespace flatearth

#define FFATAL(message, ...)                                                   \
  flatearth::core::logger::LogOutput(flatearth::core::logger::LOG_LEVEL_FATAL, \
                                     message, ##__VA_ARGS__);

#ifndef FERROR
// Logs an error-level message
#define FERROR(message, ...)                                                   \
  flatearth::core::logger::LogOutput(flatearth::core::logger::LOG_LEVEL_ERROR, \
                                     message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
// Logs an warning-level message
#define FWARN(message, ...)                                                    \
  flatearth::core::logger::LogOutput(flatearth::core::logger::LOG_LEVEL_WARN,  \
                                     message, ##__VA_ARGS__);
#else
#define FWARN(message, ...)
#endif

#if LOG_INFO_ENABLED
// Logs an info-level message
#define FINFO(message, ...)                                                    \
  flatearth::core::logger::LogOutput(flatearth::core::logger::LOG_LEVEL_INFO,  \
                                     message, ##__VA_ARGS__);
#else
#define FINFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED
// Logs a trace-level message
#define FDEBUG(message, ...)                                                   \
  flatearth::core::logger::LogOutput(flatearth::core::logger::LOG_LEVEL_DEBUG, \
                                     message, ##__VA_ARGS__);
#else
#define FDEBUG(message, ...)

#endif
#if LOG_TRACE_ENABLED
// Logs a trace-level message
#define FTRACE(message, ...)                                                   \
  flatearth::core::logger::LogOutput(flatearth::core::logger::LOG_LEVEL_TRACE, \
                                     message, ##__VA_ARGS__);
#else
#define FTRACE(message, ...)
#endif
#endif // _FLATEARTH_ENGINE_LOGGER_HPP
