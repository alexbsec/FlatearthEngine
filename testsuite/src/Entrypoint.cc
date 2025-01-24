#include "Game.hpp"

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

  return FeTrue;
}
