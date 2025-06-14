#ifndef _FLATEARTH_ENGINE_ENTRYPOINT_HPP
#define _FLATEARTH_ENGINE_ENTRYPOINT_HPP

#include "Core/Application.hpp"
#include "Core/FeMemory.hpp"
#include "Core/Logger.hpp"
#include "GameTypes.hpp"

extern bool CreateGame(flatearth::gametypes::Game *gameOut);

int main(void) {
  flatearth::gametypes::Game gameInst;

  if (!CreateGame(&gameInst)) {
    FFATAL("main(): could not create game!");
    return -2;
  }

  flatearth::core::memory::MemoryManager::Preload(&gameInst);
  flatearth::core::memory::MemoryManager &memoryManager =
      flatearth::core::memory::MemoryManager::GetInstance();

  // Ensure the function pointers are defined
  if (!gameInst.Initialize || !gameInst.Update || !gameInst.Render ||
      !gameInst.OnResize) {
    FFATAL(
        "main(): one or more of the game's function pointers are not defined");
    return -1;
  }

  // Initialization
  flatearth::core::application::App::Preload(&gameInst);
  flatearth::core::application::App &gameApp =
      flatearth::core::application::App::GetInstance();
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
