#pragma once
#include <CommonTypes.h>
#include <functional>
#include <optional>
#include <string>

#include "BindGroupAllocator.h"
#include "Resource.h"
#include "Window.h"
#include "glm/fwd.hpp"

namespace toy::renderer
{
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

	enum class CommandListType
	{
		primary,
		secondary
	};

	enum class QueueType
	{
		graphics,
		asyncCompute,
		transfer
	};


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
		glm::u32 height;
	};

	struct RendererDescriptor
	{
		core::u32 version;
		std::string instanceName;
		window::WindowHandler handler;
		window::BackendRendererMeta meta;
		std::function<WindowExtent()> windowExtentGetter;
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

	struct ShaderModule {};

	enum class Format
	{
		rgba8,
		rgba16,
		d16,
		d32
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

	struct PipelineState
	{
		bool depthTestEnabled;
	};
	
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

	struct SetBindGroupMapping
	{
		core::u32 set{0};
		Handle<BindGroupLayout> bindGroupLayout{};
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

	struct BindingDataMapping
	{
		core::u32 binding{0};
		std::variant<CBV, UAV> view{};
		core::u32 arrayElement{};
	};

	struct BufferDescriptor
	{
		core::u32 size{};
		core::Flags<BufferAccessUsage> accessUsage;
		MemoryUsage memoryUsage{ MemoryUsage::gpuOnly };
		core::Flags<QueuesSharing> queuesSharing{ QueuesSharing::graphics };
	};

	struct Extent
	{
		glm::u32 width{ 1 };
		glm::u32 height{ 1 };
		glm::u32 depth{ 1 };
	};

	struct ImageDescriptor
	{
		Format format{};
		Extent extent{};
		glm::u32 mips{};
		glm::u32 layers{};
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
}
