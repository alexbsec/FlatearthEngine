#include "Event.hpp"
#include "Containers/DArray.hpp"
#include "Core/FeMemory.hpp"
#include "Core/Logger.hpp"
#include <array>

namespace flatearth {
namespace core {
namespace events {

bool EventManager::_isInitialized = FeFalse;

EventManager &EventManager::GetInstance() {
  static EventManager instance;
  return instance;
}

EventManager::EventManager() {
  if (_isInitialized) {
    FINFO(
        "EventManager::EventManager(): event Manager is already initialized!");
    return;
  }

  core::memory::MemoryManager::ZeroMemory(&_state, sizeof(_state));
  FINFO("EventManager::EventManager(): event manager correctly initialized");
  _isInitialized = FeTrue;
}

EventManager::~EventManager() {
  FINFO("EventManager::~EventManager(): shutting down event manager...");
  for (ushort i = 0; i < MAX_EVENTS; i++) {
    if (_state.registered[i].events != nullptr) {
      _state.registered[i].events->Clear();
    }
  }
}

bool EventManager::RegisterEvent(SystemEventCode code, void *listener,
                                 EventCallback callback) {
  ushort ccode = ToUnderlying(code);
  if (_state.registered[ccode].events == nullptr) {
    _state.registered[ccode].events =
        std::make_unique<containers::DArray<RegisteredEvent>>();
  }

  uint64 registeredCount = _state.registered[ccode].events->GetLength();
  for (uint64 i = 0; i < registeredCount; i++) {
    if ((*_state.registered[ccode].events)[i].listener == listener) {
      FWARN("EventManager::RegisterEvent(): event trying to be registered "
            "already exists");
      return FeFalse;
    }
  }

  // If at this point no duplicate was found, proceed with registration
  RegisteredEvent e;
  e.listener = listener;
  e.callback = callback;
  _state.registered[ccode].events->Push(e);

  return FeTrue;
}

bool EventManager::UnregisterEvent(SystemEventCode code, void *listener,
                                   EventCallback callback) {
  ushort ccode = ToUnderlying(code);
  // On nothing is registered, do nothing
  if (_state.registered[ccode].events == nullptr) {
    return FeFalse;
  }

  uint64 registeredCount = _state.registered[ccode].events->GetLength();
  for (uint64 i = 0; i < registeredCount; i++) {
    RegisteredEvent& e = (*_state.registered[ccode].events)[i];
    if (e.listener == listener &&
        e.callback.target<void>() == callback.target<void>()) {
      _state.registered[ccode].events->PopAt(i);
      return FeTrue;
    }
  }

  // No event found
  return FeFalse;
}

bool EventManager::FireEvent(SystemEventCode code, void *sender,
                             EventContext context) {
  ushort ccode = ToUnderlying(code);
  // If nothing is registered for the code, do nothing
  if (_state.registered[ccode].events == nullptr) {
    return FeFalse;
  }

  
  uint64 registeredCount = _state.registered[ccode].events->GetLength();
  for (uint64 i = 0; i < registeredCount; i++) {
    RegisteredEvent &e = (*_state.registered[ccode].events)[i];
    if (e.callback(code, sender, e.listener, context)) {
      // Early exit
      return FeTrue;
    }
  }

  // Nothing found
  return FeFalse;
}

} // namespace events
} // namespace core
} // namespace flatearth
