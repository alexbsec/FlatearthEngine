#include "Application.hpp"
#include "Core/Event.hpp"
#include "Core/FeMemory.hpp"
#include "Core/Logger.hpp"
#include "GameTypes.hpp"
#include "Memory/LinearAllocator.hpp"
#include "Platform/Platform.hpp"
#include "Renderer/RendererFrontend.hpp"
#include "Renderer/RendererTypes.inl"

namespace flatearth {
namespace core {
namespace application {

const char *KeyToString(input::Keys key);

struct ApplicationState *App::_appState = {};

App &App::GetInstance() {
  static App instance(_appState->gameInstance);
  return instance;
}

void App::Preload(struct gametypes::Game *gameInstance) {
  if (_appState && _appState->gameInstance->applicationState) {
    FFATAL("App::Init(): app initializer was called more than once!");
    return;
  }

  gameInstance->applicationState = core::memory::MemoryManager::Allocate(
      sizeof(ApplicationState), core::memory::MEMORY_TAG_APPLICATION);
  _appState =
      reinterpret_cast<ApplicationState *>(gameInstance->applicationState);
  _appState->gameInstance = gameInstance;
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
}

bool App::Init() {
  if (!AllocateAll()) {
    FFATAL("App::Init(): failed to allocate memory for one of the "
           "application's internal systems!");
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
  _appState->isRunning = FeTrue;
  _appState->isSuspended = FeFalse;

  _appState->platform = _platform->GetState();

  if (!_appState->gameInstance->Initialize(_appState->gameInstance)) {
    FFATAL("App::Init(): failed to initialize application");
    return FeFalse;
  }

  _appState->gameInstance->OnResize(_appState->gameInstance, _appState->width,
                                    _appState->height);

  return FeTrue;
}

bool App::Run() {
  if (!_appState->gameInstance->applicationState) {
    FWARN("App::Run(): run method was called, but app was not initialized.");
    return FeFalse;
  }

  // Clock setup
  _appState->clock.Start();
  _appState->clock.Update();
  _appState->lastTime = _appState->clock.elapsed;
  float64 runningTime = 0.0f;
  uchar frameCount = 0;
  float64 targetFrameSeconds = 1.0f / 60;

  while (_appState->isRunning) {

    // TODO: fix this mess, it's actually ok for now but might complicate things
    // further
#if FEPLATFORM_WINDOWS
    _platform->PollEvents();
#elif FEPLATFORM_LINUX
    if (!_platform->PollEvents()) {
      FDEBUG("App::Run(): closing window was requested");
      _appState->isRunning = FeFalse;
    }
#endif

    if (_appState->isSuspended) {
      continue;
    }

    _appState->clock.Update();
    float64 currentTime = _appState->clock.elapsed;
    float64 deltaTime = (currentTime - _appState->lastTime);
    float64 frameStartTime = platform::Platform::GetAbsoluteTime();

    if (!_appState->gameInstance->Update(_appState->gameInstance,
                                         (float32)deltaTime)) {
      FFATAL("App::Run(): game update failed, shutting down application...");
      break;
    }

    if (!_appState->gameInstance->Render(_appState->gameInstance,
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
    _appState->lastTime = currentTime;
  }

  _appState->isRunning = FeFalse;

  return FeTrue;
}

void App::GetFrameBufferSize(uint32 *width, uint32 *height) {
  *width = _appState->width;
  *height = _appState->height;
}

void App::ShutDown() {
  if (!_appState->gameInstance->applicationState) {
    return;
  }

  _appState->isRunning = FeFalse;
}

// Private members

App::App(struct gametypes::Game *gameInstance)
    : _eventManager(core::events::EventManager::GetInstance()),
      _inputManager(core::input::InputManager::GetInstance()),
      _frontendRenderer(nullptr) {

  _appState->gameInstance = gameInstance;

  // TODO: implement logger constructor

  FINFO("App::App(): application was correctly initialized");
}

bool App::OnEvent(events::SystemEventCode code, void *sender, void *listener,
                  const events::EventContext &context) {
  switch (code) {
  case events::SystemEventCode::EVENT_CODE_APPLICATION_QUIT:
    FINFO("App::AppOnEvent(): EVENT_CODE_APPLICATION_QUIT received, shutting "
          "down...");
    _appState->isRunning = FeFalse;
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
      FDEBUG("'%s' key pressed in window.", KeyToString(keyCode));
    }
  } else if (code == events::SystemEventCode::EVENT_CODE_KEY_RELEASED) {
    std::array<ushort, 8> keyContext = context.get<std::array<ushort, 8>>();
    input::Keys keyCode = static_cast<input::Keys>(keyContext[0]);
    if (keyCode == input::Keys::KEY_B) {
      FDEBUG("Explicit - B key released");
    } else {
      FDEBUG("'%s' key released in window.", KeyToString(keyCode));
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
  if (width != _appState->width || height != _appState->height) {
    _appState->width = width;
    _appState->height = height;

    FDEBUG("App::OnResize(): window resizing: %i, %i", width, height);

    if (width == 0 || height == 0) {
      // Minimization
      FINFO("App::OnResize(): window minimized, suspending application.");
      _appState->isSuspended = FeTrue;
      return FeTrue;
    }

    if (_appState->isSuspended) {
      FINFO("App::OnResize(): window restored, resuming application.");
      _appState->isSuspended = FeFalse;
    }

    _appState->gameInstance->OnResize(_appState->gameInstance, width, height);
    _frontendRenderer->OnResize(width, height);
  }

  return FeFalse;
}

bool App::AllocateAll() {
  // Setup linear allocator
  uint64 systemAllocatorTotalSize = 64 * 1024 * 1024;
  new (&_appState->systemAllocator)
      flatearth::memory::LinearAllocator(systemAllocatorTotalSize, nullptr);

  // Allocate memory for Logger
  void *mem = memory::MemoryManager::Allocate(sizeof(core::logger::Logger),
                                              memory::MEMORY_TAG_APPLICATION);
  auto *logger = new (mem) core::logger::Logger();
  _logger = unique_logger_ptr(
      logger,
      memory::StatefulCustomDeleter<core::logger::Logger>(
          sizeof(core::logger::Logger), memory::MEMORY_TAG_APPLICATION));

  // Init with nullptr makes logger obj to own its memory
  uint64 loggerSystemMemSize = 0;
  if (!_logger->Init(&loggerSystemMemSize, nullptr)) {
    FERROR("App::AllocateAll(): failed to initialize logging system!");
    return FeFalse;
  }

  try {
    // Allocate memory for platform
    void *mem = memory::MemoryManager::Allocate(sizeof(platform::Platform),
                                                memory::MEMORY_TAG_APPLICATION);
    auto *platform = new (mem)
        platform::Platform(_appState->gameInstance->appConfig.name,
                           _appState->gameInstance->appConfig.startPosX,
                           _appState->gameInstance->appConfig.startPosY,
                           _appState->gameInstance->appConfig.startWidth,
                           _appState->gameInstance->appConfig.startHeight);
    _platform = unique_platform_ptr(
        platform,
        memory::StatefulCustomDeleter<platform::Platform>(
            sizeof(platform::Platform), memory::MEMORY_TAG_APPLICATION));
    // _appState platform must point to _platform obj
    _appState->platform = _platform->GetState();
  } catch (const std::exception &e) {
    FFATAL("App::AllocateAll(): failed to create platform: %s", e.what());
    return FeFalse;
  }

  try {
    // Allocate memory for the frontend renderer
    void *mem = core::memory::MemoryManager::Allocate(
        sizeof(renderer::FrontendRenderer), memory::MEMORY_TAG_APPLICATION);
    auto *frontendRenderer = new (mem) renderer::FrontendRenderer(
        _appState->gameInstance->appConfig.name, _appState->platform);
    _frontendRenderer = unique_frontend_renderer_ptr(
        frontendRenderer,
        core::memory::StatefulCustomDeleter<renderer::FrontendRenderer>(
            sizeof(renderer::FrontendRenderer),
            core::memory::MEMORY_TAG_APPLICATION));
  } catch (const std::exception &e) {
    FFATAL("App::AllocateAll(): failed to create frontend renderer: %s",
           e.what());
    return FeFalse;
  }

  return FeTrue;
}

const char *KeyToString(input::Keys key) {
  using namespace input;

  switch (key) {
  case KEY_0:
    return "0";
  case KEY_1:
    return "1";
  case KEY_2:
    return "2";
  case KEY_3:
    return "3";
  case KEY_4:
    return "4";
  case KEY_5:
    return "5";
  case KEY_6:
    return "6";
  case KEY_7:
    return "7";
  case KEY_8:
    return "8";
  case KEY_9:
    return "9";
  case KEY_BACKSPACE:
    return "Backspace";
  case KEY_ENTER:
    return "Enter";
  case KEY_TAB:
    return "Tab";
  case KEY_SHIFT:
    return "Shift";
  case KEY_CONTROL:
    return "Control";
  case KEY_PAUSE:
    return "Pause";
  case KEY_CAPITAL:
    return "Caps Lock";
  case KEY_ESCAPE:
    return "Escape";
  case KEY_CONVERT:
    return "Convert";
  case KEY_NONCONVERT:
    return "Nonconvert";
  case KEY_ACCEPT:
    return "Accept";
  case KEY_MODECHANGE:
    return "Mode Change";
  case KEY_SPACE:
    return "Space";
  case KEY_PRIOR:
    return "Page Up";
  case KEY_NEXT:
    return "Page Down";
  case KEY_END:
    return "End";
  case KEY_HOME:
    return "Home";
  case KEY_LEFT:
    return "Left Arrow";
  case KEY_UP:
    return "Up Arrow";
  case KEY_RIGHT:
    return "Right Arrow";
  case KEY_DOWN:
    return "Down Arrow";
  case KEY_SELECT:
    return "Select";
  case KEY_PRINT:
    return "Print";
  case KEY_EXECUTE:
    return "Execute";
  case KEY_SNAPSHOT:
    return "Print Screen";
  case KEY_INSERT:
    return "Insert";
  case KEY_DELETE:
    return "Delete";
  case KEY_HELP:
    return "Help";
  case KEY_A:
    return "A";
  case KEY_B:
    return "B";
  case KEY_C:
    return "C";
  case KEY_D:
    return "D";
  case KEY_E:
    return "E";
  case KEY_F:
    return "F";
  case KEY_G:
    return "G";
  case KEY_H:
    return "H";
  case KEY_I:
    return "I";
  case KEY_J:
    return "J";
  case KEY_K:
    return "K";
  case KEY_L:
    return "L";
  case KEY_M:
    return "M";
  case KEY_N:
    return "N";
  case KEY_O:
    return "O";
  case KEY_P:
    return "P";
  case KEY_Q:
    return "Q";
  case KEY_R:
    return "R";
  case KEY_S:
    return "S";
  case KEY_T:
    return "T";
  case KEY_U:
    return "U";
  case KEY_V:
    return "V";
  case KEY_W:
    return "W";
  case KEY_X:
    return "X";
  case KEY_Y:
    return "Y";
  case KEY_Z:
    return "Z";
  case KEY_LWIN:
    return "Left Win";
  case KEY_RWIN:
    return "Right Win";
  case KEY_APPS:
    return "Apps";
  case KEY_SLEEP:
    return "Sleep";
  case KEY_NUMPAD0:
    return "Numpad 0";
  case KEY_NUMPAD1:
    return "Numpad 1";
  case KEY_NUMPAD2:
    return "Numpad 2";
  case KEY_NUMPAD3:
    return "Numpad 3";
  case KEY_NUMPAD4:
    return "Numpad 4";
  case KEY_NUMPAD5:
    return "Numpad 5";
  case KEY_NUMPAD6:
    return "Numpad 6";
  case KEY_NUMPAD7:
    return "Numpad 7";
  case KEY_NUMPAD8:
    return "Numpad 8";
  case KEY_NUMPAD9:
    return "Numpad 9";
  case KEY_MULTIPLY:
    return "Numpad *";
  case KEY_ADD:
    return "Numpad +";
  case KEY_SEPARATOR:
    return "Separator";
  case KEY_SUBTRACT:
    return "Numpad -";
  case KEY_DECIMAL:
    return "Numpad .";
  case KEY_DIVIDE:
    return "Numpad /";
  case KEY_F1:
    return "F1";
  case KEY_F2:
    return "F2";
  case KEY_F3:
    return "F3";
  case KEY_F4:
    return "F4";
  case KEY_F5:
    return "F5";
  case KEY_F6:
    return "F6";
  case KEY_F7:
    return "F7";
  case KEY_F8:
    return "F8";
  case KEY_F9:
    return "F9";
  case KEY_F10:
    return "F10";
  case KEY_F11:
    return "F11";
  case KEY_F12:
    return "F12";
  case KEY_F13:
    return "F13";
  case KEY_F14:
    return "F14";
  case KEY_F15:
    return "F15";
  case KEY_F16:
    return "F16";
  case KEY_F17:
    return "F17";
  case KEY_F18:
    return "F18";
  case KEY_F19:
    return "F19";
  case KEY_F20:
    return "F20";
  case KEY_F21:
    return "F21";
  case KEY_F22:
    return "F22";
  case KEY_F23:
    return "F23";
  case KEY_F24:
    return "F24";
  case KEY_NUMLOCK:
    return "Num Lock";
  case KEY_SCROLL:
    return "Scroll Lock";
  case KEY_NUMPAD_EQUAL:
    return "Numpad =";
  case KEY_LSHIFT:
    return "Left Shift";
  case KEY_RSHIFT:
    return "Right Shift";
  case KEY_LCONTROL:
    return "Left Control";
  case KEY_RCONTROL:
    return "Right Control";
  case KEY_LMENU:
    return "Left Alt";
  case KEY_RMENU:
    return "Right Alt";
  case KEY_SEMICOLON:
    return "Semicolon";
  case KEY_PLUS:
    return "Plus";
  case KEY_COMMA:
    return "Comma";
  case KEY_MINUS:
    return "Minus";
  case KEY_PERIOD:
    return "Period";
  case KEY_SLASH:
    return "Slash";
  case KEY_GRAVE:
    return "Grave Accent";
  case KEY_NULL:
    return "Null";
  case KEYS_MAX_KEYS:
    return "MaxKeys";
  default:
    return "Unknown Key";
  }
}

} // namespace application
} // namespace core
} // namespace flatearth
