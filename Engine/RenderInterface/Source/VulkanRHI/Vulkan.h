﻿#pragma once

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#ifndef NDEBUG
#define VULKAN_HPP_ASSERT
#endif

#include <Core.h>
#pragma warning(disable:28251)
#pragma warning(disable:6031)
#include <vulkan/vulkan.hpp>
#pragma warning(default:6031)
#pragma warning(default:28251)

#include <vk_mem_alloc.h>

#include "RenderInterfaceTypes.h"

namespace toy::graphics::rhi::vulkan
{
	struct VulkanNativeBackend
	{
		vk::Device device;
		vk::Instance instance;
	};

	VulkanNativeBackend getVulkanNativeBackend(const NativeBackend& nativeBackend);

	struct VulkanPipeline final
	{
		vk::Pipeline pipeline{};
		vk::PipelineLayout layout{};
		vk::PipelineBindPoint bindPoint{};
	};

}
