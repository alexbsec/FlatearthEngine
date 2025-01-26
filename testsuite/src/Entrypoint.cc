#include "Game.hpp"

#include <Core/FeMemory.hpp>
#include <Entrypoint.hpp>
#include <Containers/DArray.hpp>

bool CreateGame(flatearth::gametypes::game *gameOut,
                flatearth::core::memory::MemoryManager &memManager) {
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

  gameOut->state =
      memManager.Allocate(sizeof(flatearth::testsuite::GameState),
                          flatearth::core::memory::MEMORY_TAG_GAME);

  flatearth::containers::DArray<int> array;
  array.Push(2);
  array.Push(4);
  array.Pop();


  return FeTrue;
}
