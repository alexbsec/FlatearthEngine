#include "Game.hpp"

#include <Core/Logger.hpp>

namespace flatearth {
namespace testsuite {

bool GameTest::GameInitialize(gametypes::Game *gameInstance) {
  FDEBUG("GameTest::GameInitialize() called");
  return FeTrue;
}

bool GameTest::GameUpdate(gametypes::Game *gameInstance, float32 deltaTime) {
  return FeTrue;
}

bool GameTest::GameRender(gametypes::Game *gameInstance, float32 deltaTime) {
  return FeTrue;
}

void GameTest::GameOnResize(gametypes::Game *gameInstance, uint32 width,
                            uint32 height) {}

} // namespace testsuite
} // namespace flatearth
