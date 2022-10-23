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

	struct SimpleDeclaration
	{
		BindingType type;
	};
	struct ArrayDeclaration
	{
		BindingType type;
		core::u32 elementsCount;
	};
	struct BindlessDeclaration
	{
		BindingType type;
		core::u32 maxDescriptorCount;
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

	struct Device{};

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
		core::u32 swapchainFrameCount;
		DescriptorPoolDefaultSizes size{};
	};

	class BindGroupAllocator
	{
	public:
		BindGroupAllocator(const BindGroupAllocator& other) = delete;

		void initialize(Device& device, const BindGroupAllocatorDescriptor& descriptor);
		void deinitialize();
		void nextFrame();


		[[nodiscard]] std::unique_ptr<BindGroup> allocateBindGroup(const BindGroupLayout& layout);
		[[nodiscard]] std::unique_ptr<BindGroup> allocateBindGroup(const BindGroupDescriptor& descriptor);
		[[nodiscard]] BindGroupLayout* allocateBindGroupLayout(const BindGroupDescriptor& descriptor);
		virtual ~BindGroupAllocator() = default;
	private:
		virtual std::unique_ptr<BindGroupLayout> allocateBindGroupLayoutInternal(const BindGroupDescriptor& descriptor) = 0;
		virtual std::unique_ptr<BindGroup> allocateBindGroupInternal(const BindGroupLayout& layout) = 0;
		virtual void initializeInternal() = 0;

		std::unordered_map<core::u64, std::unique_ptr<BindGroupLayout>> bindGroupLayoutCache_;
		std::vector<std::unordered_map<core::u64, std::unique_ptr<BindGroup>>> bindGroupCache_;
		
		
	protected:
		core::u32 frameIndex_{};
		core::u32 swapchainFrameCount_{};
		Device* device_;
	};
}