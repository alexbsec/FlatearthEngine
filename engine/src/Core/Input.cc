#include "Input.hpp"
#include "Event.hpp"
#include "FeMemory.hpp"
#include "Logger.hpp"

namespace flatearth {
namespace core {
namespace input {

InputManager &InputManager::GetInstance() {
  static InputManager instance = InputManager();
  return instance;
}

InputManager::~InputManager() { 
  FINFO("InputManager::~InputManager(): shutting down input manager...");
  _isInitialized = FeFalse; 
}

void InputManager::Update(float64 deltaTime) {
  // Since this is a static method, InputManager
  // could be not initialized yet
  if (!_isInitialized) {
    FWARN("InputManager::Update(): calling update method with InputManager not "
          "initialized");
    return;
  }

  // Copy current states to previous states
  core::memory::MemoryManager::CopyMemory(
      &_state.keyboardPrevious, &_state.keyboardCurrent, sizeof(KeyboardState));
  core::memory::MemoryManager::CopyMemory(
      &_state.mousePrevious, &_state.mouseCurrent, sizeof(MouseState));
}

void InputManager::ProcessKey(Keys key, bool pressed) {
  if (!_isInitialized) {
    FWARN("InputManager::ProcessKey(): calling process key method with "
          "InputManager not initialized");
    return;
  }

  string p = pressed == FeTrue ? "pressed" : "released";
  FDEBUG("InputManager::ProcessKey(): received a signal about a key %s", p.c_str());

  // Only handle this if the states actually changed
  if (_state.keyboardCurrent.keys[key] == pressed)
    return;

  FDEBUG("InputManager::ProcessKey(): creating context...")
  _state.keyboardCurrent.keys[key] = pressed;
  events::EventContext context;
  context.set(std::array<ushort, 8>{});
  context.set<std::array<ushort, 8>>(0, static_cast<ushort>(key));
  FDEBUG("InputManager::ProcessKey(): context created ok. Firing event to EventManager");
  events::SystemEventCode code =
      pressed ? events::SystemEventCode::EVENT_CODE_KEY_PRESSED
              : events::SystemEventCode::EVENT_CODE_KEY_RELEASED;
  EventManagerRef().FireEvent(code, nullptr, context);
  FDEBUG("InputManager::ProcessKey(): event fired successfully!");
}

void InputManager::ProcessButton(Buttons button, bool pressed) {
  if (!_isInitialized) {
    FWARN("InputManager::ProcessButton(): calling process button method with "
          "InputManager not initialized");
    return;
  }

  if (_state.mouseCurrent.buttons[button] == pressed)
    return;

  _state.mouseCurrent.buttons[button] = pressed;
  events::EventContext context;
  context.set(std::array<ushort, 8>{});
  context.set<std::array<ushort, 8>>(0, static_cast<ushort>(button));
  events::SystemEventCode code =
      pressed ? events::SystemEventCode::EVENT_CODE_BUTTON_PRESSED
              : events::SystemEventCode::EVENT_CODE_BUTTON_RELEASED;
  EventManagerRef().FireEvent(code, nullptr, context);
}

void InputManager::ProcessMouseMove(sshort x, sshort y) {
  if (!_isInitialized) {
    FWARN("InputManager::ProcessMouseMove(): calling process mouse move method "
          "with InputManager not initialized");
    return;
  }

  if (_state.mouseCurrent.x == x && _state.mouseCurrent.y == y)
    return;

  // FDEBUG("Mouse position: (%d, %d)", x, y);

  // Update internal state
  _state.mouseCurrent.x = x;
  _state.mouseCurrent.y = y;

  // Fire the event
  events::EventContext context;
  context.set(std::array<ushort, 8>{});
  context.set<std::array<ushort, 8>>(0, x);
  context.set<std::array<ushort, 8>>(1, y);
  events::SystemEventCode code =
      events::SystemEventCode::EVENT_CODE_MOUSE_MOVED;
  EventManagerRef().FireEvent(code, nullptr, context);
}

void InputManager::ProcessMouseWheel(schar zDelta) {
  if (!_isInitialized) {
    FWARN("InputManager::ProcessMouseWheel(): calling process mouse wheel "
          "method with InputManager not initialized");
    return;
  }
  events::EventContext context;
  context.set(std::array<uchar, 16>{});
  context.set<std::array<uchar, 16>>(0, zDelta);
  events::SystemEventCode code =
      events::SystemEventCode::EVENT_CODE_MOUSE_WHEEL;
  EventManagerRef().FireEvent(code, nullptr, context);
}

bool InputManager::IsKeyDown(Keys key) {
  if (!_isInitialized)
    return FeFalse;

  return _state.keyboardCurrent.keys[key] == FeTrue;
}

bool InputManager::IsKeyUp(Keys key) {
  if (!_isInitialized)
    return FeFalse;

  return _state.keyboardCurrent.keys[key] == FeFalse;
}

bool InputManager::WasKeyDown(Keys key) {
  if (!_isInitialized)
    return FeFalse;

  return _state.keyboardPrevious.keys[key] == FeTrue;
}

bool InputManager::WasKeyUp(Keys key) {
  if (!_isInitialized)
    return FeFalse;

  return _state.keyboardPrevious.keys[key] == FeFalse;
}

bool InputManager::IsButtonDown(Buttons button) {
  if (!_isInitialized)
    return FeFalse;

  return _state.mouseCurrent.buttons[button] == FeTrue;
}

bool InputManager::IsButtonUp(Buttons button) {
  if (!_isInitialized)
    return FeFalse;

  return _state.mouseCurrent.buttons[button] == FeFalse;
}

bool InputManager::WasButtonDown(Buttons button) {
  if (!_isInitialized)
    return FeFalse;

  return _state.mousePrevious.buttons[button] == FeTrue;
}

bool InputManager::WasButtonUp(Buttons button) {
  if (!_isInitialized)
    return FeFalse;

  return _state.mousePrevious.buttons[button] == FeFalse;
}

InputState &InputManager::GetState() { return _state; }

// PRIVATE

InputManager::InputManager() {
  if (_isInitialized) {
    FERROR("InputManager::InputManager(): attempt to create another instance "
           "of InputManager!");
    return;
  }

  core::memory::MemoryManager::ZeroMemory(&_state, sizeof(_state));
  FINFO("InputManager::InputManager(): input manager correctly initialized");
  _isInitialized = FeTrue;
}

events::EventManager &InputManager::EventManagerRef() {
  return events::EventManager::GetInstance();
}

InputState InputManager::_state = {};
bool InputManager::_isInitialized = FeFalse;

} // namespace input
} // namespace core
} // namespace flatearth
