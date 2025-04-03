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

uchar TestLinearAllocatorParameterizedSizes_Success() {
  constexpr uint64 elementSize = sizeof(uint64);
  constexpr uint64 align = alignof(uint64);
  uint64 bufferSizes[] = {elementSize, elementSize * 4, elementSize * 128,
                          elementSize * 1024};

  for (uint64 bufferSize : bufferSizes) {
    memory::LinearAllocator alloc(bufferSize, nullptr);

    uint64 allocationCount = bufferSize / elementSize;
    for (uint64 i = 0; i < allocationCount; ++i) {
      void *block = alloc.Allocate(elementSize, align);
      ASSERT_NEQ_PTR(nullptr, block);
      ASSERT_EQ_INT(elementSize * (i + 1), alloc.GetAllocatedSize());
    }

    // Expect failure on one extra allocation
    void *failBlock = alloc.Allocate(elementSize, align);
    ASSERT_EQ_PTR(nullptr, failBlock);
    ASSERT_EQ_INT(allocationCount * elementSize, alloc.GetAllocatedSize());

    alloc.FreaAll();
    ASSERT_EQ_INT(0, alloc.GetAllocatedSize());
  }

  return FeTrue;
}

uchar TestLinearAllocatorAlignmentGaps_Success() {
  constexpr uint64 elementSize = sizeof(uint64);
  uint64 alignments[] = {8, 16, 32, 64, 128};
  constexpr uint64 totalSize = 1024;

  for (uint64 align : alignments) {
    memory::LinearAllocator alloc(totalSize, nullptr);

    void *prevBlock = nullptr;
    for (int i = 0; i < 4; ++i) {
      void *block = alloc.Allocate(elementSize, align);
      ASSERT_NEQ_PTR(nullptr, block);
      ASSERT_EQ_INT(reinterpret_cast<uintptr_t>(block) % align, 0);

      if (prevBlock) {
        auto diff = reinterpret_cast<uintptr_t>(block) -
                    reinterpret_cast<uintptr_t>(prevBlock);
        ASSERT_TRUE(diff >=
                    elementSize); // Deve ter pelo menos o tamanho + padding
      }

      prevBlock = block;
    }

    alloc.FreaAll();
    ASSERT_EQ_INT(0, alloc.GetAllocatedSize());
  }

  return FeTrue;
}

uchar TestLinearAllocatorReuseAfterFreeAll_Success() {
  constexpr uint64 elementSize = sizeof(uint64);
  constexpr uint64 allocCount = 16;
  constexpr uint64 totalSize = elementSize * allocCount;

  memory::LinearAllocator alloc(totalSize, nullptr);

  for (uint64 i = 0; i < allocCount; ++i) {
    void *block = alloc.Allocate(elementSize, alignof(uint64));
    ASSERT_NEQ_PTR(nullptr, block);
  }

  ASSERT_EQ_PTR(nullptr, alloc.Allocate(elementSize, alignof(uint64)));

  alloc.FreaAll();
  ASSERT_EQ_INT(0, alloc.GetAllocatedSize());

  for (uint64 i = 0; i < allocCount; ++i) {
    void *block = alloc.Allocate(elementSize, alignof(uint64));
    ASSERT_NEQ_PTR(nullptr, block);
  }

  return FeTrue;
}

uchar TestMultipleAllocatorsFromSingleBuffer_Success() {
  constexpr uint64 elementSize = sizeof(uint64);
  constexpr uint64 totalElements = 1024;
  constexpr uint64 halfElements = totalElements / 2;

  // Buffer único bruto
  void *rawBuffer = core::memory::MemoryManager::Allocate(
      totalElements * elementSize, core::memory::MEMORY_TAG_LINEAR_ALLOCATOR);

  // Alocador A usa a 1ª metade do buffer
  memory::LinearAllocator allocatorA(halfElements * elementSize, rawBuffer);

  // Alocador B usa a 2ª metade do buffer (com offset)
  void *secondHalf = reinterpret_cast<void *>(
      reinterpret_cast<uintptr_t>(rawBuffer) + halfElements * elementSize);
  memory::LinearAllocator allocatorB(halfElements * elementSize, secondHalf);

  // Preenche os dois até o limite
  for (uint64 i = 0; i < halfElements; ++i) {
    void *blockA = allocatorA.Allocate(elementSize, alignof(uint64));
    void *blockB = allocatorB.Allocate(elementSize, alignof(uint64));
    ASSERT_NEQ_PTR(nullptr, blockA);
    ASSERT_NEQ_PTR(nullptr, blockB);
  }

  // Ambos devem falhar agora
  ASSERT_EQ_PTR(nullptr, allocatorA.Allocate(elementSize, alignof(uint64)));
  ASSERT_EQ_PTR(nullptr, allocatorB.Allocate(elementSize, alignof(uint64)));

  // Fim: libera o buffer bruto
  core::memory::MemoryManager::Free(rawBuffer, totalElements * elementSize,
                                    core::memory::MEMORY_TAG_LINEAR_ALLOCATOR);

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
  tm.RegisterTest(TestLinearAllocatorParameterizedSizes_Success,
                  "Linear allocator should work with multiple buffer sizes");
  tm.RegisterTest(TestLinearAllocatorAlignmentGaps_Success,
                  "Linear allocator should align and pad correctly");
  tm.RegisterTest(TestLinearAllocatorReuseAfterFreeAll_Success,
                  "Linear allocator should reuse memory after FreaAll()");
  tm.RegisterTest(
      TestMultipleAllocatorsFromSingleBuffer_Success,
      "Multiple linear allocators should work from a shared buffer");
}

} // namespace tests
} // namespace flatearth
