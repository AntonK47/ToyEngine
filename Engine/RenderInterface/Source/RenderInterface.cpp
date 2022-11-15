#include "RenderInterface.h"

#include <Hash.h>

using namespace toy::renderer;

NativeBackend RenderInterface::getNativeBackend()
{
	return getNativeBackendInternal();
}

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

CommandList& RenderInterface::acquireCommandList(
	const QueueType queueType,
	const UsageScope scope)
{
	return acquireCommandListInternal(queueType, scope);
}

void RenderInterface::submitCommandList(std::unique_ptr<CommandList> commandList)
{
	submitCommandListInternal(std::move(commandList));
}

RenderInterface::SubmitDependency RenderInterface::submitCommandList(
	const QueueType queueType,
	const std::initializer_list<CommandList*>& commandLists,
	const std::initializer_list<SubmitDependency>& dependencies)
{
	return submitCommandListInternal(queueType, commandLists, dependencies);
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

Buffer RenderInterface::createBuffer(
	const BufferDescriptor& descriptor,
	[[maybe_unused]] const DebugLabel label)
{
	return Buffer
	{
		.nativeHandle = createBufferInternal(descriptor, label),
		.size = descriptor.size,
#ifdef TOY_ENGINE_ENABLE_RENDERER_INTERFACE_VALIDATION
		.debugLabel = label
#endif
	};
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

Handle<BindGroupLayout> RenderInterface::createBindGroupLayout(const BindGroupDescriptor& descriptor)
{
	return createBindGroupLayoutInternal(descriptor);
}

Handle<BindGroup> RenderInterface::allocateBindGroup(
	const Handle<BindGroupLayout>& bindGroupLayout, const UsageScope& scope)
{
	return allocateBindGroupInternal(bindGroupLayout, 1, scope).front();
}

folly::small_vector<Handle<BindGroup>> RenderInterface::allocateBindGroup(
	const Handle<BindGroupLayout>& bindGroupLayout,
	const u32 bindGroupCount, const UsageScope& scope)
{
	return allocateBindGroupInternal(bindGroupLayout, bindGroupCount, scope);
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

