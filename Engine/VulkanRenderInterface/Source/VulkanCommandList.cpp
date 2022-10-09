#include "VulkanCommandList.h"

using namespace toy::renderer;

namespace
{
	vk::ImageView getImageView(const Handle<Texture>& handle)
	{
		return vk::ImageView{};
	}

	vk::ImageView getImageView(const Accessor<ImageResource>& handle)
	{
		return vk::ImageView{};
	}

	vk::ResolveModeFlagBits mapResolve(ResolveMode resolveMode)
	{
		return vk::ResolveModeFlagBits::eNone;
	}

	vk::AttachmentLoadOp mapLoadOp(LoadOperation load)
	{
		return vk::AttachmentLoadOp::eClear;
	}

	vk::ClearValue mapClearValue(const ColorClear& colorClear)
	{
		return vk::ClearValue{ vk::ClearColorValue{ std::array<float,4>{colorClear.r,colorClear.g,colorClear.b,colorClear.a} } };
	}

	vk::AttachmentStoreOp mapStoreOp(StoreOperation store)
	{
		//TODO: it should have proper mapping now
		return vk::AttachmentStoreOp::eStore;
	}

	bool hasDepthAttachment(const RenderingDescriptor& descriptor)
	{
		return false;
	}

	bool hasStencilAttachment(const RenderingDescriptor& descriptor)
	{
		return false;
	}
}

api::vulkan::VulkanCommandList::~VulkanCommandList()
{
}

void api::vulkan::VulkanCommandList::barrierInternal(const std::initializer_list<BarrierDescriptor>& descriptors)
{
	auto imageBarriers = std::vector<vk::ImageMemoryBarrier2>{};

	for(const auto& barrierDescriptor: descriptors)
	{
		if(std::holds_alternative<ImageBarrierDescriptor>(barrierDescriptor))
		{
			const auto& imageBarrierDescriptor = std::get<ImageBarrierDescriptor>(barrierDescriptor);

			auto barrier = vk::ImageMemoryBarrier2{};
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = static_cast<VulkanImage*>(imageBarrierDescriptor.image)->image;
			barrier.subresourceRange = vk::ImageSubresourceRange
			{
				vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
			};

			switch (imageBarrierDescriptor.srcLayout)
			{
			case Layout::Undefined:
				barrier.oldLayout = vk::ImageLayout::eUndefined;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eAllCommands;
				barrier.dstAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite;
				break;
			case Layout::ColorRenderTarget:
				barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
				barrier.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
				break;
			case Layout::Present:
				barrier.oldLayout = vk::ImageLayout::ePresentSrcKHR;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eNone;
				barrier.dstAccessMask = vk::AccessFlagBits2::eNone;
				break;
			default:

				barrier.newLayout = vk::ImageLayout::eUndefined;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eNone;
				barrier.dstAccessMask = vk::AccessFlagBits2::eNone;

				break;
			}

			switch (imageBarrierDescriptor.dstLayout)
			{
			case Layout::ColorRenderTarget:
				barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
				barrier.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
				break;
			case Layout::Present:
				barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eNone;
				barrier.dstAccessMask = vk::AccessFlagBits2::eNone;
				break;
			default:

				barrier.newLayout = vk::ImageLayout::eUndefined;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eNone;
				barrier.dstAccessMask = vk::AccessFlagBits2::eNone;
				break;
			}
			
			imageBarriers.push_back(barrier);
		}
	}

	const auto dependency = vk::DependencyInfo
	{
		.dependencyFlags = vk::DependencyFlagBits::eByRegion,
		.memoryBarrierCount = 0,
		.pMemoryBarriers = nullptr,
		.bufferMemoryBarrierCount = 0,
		.pBufferMemoryBarriers = nullptr,
		.imageMemoryBarrierCount = static_cast<u32>(imageBarriers.size()),
		.pImageMemoryBarriers = imageBarriers.data()
	};

	cmd_.pipelineBarrier2(dependency);

}

void api::vulkan::VulkanCommandList::beginRenderingInternal(const RenderingDescriptor& descriptor, const RenderArea& area)
{
	const auto containsSecondaryBuffers = false;
	auto flags = vk::RenderingFlags{};
	if(containsSecondaryBuffers)
	{
		flags |= vk::RenderingFlagBits::eContentsSecondaryCommandBuffers;
	}
			
	auto colorAttachments = std::vector<vk::RenderingAttachmentInfo>(descriptor.colorRenderTargets.size());

	for (u32 i{}; i< descriptor.colorRenderTargets.size(); i++)
	{
		const auto& colorRenderTarget = descriptor.colorRenderTargets[i];

		auto imageView = static_cast<VulkanImageView&>(*colorRenderTarget.renderTargetImageAccessor).vulkanImageView;

		colorAttachments[i] = vk::RenderingAttachmentInfo
		{
			.imageView = imageView,
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.resolveMode = mapResolve(colorRenderTarget.resolveMode),
			.loadOp = mapLoadOp(colorRenderTarget.load),
			.storeOp = mapStoreOp(colorRenderTarget.store),
			.clearValue = mapClearValue(colorRenderTarget.clearValue)
		};
	}

	auto depthAttachment = vk::RenderingAttachmentInfo{};

	if(hasDepthAttachment(descriptor))
	{
		auto imageView = static_cast<VulkanImageView&>(*descriptor.depthRenderTarget.renderTargetImageAccessor).vulkanImageView;
		depthAttachment = vk::RenderingAttachmentInfo
		{
			.imageView = imageView,
			.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
			.resolveMode = mapResolve(descriptor.depthRenderTarget.resolveMode),
			.loadOp = mapLoadOp(descriptor.depthRenderTarget.load),
			.storeOp = mapStoreOp(descriptor.depthRenderTarget.store),
			.clearValue = mapClearValue(descriptor.depthRenderTarget.clearValue)
		};
	}

	auto stencilAttachment = vk::RenderingAttachmentInfo{};
	if(hasStencilAttachment(descriptor))
	{
		auto imageView = static_cast<VulkanImageView&>(*descriptor.stencilRenderTarget.renderTargetImageAccessor).vulkanImageView;
		stencilAttachment = vk::RenderingAttachmentInfo
		{
			.imageView = imageView,
			.imageLayout = vk::ImageLayout::eStencilAttachmentOptimal,
			.resolveMode = mapResolve(descriptor.stencilRenderTarget.resolveMode),
			.loadOp = mapLoadOp(descriptor.stencilRenderTarget.load),
			.storeOp = mapStoreOp(descriptor.stencilRenderTarget.store),
			.clearValue = mapClearValue(descriptor.stencilRenderTarget.clearValue)
		};
	}

	const auto renderingInfo = vk::RenderingInfo
	{
		.flags = flags,
		.renderArea = vk::Rect2D{ vk::Offset2D(area.x, area.y), area.width, area.height},
		.layerCount = 1,
		.viewMask = 0,
		.colorAttachmentCount = static_cast<u32>(colorAttachments.size()),
		.pColorAttachments = colorAttachments.data(),
		.pDepthAttachment = hasDepthAttachment(descriptor)?&depthAttachment:nullptr,
		.pStencilAttachment = hasStencilAttachment(descriptor)?&stencilAttachment:nullptr
	};

	cmd_.beginRendering(renderingInfo);
}

void api::vulkan::VulkanCommandList::endRenderingInternal()
{
	cmd_.endRendering();
}

void api::vulkan::VulkanCommandList::drawInternal(const u32 vertexCount, const u32 instanceCount, const u32 firstVertex, const u32 firstInstance)
{
	cmd_.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void api::vulkan::VulkanCommandList::bindPipelineInternal(const Ref<Pipeline>& pipeline)
{
	currentPipeline_ = const_cast<VulkanPipeline*>(&pipeline.query<VulkanPipeline>());
	cmd_.bindPipeline(vk::PipelineBindPoint::eGraphics, currentPipeline_->pipeline);
}

void api::vulkan::VulkanCommandList::setScissorInternal(const Scissor& scissor)
{
	const auto vulkanScissor = vk::Rect2D{ {scissor.x,scissor.y},{scissor.width, scissor.height}};
	
	cmd_.setScissor(0, 1, &vulkanScissor);
}

void api::vulkan::VulkanCommandList::setViewportInternal(
	const Viewport& viewport)
{
	const auto vulkanViewport = vk::Viewport{ viewport.x,viewport.y,viewport.width,viewport.height,0.0,1.0 };

	//TODO:: Whats about depth value?

	cmd_.setViewport(0, 1, &vulkanViewport);
}

void api::vulkan::VulkanCommandList::bindGroupInternal(u32 set,
	const Handle<BindGroup>& handle)
{
	const auto vulkanBindGroup = renderInterface_->bindGroupStorage_.get(handle);
	const auto vulkanPipeline = *currentPipeline_;


	cmd_.bindDescriptorSets(vulkanPipeline.bindPoint, vulkanPipeline.layout, 0, 1, &vulkanBindGroup.descriptorSet, 0, nullptr);
}

api::vulkan::VulkanCommandList::VulkanCommandList(
	VulkanRenderInterface& parent,
	vk::CommandBuffer commandBuffer,
	vk::CommandBufferLevel level, QueueType ownedQueueType):
	renderInterface_(&parent),
	CommandList(ownedQueueType),
	cmd_(commandBuffer),
	level_(level)
{
}
