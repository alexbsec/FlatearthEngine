#include <Core/Logger.hpp>
#include <Platform/Platform.hpp>

using namespace flatearth::core::logger;
using namespace flatearth::platform;

int main() {
  FFATAL("Test fatal message, %f", 3.14);
  FERROR("Test error message, %f", 3.14);
  FWARN("Test warn message, %f", 3.14);
  FDEBUG("Test debug message, %f", 3.14);
  FINFO("Test info message, %f", 3.14);
  FTRACE("Test trace message, %f", 3.14);

  Platform platform("Flatearth engine testsuite", 100, 100, 1280, 720);

  while (FeTrue) {
    if (!platform.WindowIsOpen()) {
    }
  }

  return 0;
}
