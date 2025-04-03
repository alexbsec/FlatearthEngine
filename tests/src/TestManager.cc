#include "TestManager.hpp"

#include <Containers/DArray.hpp>
#include <Core/Clock.hpp>
#include <Core/FeString.hpp>
#include <Core/Logger.hpp>

namespace flatearth {
namespace tests {

void TestManager::RegisterTest(TestFn fn, string description) {
  TestEntry e;
  e.func = fn;
  e.description = description;
  _tests.Push(e);
}

void TestManager::RunTests() {
  uint32 passed = 0;
  uint32 failed = 0;
  uint32 skipped = 0;

  uint32 count = _tests.GetLength();

  core::clock::Clock totalTimeClock;
  totalTimeClock.Start();

  for (uint32 i = 0; i < count; i++) {
    core::clock::Clock testTimeClock;
    testTimeClock.Start();
    uchar result = _tests[i].func();
    testTimeClock.Update();

    if (result == FeTrue) {
      passed++;
    } else if (result == BYPASS) {
      FWARN("[SKIPPED]: %s", _tests[i].description.c_str());
      skipped++;
    } else {
      FERROR("[FAILED]: %s", _tests[i].description.c_str());
      failed++;
    }

    string status;
    if (result == FeTrue) {
      status = "[✓] PASS: %d";
      status = core::Sprintf(status, passed);
    } else if (result == BYPASS) {
      status = "[~] SKIPPED: %d";
      status = core::Sprintf(status, skipped); 
    } else {
      status = "[✗] FAIL: %d";
      status = core::Sprintf(status, failed);
    }

    testTimeClock.Update();
    FINFO("Executed %d of %d (skipped %d) %s (%.6f sec / %.6f sec total)",
          i + 1, count, skipped, status.c_str(), testTimeClock.elapsed,
          totalTimeClock.elapsed);
  }

  totalTimeClock.Stop();
  uint32 total = passed + failed + skipped;
  FINFO("Results: %d passed | %d failed | %d skipped | %d total", passed,
        failed, skipped, total);
}

} // namespace tests
} // namespace flatearth
