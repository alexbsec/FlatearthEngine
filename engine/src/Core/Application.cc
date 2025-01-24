#include "Application.hpp"
#include "GameTypes.hpp"

namespace flatearth {
namespace core {
namespace application {

static bool initialized = FeFalse;

App::App(struct gametypes::game *gameInstance) {

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
}

App::~App() {
  initialized = FeFalse;
}

bool App::Init() {
  if (initialized) {
    FFATAL("App::Init(): app initializer was called more than once!");
    return FeFalse;
  }

  // Set the application as running and not suspended
  _appState.isRunning = FeTrue;
  _appState.isSuspended = FeFalse;

  try {
    _platform = std::make_unique<platform::Platform>(
        _appState.gameInstance->appConfig.name,
        _appState.gameInstance->appConfig.startPosX,
        _appState.gameInstance->appConfig.startPosY,
        _appState.gameInstance->appConfig.startWidth,
        _appState.gameInstance->appConfig.startHeight);
  } catch (const std::exception &e) {
    FFATAL("App::Init(): failed to create platform: %s", e.what());
    return FeFalse;
  }

  if (!_appState.gameInstance->Initialize(_appState.gameInstance)) {
    FFATAL("App::Init(): failed to initialize application");
    return FeFalse;
  }

  _appState.gameInstance->OnResize(_appState.gameInstance, _appState.width,
                                   _appState.height);

  initialized = FeTrue;
  return FeTrue;
}

bool App::Run() {
  if (!initialized) {
    FWARN("App::Run(): run method was called, but app was not initialized.");
    return FeFalse;
  }   

  while (_appState.isRunning) {
    if (!_platform->PollEvents()) {
      FDEBUG("App::Run(): closing window was requested");
      _appState.isRunning = FeFalse;
    }

    if (_appState.isSuspended) {
      continue;
    }

    if (!_appState.gameInstance->Update(_appState.gameInstance, (float32)0)) {
      FFATAL("App::Run(): game update failed, shutting down application...");
      break;
    }

    if (!_appState.gameInstance->Render(_appState.gameInstance, (float32)0)) {
      FFATAL("App::Run(): game render failed, shutting down application...");
      break;
    }
  }

  _appState.isRunning = FeFalse;

  return FeTrue;
}

} // namespace application
} // namespace core
} // namespace flatearth
