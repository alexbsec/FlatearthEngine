#include "Logger.hpp"
#include "Asserts.hpp"
#include "Core/FeMemory.hpp"
#include "Platform/Platform.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstring>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

namespace flatearth {
namespace core {
namespace logger {

constexpr size_t LOGGER_BUFFER_SIZE = 32000;

Logger::Logger() {
  FINFO("Logger::Logger(): logger was correctly initialized");
}

Logger::~Logger() {
  FINFO("Logger::~Logger(): shutting down logger...");

  if (_ownsMemory && _loggerState) {
    core::memory::MemoryManager::Free(_loggerState, sizeof(LoggerSystemState),
                                      memory::MEMORY_TAG_APPLICATION);
    _loggerState = nullptr;
  }
}

constexpr uint64 Logger::SizeOfLoggerSystem() {
  return sizeof(LoggerSystemState);
}

constexpr uint64 Logger::AlignOfLoggerSystem() {
  return alignof(LoggerSystemState);
}

bool Logger::Init(uint64 *memoryRequirement, void *state) {
  *memoryRequirement = sizeof(LoggerSystemState);
  if (state == nullptr) {
    _loggerState = reinterpret_cast<LoggerSystemState *>(
        core::memory::MemoryManager::Allocate(
            *memoryRequirement, core::memory::MEMORY_TAG_APPLICATION));
    _ownsMemory = true;
    return FeTrue;
  } 

  _loggerState = reinterpret_cast<LoggerSystemState *>(state);
  _ownsMemory = false;
  _loggerState->initialized = FeTrue;
  return FeTrue;
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
  if (isError) {
    platform::Platform::ConsoleError(sOut, level);
  } else {
    platform::Platform::ConsoleWrite(sOut, level);
  }
}

} // namespace logger

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
} // namespace core
} // namespace flatearth
