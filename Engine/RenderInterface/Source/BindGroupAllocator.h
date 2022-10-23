#pragma once
#include <CommonTypes.h>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

namespace toy::renderer
{
	enum class BindingType
	{
		Texture1D,
		Texture2D,
		Texture3D,
		Texture2DArray,
		UniformBuffer,
		StorageBuffer,
		AccelerationStructure,
		Sampler
	};

	struct BindingDescriptor
	{
		BindingType type{};
		core::u32 descriptorCount{1};
	};

	struct Binding
	{
		core::u32 binding{};
		BindingDescriptor descriptor{};
	};

	struct BindGroupLayout {};

	enum class BindGroupFlag : core::FlagBits
	{
		none = 0 << 0,
		unboundLast = 1 << 0,
	};

	enum class BindGroupUsageScope
	{
		perFrame,
		persistent //global?
	};

	struct BindGroupDescriptor
	{
		std::vector<Binding> bindings;
		core::Flags<BindGroupFlag> flags{ BindGroupFlag::none };

	};

	struct BindGroup
	{
		BindGroupUsageScope scope;
	};


	struct DescriptorPoolDefaultSizes
	{
		core::u32 accelerationStructures{ 100 };
		core::u32 storageBuffers{ 10 };
		core::u32 uniformBuffers{ 100 };
		core::u32 images{ 100 };
		core::u32 samplers{ 100 };
	};

	struct BindGroupAllocatorDescriptor
	{
		core::u32 swapchainFrameCount{};
		DescriptorPoolDefaultSizes size{};
	};

	class BindGroupAllocator
	{
	
	};
}