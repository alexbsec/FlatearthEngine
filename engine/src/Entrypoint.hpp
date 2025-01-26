#ifndef _FLATEARTH_ENGINE_ENTRYPOINT_HPP
#define _FLATEARTH_ENGINE_ENTRYPOINT_HPP

#include "Core/Application.hpp"
#include "Core/FeMemory.hpp"
#include "Core/Logger.hpp"
#include "GameTypes.hpp"

extern bool CreateGame(flatearth::gametypes::game *gameOut);

int main(void) {
  flatearth::core::memory::MemoryManager memoryManager;

  flatearth::gametypes::game gameInst;

  if (!CreateGame(&gameInst)) {
    FFATAL("main(): could not create game!");
    return -2;
  }

  // Ensure the function pointers are defined
  if (!gameInst.Initialize || !gameInst.Update || !gameInst.Render ||
      !gameInst.OnResize) {
    FFATAL(
        "main(): one or more of the game's function pointers are not defined");
    return -1;
  }

  // Initialization
  flatearth::core::application::App gameApp(&gameInst);
  if (!gameApp.Init()) {
    FINFO("main(): the game's function failed to create");
    return 1;
  }

  memoryManager.PrintMemoryUsage();

  // Begin game loop
  if (!gameApp.Run()) {
    FINFO("main(): application did not shutdown gracefully");
    return 2;
  }

  return 0;
}

#endif // _FLATEARTH_ENGINE_ENTRYPOINT_HPP
