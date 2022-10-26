#pragma once
#include <vector>
#include <optional>

#include "BindGroupAllocator.h"
#include "CommandList.h"
#include "Resource.h"
#include "RenderInterfaceValidator.h"

namespace toy::renderer
{
	using namespace core;
	class CommandList;

	class RenderInterface
	{
	public:
		RenderInterface(const RenderInterface& other) = delete;
		RenderInterface(RenderInterface&& other) noexcept = default;

		RenderInterface& operator=(const RenderInterface& other) = default;
		RenderInterface& operator=(RenderInterface&& other) noexcept = default;

		NativeBackend getNativeBackend();

		RenderInterface() = default;
		virtual ~RenderInterface() = default;

		void initialize(const RendererDescriptor& descriptor);
		void deinitialize();

		[[nodiscard]] std::unique_ptr<CommandList> acquireCommandList(
			QueueType queueType,
			CommandListType commandListType = CommandListType::primary);

		void submitCommandList(std::unique_ptr<CommandList> commandList);
		
		void nextFrame();

		[[nodiscard]] SwapchainImage acquireNextSwapchainImage();
		void present();

		[[nodiscard]] Handle<Buffer> createBuffer(
			const BufferDescriptor& descriptor);

		[[nodiscard]] Handle<Image> createImage(
			const ImageDescriptor& descriptor);

		[[nodiscard]] Handle<ImageView> createImageView(
			const ImageViewDescriptor& descriptor);

		void map(const Handle<Buffer>& buffer, void** data);
		void unmap(const Handle<Buffer>& buffer);

		[[nodiscard]] Handle<BindGroupLayout> createBindGroupLayout(
			const BindGroupDescriptor& descriptor);

		[[nodiscard]] Handle<BindGroup> allocateBindGroup(
			const Handle<BindGroupLayout>& bindGroupLayout,
			const BindGroupUsageScope& scope = BindGroupUsageScope::perFrame);

		[[nodiscard]] std::vector<Handle<BindGroup>> allocateBindGroup(
			const Handle<BindGroupLayout>& bindGroupLayout,
			u32 bindGroupCount, 
			const BindGroupUsageScope& scope = BindGroupUsageScope::perFrame);

		//this function should be thread save????
		//Do I need make multi threaded resource creation? It can depend on Frame Graph resource management.
		//virtual Handle<RenderTarget> createRenderTarget(RenderTargetDescriptor) = 0;

		//draft for pipeline creation
		[[nodiscard]] Handle<Pipeline> createPipeline(
			const GraphicsPipelineDescriptor& graphicsPipelineDescriptor, const std::vector<SetBindGroupMapping>& bindGroups = {});

		[[nodiscard]] Handle<Pipeline> createPipeline(
			const ComputePipelineDescriptor& descriptor,
			const std::vector<SetBindGroupMapping>& bindGroups = {}); //TODO: Why do not move bindGroups into a descriptor

		[[nodiscard]] Handle<Pipeline> createPipeline(
			const RayTracingPipelineDescriptor& descriptor,
			const std::vector<SetBindGroupMapping>& bindGroups = {});

		[[nodiscard]] Handle<ShaderModule> createShaderModule(
			ShaderStage stage,
			const ShaderCode& code);


		//TODO This function should be thread safe
		void updateBindGroup(
			const Handle<BindGroup>& bindGroup,
			const std::initializer_list<BindingDataMapping>& mappings);

		virtual void updateBindGroupInternal(
			const Handle<BindGroup>& bindGroup,
			const std::initializer_list<BindingDataMapping>& mappings)= 0;

		//=================
	protected:
		virtual void initializeInternal(const RendererDescriptor& descriptor) = 0;
		virtual void deinitializeInternal() = 0;

		virtual NativeBackend getNativeBackendInternal() = 0;

		virtual [[nodiscard]] SwapchainImage acquireNextSwapchainImageInternal() = 0;

		virtual [[nodiscard]] std::unique_ptr<CommandList> acquireCommandListInternal(
			QueueType queueType,
			CommandListType commandListType = CommandListType::primary) = 0;

		virtual [[nodiscard]] void submitCommandListInternal(
			std::unique_ptr<CommandList> commandList) = 0;

		virtual [[nodiscard]] Handle<Pipeline> createPipelineInternal(
			const GraphicsPipelineDescriptor& descriptor,
			const std::vector<SetBindGroupMapping>& bindGroups) = 0;

		virtual [[nodiscard]] Handle<Pipeline> createPipelineInternal(
			const ComputePipelineDescriptor& descriptor,
			const std::vector<SetBindGroupMapping>& bindGroups) = 0;

		virtual [[nodiscard]] Handle<ShaderModule> createShaderModuleInternal(
			ShaderStage stage,
			const ShaderCode& code) = 0;

		virtual void nextFrameInternal() = 0;
		virtual void presentInternal() = 0;

		virtual void mapInternal(
			const Handle<Buffer>& buffer,
			void** data) = 0;

		virtual [[nodiscard]] Handle<BindGroupLayout> createBindGroupLayoutInternal(
			const BindGroupDescriptor& descriptor) = 0;

		virtual [[nodiscard]] std::vector<Handle<BindGroup>> allocateBindGroupInternal(
			const Handle<BindGroupLayout>& bindGroupLayout,
			u32 bindGroupCount,
			const BindGroupUsageScope& scope) = 0;


		//TODO: resource creation can be moved in a separate resource management class 
		virtual [[nodiscard]] Handle<Buffer> createBufferInternal(
			const BufferDescriptor& descriptor) = 0;

		virtual [[nodiscard]] Handle<Image> createImageInternal(
			const ImageDescriptor& descriptor) = 0;

		virtual [[nodiscard]] Handle<ImageView> createImageViewInternal(
			const ImageViewDescriptor& descriptor) = 0;

		DECLARE_VALIDATOR(validation::RenderInterfaceValidator);
	};
}