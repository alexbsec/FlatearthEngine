#include "Application.hpp"
#include "Core/Event.hpp"
#include "GameTypes.hpp"
#include "Renderer/RendererFrontend.hpp"
#include "Renderer/RendererTypes.inl"

namespace flatearth {
namespace core {
namespace application {

struct ApplicationState App::_appState = {};

bool App::_initialized = FeFalse;

App &App::GetInstance() {
  static App instance(_appState.gameInstance);
  return instance;
}

void App::SetGameInstance(struct gametypes::Game *gameInstance) {
  _appState.gameInstance = gameInstance;
}

App::~App() {
  FINFO("App::~App(): shutting down application...");
  _eventManager.UnregisterEvent(
      events::SystemEventCode::EVENT_CODE_APPLICATION_QUIT, 0, OnEventCallback);
  _eventManager.UnregisterEvent(events::SystemEventCode::EVENT_CODE_KEY_PRESSED,
                                0, OnKeyCallback);
  _eventManager.UnregisterEvent(
      events::SystemEventCode::EVENT_CODE_KEY_RELEASED, 0, OnKeyCallback);
  _eventManager.UnregisterEvent(events::SystemEventCode::EVENT_CODE_RESIZED, 0,
                                OnResizedCallback);
  _initialized = FeFalse;
}

bool App::Init() {
  if (_initialized) {
    FFATAL("App::Init(): app initializer was called more than once!");
    return FeFalse;
  }

  OnEventCallback = [this](events::SystemEventCode code, void *sender,
                           void *listener,
                           const events::EventContext &context) {
    return OnEvent(code, sender, listener, context);
  };

  OnKeyCallback = [this](events::SystemEventCode code, void *sender,
                         void *listener, const events::EventContext &context) {
    return OnKey(code, sender, listener, context);
  };

  OnResizedCallback = [this](events::SystemEventCode code, void *sender,
                             void *listener,
                             const events::EventContext &context) {
    return OnResized(code, sender, listener, context);
  };

  // Register the events
  _eventManager.RegisterEvent(
      events::SystemEventCode::EVENT_CODE_APPLICATION_QUIT, 0, OnEventCallback);
  _eventManager.RegisterEvent(events::SystemEventCode::EVENT_CODE_KEY_PRESSED,
                              nullptr, OnKeyCallback);
  _eventManager.RegisterEvent(events::SystemEventCode::EVENT_CODE_KEY_RELEASED,
                              nullptr, OnKeyCallback);
  _eventManager.RegisterEvent(events::SystemEventCode::EVENT_CODE_RESIZED,
                              nullptr, OnResizedCallback);

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

  _appState.platform = _platform->GetState();

  try {
    _frontendRenderer = std::make_unique<renderer::FrontendRenderer>(
        _appState.gameInstance->appConfig.name, _appState.platform);
  } catch (const std::exception &e) {
    FFATAL("App::Init(): failed to create frontend renderer: %s", e.what());
    return FeFalse;
  }

  if (!_appState.gameInstance->Initialize(_appState.gameInstance)) {
    FFATAL("App::Init(): failed to initialize application");
    return FeFalse;
  }

  _appState.gameInstance->OnResize(_appState.gameInstance, _appState.width,
                                   _appState.height);

  _initialized = FeTrue;
  return FeTrue;
}

bool App::Run() {
  if (!_initialized) {
    FWARN("App::Run(): run method was called, but app was not initialized.");
    return FeFalse;
  }

  // Clock setup
  _appState.clock.Start();
  _appState.clock.Update();
  _appState.lastTime = _appState.clock.elapsed;
  float64 runningTime = 0.0f;
  uchar frameCount = 0;
  float64 targetFrameSeconds = 1.0f / 60;

  while (_appState.isRunning) {

    // TODO: fix this mess, it's actually ok for now but might complicate things
    // further
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

    _appState.clock.Update();
    float64 currentTime = _appState.clock.elapsed;
    float64 deltaTime = (currentTime - _appState.lastTime);
    float64 frameStartTime = platform::Platform::GetAbsoluteTime();

    if (!_appState.gameInstance->Update(_appState.gameInstance,
                                        (float32)deltaTime)) {
      FFATAL("App::Run(): game update failed, shutting down application...");
      break;
    }

    if (!_appState.gameInstance->Render(_appState.gameInstance,
                                        (float32)deltaTime)) {
      FFATAL("App::Run(): game render failed, shutting down application...");
      break;
    }

    // Hardcoded just to make it up and running
    // TODO: remove
    renderer::RenderPacket packet;
    packet.deltaTime = deltaTime;
    _frontendRenderer->DrawFrame(&packet);

    // Figure out how long the frame took
    float64 frameEndTime = platform::Platform::GetAbsoluteTime();
    float64 frameElapsedTime = frameEndTime - frameStartTime;
    runningTime += frameElapsedTime;
    float64 remainingSeconds = targetFrameSeconds - frameElapsedTime;

    if (remainingSeconds > 0.0f) {
      float64 remainingMilliseconds = (remainingSeconds * 1000);
      // Hardcoded for debugging purposes
      bool limitFrames = FeFalse;
      if (remainingMilliseconds > 0.0f && limitFrames) {
        platform::Platform::Sleep(remainingMilliseconds - 1);
      }

      // Increase frame count
      frameCount++;
    }

    _inputManager.Update(deltaTime);

    // Update last time
    _appState.lastTime = currentTime;
  }

  _appState.isRunning = FeFalse;

  return FeTrue;
}

void App::GetFrameBufferSize(uint32 *width, uint32 *height) {
  *width = _appState.width;
  *height = _appState.height;
}

void App::ShutDown() {
  if (!_initialized) {
    return;
  }

  _appState.isRunning = FeFalse;
}

// Private members

App::App(struct gametypes::Game *gameInstance)
    : _eventManager(core::events::EventManager::GetInstance()),
      _inputManager(core::input::InputManager::GetInstance()),
      _frontendRenderer(nullptr) {

  _appState.gameInstance = gameInstance;

  // TODO: implement logger constructor
  _logger = std::make_unique<logger::Logger>();

  FINFO("App::App(): application was correctly initialized");
}

bool App::OnEvent(events::SystemEventCode code, void *sender, void *listener,
                  const events::EventContext &context) {
  switch (code) {
  case events::SystemEventCode::EVENT_CODE_APPLICATION_QUIT:
    FINFO("App::AppOnEvent(): EVENT_CODE_APPLICATION_QUIT received, shutting "
          "down...");
    _appState.isRunning = FeFalse;
    return FeTrue;
  default:
    break;
  }

  return FeFalse;
}

bool App::OnKey(events::SystemEventCode code, void *sender, void *listener,
                const events::EventContext &context) {
  if (code == events::SystemEventCode::EVENT_CODE_KEY_PRESSED) {
    std::array<ushort, 8> keyContext = context.get<std::array<ushort, 8>>();
    input::Keys keyCode = static_cast<input::Keys>(keyContext[0]);
    if (keyCode == input::Keys::KEY_ESCAPE) {
      events::EventContext data = {};
      _eventManager.FireEvent(
          events::SystemEventCode::EVENT_CODE_APPLICATION_QUIT, 0, data);
      return FeTrue;
    } else if (keyCode == input::Keys::KEY_A) {
      FDEBUG("Explicit - A pressed");
    } else {
      FDEBUG("'%c' key pressed in window.", static_cast<ushort>(keyCode));
    }
  } else if (code == events::SystemEventCode::EVENT_CODE_KEY_RELEASED) {
    std::array<ushort, 8> keyContext = context.get<std::array<ushort, 8>>();
    input::Keys keyCode = static_cast<input::Keys>(keyContext[0]);
    if (keyCode == input::Keys::KEY_B) {
      FDEBUG("Explicit - B key released");
    } else {
      FDEBUG("'%c' key released in window.", static_cast<ushort>(keyCode));
    }
  }

  return FeFalse;
}

bool App::OnResized(events::SystemEventCode code, void *sender, void *listener,
                    const events::EventContext &context) {
  if (code != events::SystemEventCode::EVENT_CODE_RESIZED) {
    return FeFalse;
  }

  std::array<ushort, 8> changeContext = context.get<std::array<ushort, 8>>();
  ushort width = changeContext[0];
  ushort height = changeContext[1];

  // Check if different, if so trigger event
  if (width != _appState.width || height != _appState.height) {
    _appState.width = width;
    _appState.height = height;

    FDEBUG("App::OnResize(): window resizing: %i, %i", width, height);

    if (width == 0 || height == 0) {
      // Minimization
      FINFO("App::OnResize(): window minimized, suspending application.");
      _appState.isSuspended = FeTrue;
      return FeTrue;
    }

    if (_appState.isSuspended) {
      FINFO("App::OnResize(): window restored, resuming application.");
      _appState.isSuspended = FeFalse;
    }

    _appState.gameInstance->OnResize(_appState.gameInstance, width, height);
    _frontendRenderer->OnResize(width, height);
  }

  return FeFalse;
}

} // namespace application
} // namespace core
} // namespace flatearth
