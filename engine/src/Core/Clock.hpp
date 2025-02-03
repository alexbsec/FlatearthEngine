#ifndef _FLATEARTH_ENGINE_CLOCK_HPP
#define _FLATEARTH_ENGINE_CLOCK_HPP

#include "Definitions.hpp"

namespace flatearth {
namespace core {
namespace clock {

class Clock {
public:
  void Start();
  void Update();
  void Stop();

  // Public variables
  float64 startTime;
  float64 elapsed;
};

}
} // namespace core
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_CLOCK_HPP
