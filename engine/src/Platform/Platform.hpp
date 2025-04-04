#ifndef _FLATEARTH_ENGINE_PLATFORM_HPP
#define _FLATEARTH_ENGINE_PLATFORM_HPP

#include "Definitions.hpp"

namespace flatearth {
namespace platform {

struct PlatformState {
  unique_void_ptr internalState;

  PlatformState() : internalState(nullptr, [](const void *) {}) {}
};

class Platform {
public:
  Platform(const string &applicationName, sint32 x, sint32 y,
                 sint32 width, sint32 height);
  ~Platform();

  bool PollEvents();

  static void *PAllocateMemory(uint64 size, bool aligned);
  static void PFreeMemory(void *block, bool aligned);
  static void *PZeroMemory(void *block, uint64 size);
  static void *PCopyMemory(void *dest, const void *source, uint64 size);
  static void *PSetMemory(void *dest, sint32 value, uint64 size);
  static void ConsoleWrite(const string &message, uchar color);
  static void ConsoleError(const string &message, uchar color);
  static float64 GetAbsoluteTime();
  static void Sleep(uint64 milliseconds);

  PlatformState *GetState() {
    return _state;
  }

private:
  PlatformState *_state;
  sint32 _x_pos;
  sint32 _y_pos;
  sint32 _width;
  sint32 _height;
};

} // namespace platform
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_PLATFORM_HPP
