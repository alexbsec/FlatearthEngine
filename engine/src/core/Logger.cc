#include "Logger.hpp"
#include "Asserts.hpp"

#include <cstdarg>
#include <cstdio>
#include <print>
#include <cstdio>
#include <cstring>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

using namespace flatearth::logger;

constexpr size_t LOGGER_BUFFER_SIZE = 32000;

Logger::Logger() {
  // TODO: create constructor
}

Logger::~Logger() {
  // TODO: create destructor
}

FEAPI void LogOutput(LogLevel level, const char *message, ...) {
  const string levelStrings[6] = {
        "[FATAL]: ",
        "[ERROR]: ",
        "[WARN]:  ",
        "[INFO]:  ",
        "[DEBUG]: ",
        "[TRACE]: "
  };

  bool isError = (level < LOG_LEVEL_WARN);

  char out_message[LOGGER_BUFFER_SIZE];
  memset(out_message, 0, sizeof(out_message));

  std::va_list argPtr;
  va_start(argPtr, message);
  std::vsnprintf(out_message, sizeof(out_message), message, argPtr);
  va_end(argPtr);

  string sOut = string(levelStrings[level]) + string(out_message) + "\n";
  std::println("%s", sOut);
}
