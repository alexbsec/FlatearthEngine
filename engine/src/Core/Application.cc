#include "Application.hpp"
#include "GameTypes.hpp"
#include <stdexcept>

namespace flatearth {
namespace core {
namespace application {

static bool initialized = FeFalse;

App::App(struct gametypes::game *gameInstance) {
  if (initialized) {
    FFATAL("App constructor was called more than once!");
    throw std::runtime_error("App instance called more than once!");
  }

  _appState.gameInstance = gameInstance;

  // TODO: implement logger constructor
  _logger = std::make_unique<logger::Logger>();

  // TODO: remove this once fininshed
  FFATAL("Test fatal message, %f", 3.14);
  FERROR("Test error message, %f", 3.14);
  FWARN("Test warn message, %f", 3.14);
  FDEBUG("Test debug message, %f", 3.14);
  FINFO("Test info message, %f", 3.14);
  FTRACE("Test trace message, %f", 3.14);

  // Set the application as running and not suspended
  _appState.isRunning = FeTrue;
  _appState.isSuspended = FeFalse;

  try {
    _platform = std::make_unique<platform::Platform>(
        gameInstance->appConfig.name, gameInstance->appConfig.startPosX,
        gameInstance->appConfig.startPosY, gameInstance->appConfig.startWidth,
        gameInstance->appConfig.startHeight);
  } catch (const std::exception& e) {
    // TODO: think how to handle this error more gracefully
    FFATAL("Failed to create platform: %s", e.what());
    throw e;
  }
}

} // namespace application
} // namespace core
} // namespace flatearth
