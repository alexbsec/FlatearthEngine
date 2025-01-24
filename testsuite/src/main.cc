#include <Core/Logger.hpp>
#include <Platform/Platform.hpp>

using namespace flatearth::core::logger;
using namespace flatearth::platform;

int main() {

  Platform platform("Flatearth engine testsuite", 100, 100, 1280, 720);

  while (FeTrue) {
#if FEPLATFORM_WINDOWS
    platform.PollEvents();
#elif FEPLATFORM_LINUX
    if (!platform.PollEvents()) {
      break;
    }
#endif
  }

  return 0;
}
