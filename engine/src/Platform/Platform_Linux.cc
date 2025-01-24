#include "Platform.hpp"
#include <cstring>

#if FEPLATFORM_LINUX

#include "Core/Logger.hpp"

#include <print>
#include <stdexcept>
#include <X11/XKBlib.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <ctime>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

struct InternalState {
  Display *display;
  xcb_connection_t *connection;
  xcb_window_t window;
  xcb_screen_t *screen;
  xcb_atom_t wm_protocols;
  xcb_atom_t wm_delete_win;
};

namespace flatearth {
namespace platform {

Platform::Platform(const string &applicationName, sint32 x, sint32 y,
                   sint32 width, sint32 height)
    : _x_pos(x), _y_pos(y), _width(width), _height(height) {

  // Create internal state
  _state = new PlatformState();
  _state->internalState = make_unique_void<InternalState>(new InternalState());

  InternalState *inStatePtr =
      get_unique_void_ptr<InternalState>(_state->internalState);

  // Connect to X server
  inStatePtr->display = XOpenDisplay(nullptr);

  // Turn off key repeats
  XAutoRepeatOff(inStatePtr->display);

  // Retrieve the connection from the display
  inStatePtr->connection = XGetXCBConnection(inStatePtr->display);

  if (xcb_connection_has_error(inStatePtr->connection)) {
    FFATAL("Failed to connect to X server via XCB");
    throw std::runtime_error("Platform: an error occurred while attempting to "
                             "connect to X server via XCB");
  }

  // Get data from the X server
  const struct xcb_setup_t *setup = xcb_get_setup(inStatePtr->connection);

  // Loop through screens using iterator
  xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
  int screenP = 0;
  for (sint32 s = screenP; s > 0; s--) {
    xcb_screen_next(&it);
  }

  // After screens have been looped, assign them
  inStatePtr->screen = it.data;

  // Allocate a XID for the window to be created
  inStatePtr->window = xcb_generate_id(inStatePtr->connection);

  // Register event types
  uint32 eventMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

  // Listener for keyboard and mouse input
  uint32 eventValues =
      XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
      XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
      XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION |
      XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;

  // List of values to be sent over to XCB
  uint32 valueList[] = {inStatePtr->screen->black_pixel, eventValues};

  // Create the window
  xcb_void_cookie_t cookie =
      xcb_create_window(inStatePtr->connection, XCB_COPY_FROM_PARENT,
                        inStatePtr->window, inStatePtr->screen->root, x, y,
                        width, height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                        inStatePtr->screen->root_visual, eventMask, valueList);

  // Change title
  xcb_change_property(inStatePtr->connection, XCB_PROP_MODE_REPLACE,
                      inStatePtr->window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                      applicationName.length(), applicationName.c_str());

  // Tell the server to notify when the window manager attempts to destroy the
  // window

  const string &deleteStr = "WM_DELETE_WINDOW";
  const string &protocolsStr = "WM_PROTOCOLS";

  xcb_intern_atom_cookie_t wm_delete_cookie = xcb_intern_atom(
      inStatePtr->connection, 0, deleteStr.length(), deleteStr.c_str());

  xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(
      inStatePtr->connection, 0, protocolsStr.length(), protocolsStr.c_str());

  xcb_intern_atom_reply_t *wm_delete_reply =
      xcb_intern_atom_reply(inStatePtr->connection, wm_delete_cookie, NULL);

  xcb_intern_atom_reply_t *wm_protocols_reply =
      xcb_intern_atom_reply(inStatePtr->connection, wm_protocols_cookie, NULL);

  inStatePtr->wm_delete_win = wm_delete_reply->atom;
  inStatePtr->wm_protocols = wm_protocols_reply->atom;

  xcb_change_property(inStatePtr->connection, XCB_PROP_MODE_REPLACE,
                      inStatePtr->window, inStatePtr->wm_protocols, 4, 32, 1,
                      &inStatePtr->wm_delete_win);

  // Map the window onto the screen
  xcb_map_window(inStatePtr->connection, inStatePtr->window);

  // Flush the stream
  sint32 streamResult = xcb_flush(inStatePtr->connection);
  if (streamResult <= 0) {
    FFATAL("An error occurred when flushing the stream: %d", streamResult);
    throw std::runtime_error(
        "Platform: failed to flush the stream when creating the window");
  }
}

Platform::~Platform() {
  InternalState *inStatePtr =
      get_unique_void_ptr<InternalState>(_state->internalState);

  XAutoRepeatOn(inStatePtr->display);

  xcb_destroy_window(inStatePtr->connection, inStatePtr->window);

  delete _state;
}

bool Platform::PollEvents() {
  // Cold-cast state
  InternalState *inStatePtr =
      get_unique_void_ptr<InternalState>(_state->internalState);

  xcb_generic_event_t *event;
  xcb_client_message_event_t *cm;

  bool quitFlag = FeFalse;

  while ((event = xcb_poll_for_event(inStatePtr->connection)) != nullptr) {
    if (event == nullptr) 
      break;

    // Input events
    switch (event->response_type & 0x7f) {
    case XCB_KEY_PRESS:
    case XCB_KEY_RELEASE: {
      // Key presses and releases
    } break;
    case XCB_BUTTON_PRESS:
    case XCB_BUTTON_RELEASE: {
      // Mouse button pressed or released
    } break;
    case XCB_MOTION_NOTIFY:
      // Mouse movement
      break;

    case XCB_CONFIGURE_NOTIFY: {
      // Resizing
    } break;

    case XCB_CLIENT_MESSAGE: {
      // Close request
      cm = (xcb_client_message_event_t *)event;
      if (cm->data.data32[0] == inStatePtr->wm_delete_win) {
        // window close event
        FDEBUG("Platform::PollEvents(): close requested, quit flagging true.");
        quitFlag = FeTrue;
      }
    } break;

    default:
      break;
    }

    free(event);
  }

  return !quitFlag;
}

void *Platform::AllocateMemory(uint64 size, bool aligned) {
  // TODO: alignment
  return malloc(size);
}

void Platform::FreeMemory(void *block, bool aligned) {
  free(block);
}

void *Platform::ZeroMemory(void *block, uint64 size) {
  return memset(block, 0, size);
}

void *Platform::CopyMemory(void *dest, const void *source, uint64 size) {
  return memcpy(dest, source, size);
}

void *Platform::SetMemory(void *dest, sint32 value, uint64 size) {
  return memset(dest, value, size);
}

void Platform::ConsoleWrite(const string& message, uchar color) {
  // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
  const char *colorStrings[] = {"0;41", "1;31", "1;33",
                                 "1;32", "1;34", "1;30"};
  std::println("\033[{0}m{1}\033[0m", colorStrings[color], message);
}

void Platform::ConsoleError(const string& message, uchar color) {
  // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
  const char *colorStrings[] = {"0;41", "1;31", "1;33",
                                 "1;32", "1;34", "1;30"};
  std::println("\033[{0}m{1}\033[0m", colorStrings[color], message);
}

} // namespace platform
} // namespace flatearth

#endif
