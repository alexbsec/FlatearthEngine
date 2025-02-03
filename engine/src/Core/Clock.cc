#include "Clock.hpp"
#include "Platform/Platform.hpp"

namespace flatearth {
namespace core {
namespace clock {

void Clock::Start() { startTime = platform::Platform::GetAbsoluteTime(); }

void Clock::Update() {
  if (startTime != 0) {
    elapsed = platform::Platform::GetAbsoluteTime() - startTime;
  }
}

void Clock::Stop() { startTime = 0; }

} // namespace clock
} // namespace core
} // namespace flatearth
