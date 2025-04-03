#ifndef _FLATEARTH_TESTS_TEST_MANAGER_HPP
#define _FLATEARTH_TESTS_TEST_MANAGER_HPP

#include <Definitions.hpp>
#include <Containers/DArray.hpp>
#include <functional>

namespace flatearth {
namespace tests {

constexpr uint32 BYPASS = 2;

using TestFn = std::function<uchar()>;

struct TestEntry {
  TestFn func;
  string description;
};

class TestManager {
public:
  void RegisterTest(TestFn, string description);
  void RunTests();

private:
  containers::DArray<TestEntry> _tests; 
};

} // namespace tests
} // namespace flatearth
#endif // _FLATEARTH_TESTS_TEST_MANAGER_HPP
