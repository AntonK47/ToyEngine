#pragma once
#include <Core.h>
#include <optional>
#include <vector>
#include <functional>
#include <string>
#include <variant>
#include <array>
#include <glm/glm.hpp>
#include <thread>

#include "Handle.h"

namespace toy::graphics::rhi
{
	enum class HeapType
	{
		unknown,
		host,
		device,
		coherent
	};

	struct PerHeapBudget
	{
		HeapType type{HeapType::unknown};
		core::u64 totalMemory;
		core::u64 availableMemory;
		core::u64 budgetMemory;
	};

	struct MemoryBudget
	{
		core::u32 heapCount{};
		std::array<PerHeapBudget, 16> budget{};
	};

	enum class ImageViewType
	{
		_1D,
		_2D,
		_3D
	};

	enum class ImageViewAspect
	{
		color,
		depth
	};

	struct ImageView {};
	struct Image {};
	struct Buffer {};

	struct Sampler {};

	struct VirtualTexture
	{};

	struct Rectangle2D
	{
		core::i32 x;
		core::i32 y;
		core::u32 width;
		core::u32 height;
	};
	using RenderArea = Rectangle2D;

	using Scissor = Rectangle2D;

	struct Viewport
	{
		float x;
		float y;
		float width;
		float height;
	};

	struct Pipeline {};

	enum class BufferAccessUsage : core::FlagBits
	{
		none = 0 << 0,
		uniform = 1 << 0,
		index = 1 << 1,
		vertex = 1 << 2,
		storage = 1 << 3,
		indirect = 1 << 4,
		accelerationStructure = 1 << 5,
		transferSrc = 1 << 6,
		transferDst = 1 << 7
	};

	enum class ImageAccessUsage : core::FlagBits
	{
		none = 0 << 0,
		sampled = 1 << 0,
		storage = 1 << 1,
		colorAttachment = 1 << 2,
		depthStencilAttachment = 1 << 3,
		transferSrc = 1 << 4,
		transferDst = 1 << 5
	};

	enum class QueuesSharing : core::FlagBits
	{
		graphics = 0 << 0,
		asyncCompute = 1 << 0,
		transfer = 1 << 1,
	};

	enum class MemoryUsage
	{
		gpuOnly,
		cpuOnly,
		cpuRead,
		cpuWrite
	};

	struct WindowExtent
	{
		core::u32 width;
		core::u32 height;
	};

	struct RendererDescriptor
	{
		core::u32 version;
		std::string instanceName;
		window::WindowHandler handler;
		window::BackendRendererMeta meta;
		std::function<WindowExtent()> windowExtentGetter;
		std::vector<std::thread::id> workers{};
	}; 
	
	struct WorkerThreadId
	{
		core::u32 index;
	};


	struct RenderTarget {};
	struct Texture {};

	//TODO: move to shared struct definitions
	enum class ShaderStage
	{
		vertex,
		fragment,
		geometry,
		tessellationControl,
		tessellationEvaluation,
		task,
		mesh,
		anyHit,
		closestHit,
		miss,
		rayGeneration,
		intersection,
	};

	using ShaderByteCode = std::vector<core::u32>;

	enum class ShaderLanguage
	{
		spirv1_6
	};

	struct ShaderCode
	{
		ShaderLanguage language;
		ShaderByteCode code;
	};

	

	enum class Format
	{
		r8,
		rgba8,
		rgba16,
		d16,
		d32,
		bc7
	};

	enum class ColorFormat
	{
		rgba8,
		rgba16,
	};

	enum class DepthFormat
	{
		d16,
		d32
	};
	using StencilFormat = Format;

	struct ColorRenderTargetDescriptor
	{
		ColorFormat format;
	};
	struct DepthRenderTargetDescriptor
	{
		DepthFormat format;
	};
	struct StencilRenderTargetDescriptor
	{
		StencilFormat format;
	};
	struct RenderTargetsDescriptor
	{
		std::vector<ColorRenderTargetDescriptor> colorRenderTargets;
		std::optional<DepthRenderTargetDescriptor> depthRenderTarget;
		std::optional<StencilRenderTargetDescriptor> stencilRenderTarget;
	};

	enum class FaceCull
	{
		front,
		back,
		none,
	};

	enum class Blending
	{
		alphaBlend,
		none
	};

	struct PipelineState
	{
		bool depthTestEnabled{ false };
		FaceCull faceCulling{ FaceCull::none };
		Blending blending{ Blending::none };
	};

	struct ShaderModule{};
	
	struct GraphicsPipelineDescriptor
	{
		Handle<ShaderModule> vertexShader;
		Handle<ShaderModule> fragmentShader;
		RenderTargetsDescriptor renderTargetDescriptor;
		PipelineState state{};
	};

	struct ComputePipelineDescriptor
	{
		Handle<ShaderModule> computeShader;
	};

	struct RayTracingPipelineDescriptor
	{
		
	};

	struct AccelerationStructure
	{};

	struct AccelerationStructureDescriptor
	{
		core::u32 primitiveCount;
		core::u32 primitiveOffset;
	};

	struct BindGroupLayout{};

	struct SetBindGroupMapping
	{
		core::u32 set{0};
		Handle<BindGroupLayout> bindGroupLayout{};
	};

	struct PushConstant
	{
		core::u32 size;
	};

	struct SwapchainImage
	{
		Handle<Image> image;
		Handle<ImageView> view;
	};

	struct BufferView
	{
		Handle<Buffer> buffer;
		core::u32 offset{};
		core::u64 size{};
	};

	struct CBV
	{
		BufferView bufferView;
	};

	struct UAV
	{
		BufferView bufferView;
	};

	struct Texture2DSRV
	{
		Handle<ImageView> imageView;
	};

	struct SamplerSRV
	{
		Handle<Sampler> sampler;
	};

	

	struct BindingDataMapping
	{
		core::u32 binding{0};
		std::variant<CBV, UAV, Texture2DSRV, SamplerSRV> view{};
		core::u32 arrayElement{};
	};

	struct BufferDescriptor
	{
		core::u32 size{};
		core::Flags<BufferAccessUsage> accessUsage;
		MemoryUsage memoryUsage{ MemoryUsage::gpuOnly };
		core::Flags<QueuesSharing> queuesSharing{ QueuesSharing::graphics };
	};

	enum class Filter
	{
		nearest,
		linear,
		cubic
	};

	enum class MipFilter
	{
		nearest,
		linear
	};

	struct SamplerDescriptor
	{
		Filter magFilter;
		Filter minFilter;
		MipFilter mipFilter;
	};

	struct Extent
	{
		core::u32 width{ 1 };
		core::u32 height{ 1 };
		core::u32 depth{ 1 };
	};

	struct ImageDescriptor
	{
		Format format{};
		Extent extent{};
		core::u32 mips{};
		core::u32 layers{};
		core::Flags<ImageAccessUsage> accessUsage{ ImageAccessUsage::none };
		MemoryUsage memoryUsage{ MemoryUsage::gpuOnly };
		core::Flags<QueuesSharing> queuesSharing{ QueuesSharing::graphics };
	};

	struct ImageViewDescriptor
	{
		Handle<Image> image{};
		Format format{}; //TODO: it will cause lot of vulkan errors, maybe it can be derived
		ImageViewType type{};
		ImageViewAspect aspect{ ImageViewAspect::color };
	};

	struct VirtualTextureDescriptor
	{

	};

	template <typename T>
	class Ref
	{
	public:
		explicit Ref(T* object) : object_{ object }
		{}

		template <typename R>
		[[nodiscard]] const R& query() const
		{
			return *static_cast<const R*>(object_);
		}
	private:
		T* object_{};
	};

	enum class Layout
	{
		undefined,
		present,
		colorRenderTarget,
		depthStencilRenderTarget,
		transferSrc,
		transferDst,
		shaderRead,
		shaderReadWrite
	};

	enum class ShaderStageUsage
	{
		vertex,
		fragment,
		task
	};

	enum class ResourcePipelineStageUsageFlagBits : core::FlagBits
	{
		none = 0 << 0,
		vertex = 1 << 0,
		fragment = 1 << 1,
		compute = 1 << 2,
	};

	using ResourcePipelineStageUsageFlags = core::Flags<ResourcePipelineStageUsageFlagBits>;


	struct ImageBarrierDescriptor
	{
		Layout srcLayout;
		Layout dstLayout;
		ResourcePipelineStageUsageFlags srcStage{ ResourcePipelineStageUsageFlagBits::none }; //TODO: I don't like this design desition, because in contextes like data transfer it doesn't make sence.
		ResourcePipelineStageUsageFlags dstStage{ ResourcePipelineStageUsageFlagBits::none };
		ImageViewAspect aspect;
		Handle<Image> image;
		//???
	};
	struct MemoryBarrierDescriptor {};
	struct BufferBarrierDescriptor {};


	using BarrierDescriptor = std::variant<ImageBarrierDescriptor, BufferBarrierDescriptor, MemoryBarrierDescriptor>;

	struct SplitBarrier
	{
		/*
		 * in vulkan for instance, we have to store event and wait for it
		 */
	};

	enum class LoadOperation
	{
		load,
		clear,
		dontCare,
		none
	};

	enum class StoreOperation
	{
		store,
		dontCare,
		none
	};

	enum class ResolveMode
	{
		min,
		max,
		avg,
		none
	};

	struct ColorClear
	{
		float r;
		float g;
		float b;
		float a;
	};

	struct DepthClear
	{
		float depth;
	};

	using ClearValue = std::variant<ColorClear, DepthClear>;

	struct RenderTargetDescriptor
	{
		Handle<ImageView> imageView{};
		LoadOperation load{};
		StoreOperation store{};
		ResolveMode resolveMode{};
		ClearValue clearValue{};
	};

	struct RenderingDescriptor
	{
		std::vector<RenderTargetDescriptor> colorRenderTargets{};//TODO: smallvector
		std::optional<RenderTargetDescriptor> depthRenderTarget{};
		std::optional<RenderTargetDescriptor> stencilRenderTarget{};
	};

	enum class GeometryBehavior
	{
		none,
		translucent,
		opaque,
		hitAnyOnce
	};

	struct TriangleGeometry
	{
		Handle<Buffer> indexBuffer;
		Handle<Buffer> vertexBuffer;
		core::u32 totalVertices{ 0 };
		core::u32 vertexStride{ 0 };
		GeometryBehavior behavior{ GeometryBehavior::none };
	};

	struct AccelerationStructureInstance
	{
		glm::mat3x4 transform{};
		core::u32 index{};
		core::u8 visibilityMask{};
		//...
		Handle<AccelerationStructure> blas;
	};

	struct SourceBufferDescriptor
	{
		Handle<Buffer> buffer;
		core::u32 offset;
	};

	struct Region
	{
		core::u32 mip{};
		core::u32 baseLayer{};
		core::u32 layerCount{};
		glm::uvec3 extent{};
		glm::ivec3 offset{ 0,0,0 };
		core::u32 bufferOffset{};
	};

	struct DestinationImageDescriptor
	{
		Handle<Image> image;
		std::vector<Region> regions{};
	};

	enum class IndexType
	{
		index16,
		index32
	};

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
		core::u32 descriptorCount{ 1 };
	};

	struct Binding
	{
		core::u32 binding{};
		BindingDescriptor descriptor{};
	};


	enum class BindGroupFlag : core::FlagBits
	{
		none = 0 << 0,
		unboundLast = 1 << 0,
	};

	enum class UsageScope
	{
		inFrame,
		async
	};

	struct BindGroupDescriptor
	{
		std::vector<Binding> bindings{};
		core::Flags<BindGroupFlag> flags{ BindGroupFlag::none };

	};

	struct BindGroup
	{
		UsageScope scope;
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
}
