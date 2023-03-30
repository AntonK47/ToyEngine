#include "VulkanRHI/Vulkan.h"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace toy::graphics::rhi::vulkan
{
	VulkanNativeBackend getVulkanNativeBackend(
	const NativeBackend& nativeBackend)
	{
		return *static_cast<VulkanNativeBackend*>(nativeBackend.nativeBackend);
	}
}


