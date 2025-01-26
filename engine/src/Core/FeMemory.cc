#include "FeMemory.hpp"

#include "Logger.hpp"
#include "Platform/Platform.hpp"
#include <ostream>
#include <print>
#include <sstream>

namespace flatearth {
namespace core {
namespace memory {

MemoryBlock MemoryManager::_memoryBlock;

static constexpr std::array<vstring, MEMORY_TAG_MAX_TAGS> memTagNames = {
    "UNKNOWN", "ARRAY",       "DARRAY", "DICT",        "RING_QUEUE", "BST",
    "STRING",  "APPLICATION", "JOB",    "TEXTURE",     "MAT_INST",   "RENDERER",
    "GAME",    "TRANSFORM",   "ENTITY", "ENTITY_NODE", "SCENE"};

MemoryManager::MemoryManager() {
  platform::Platform::PZeroMemory(&_memoryBlock, sizeof(_memoryBlock));
}

void *MemoryManager::Allocate(uint64 size, MemoryTag tag) {
  CheckTag(tag, "MemoryManager::Allocate()");

  // Track how much memory used by category
  _memoryBlock.totalAllocated += size;
  _memoryBlock.taggedAllocations[tag] += size;

  // TODO: memory alignment
  bool aligned = FeFalse;
  void *block = platform::Platform::PAllocateMemory(size, aligned);
  platform::Platform::PZeroMemory(block, size);
  return block;
}

void MemoryManager::Free(void *block, uint64 size, MemoryTag tag) {
  CheckTag(tag, "MemoryManager::Free()");

  _memoryBlock.totalAllocated -= size;
  _memoryBlock.taggedAllocations[tag] -= size;

  bool aligned = FeFalse;
  platform::Platform::PFreeMemory(block, aligned);
}

void *MemoryManager::ZeroMemory(void *block, uint64 size) {
  return platform::Platform::PZeroMemory(block, size);
}

void *MemoryManager::CopyMemory(void *dest, const void *source, uint64 size) {
  return platform::Platform::PCopyMemory(dest, source, size);
}

void *MemoryManager::SetMemory(void *dest, sint32 value, uint64 size) {
  return platform::Platform::PSetMemory(dest, value, size);
}

string MemoryManager::PrintMemoryUsage() const {
  const uint64 kib = 1024;
  const uint64 mib = 1024 * kib;
  const uint64 gib = 1024 * mib;

  std::ostringstream oss;
  oss << "System memory usage (tagged):\n";
  std::print("{}", oss.str());
  for (uint64 i = 0; i < MEMORY_TAG_MAX_TAGS; i++) {
    string unit = "XiB";
    float32 amount = 1.0f;
    const uint64 &taggedAlloc = _memoryBlock.taggedAllocations[i];
    if (taggedAlloc >= gib) {
      unit[0] = 'G';
      amount = taggedAlloc / (float32)gib;
    } else if (taggedAlloc >= mib) {
      unit[0] = 'M';
      amount = taggedAlloc / (float32)mib;
    } else if (taggedAlloc >= kib) {
      unit[0] = 'K';
      amount = taggedAlloc / (float32)kib;
    } else {
      unit = "B";
      amount = (float32)taggedAlloc;
    }

    const string &out =
        std::format("{:<15}: {:>10.2f} {}\n", memTagNames[i], amount, unit);
    oss << out;
    std::print("{}", out);
  }

  return oss.str();
}

void MemoryManager::CheckTag(MemoryTag tag, const string &from) {
  if (tag == MEMORY_TAG_UNKNOWN) {
    FWARN("%s: calling memory management using MEMORY_TAG_UNKNOWN. Consider "
          "re-classing this block",
          from.c_str());
  }
}

} // namespace memory
} // namespace core
} // namespace flatearth
