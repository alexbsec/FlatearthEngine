#ifndef _FLATEARTH_ENGINE_EVENT_HPP
#define _FLATEARTH_ENGINE_EVENT_HPP

#include "Definitions.hpp"
#include <array>
#include <functional>
#include <variant>

namespace flatearth {
namespace core {
namespace events {

enum class SystemEventCode : ushort {
  // Shuts the application down on the next frame.
  EVENT_CODE_APPLICATION_QUIT = 0x01,

  // Keyboard key pressed.
  /*  Context usage:
   *  u16 key_code = data.data.u16[0];
   */
  EVENT_CODE_KEY_PRESSED = 0x02,

  // Keyboard key released.
  /*  Context usage:
   *  u16 key_code = data.data.u16[0];
   */
  EVENT_CODE_KEY_RELEASED = 0x03,

  // Mouse button pressed.
  /* Context usage:
   * u16 button = data.data.u16[0];
   */
  EVENT_CODE_BUTTON_PRESSED = 0x04,

  // Mouse button released.
  /* Context usage:
   * u16 button = data.data.u16[0];
   */
  EVENT_CODE_BUTTON_RELEASED = 0x05,

  // Mouse moved
  /* Context usage:
   * u16 x = data.data.u16[0];
   * u16 y = data.data.u16[1];
   */
  EVENT_CODE_MOUSE_MOVED = 0x06,

  // Mouse wheel moved
  /* Context usage:
   * u8 z_delta = data.data.u8[0];
   */
  EVENT_CODE_MOUSE_WHEEL = 0x07,

  // Resized/resolution changed from the OS
  /* Context usage:
   * u16 width = data.data.u16[0];
   * u16 height = data.data.u16[1];
   */
  EVENT_CODE_RESIZED = 0x08,

  MAX_EVENT_CODE = 0xFF,
};

class EventContext {
public:
  using VariantType = std::variant<
      std::array<sint64, 2>, std::array<uint64, 2>, std::array<float64, 2>,
      std::array<sint32, 4>, std::array<uint32, 4>, std::array<float32, 4>,
      std::array<sshort, 8>, std::array<ushort, 8>, std::array<schar, 16>,
      std::array<uchar, 16>, std::array<char, 16>>;

  VariantType data;

  template <typename T> void set(const T &value) { data = value; }

  template <typename T> T get() const { return std::get<T>(data); }

  template <typename T> const T *SafeGet() const {
    return std::get_if<T>(&data);
  }
};

using EventCallback = std::function<bool(const EventContext &)>;

class EventManager {
public:
  FEAPI EventManager();
  FEAPI ~EventManager();

  /**
   * Registers a callback to listen for events with the specified code.
   * Duplicate listener/callback pairs will not be re-registered.
   *
   * @param code The event code to listen for.
   * @param listener A pointer to the listener instance (optional, can be
   * nullptr).
   * @param callback The callback function to invoke when the event is fired.
   * @returns True if the event was successfully registered, false otherwise.
   */
  FEAPI bool RegisterEvent(SystemEventCode code, void *listener,
                           EventCallback callback);

  /**
   * Unregisters a callback for the specified event code.
   * If no matching registration is found, this function does nothing.
   *
   * @param code The event code to unregister from.
   * @param listener A pointer to the listener instance (optional, can be
   * nullptr).
   * @param callback The callback function to be unregistered.
   * @returns True if the event was successfully unregistered, false otherwise.
   */
  FEAPI bool UnregisterEvent(SystemEventCode code, void *listener,
                             EventCallback callback);

  /**
   * Fires an event to all listeners registered for the specified code.
   * If a callback returns true, the event is considered handled, and no further
   * callbacks are invoked.
   *
   * @param code The event code to fire.
   * @param sender A pointer to the sender instance (optional, can be nullptr).
   * @param context The event context data to pass to listeners.
   * @returns True if the event was handled by any listener, false otherwise.
   */
  FEAPI bool FireEvent(SystemEventCode code, void *sender,
                       EventContext &context);

private:
  struct Listener {
    void *listener;
    EventCallback callback;

    bool operator==(const Listener &other) const {
      return listener == other.listener &&
             callback.target_type() == other.callback.target_type();
    }
  };

  std::unordered_map<ushort, std::vector<Listener>> _listeners;
};

} // namespace events
} // namespace core
} // namespace flatearth

#endif
