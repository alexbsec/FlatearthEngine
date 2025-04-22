#include "DArrayTests.hpp"
#include "../Expect.hpp"
#include "../TestManager.hpp"

#include <Containers/DArray.hpp>
#include <Math/FeMath.hpp>
#include <Math/MathTypes.inl>

namespace flatearth {
namespace tests {

using namespace containers;

uchar TestDArrayCreateSimpleType_Success() {
  DArray<uint64> array;
  array.Push(1);
  array.Push(2);
  array.Push(3);

  ASSERT_EQ_INT(3, array.GetLength());
  ASSERT_EQ_INT(sizeof(uint64), array.GetStride());
  ASSERT_EQ_INT(1, array[0]);
  ASSERT_EQ_INT(2, array[1]);
  ASSERT_EQ_INT(3, array[2]);

  return FeTrue;
}

uchar TestDArrayPushPop_Success() {
  DArray<uint64> array;
  array.Push(10);
  array.Push(20);
  array.Pop();

  ASSERT_EQ_INT(1, array.GetLength());
  ASSERT_EQ_INT(10, array[0]);

  return FeTrue;
}

uchar TestDArrayInsertAt_Success() {
  DArray<uint32> array;
  array.Push(1);
  array.Push(2);
  array.Push(3);
  array.InsertAt(100, 1); // [1, 100, 2, 3]

  ASSERT_EQ_INT(4, array.GetLength());
  ASSERT_EQ_INT(1, array[0]);
  ASSERT_EQ_INT(100, array[1]);
  ASSERT_EQ_INT(2, array[2]);
  ASSERT_EQ_INT(3, array[3]);

  return FeTrue;
}

uchar TestDArrayResizeMemory_Success() {
  DArray<uint32> array(2, sizeof(uint32));
  array.Push(1);
  array.Push(2);
  array.Push(3); // should trigger resize

  ASSERT_EQ_INT(3, array.GetLength());
  ASSERT_EQ_INT(3, array[2]);
  return FeTrue;
}

uchar TestDArrayClear_Success() {
  DArray<ushort> array;
  array.Push(8);
  array.Push(9);
  array.Clear();

  ASSERT_EQ_INT(0, array.GetLength());
  ASSERT_TRUE(array.IsEmpty());

  return FeTrue;
}

uchar TestDArrayComplexType_Success() {
  DArray<math::Vec2i> array;
  array.Push(math::Vec2i(1.0f, 2.0f));
  array.Push(math::Vec2i(3.0f, 4.0f));

  ASSERT_EQ_INT(2, array.GetLength());
  ASSERT_EQ_FLOAT(1.0f, array[0].Real());
  ASSERT_EQ_FLOAT(4.0f, array[1].Imag());
  return FeTrue;
}

uchar TestDArrayPopAt_Success() {
  DArray<uint32> array;
  array.Push(10);
  array.Push(20);
  array.Push(30);
  array.PopAt(1);  // Should remove 20

  ASSERT_EQ_INT(2, array.GetLength());
  ASSERT_EQ_INT(10, array[0]);
  ASSERT_EQ_INT(30, array[1]);
  return FeTrue;
}

uchar TestDArrayInsertAtEnd_Success() {
  DArray<uint32> array;
  array.Push(1);
  array.Push(2);
  array.InsertAt(3, 2);  // Insert at end (like Push)

  ASSERT_EQ_INT(3, array.GetLength());
  ASSERT_EQ_INT(3, array[2]);
  return FeTrue;
}

uchar TestDArrayInsertAtBeginning_Success() {
  DArray<uint32> array;
  array.Push(100);
  array.Push(200);
  array.InsertAt(50, 0);  // Insert at start

  ASSERT_EQ_INT(3, array.GetLength());
  ASSERT_EQ_INT(50, array[0]);
  ASSERT_EQ_INT(100, array[1]);
  ASSERT_EQ_INT(200, array[2]);
  return FeTrue;
}

uchar TestDArrayPopAtInvalidIndex_DoesNothing() {
  DArray<uint32> array;
  array.Push(1);
  array.PopAt(5);  // Invalid pop, should not crash
  ASSERT_EQ_INT(1, array.GetLength());
  return FeTrue;
}

uchar TestDArrayAccessOutOfBounds_Throws() {
  DArray<uint32> array;
  array.Push(1);

  bool caught = false;
  try {
    auto v = array[3];
  } catch (const std::out_of_range &) {
    caught = true;
  }

  ASSERT_TRUE(caught);
  return FeTrue;
}

uchar TestDArrayReserveCapacityOnly() {
  DArray<uint32> array;
  array.Push(1);
  array.Reserve(32);

  ASSERT_EQ_INT(32, array.GetLength());
  ASSERT_TRUE(array.GetCapacity() >= 32);

  return FeTrue;
}

void DArrayRegisterTests(TestManager &tm) {
  tm.RegisterTest(TestDArrayCreateSimpleType_Success, "DArray: Create uint64");
  tm.RegisterTest(TestDArrayPushPop_Success, "DArray: Push & Pop");
  tm.RegisterTest(TestDArrayInsertAt_Success, "DArray: InsertAt");
  tm.RegisterTest(TestDArrayResizeMemory_Success, "DArray: Resize");
  tm.RegisterTest(TestDArrayClear_Success, "DArray: Clear");
  tm.RegisterTest(TestDArrayComplexType_Success, "DArray: Complex Type (Vec2i)");
  tm.RegisterTest(TestDArrayPopAt_Success, "DArray: PopAt");
  tm.RegisterTest(TestDArrayInsertAtEnd_Success, "DArray: InsertAt End");
  tm.RegisterTest(TestDArrayInsertAtBeginning_Success, "DArray: InsertAt Beginning");
  tm.RegisterTest(TestDArrayPopAtInvalidIndex_DoesNothing, "DArray: PopAt invalid index");
  tm.RegisterTest(TestDArrayAccessOutOfBounds_Throws, "DArray: Access out-of-bounds throws");
  tm.RegisterTest(TestDArrayReserveCapacityOnly, "DArray: Reserve");
}

}
}
