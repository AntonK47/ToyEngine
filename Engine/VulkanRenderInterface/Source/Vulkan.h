#pragma once

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#ifndef NDEBUG
#define VULKAN_HPP_ASSERT
#endif

#include <Common.h>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

#include "RenderInterfaceCommonTypes.h"

namespace toy::renderer::api::vulkan
{
	struct VulkanNativeBackend
	{
		vk::Device device;
		vk::Instance instance;
	};

	VulkanNativeBackend getVulkanNativeBackend(const NativeBackend& nativeBackend);

	struct VulkanPipeline final : Pipeline
	{
		vk::Pipeline pipeline{};
		vk::PipelineLayout layout{};
		vk::PipelineBindPoint bindPoint{};
	};

}
