#ifndef _FLATEARTH_ENGINE_APPLICATION_HPP
#define _FLATEARTH_ENGINE_APPLICATION_HPP

#include "Definitions.hpp"
#include "Platform/Platform.hpp"
#include "Logger.hpp"

namespace flatearth {

namespace gametypes {

struct game;

} // gametypes

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
  gametypes::game *gameInstance;
  bool isRunning;
  bool isSuspended;
  platform::PlatformState platform;
  sshort width;
  sshort height;
  float64 lastTime;
};


class App {
public:
  FEAPI App(struct gametypes::game *gameInstance);
  FEAPI ~App();

  FEAPI bool Init();
  FEAPI bool Run();

private:
  ApplicationState _appState;
  std::unique_ptr<logger::Logger> _logger;
  std::unique_ptr<platform::Platform> _platform;
};

} // namespace application
} // namespace core
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_APPLICATION_HPP
