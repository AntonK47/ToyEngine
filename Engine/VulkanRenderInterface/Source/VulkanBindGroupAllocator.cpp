#include "VulkanBindGroupAllocator.h"

#include "ValidationCommon.h"

using namespace toy::renderer::api::vulkan;
using namespace toy::core;
using namespace toy::renderer;

namespace 
{
    vk::DescriptorType mapDescriptorType(const BindingType type)
    {
        switch (type)
        {
        case BindingType::Texture1D:

        case BindingType::Texture2D:

        case BindingType::Texture3D:

        case BindingType::Texture2DArray:
            return vk::DescriptorType::eSampledImage;
        case BindingType::UniformBuffer:
            return vk::DescriptorType::eUniformBuffer;
        case BindingType::StorageBuffer:
            return vk::DescriptorType::eStorageBuffer;

        case BindingType::AccelerationStructure:
            return vk::DescriptorType::eAccelerationStructureKHR;

        case BindingType::Sampler:
            return vk::DescriptorType::eSampler;
        }
    }

	vk::DescriptorPool allocateDescriptorPool(const vk::Device& device, const DescriptorPoolDefaultSizes& defaultSizes)
	{
		const auto poolSizes = std::array
		{
			vk::DescriptorPoolSize
			{
				.type = vk::DescriptorType::eUniformBuffer,
				.descriptorCount = defaultSizes.uniformBuffers
			}
		};

		const auto poolCreateInfo = vk::DescriptorPoolCreateInfo
		{
			.flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
			.maxSets = 1000,
			.poolSizeCount = poolSizes.size(),
			.pPoolSizes = poolSizes.data()
		};
		const auto [result, descriptorPool] = device.createDescriptorPool(poolCreateInfo);


		return descriptorPool;
	}
}

std::unique_ptr<BindGroupLayout> VulkanBindingGroupAllocator::allocateBindGroupLayoutInternal(
	const BindGroupDescriptor& descriptor)
{
    auto bindings = std::vector<vk::DescriptorSetLayoutBinding>{};
    auto bindingFlags = std::vector<vk::DescriptorBindingFlags>{};
    bindings.resize(descriptor.bindings.size());
    bindingFlags.resize(descriptor.bindings.size());
    for (u32 i{}; i < bindings.size(); i++)
    {
        bindings[i].binding = descriptor.bindings[i].binding;
        bindingFlags[i] = vk::DescriptorBindingFlagBits::eUpdateAfterBind;
        if (std::holds_alternative<SimpleDeclaration>(descriptor.bindings[i].descriptor))
        {
            const auto simpleBinding = std::get<SimpleDeclaration>(descriptor.bindings[i].descriptor);
            bindings[i].descriptorType = mapDescriptorType(simpleBinding.type);
            bindings[i].descriptorCount = 1;
        }
        if (std::holds_alternative<ArrayDeclaration>(descriptor.bindings[i].descriptor))
        {
            const auto arrayBinding = std::get<ArrayDeclaration>(descriptor.bindings[i].descriptor);
            bindings[i].descriptorType = mapDescriptorType(arrayBinding.type);
            bindings[i].descriptorCount = arrayBinding.elementsCount;
        }
        if (std::holds_alternative<BindlessDeclaration>(descriptor.bindings[i].descriptor))
        {
            //TODO: this binding should be the last one in the descriptor set
            const auto bindlessBinding = std::get<BindlessDeclaration>(descriptor.bindings[i].descriptor);
            bindings[i].descriptorType = mapDescriptorType(bindlessBinding.type);
            bindings[i].descriptorCount = bindlessBinding.maxDescriptorCount;
            bindingFlags[i] = vk::DescriptorBindingFlagBits::eVariableDescriptorCount;
        }

    }

    //TODO: check for bindless feature

    const auto createInfo = vk::StructureChain
    {
        vk::DescriptorSetLayoutCreateInfo
        {
            .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
            .bindingCount = static_cast<u32>(bindings.size()),
            .pBindings = bindings.data()
        },
        vk::DescriptorSetLayoutBindingFlagsCreateInfo
        {
            .bindingCount = static_cast<u32>(bindings.size()),
            .pBindingFlags = bindingFlags.data()
        }
    };
    const auto vulkanDevice = static_cast<VulkanDevice&>(*device_);

    auto [result, layout] = vulkanDevice.logicalDevice.createDescriptorSetLayout(createInfo.get());

    TOY_ASSERT(result == vk::Result::eSuccess);

    return std::make_unique<VulkanBindGroupLayout>(VulkanBindGroupLayout{ .layout = layout });
}

void VulkanBindingGroupAllocator::initializeInternal()
{
	freeDescriptorPools_ = {};
}

void VulkanBindingGroupAllocator::resetPoolsUpToFrame(core::u32 frameIndex)
{
    const auto vulkanDevice = static_cast<VulkanDevice&>(*device_);

    for (u32 i{ (frameIndex_ + 1) % swapchainFrameCount_ }; i != frameIndex; i = (i + 1) % swapchainFrameCount_)
    {
	    for(const auto pool: usedDescriptorPools_[i])
	    {
            vulkanDevice.logicalDevice.resetDescriptorPool(pool);
	    }
    }
}
