#pragma once
//#define RENDERER_VALIDATION
#include <CommonTypes.h>
#include <Window.h>

#include <functional>
#include <string>
#include <vector>
#include <variant>
#include "BindGroupAllocator.h"
#include "CommandListValidator.h"



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


	using FlagBits = uint32_t;

	enum class AccessUsage : FlagBits
	{
		none = 0 << 0,
		uniform = 1 << 0,
		index = 1 << 1,
		vertex = 1 << 2,
		storage = 1 << 3
	};

	template<typename FlagBits>
	struct Flags
	{
		explicit Flags(const FlagBits& bit)
		{
			*this |= (bit);
		}

		Flags& operator |=(const FlagBits& bit)
		{
			flags |= static_cast<uint32_t>(bit);
			return *this;
		}

		[[nodiscard]] bool containBit(const FlagBits& bit) const
		{
			return static_cast<bool>(flags & static_cast<uint32_t>(bit));
		}

	private:
		uint32_t flags{};
	};

	/*template<typename FlagBits>
	Flags<FlagBits> operator|(const FlagBits& lhs, const FlagBits& rhs)
	{
		Flags<FlagBits> flag;
		flag |= lhs;
		flag |= rhs;
		return flag;
	}*/


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

	struct Resource
	{};

	struct ImageResource : Resource
	{
		
	};

	struct ImageView : Resource
	{
		
	};

	template <typename T>
	struct Accessor
	{
		/*T* resource;
		View<T> view;*/
	};

	struct RenderTargetDescriptor
	{
		ImageView* renderTargetImageAccessor{};
		LoadOperation load{};
		StoreOperation store{};
		ResolveMode resolveMode{};
		ColorClear clearValue{};
	};


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
		virtual Handle<Buffer> createBuffer(const BufferDescriptor& descriptor) = 0;
		virtual std::unique_ptr<CommandList> acquireCommandList(QueueType queueType, CommandListType commandListType = CommandListType::primary) = 0;

		[[nodiscard]] BindGroup allocateBindGroup(const BindGroupDescriptor& descriptor, const BindGroupLayout& layout);
		[[nodiscard]] BindGroup allocateBindGroup(const BindGroupDescriptor& descriptor);
		[[nodiscard]] BindGroupLayout allocateBindGroupLayout(const BindGroupDescriptor& descriptor);

		//this function should be thread save????
		//Do I need make multi threaded resource creation? It can depend on Frame Graph resource management.
		virtual Handle<RenderTarget> createRenderTarget(RenderTargetDescriptor) = 0;
		//=================
		virtual void nextFrame() = 0;
		/*virtual void present();
		virtual void waitForSwapchain();*/

		//draft for pipeline creation
		virtual Handle<Pipeline> createPipeline(const GraphicsPipelineDescriptor& graphicsPipelineDescription, const std::vector<BindGroupDescriptor>& bindGroups = {}) = 0;


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



	
	enum class Layout
	{
		Undefined,
		Present,
		ColorRenderTarget,
		DepthStencilRenderTarget,
		TransferSrc,
		TransferDst,
		ShaderRead,
		ShaderReadWrite
	};

	enum class ShaderStageUsage
	{
		vertex,
		fragment,
		task
	};

	enum class ResourcePipelineStageUsageFlagBits : FlagBits
	{
		none = 0 << 0,
		vertex = 1 << 0,
		fragment = 1 << 1,
		compute = 1 << 2,
	};

	using ResourcePipelineStageUsageFlags = Flags<ResourcePipelineStageUsageFlagBits>;


	struct ImageBarrierDescriptor
	{
		Layout srcLayout;
		Layout dstLayout;
		ResourcePipelineStageUsageFlags srcStage{ResourcePipelineStageUsageFlagBits::none};
		ResourcePipelineStageUsageFlags dstStage{ ResourcePipelineStageUsageFlagBits::none };
		ImageResource* image;
		//???
	};
	struct MemoryBarrierDescriptor{};
	struct BufferBarrierDescriptor{};


	using BarrierDescriptor = std::variant<ImageBarrierDescriptor, BufferBarrierDescriptor, MemoryBarrierDescriptor>;

	struct SplitBarrier
	{
		/*
		 * in vulkan for instance, we have to store event and wait for it
		 */
	};

	struct RenderingDescriptor
	{
		std::vector<RenderTargetDescriptor> colorRenderTargets{};
		RenderTargetDescriptor depthRenderTarget{};
		RenderTargetDescriptor stencilRenderTarget{};
	};

	struct RenderArea
	{
		i32 x;
		i32 y;
		u32 width;
		u32 height;
	};

	class CommandList
	{
	public:
		CommandList(const CommandList& other) = delete;
		CommandList(CommandList&& other) noexcept = default;
		CommandList& operator=(const CommandList& other) = default;
		CommandList& operator=(CommandList&& other) noexcept = default;

		explicit CommandList(QueueType queueType);
		virtual ~CommandList() = default;

		void barrier(const std::initializer_list<BarrierDescriptor>& descriptors);
		/*Handle<SplitBarrier> beginSplitBarrier(const BarrierDescriptor& descriptor);
		void endSplitBarrier(Handle<SplitBarrier> barrier);*/

		void beginRendering(const RenderingDescriptor& descriptor);
		void beginRendering(const RenderingDescriptor& descriptor, const RenderArea& area);
		void endRendering();
	protected:

		virtual void barrierInternal(const std::initializer_list<BarrierDescriptor>& descriptors) = 0;

		virtual void beginRenderingInternal(const RenderingDescriptor& descriptor, const RenderArea& area) = 0;
		virtual void endRenderingInternal() = 0;

		QueueType ownedQueueType_{};

	private:
#ifdef RENDERER_VALIDATION
		DECLARE_VALIDATOR(validation::CommandListValidator);
#endif
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
