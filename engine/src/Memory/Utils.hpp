#ifndef _FLATEARTH_ENGINE_MEMORY_UTILS_HPP
#define _FLATEARTH_ENGINE_MEMORY_UTILS_HPP

#include "Memory/LinearAllocator.hpp"

namespace flatearth {
namespace memory {

template <typename T, typename... Args>
T *AllocateObject(LinearAllocator &allocator, Args&&... args) {
  void *mem = allocator.Allocate(sizeof(T), alignof(T));
  return new (mem) T(std::forward<Args>(args)...);
}

}
}

#endif // _FLATEARTH_ENGINE_MEMORY_UTILS_HPP
