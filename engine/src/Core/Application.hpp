#ifndef _FLATEARTH_ENGINE_APPLICATION_HPP
#define _FLATEARTH_ENGINE_APPLICATION_HPP

#include "Core/Clock.hpp"
#include "Core/Event.hpp"
#include "Core/FeMemory.hpp"
#include "Core/Input.hpp"
#include "Definitions.hpp"
#include "Logger.hpp"
#include "Memory/LinearAllocator.hpp"
#include "Platform/Platform.hpp"
#include "Renderer/RendererFrontend.hpp"

namespace flatearth {

namespace gametypes {

struct Game;

} // namespace gametypes

namespace core {
namespace application {

using unique_logger_ptr =
    std::unique_ptr<core::logger::Logger,
                    memory::StatefulCustomDeleter<core::logger::Logger>>;

using unique_platform_ptr =
    std::unique_ptr<platform::Platform,
                    memory::StatefulCustomDeleter<platform::Platform>>;

using unique_frontend_renderer_ptr = 
    std::unique_ptr<renderer::FrontendRenderer,
                    memory::StatefulCustomDeleter<renderer::FrontendRenderer>>;

struct AppConfig {
  // Window starting x axis position
  sshort startPosX;

  // Window starting y axis position
  sshort startPosY;

  // Window starting width, if applicacable
  sshort startWidth;

  // Window starting height, if applicable
  sshort startHeight;

  // Application name
  string name;
};

struct ApplicationState {
  gametypes::Game *gameInstance;
  bool isRunning;
  bool isSuspended;
  platform::PlatformState *platform;
  sshort width;
  sshort height;
  float64 lastTime;
  core::clock::Clock clock;

  flatearth::memory::LinearAllocator systemAllocator;
};

class App {
public:
  FEAPI static App &GetInstance();
  FEAPI static void Preload(struct gametypes::Game *gameInstance);
  FEAPI ~App();

  FEAPI bool Init();
  FEAPI bool Run();

  static void GetFrameBufferSize(uint32 *width, uint32 *height);

  void ShutDown();

private:
  // Private constructor
  App(struct gametypes::Game *gameInstance);
  bool OnEvent(events::SystemEventCode code, void *sender, void *listener,
               const events::EventContext &context);
  bool OnKey(events::SystemEventCode code, void *sender, void *listener,
             const events::EventContext &context);
  bool OnResized(events::SystemEventCode code, void *sender, void *listener,
                 const events::EventContext &context);

  bool AllocateAll();

  // Private variables

  // Event Callbacks 
  events::EventCallback OnResizedCallback;
  events::EventCallback OnEventCallback;
  events::EventCallback OnKeyCallback;

  // Application state ptr
  static ApplicationState *_appState;

  // Internals
  unique_logger_ptr _logger;
  unique_platform_ptr _platform;
  unique_frontend_renderer_ptr _frontendRenderer;

  // Make these as copies because their ownership are shared
  core::events::EventManager &_eventManager;
  core::input::InputManager &_inputManager;
};

} // namespace application
} // namespace core
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_APPLICATION_HPP
