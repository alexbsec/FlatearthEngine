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
      FWARN("[FE/TESTS] - [SKIPPED]: %s", _tests[i].description.c_str());
      skipped++;
    } else {
      FERROR("[FE/TESTS] - [FAILED]: %s", _tests[i].description.c_str());
      failed++;
    }

    string status;
    if (result == FeTrue) {
      status = "[FE/TESTS] - [✓] PASS: %d";
      status = core::Sprintf(status, passed);
    } else if (result == BYPASS) {
      status = "[FE/TESTS] - [~] SKIPPED: %d";
      status = core::Sprintf(status, skipped);
    } else {
      status = "[FE/TESTS] - [✗] FAIL: %d";
      status = core::Sprintf(status, failed);
    }

    testTimeClock.Update();
    totalTimeClock.Update();
    FINFO("%s. Executed %d of %d in (%.6f sec / %.6f sec total)",
          status.c_str(), i + 1, count, testTimeClock.elapsed,
          totalTimeClock.elapsed);
  }

  totalTimeClock.Stop();
  uint32 total = passed + failed + skipped;
  FINFO("[FE/TESTS] - Results: %d passed | %d failed | %d skipped | %d total", passed,
        failed, skipped, total);
}

} // namespace tests
} // namespace flatearth
