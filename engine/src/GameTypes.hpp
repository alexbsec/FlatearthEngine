#ifndef _FLATEARTH_ENGINE_GAME_TYPES_HPP
#define _FLATEARTH_ENGINE_GAME_TYPES_HPP

#include "Core/Application.hpp"
#include "Definitions.hpp"
#include <functional>

namespace flatearth {
namespace gametypes {

struct game {
  // The application configuration
  core::application::AppConfig appConfig;

  // Function pointer to game's initialize function
  std::function<bool(struct game *gameInstance)> Initialize;

  // Function pointer to game's update function
  std::function<bool(struct game *gameInstance, float32 deltaTime)> Update;

  // Function pointer to game's render function
  std::function<bool(struct game *gameInstance, float32 deltaTime)> Render;

  // Function pointer to handle resize if applicable
  std::function<void(struct game *gameInstance, uint32 width, uint32 height)>
      OnResize;

  // Game-specific game state. Created and managed by the game
  unique_void_ptr state;
};

} // namespace gametypes
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_GAME_TYPES_HPP
