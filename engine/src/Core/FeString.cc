#include "FeString.hpp"

#include <Containers/DArray.hpp>

namespace flatearth {
namespace core {

string Sprintf(string arg, ...) {
  va_list args1;
  va_start(args1, arg);

  va_list args2;
  va_copy(args2, args1);
  uint32 size = std::vsnprintf(nullptr, 0, arg.c_str(), args2);
  va_end(args2);

  containers::DArray<char> buffer;
  buffer.Reserve(size + 1);
  std::vsnprintf(buffer.Data(), buffer.GetLength(), arg.c_str(), args1);
  va_end(args1);

  return string(buffer.Data(), size);
}

}
}
