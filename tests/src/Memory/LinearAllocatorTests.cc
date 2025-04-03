#include "LinearAllocatorTests.hpp"
#include "../Expect.hpp"
#include "../TestManager.hpp"

#include <Memory/LinearAllocator.hpp>

namespace flatearth {
namespace tests {

uchar TestLinearAllocatorCreate_Success() {
  memory::LinearAllocator alloc(sizeof(uint64), nullptr);

  ASSERT_NEQ_PTR(nullptr, alloc.GetMemory());
  ASSERT_EQ_INT(sizeof(uint64), alloc.GetTotalSize());
  ASSERT_EQ_INT(0, alloc.GetAllocatedSize());

  return FeTrue;
}

uchar TestLinearAllocatorSingleAllocation_Success() {
  memory::LinearAllocator alloc(sizeof(uint64), nullptr);

  void *block = alloc.Allocate(sizeof(uint64), alignof(uint64));

  uint64 allocSize = alloc.GetAllocatedSize();

  ASSERT_NEQ_PTR(nullptr, block);
  ASSERT_EQ_INT(sizeof(uint64), allocSize);

  return FeTrue;
}

uchar TestLinearAllocatorMultiAllocation_Success() {
  uint64 maxAllocs = 1024;
  memory::LinearAllocator alloc(sizeof(uint64) * maxAllocs, nullptr);

  void *block;
  for (uint64 i = 0; i < maxAllocs; i++) {
    block = alloc.Allocate(sizeof(uint64), alignof(uint64));
    ASSERT_NEQ_PTR(nullptr, block);
    ASSERT_EQ_INT(sizeof(uint64) * (i + 1), alloc.GetAllocatedSize());
  }

  return FeTrue;
}

uchar TestLinearAllocatorMultiAllocation_Fails() {
  uint64 maxAllocs = 1024;
  memory::LinearAllocator alloc(sizeof(uint64) * maxAllocs, nullptr);

  void *block;
  for (uint64 i = 0; i < maxAllocs; i++) {
    block = alloc.Allocate(sizeof(uint64), alignof(uint64));
    ASSERT_NEQ_PTR(nullptr, block);
    ASSERT_EQ_INT(sizeof(uint64) * (i + 1), alloc.GetAllocatedSize());
  }

  block = alloc.Allocate(sizeof(uint64), alignof(uint64));
  ASSERT_EQ_PTR(nullptr, block);
  ASSERT_EQ_INT(sizeof(uint64) * maxAllocs, alloc.GetAllocatedSize());

  return FeTrue;
}

uchar TestLinearAllocatorMultiAllocationThenFree_Success() {
  uint64 maxAllocs = 1024;
  memory::LinearAllocator alloc(sizeof(uint64) * maxAllocs, nullptr);

  void *block;
  for (uint64 i = 0; i < maxAllocs; i++) {
    block = alloc.Allocate(sizeof(uint64), alignof(uint64));
    ASSERT_NEQ_PTR(nullptr, block);
    ASSERT_EQ_INT(sizeof(uint64) * (i + 1), alloc.GetAllocatedSize());
  }

  alloc.FreaAll();
  ASSERT_EQ_INT(0, alloc.GetAllocatedSize());

  return FeTrue;
}

uchar TestLinearAllocatorAlignedAllocation_Success() {
  // Alinhamento maior que o tamanho da alocação
  constexpr uint64 alignment = 64;
  constexpr uint64 allocSize = sizeof(uint64);
  constexpr uint64 totalSize = allocSize * 4 + alignment * 4;

  memory::LinearAllocator alloc(totalSize, nullptr);

  void *block = alloc.Allocate(allocSize, alignment);

  ASSERT_NEQ_PTR(nullptr, block);
  ASSERT_EQ_INT(reinterpret_cast<uintptr_t>(block) % alignment, 0);

  return FeTrue;
}

void LinearAllocatorRegisterTests(TestManager &tm) {
  tm.RegisterTest(TestLinearAllocatorCreate_Success,
                  "Linear allocator should create");
  tm.RegisterTest(TestLinearAllocatorSingleAllocation_Success,
                  "Linear allocator should allocate one time");
  tm.RegisterTest(TestLinearAllocatorMultiAllocation_Success,
                  "Linear allocator should allocate multiple times");
  tm.RegisterTest(TestLinearAllocatorMultiAllocation_Fails,
                  "Linear allocator must fail one too many allocations");
  tm.RegisterTest(TestLinearAllocatorMultiAllocationThenFree_Success,
                  "Linear allocator should allocate multiple and free all");
  tm.RegisterTest(TestLinearAllocatorAlignedAllocation_Success,
                  "Linear allocator should align properly");
}

} // namespace tests
} // namespace flatearth
