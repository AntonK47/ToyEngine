#pragma once

#include <BindGroupAllocator.h>
#include <vector>

#include "Vulkan.h"

namespace toy::renderer::api::vulkan
{

	struct VulkanDevice : Device
	{
		vk::Device logicalDevice;
	};

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
	private:
		std::unique_ptr<BindGroupLayout>
		allocateBindGroupLayoutInternal(const BindGroupDescriptor& descriptor) override;
		std::unique_ptr<BindGroup> allocateBindGroupInternal(const BindGroupLayout& layout) override;
		void initializeInternal() override;

		void resetPoolsUpToFrame(core::u32 frameIndex);
		void createFreeDescriptorPool();

		std::vector<vk::DescriptorPool> freeDescriptorPools_{};
		std::vector<std::vector<vk::DescriptorPool>> usedDescriptorPools_{};
	};
	
}
