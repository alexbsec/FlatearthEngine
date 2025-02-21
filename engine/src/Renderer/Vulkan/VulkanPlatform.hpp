#ifndef _FLATEARTH_ENGINE_VULKAN_PLATFORM_HPP
#define _FLATEARTH_ENGINE_VULKAN_PLATFORM_HPP

#include "Containers/DArray.hpp"
#include "Definitions.hpp"

namespace flatearth {
namespace platform {

void GetRequiredExtNames(containers::DArray<const char *> *namesDArray);

}
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_VULKAN_PLATFORM_HPP
