#pragma once

#include <BindGroupAllocator.h>
#include <vector>

#include "Vulkan.h"

namespace toy::renderer::api::vulkan
{
	struct VulkanBindGroupLayout : BindGroupLayout
	{
		vk::DescriptorSetLayout layout;
	};

	struct VulkanBindGroup : BindGroup
	{
		vk::DescriptorSet descriptorSet;
	};

	class VulkanBindingGroupAllocator final : public BindGroupAllocator
	{
	
	};
}
