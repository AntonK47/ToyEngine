#include "Vulkan.h"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

using namespace toy::renderer::api::vulkan;


VulkanNativeBackend toy::renderer::api::vulkan::getVulkanNativeBackend(
	const NativeBackend& nativeBackend)
{
	return *static_cast<VulkanNativeBackend*>(nativeBackend.nativeBackend);
}
