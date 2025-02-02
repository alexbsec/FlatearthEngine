#ifndef _FLATEARTH_ENGINE_APPLICATION_HPP
#define _FLATEARTH_ENGINE_APPLICATION_HPP

#include "Containers/SArray.hpp"
#include "Core/Event.hpp"
#include "Core/Input.hpp"
#include "Definitions.hpp"
#include "Logger.hpp"
#include "Platform/Platform.hpp"

namespace flatearth {

namespace gametypes {

struct Game;

} // namespace gametypes

namespace core {
namespace application {

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
  platform::PlatformState platform;
  sshort width;
  sshort height;
  float64 lastTime;
};

class App {
public:
  FEAPI static App &GetInstance();
  FEAPI static void SetGameInstance(struct gametypes::Game *gameInstance);
  FEAPI ~App();

  FEAPI bool Init();
  FEAPI bool Run();

  void ShutDown();

private:
  // Private constructor
  App(struct gametypes::Game *gameInstance);
  bool OnEvent(events::SystemEventCode code, void *sender, void *listener,
               const events::EventContext &context);
  bool OnKey(events::SystemEventCode code, void *sender, void *listener,
             const events::EventContext &context);

  // Private variables
  events::EventCallback OnEventCallback;
  events::EventCallback OnKeyCallback;
  static ApplicationState _appState;
  static bool _initialized;
  std::unique_ptr<logger::Logger> _logger;
  std::unique_ptr<platform::Platform> _platform;
  core::events::EventManager &_eventManager;
  core::input::InputManager &_inputManager;
};

} // namespace application
} // namespace core
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_APPLICATION_HPP
