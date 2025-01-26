#include "Game.hpp"

#include <Containers/DArray.hpp>
#include <Core/FeMemory.hpp>
#include <Entrypoint.hpp>

bool CreateGame(flatearth::gametypes::game *gameOut) {
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

  flatearth::containers::DArray<int> array;
  for (int i = 0; i < 10; i++) {
    array.Push(i);
  }

  array.InsertAt(43, 5);

  for (int i = 0; i < array.GetLength(); i++) {
    FDEBUG("Array entry at %d: %d", i, array[i]);
  }

  uint64 size = array.GetLength();
  FDEBUG("Array len: %d", size);

  array.PopAt(343);

  return FeTrue;
}
