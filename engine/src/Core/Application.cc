#include "Application.hpp"
#include "GameTypes.hpp"

namespace flatearth {
namespace core {
namespace application {

static bool initialized = FeFalse;

// Event handlers forward definition
bool AppOnEvent(events::SystemEventCode code, void* sender, void* listener, const events::EventContext& context);
bool AppOnKey(events::SystemEventCode code, void* sender, void* listener, const events::EventContext& context);

struct ApplicationState App::_appState = {};

App& App::GetInstance() {
  static App instance(_appState.gameInstance);
  return instance;
}

void App::SetGameInstance(struct gametypes::game* gameInstance) {
  _appState.gameInstance = gameInstance;
}

App::~App() {
  FINFO("App::~App(): shutting down application...");
  _eventManager.UnregisterEvent(events::SystemEventCode::EVENT_CODE_APPLICATION_QUIT, 0, AppOnEvent);
  _eventManager.UnregisterEvent(events::SystemEventCode::EVENT_CODE_KEY_PRESSED, 0, AppOnKey);
  _eventManager.UnregisterEvent(events::SystemEventCode::EVENT_CODE_KEY_RELEASED, 0, AppOnKey);
  initialized = FeFalse;
}

bool App::Init() {
  if (initialized) {
    FFATAL("App::Init(): app initializer was called more than once!");
    return FeFalse;
  }

  // Register the events
  _eventManager.RegisterEvent(events::SystemEventCode::EVENT_CODE_APPLICATION_QUIT, 0, AppOnEvent);
  _eventManager.RegisterEvent(events::SystemEventCode::EVENT_CODE_KEY_PRESSED, 0, AppOnKey);
  _eventManager.RegisterEvent(events::SystemEventCode::EVENT_CODE_KEY_RELEASED, 0, AppOnKey);


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

#if FEPLATFORM_WINDOWS
    _platform->PollEvents();
#elif FEPLATFORM_LINUX
    if (!_platform->PollEvents()) {
      FDEBUG("App::Run(): closing window was requested");
      _appState.isRunning = FeFalse;
    }
#endif

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

    _inputManager.Update(0);
  }

  _appState.isRunning = FeFalse;

  return FeTrue;
}

void App::ShutDown() {
  if (!initialized)
    return;

  _appState.isRunning = FeFalse;
}

// Private members

App::App(struct gametypes::game* gameInstance)
  : _eventManager(core::events::EventManager::GetInstance())
  , _inputManager(core::input::InputManager::GetInstance()) {

  _appState.gameInstance = gameInstance;

  // TODO: implement logger constructor
  _logger = std::make_unique<logger::Logger>();

  FINFO("App::App(): application was correctly initialized");
}


// Event handlers implementation
bool AppOnEvent(events::SystemEventCode code, void* sender, void* listener, const events::EventContext& context) {
  switch (code) {
  case events::SystemEventCode::EVENT_CODE_APPLICATION_QUIT:
    FINFO("AppOnEvent(): EVENT_CODE_APPLICATION_QUIT received, shutting down...");
    App::GetInstance().ShutDown();
    return FeTrue;
  default:
    break;
  }

  return FeFalse;
}

bool AppOnKey(events::SystemEventCode code, void* sender, void* listener, const events::EventContext& context) {
  if (code == events::SystemEventCode::EVENT_CODE_KEY_PRESSED) {
    std::array<ushort, 8> keyContext = context.get<std::array<ushort, 8>>();
    input::Keys keyCode = static_cast<input::Keys>(keyContext[0]);
    if (keyCode == input::Keys::KEY_ESCAPE) {
      events::EventContext data = {};
      events::EventManager::GetInstance().FireEvent(events::SystemEventCode::EVENT_CODE_APPLICATION_QUIT, 0, data);
      return FeTrue;
    }
    else if (keyCode == input::Keys::KEY_A) {
      FDEBUG("Explicit - A pressed");
    }
    else {
      FDEBUG("'%c' key pressed in window.", static_cast<ushort>(keyCode));
    }
  }
  else if (code == events::SystemEventCode::EVENT_CODE_KEY_RELEASED) {
    std::array<ushort, 8> keyContext = context.get<std::array<ushort, 8>>();
    input::Keys keyCode = static_cast<input::Keys>(keyContext[0]);
    if (keyCode == input::Keys::KEY_B) {
      FDEBUG("Explicit - B key released");
    }
    else {
      FDEBUG("'%c' key released in window.", static_cast<ushort>(keyCode));
    }
  }

  return FeFalse;
}

} // namespace application
} // namespace core
} // namespace flatearth
