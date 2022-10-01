#pragma once
#include <functional>
#include <string>
#include <vector>
#include <Window.h>

#include "BindGroupAllocator.h"
#include "CommandList.h"
#include "Resource.h"

namespace toy::renderer
{
	using namespace core;
	class CommandList;

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

	template <typename T>
	struct Handle
	{
		uint32_t index{};
	};


	enum class AccessUsage : FlagBits
	{
		none = 0 << 0,
		uniform = 1 << 0,
		index = 1 << 1,
		vertex = 1 << 2,
		storage = 1 << 3
	};
	

	enum class UsageContext
	{
		gpuOnly,
		cpuRead,
		cpuWrite
	};


	struct BufferDescriptor
	{
		uint32_t size{};
		Flags<AccessUsage> accessUsage;
		Flags<UsageContext> usageContext;
	};

	struct Buffer
	{
		uint64_t size;
	};

	struct Extent
	{
		u32 width;
		u32 height;
	};

	struct RendererDescriptor
	{
		u32 version;
		std::string instanceName;
		window::WindowHandler handler;
		window::BackendRendererMeta meta;
		std::function<Extent()> windowExtentGetter;
	};

	struct RenderTarget {};
	struct Texture {};

	

	

	


	struct Pipeline {};
	struct ShaderModule {};

	enum class Format
	{
		RGBA8,
		RGBA16,
		R11G11B10
	};

	struct ColorRenderTargetDescriptor
	{
		Format format;
	};
	struct DepthRenderTargetDescriptor {};
	struct StencilRenderTargetDescriptor {};
	struct RenderTargetsDescription
	{
		std::vector<ColorRenderTargetDescriptor> colorRenderTargets;
		DepthRenderTargetDescriptor depthRenderTarget;
		StencilRenderTargetDescriptor stencilRenderTarget;
	};

	struct PipelineState
	{
		bool depthTestEnabled;
	};

	struct GraphicsPipelineDescriptor
	{
		ShaderModule vertexShader;
		ShaderModule fragmentShader;
		RenderTargetsDescription renderTargetDescription;
		PipelineState state{};
	};


	struct SwapchainImage
	{
		std::unique_ptr<ImageResource> image;
		std::unique_ptr<ImageView> view;
	};

	class RenderInterface
	{
	public:
		virtual void initialize(RendererDescriptor descriptor) = 0;
		virtual void deinitialize() = 0;

		virtual ~RenderInterface() = default;
		/*virtual Handle<Buffer> createBuffer(const BufferDescriptor& descriptor) = 0;*/
		virtual std::unique_ptr<CommandList> acquireCommandList(QueueType queueType, CommandListType commandListType = CommandListType::primary) = 0;

		[[nodiscard]] BindGroup allocateBindGroup(const BindGroupDescriptor& descriptor, const BindGroupLayout& layout);
		[[nodiscard]] BindGroup allocateBindGroup(const BindGroupDescriptor& descriptor);
		[[nodiscard]] BindGroupLayout allocateBindGroupLayout(const BindGroupDescriptor& descriptor);

		//this function should be thread save????
		//Do I need make multi threaded resource creation? It can depend on Frame Graph resource management.
		//virtual Handle<RenderTarget> createRenderTarget(RenderTargetDescriptor) = 0;
		//=================
		virtual void nextFrame() = 0;
		/*virtual void present();
		virtual void waitForSwapchain();*/

		//draft for pipeline creation
		/*virtual Handle<Pipeline> createPipeline(const GraphicsPipelineDescriptor& graphicsPipelineDescription, const std::vector<BindGroupDescriptor>& bindGroups = {}) = 0;*/


		virtual [[nodiscard]] SwapchainImage/*Accessor<ImageResource>*/ acquireNextSwapchainImage() = 0;

		virtual void submitCommandList(const std::unique_ptr<CommandList> commandList) = 0;

		virtual void present() = 0;
	private:
		[[nodiscard]] virtual BindGroupLayout allocateBindGroupLayoutInternal(const BindGroupDescriptor& descriptor) = 0;
		std::unordered_map<u64, BindGroupLayout> bindGroupLayoutCache_;
	};

	struct PipelineDescriptor
	{

	};

	struct PipelinePool
	{
	};
}




namespace ideas::renderer
{
	struct BindGroup {};


	template <typename T>
	struct Handle
	{
		T* type;
		T* operator->()
		{
			return type;
		}
	};

	struct Viewport {};
	struct Scissor {};

	struct RenderInterface
	{
		struct ShaderProgram {};

		struct CommandList
		{
			void beginRendering() {}
			void endRendering() {}

			void bindProgram(Handle<ShaderProgram> program)
			{}

			void setViewport(Viewport& viewport) {}
			void setScissor(Scissor& scissor) {}

			void bindBindGroup(BindGroup& group) {}

			void draw(uint32_t vertexCount,
				uint32_t instanceCount,
				uint32_t firstVertex,
				uint32_t firstInstance) {}
		};

		struct PipelineLayout {};
		struct BindGroupLayout {};
		struct RayTracingShaderModules {};
		struct ComputeShaderModules {};
		struct GraphicsShaderModules {};
		struct AccelerationStructure {};

		Handle<ShaderProgram> createShaderProgram(PipelineLayout& layout, RayTracingShaderModules& modules)
		{
			return Handle<ShaderProgram>();
		}

		Handle<ShaderProgram> createShaderProgram(PipelineLayout& layout, ComputeShaderModules& modules)
		{
			return Handle<ShaderProgram>();
		}

		Handle<ShaderProgram> createShaderProgram(PipelineLayout& layout, GraphicsShaderModules& modules)
		{
			return Handle<ShaderProgram>();
		}

		Handle<BindGroup> createBindGroup(BindGroupLayout)
		{
			return Handle<BindGroup>();
		}

		Handle<CommandList> acquireCommandList()
		{
			return Handle<CommandList>();
		}

		Handle<AccelerationStructure> createAccelerationStructure()
		{
			return Handle<AccelerationStructure>();
		}

	};

	struct FrameResource {};

	struct FrameGraphBuilder
	{
		FrameResource createTexture() {}
		FrameResource createBuffer() {}

		FrameResource renderTarget(FrameResource renderTarget) {}

	};

	struct FrameGraph
	{
		void addPass();
	};
}
