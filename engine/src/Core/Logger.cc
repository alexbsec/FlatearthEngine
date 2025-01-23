#include "Logger.hpp"
#include "Asserts.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <print>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

constexpr size_t LOGGER_BUFFER_SIZE = 32000;

namespace flatearth {
namespace core {
namespace logger {

Logger::Logger() {
  // TODO: create constructor
}

Logger::~Logger() {
  // TODO: create destructor
}

FEAPI void LogOutput(LogLevel level, const char *message, ...) {
  const string levelStrings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ",
                                  "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};

  bool isError = (level < LOG_LEVEL_WARN);

  char out_message[LOGGER_BUFFER_SIZE];
  memset(out_message, 0, sizeof(out_message));

  std::va_list argPtr;
  va_start(argPtr, message);
  std::vsnprintf(out_message, sizeof(out_message), message, argPtr);
  va_end(argPtr);

  string sOut = string(levelStrings[level]) + string(out_message);
  std::println("{}", sOut);
}

} // namespace logger
} // namespace core

namespace asserts {

FEAPI void ReportAssertionFailure(const string &expression,
                                  const string &message, const string &file,
                                  sint32 line) {
  core::logger::LogOutput(
      core::logger::LOG_LEVEL_FATAL,
      "Assertion Failure: %s, message: '%s', in file: %s, line: %d\n",
      expression.c_str(), message.c_str(), file.c_str(), line);
}

} // namespace asserts

} // namespace flatearth
