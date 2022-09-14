#pragma once

#include <CommonTypes.h>
#include <Window.h>

#include <functional>
#include <string>
#include <vector>


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

	struct RenderTargetDescription
	{
		Handle<Texture> texture;//Maybe I should have some type for render targets, that also contains image view and appropriate image layout.
		LoadOperation load;
		StoreOperation store;
		ResolveMode resolveMode;
		ColorClear clearValue;
	};


	struct Pipeline {};
	struct ShaderModule {};

	struct ColorRenderTargetDescription {};
	struct DepthRenderTargetDescription {};
	struct StencilRenderTargetDescription {};
	struct RenderTargetsDescription
	{
		std::vector<ColorRenderTargetDescription> colorRenderTargets;
		DepthRenderTargetDescription depthRenderTarget;
		StencilRenderTargetDescription stencilRenderTarget;
	};

	struct PipelineState
	{
		bool depthTestEnabled;
	};

	struct GraphicsPipelineDescription
	{
		ShaderModule vertexShader;
		ShaderModule fragmentShader;
		RenderTargetsDescription renderTargetDescription;
		PipelineState state{};
	};

	struct UniformDeclaration {};
	struct Sampler2DDeclaration {};
	struct BindlessArrayDeclaration {};

	struct Binding
	{
		u32 binding;
		union
		{
			UniformDeclaration uniform;
			Sampler2DDeclaration sampler2D;
			BindlessArrayDeclaration bindlessArray;
		};
	};

	struct BindGroup
	{
		u32 set;
		std::vector<Binding> bindings;
	};

	class RenderInterface
	{
	public:
		virtual void initialize(RendererDescriptor descriptor) = 0;
		virtual void deinitialize() = 0;

		virtual ~RenderInterface() = default;
		virtual Handle<Buffer> createBuffer(const BufferDescriptor& descriptor) = 0;
		virtual std::unique_ptr<CommandList> acquireCommandList(QueueType queueType, CommandListType commandListType = CommandListType::primary) = 0;

		//this function should be thread save????
		//Do I need make multi threaded resource creation? It can depend on Frame Graph resource management.
		virtual Handle<RenderTarget> createRenderTarget(RenderTargetDescription) = 0;
		//=================
		virtual void nextFrame() = 0;
		/*virtual void present();
		virtual void waitForSwapchain();*/

		//draft for pipeline creation
		virtual Handle<Pipeline> createPipeline(const GraphicsPipelineDescription& graphicsPipelineDescription, const std::vector<BindGroup>& bindGroups = {}) = 0;
	};


	struct BarrierDescription {};
	struct SplitBarrier
	{
		/*
		 * in vulkan for instance, we have to store event and wait for it
		 */
	};

	struct RenderingDescription
	{
		std::vector<RenderTargetDescription> colorRenderTargets;
		RenderTargetDescription depthRenderTarget{};
		RenderTargetDescription stencilRenderTarget{};
	};

	class CommandList
	{
	public:
		virtual ~CommandList() = default;
		virtual void barrier(const std::initializer_list<BarrierDescription>& descriptions) = 0;
		virtual Handle<SplitBarrier> beginSplitBarrier(const BarrierDescription& description) = 0;
		virtual void endSplitBarrier(Handle<SplitBarrier> barrier) = 0;
		virtual void beginRendering(RenderingDescription description) = 0;
		virtual void endRendering() = 0;
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
