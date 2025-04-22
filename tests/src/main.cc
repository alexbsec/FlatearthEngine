#include "Core/FeMemory.hpp"
#include "TestManager.hpp"
#include "Memory/LinearAllocatorTests.hpp"

#include <Core/Logger.hpp>

using namespace flatearth;

int main() {
  tests::TestManager tm;
  core::memory::MemoryManager::TestPreload();
  tests::LinearAllocatorRegisterTests(tm);
  FDEBUG("Starting tests...");
  tm.RunTests();
  return 0;
}
