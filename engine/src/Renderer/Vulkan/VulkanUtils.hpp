#ifndef _FLATEARTH_ENGINE_VULKAN_UTILS_HPP
#define _FLATEARTH_ENGINE_VULKAN_UTILS_HPP

#include "Definitions.hpp"
#include <vulkan/vulkan_core.h>

namespace flatearth {
namespace renderer {
namespace vulkan {
namespace utils {

/**
* Returns the string representation of VkResult
* @param result The result to get string for
* @param getExtended Indicates whether to also return an extended result
* @returns The error code and/or extended error message in string form
*/
const char *VkResultToString(VkResult result, bool getExtended = FeFalse);

/**
* Indicates if the passed result is a VK_SUCCESS or an error as defined by
* the Vulkan spec.
* @returns True if VK_SUCCESS; otherwise false.
*/
bool VkResultIsSuccess(VkResult result);

}
}
}
}

#endif // _FLATEARTH_ENGINE_VULKAN_UTILS_HPP
