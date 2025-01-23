#include <core/Logger.hpp>

int main() {
  FFATAL("Test fatal message, %f", 3.14);
  FERROR("Test error message, %f", 3.14);
  FWARN("Test warn message, %f", 3.14);
  FDEBUG("Test debug message, %f", 3.14);
  FINFO("Test info message, %f", 3.14);
  FTRACE("Test trace message, %f", 3.14);
  return 0;
}
