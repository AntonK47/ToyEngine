#include "RenderInterface.h"

#include <Hash.h>



using namespace toy::renderer;

namespace toy::renderer
{
	void RenderInterface::initialize(const RendererDescriptor& descriptor)
	{
		VALIDATE(validateInitialize(descriptor));
		initializeInternal(descriptor);
	}

	void RenderInterface::deinitialize()
	{
		VALIDATE(validateDeinitialize());
		deinitializeInternal();
	}

	std::unique_ptr<CommandList> RenderInterface::acquireCommandList(const QueueType queueType,
	                                                                 const CommandListType commandListType)
	{
		return acquireCommandListInternal(queueType, commandListType);
	}

	void RenderInterface::submitCommandList(std::unique_ptr<CommandList> commandList)
	{
		submitCommandListInternal(std::move(commandList));
	}

	void RenderInterface::nextFrame()
	{
		nextFrameInternal();
	}

	SwapchainImage RenderInterface::acquireNextSwapchainImage()
	{
		return acquireNextSwapchainImageInternal();
	}

	void RenderInterface::present()
	{
		presentInternal();
	}

	Handle<Buffer> RenderInterface::createBuffer(
		const BufferDescriptor& descriptor)
	{
		return createBufferInternal(descriptor);
	}

	Handle<Image> RenderInterface::createImage(
		const ImageDescriptor& descriptor)
	{
		return createImageInternal(descriptor);
	}

	Handle<ImageView> RenderInterface::createImageView(
		const ImageViewDescriptor& descriptor)
	{
		return createImageViewInternal(descriptor);
	}

	void RenderInterface::map(const Handle<Buffer>& buffer, void** data)
	{
		mapInternal(buffer, data);
	}

	void RenderInterface::unmap(const Handle<Buffer>& buffer)
	{
	}

	BindGroup RenderInterface::allocateBindGroup(const BindGroupDescriptor& descriptor, const BindGroupLayout& layout)
	{
		return {};
	}

	BindGroup RenderInterface::allocateBindGroup(const BindGroupDescriptor& descriptor)
	{
		return {};
	}

	

	Handle<BindGroupLayout> RenderInterface::allocateBindGroupLayout(const BindGroupDescriptor& descriptor)
	{
		return allocateBindGroupLayoutInternal(descriptor);
	}

	Handle<BindGroup> RenderInterface::allocateBindGroup(
		const Handle<BindGroupLayout>& bindGroupLayout)
	{
		return allocateBindGroupInternal(bindGroupLayout);
	}

	std::vector<Handle<BindGroup>> RenderInterface::allocateBindGroup(
		const Handle<BindGroupLayout>& bindGroupLayout,
		const u32 bindGroupCount)
	{
		return allocateBindGroupInternal(bindGroupLayout, bindGroupCount);

	}

	Handle<Pipeline> RenderInterface::createPipeline(
		const GraphicsPipelineDescriptor& descriptor,
		const std::vector<SetBindGroupMapping>& bindGroups)
	{
		return createPipelineInternal(descriptor, bindGroups);
	}

	Handle<Pipeline> RenderInterface::createPipeline(
		const ComputePipelineDescriptor& descriptor,
		const std::vector<SetBindGroupMapping>& bindGroups)
	{
		return createPipelineInternal(descriptor, bindGroups);
	}

	Handle<ShaderModule> RenderInterface::createShaderModule(ShaderStage stage, const ShaderCode& code)
	{
		return createShaderModuleInternal(stage, code);
	}

	void RenderInterface::updateBindGroup(const Handle<BindGroup>& bindGroup,
		const std::initializer_list<BindingDataMapping>& mappings)
	{
		updateBindGroupInternal(bindGroup, mappings);
	}
}
