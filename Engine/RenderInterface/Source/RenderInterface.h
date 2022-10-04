#pragma once
#include <functional>
#include <string>
#include <vector>
#include <Window.h>
#include <optional>

#include "BindGroupAllocator.h"
#include "CommandList.h"
#include "Resource.h"
#include "RenderInterfaceValidator.h"

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
	
	using ShaderByteCode = std::vector<u32>;

	enum class ShaderLanguage
	{
		Spirv1_6
	};

	struct ShaderCode
	{
		ShaderLanguage language;
		ShaderByteCode code;
	};


	//struct Pipeline {};
	struct ShaderModule {};

	enum class Format
	{
		RGBA8,
		RGBA16,
		R11G11B10
	};

	using DepthFormat = Format;
	using StencilFormat = Format;

	struct ColorRenderTargetDescriptor
	{
		Format format;
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
		ShaderModule* vertexShader;
		ShaderModule* fragmentShader;
		RenderTargetsDescriptor renderTargetDescriptor;
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
		RenderInterface(const RenderInterface& other) = delete;
		RenderInterface(RenderInterface&& other) noexcept = default;

		RenderInterface& operator=(const RenderInterface& other) = default;
		RenderInterface& operator=(RenderInterface&& other) noexcept = default;

		RenderInterface() = default;
		virtual ~RenderInterface() = default;

		void initialize(const RendererDescriptor& descriptor);
		void deinitialize();


		[[nodiscard]] std::unique_ptr<CommandList> acquireCommandList(QueueType queueType, CommandListType commandListType = CommandListType::primary);
		void submitCommandList(std::unique_ptr<CommandList> commandList);
		
		void nextFrame();

		[[nodiscard]] SwapchainImage acquireNextSwapchainImage();
		void present();


		[[nodiscard]] BindGroup allocateBindGroup(const BindGroupDescriptor& descriptor, const BindGroupLayout& layout);
		[[nodiscard]] BindGroup allocateBindGroup(const BindGroupDescriptor& descriptor);
		[[nodiscard]] BindGroupLayout allocateBindGroupLayout(const BindGroupDescriptor& descriptor);

		//this function should be thread save????
		//Do I need make multi threaded resource creation? It can depend on Frame Graph resource management.
		//virtual Handle<RenderTarget> createRenderTarget(RenderTargetDescriptor) = 0;

		//draft for pipeline creation
		[[nodiscard]] std::unique_ptr<Pipeline> createPipeline(const GraphicsPipelineDescriptor& graphicsPipelineDescriptor, const std::vector<BindGroupDescriptor>& bindGroups = {});

		[[nodiscard]] std::unique_ptr<ShaderModule> createShaderModule(ShaderStage stage, const ShaderCode& code);

		/*virtual Handle<Buffer> createBuffer(const BufferDescriptor& descriptor) = 0;*/
		//=================
	protected:
		virtual void initializeInternal(const RendererDescriptor& descriptor) = 0;
		virtual void deinitializeInternal() = 0;

		virtual [[nodiscard]] SwapchainImage acquireNextSwapchainImageInternal() = 0;

		virtual [[nodiscard]] std::unique_ptr<CommandList> acquireCommandListInternal(QueueType queueType, CommandListType commandListType = CommandListType::primary) = 0;
		virtual [[nodiscard]] void submitCommandListInternal(std::unique_ptr<CommandList> commandList) = 0;

		virtual [[nodiscard]] std::unique_ptr<Pipeline> createPipelineInternal(const GraphicsPipelineDescriptor& descriptor, const std::vector<BindGroupDescriptor>& bindGroups) = 0;

		virtual [[nodiscard]] std::unique_ptr<ShaderModule> createShaderModuleInternal(ShaderStage stage, const ShaderCode& code) = 0;

		virtual void nextFrameInternal() = 0;
		virtual void presentInternal() = 0;
	private:
		[[nodiscard]] virtual BindGroupLayout allocateBindGroupLayoutInternal(const BindGroupDescriptor& descriptor) = 0;
		std::unordered_map<u64, BindGroupLayout> bindGroupLayoutCache_;

		DECLARE_VALIDATOR(validation::RenderInterfaceValidator);
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
