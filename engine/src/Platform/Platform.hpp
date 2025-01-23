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
  FEAPI Platform(const string &application_name, sint32 x, sint32 y,
                 sint32 width, sint32 height);
  ~Platform();

  bool WindowIsOpen();
  static void ConsoleWrite(const string &message, uchar color);
  static void ConsoleError(const string &message, uchar color);

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
