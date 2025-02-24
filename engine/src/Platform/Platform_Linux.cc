#include "Platform.hpp"

#if FEPLATFORM_LINUX

#include "Core/Input.hpp"
#include "Core/Logger.hpp"
#include "Renderer/Vulkan/VulkanPlatform.hpp"

#include <X11/XKBlib.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <cstring>
#include <ctime>
#include <print>
#include <stdexcept>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>
#else
#include <unistd.h>
#endif

#define VK_USE_PLATFORM_XCB_KHR
#include "Renderer/Vulkan/VulkanTypes.inl"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_xcb.h>

struct InternalState {
  Display *display;
  xcb_connection_t *connection;
  xcb_window_t window;
  xcb_screen_t *screen;
  xcb_atom_t wm_protocols;
  xcb_atom_t wm_delete_win;
  VkSurfaceKHR surface;
};

namespace flatearth {
namespace platform {

core::input::Keys TranslateKeysymbol(uint32 keySymbol);

Platform::Platform(const string &applicationName, sint32 x, sint32 y,
                   sint32 width, sint32 height)
    : _x_pos(x), _y_pos(y), _width(width), _height(height) {

  // Create internal state
  _state = new PlatformState();
  _state->internalState =
      core::memory::make_unique_void<InternalState>(new InternalState());

  InternalState *inStatePtr =
      core::memory::get_unique_void_ptr<InternalState>(_state->internalState);

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
      core::memory::get_unique_void_ptr<InternalState>(_state->internalState);

  XAutoRepeatOn(inStatePtr->display);

  xcb_destroy_window(inStatePtr->connection, inStatePtr->window);

  delete _state;
}

bool Platform::PollEvents() {
  // Cold-cast state
  InternalState *inStatePtr =
      core::memory::get_unique_void_ptr<InternalState>(_state->internalState);

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
      xcb_key_press_event_t *keyEvent = (xcb_key_press_event_t *)event;
      bool pressed = event->response_type == XCB_KEY_PRESS;
      xcb_keycode_t code = keyEvent->detail;
      KeySym keySymbol = XkbKeycodeToKeysym(inStatePtr->display, (KeyCode)code,
                                            0, code & ShiftMask ? 1 : 0);
      core::input::Keys key = TranslateKeysymbol(keySymbol);
      core::input::InputManager::ProcessKey(key, pressed);
    } break;
    case XCB_BUTTON_PRESS:
    case XCB_BUTTON_RELEASE: {
      xcb_button_press_event_t *buttonEvent = (xcb_button_press_event_t *)event;
      bool pressed = event->response_type == XCB_BUTTON_PRESS;
      core::input::Buttons button = core::input::Buttons::BUTTON_MAX_BUTTONS;
      switch (buttonEvent->detail) {
      case XCB_BUTTON_INDEX_1:
        button = core::input::Buttons::BUTTON_LEFT;
        break;
      case XCB_BUTTON_INDEX_2:
        button = core::input::Buttons::BUTTON_MIDDLE;
        break;
      case XCB_BUTTON_INDEX_3:
        button = core::input::Buttons::BUTTON_RIGHT;
        break;
      }

      if (button != core::input::Buttons::BUTTON_MAX_BUTTONS) {
        core::input::InputManager::ProcessButton(button, pressed);
      }
    } break;
    case XCB_MOTION_NOTIFY: {
      xcb_motion_notify_event_t *moveEvent = (xcb_motion_notify_event_t *)event;
      core::input::InputManager::ProcessMouseMove(moveEvent->event_x,
                                                  moveEvent->event_y);
      break;
    }

      // TODO: mouse wheel detection

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

void *Platform::PAllocateMemory(uint64 size, bool aligned) {
  // TODO: alignment
  return malloc(size);
}

void Platform::PFreeMemory(void *block, bool aligned) { free(block); }

void *Platform::PZeroMemory(void *block, uint64 size) {
  return memset(block, 0, size);
}

void *Platform::PCopyMemory(void *dest, const void *source, uint64 size) {
  return memcpy(dest, source, size);
}

void *Platform::PSetMemory(void *dest, sint32 value, uint64 size) {
  return memset(dest, value, size);
}

void Platform::ConsoleWrite(const string &message, uchar color) {
  // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
  const char *colorStrings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
  std::println("\033[{0}m{1}\033[0m", colorStrings[color], message);
}

void Platform::ConsoleError(const string &message, uchar color) {
  // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
  const char *colorStrings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
  std::println("\033[{0}m{1}\033[0m", colorStrings[color], message);
}

float64 Platform::GetAbsoluteTime() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return now.tv_sec + now.tv_nsec * 0.000000001;
}

void Platform::Sleep(uint64 milliseconds) {
#if _POSIX_C_SOURCE >= 199309L
  struct timespec ts;
  ts.tv_sec = milliseconds / 1000;
  ts.tv_nsec = (milliseconds % 1000) * 1000 * 1000;
  nanosleep(&ts, 0);
#else
  if (milliseconds >= 1000) {
    sleep(milliseconds / 1000);
  }
  usleep((ms % 1000) * 1000);
#endif
}

core::input::Keys TranslateKeysymbol(uint32 keySymbol) {
  switch (keySymbol) {
  case XK_BackSpace:
    return core::input::KEY_BACKSPACE;
  case XK_Return:
    return core::input::KEY_ENTER;
  case XK_Tab:
    return core::input::KEY_TAB;
    // case XK_Shift: return core::input::KEY_SHIFT;
    // case XK_Control: return core::input::KEY_CONTROL;
  case XK_Pause:
    return core::input::KEY_PAUSE;
  case XK_Caps_Lock:
    return core::input::KEY_CAPITAL;
  case XK_Escape:
    return core::input::KEY_ESCAPE;
    // Not supported
    // case : return core::input::KEY_CONVERT;
    // case : return core::input::KEY_NONCONVERT;
    // case : return core::input::KEY_ACCEPT;
  case XK_Mode_switch:
    return core::input::KEY_MODECHANGE;
  case XK_space:
    return core::input::KEY_SPACE;
  case XK_Prior:
    return core::input::KEY_PRIOR;
  case XK_Next:
    return core::input::KEY_NEXT;
  case XK_End:
    return core::input::KEY_END;
  case XK_Home:
    return core::input::KEY_HOME;
  case XK_Left:
    return core::input::KEY_LEFT;
  case XK_Up:
    return core::input::KEY_UP;
  case XK_Right:
    return core::input::KEY_RIGHT;
  case XK_Down:
    return core::input::KEY_DOWN;
  case XK_Select:
    return core::input::KEY_SELECT;
  case XK_Print:
    return core::input::KEY_PRINT;
  case XK_Execute:
    return core::input::KEY_EXECUTE;
  // case XK_snapshot: return core::input::KEY_SNAPSHOT; // not supported
  case XK_Insert:
    return core::input::KEY_INSERT;
  case XK_Delete:
    return core::input::KEY_DELETE;
  case XK_Help:
    return core::input::KEY_HELP;
  case XK_Meta_L:
    return core::input::KEY_LWIN; // TODO: not sure this is right
  case XK_Meta_R:
    return core::input::KEY_RWIN;
    // case XK_apps: return core::input::KEY_APPS; // not supported
    // case XK_sleep: return core::input::KEY_SLEEP; //not supported
  case XK_KP_0:
    return core::input::KEY_NUMPAD0;
  case XK_KP_1:
    return core::input::KEY_NUMPAD1;
  case XK_KP_2:
    return core::input::KEY_NUMPAD2;
  case XK_KP_3:
    return core::input::KEY_NUMPAD3;
  case XK_KP_4:
    return core::input::KEY_NUMPAD4;
  case XK_KP_5:
    return core::input::KEY_NUMPAD5;
  case XK_KP_6:
    return core::input::KEY_NUMPAD6;
  case XK_KP_7:
    return core::input::KEY_NUMPAD7;
  case XK_KP_8:
    return core::input::KEY_NUMPAD8;
  case XK_KP_9:
    return core::input::KEY_NUMPAD9;
  case XK_multiply:
    return core::input::KEY_MULTIPLY;
  case XK_KP_Add:
    return core::input::KEY_ADD;
  case XK_KP_Separator:
    return core::input::KEY_SEPARATOR;
  case XK_KP_Subtract:
    return core::input::KEY_SUBTRACT;
  case XK_KP_Decimal:
    return core::input::KEY_DECIMAL;
  case XK_KP_Divide:
    return core::input::KEY_DIVIDE;
  case XK_F1:
    return core::input::KEY_F1;
  case XK_F2:
    return core::input::KEY_F2;
  case XK_F3:
    return core::input::KEY_F3;
  case XK_F4:
    return core::input::KEY_F4;
  case XK_F5:
    return core::input::KEY_F5;
  case XK_F6:
    return core::input::KEY_F6;
  case XK_F7:
    return core::input::KEY_F7;
  case XK_F8:
    return core::input::KEY_F8;
  case XK_F9:
    return core::input::KEY_F9;
  case XK_F10:
    return core::input::KEY_F10;
  case XK_F11:
    return core::input::KEY_F11;
  case XK_F12:
    return core::input::KEY_F12;
  case XK_F13:
    return core::input::KEY_F13;
  case XK_F14:
    return core::input::KEY_F14;
  case XK_F15:
    return core::input::KEY_F15;
  case XK_F16:
    return core::input::KEY_F16;
  case XK_F17:
    return core::input::KEY_F17;
  case XK_F18:
    return core::input::KEY_F18;
  case XK_F19:
    return core::input::KEY_F19;
  case XK_F20:
    return core::input::KEY_F20;
  case XK_F21:
    return core::input::KEY_F21;
  case XK_F22:
    return core::input::KEY_F22;
  case XK_F23:
    return core::input::KEY_F23;
  case XK_F24:
    return core::input::KEY_F24;
  case XK_Num_Lock:
    return core::input::KEY_NUMLOCK;
  case XK_Scroll_Lock:
    return core::input::KEY_SCROLL;
  case XK_KP_Equal:
    return core::input::KEY_NUMPAD_EQUAL;
  case XK_Shift_L:
    return core::input::KEY_LSHIFT;
  case XK_Shift_R:
    return core::input::KEY_RSHIFT;
  case XK_Control_L:
    return core::input::KEY_LCONTROL;
  case XK_Control_R:
    return core::input::KEY_RCONTROL;
  // case XK_Menu: return core::input::KEY_LMENU;
  case XK_Menu:
    return core::input::KEY_RMENU;
  case XK_semicolon:
    return core::input::KEY_SEMICOLON;
  case XK_plus:
    return core::input::KEY_PLUS;
  case XK_comma:
    return core::input::KEY_COMMA;
  case XK_minus:
    return core::input::KEY_MINUS;
  case XK_period:
    return core::input::KEY_PERIOD;
  case XK_slash:
    return core::input::KEY_SLASH;
  case XK_grave:
    return core::input::KEY_GRAVE;
  case XK_a:
  case XK_A:
    return core::input::KEY_A;
  case XK_b:
  case XK_B:
    return core::input::KEY_B;
  case XK_c:
  case XK_C:
    return core::input::KEY_C;
  case XK_d:
  case XK_D:
    return core::input::KEY_D;
  case XK_e:
  case XK_E:
    return core::input::KEY_E;
  case XK_f:
  case XK_F:
    return core::input::KEY_F;
  case XK_g:
  case XK_G:
    return core::input::KEY_G;
  case XK_h:
  case XK_H:
    return core::input::KEY_H;
  case XK_i:
  case XK_I:
    return core::input::KEY_I;
  case XK_j:
  case XK_J:
    return core::input::KEY_J;
  case XK_k:
  case XK_K:
    return core::input::KEY_K;
  case XK_l:
  case XK_L:
    return core::input::KEY_L;
  case XK_m:
  case XK_M:
    return core::input::KEY_M;
  case XK_n:
  case XK_N:
    return core::input::KEY_N;
  case XK_o:
  case XK_O:
    return core::input::KEY_O;
  case XK_p:
  case XK_P:
    return core::input::KEY_P;
  case XK_q:
  case XK_Q:
    return core::input::KEY_Q;
  case XK_r:
  case XK_R:
    return core::input::KEY_R;
  case XK_s:
  case XK_S:
    return core::input::KEY_S;
  case XK_t:
  case XK_T:
    return core::input::KEY_T;
  case XK_u:
  case XK_U:
    return core::input::KEY_U;
  case XK_v:
  case XK_V:
    return core::input::KEY_V;
  case XK_w:
  case XK_W:
    return core::input::KEY_W;
  case XK_x:
  case XK_X:
    return core::input::KEY_X;
  case XK_y:
  case XK_Y:
    return core::input::KEY_Y;
  case XK_z:
  case XK_Z:
    return core::input::KEY_Z;
  default:
    return core::input::KEY_NULL;
  }
}

// Vulkan platform specifics

bool CreateVulkanSurface(struct PlatformState *platState,
                         struct renderer::vulkan::Context *context) {
  InternalState *inStatePtr = core::memory::get_unique_void_ptr<InternalState>(
      platState->internalState);

  VkXcbSurfaceCreateInfoKHR createInfo = {
      VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR};
  createInfo.connection = inStatePtr->connection;
  createInfo.window = inStatePtr->window;

  VkResult result = vkCreateXcbSurfaceKHR(
      context->instance, &createInfo, context->allocator, &inStatePtr->surface);
  if (result != VK_SUCCESS) {
    FFATAL("CreateVulkanSurface(): Vulkan surface failed to create");
    return FeFalse;
  }

  context->surface = inStatePtr->surface;
  return FeTrue;
}

void GetRequiredExtNames(containers::DArray<const char *> *namesDArray) {
  namesDArray->Push("VK_KHR_xcb_surface");
}

} // namespace platform
} // namespace flatearth

#endif
