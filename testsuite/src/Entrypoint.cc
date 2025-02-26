#include "Game.hpp"

#include <Core/FeMemory.hpp>
#include <Entrypoint.hpp>

bool CreateGame(flatearth::gametypes::Game *gameOut) {
  gameOut->appConfig.startPosX = 100;
  gameOut->appConfig.startPosY = 100;
  gameOut->appConfig.startWidth = 1280;
  gameOut->appConfig.startHeight = 720;
  gameOut->appConfig.name = "Flatearth Engine Testsuite";

  // Assign the function pointers
  gameOut->Initialize = flatearth::testsuite::GameTest::GameInitialize;
  gameOut->Update = flatearth::testsuite::GameTest::GameUpdate;
  gameOut->Render = flatearth::testsuite::GameTest::GameRender;
  gameOut->OnResize = flatearth::testsuite::GameTest::GameOnResize;

  gameOut->state = flatearth::core::memory::MemoryManager::Allocate(
      sizeof(flatearth::testsuite::GameState),
      flatearth::core::memory::MEMORY_TAG_GAME);
  return FeTrue;
}
