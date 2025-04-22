#include "Core/FeMemory.hpp"
#include "Containers/DArrayTests.hpp"
#include "TestManager.hpp"
#include "Memory/LinearAllocatorTests.hpp"

#include <Core/Logger.hpp>

using namespace flatearth;

int main() {
  tests::TestManager tm;
  core::memory::MemoryManager::TestPreload();
  tests::LinearAllocatorRegisterTests(tm);
  tests::DArrayRegisterTests(tm);
  FDEBUG("Starting tests...");
  tm.RunTests();
  return 0;
}
