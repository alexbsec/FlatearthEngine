#ifndef _FLATEARTH_TESTSUITE_GAME_HPP
#define _FLATEARTH_TESTSUITE_GAME_HPP

#include <Definitions.hpp>
#include <GameTypes.hpp>

namespace flatearth {
namespace testsuite {

struct GameState {
  float32 deltaTime;
};

class GameTest {
public:
  static bool GameInitialize(gametypes::Game *gameInstance);
  static bool GameUpdate(gametypes::Game *gameInstance, float32 deltaTime);
  static bool GameRender(gametypes::Game *gameInstance, float32 deltaTime);
  static void GameOnResize(gametypes::Game *gameInstance, uint32 width,
                           uint32 height);
};

} // namespace testsuite
} // namespace flatearth

#endif // _FLATEARTH_TESTSUITE_GAME_HPP
