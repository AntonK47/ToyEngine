#pragma once

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_ASSERT
#include <Common.h>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace toy::renderer::api::vulkan
{
	struct VulkanNativeBackend
	{
		vk::Device device;
		vk::Instance instance;
	};

	VulkanNativeBackend getVulkanNativeBackend(const NativeBackend& nativeBackend);
}
