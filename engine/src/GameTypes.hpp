#ifndef _FLATEARTH_ENGINE_GAME_TYPES_HPP
#define _FLATEARTH_ENGINE_GAME_TYPES_HPP

#include "Core/Application.hpp"
#include "Definitions.hpp"
#include <functional>

namespace flatearth {
namespace gametypes {

struct Game {
  // The application configuration
  core::application::AppConfig appConfig;

  // Function pointer to Game's initialize function
  std::function<bool(struct Game *gameInstance)> Initialize;

  // Function pointer to Game's update function
  std::function<bool(struct Game *gameInstance, float32 deltaTime)> Update;

  // Function pointer to Game's render function
  std::function<bool(struct Game *gameInstance, float32 deltaTime)> Render;

  // Function pointer to handle resize if applicable
  std::function<void(struct Game *gameInstance, uint32 width, uint32 height)>
      OnResize;

  // Game-specific Game state. Created and managed by the Game
  void *state;

  // Application state
  void *applicationState;

  // Memory state
  void *memoryState;

  Game()
      : Initialize(nullptr), Update(nullptr), Render(nullptr),
        OnResize(nullptr) {}
};

} // namespace gametypes
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_GAME_TYPES_HPP
