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

	vk::ShaderModule createShaderModule(const vk::Device device)
	{
		const auto moduleCreateInfo = vk::ShaderModuleCreateInfo
		{

		};
		return device.createShaderModule(moduleCreateInfo).value;
	}

	vk::Pipeline createGraphicsPipeline(const vk::Device device)
	{
		auto vertexModule = vk::ShaderModule{};
		auto fragmentModule = vk::ShaderModule{};

		const auto stages = std::array
		{
			vk::PipelineShaderStageCreateInfo
			{
				.stage = vk::ShaderStageFlagBits::eVertex,
				.module = vertexModule,
				.pName = "main",
				.pSpecializationInfo = nullptr
			},
			vk::PipelineShaderStageCreateInfo
			{
				.stage = vk::ShaderStageFlagBits::eFragment,
				.module = fragmentModule,
				.pName = "main",
				.pSpecializationInfo = nullptr
			}
		};

		const auto inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo
		{
			.topology = vk::PrimitiveTopology::eTriangleList
		};

		const auto rasterizationState = vk::PipelineRasterizationStateCreateInfo
		{

		};

		const auto dynamicStates = std::array
		{
			vk::DynamicState::eDepthTestEnable
		};

		const auto dynamicState = vk::PipelineDynamicStateCreateInfo
		{
			.dynamicStateCount = dynamicStates.size(),
			.pDynamicStates = dynamicStates.data()
		};


		const auto layoutCreateInfo = vk::PipelineLayoutCreateInfo
		{

		};

		const auto pipelineLayout = device.createPipelineLayout(layoutCreateInfo).value;

		const auto pipelineCreateInfo = vk::StructureChain
		{
			vk::GraphicsPipelineCreateInfo
			{
				.stageCount = stages.size(),
				.pStages = stages.data(),
				.pVertexInputState = nullptr,
				.pInputAssemblyState = &inputAssemblyState,
				.pTessellationState = nullptr,
				.pViewportState = nullptr,
				.pRasterizationState = &rasterizationState,
				.pMultisampleState = nullptr,
				.pDepthStencilState = nullptr,
				.pColorBlendState = nullptr,
				.pDynamicState = &dynamicState,
				.layout = pipelineLayout,
			},
			vk::PipelineRenderingCreateInfo
			{

			}

		};

		auto pipelineCache = vk::PipelineCache{};

		return device.createGraphicsPipeline(pipelineCache, pipelineCreateInfo.get()).value;
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

toy::renderer::api::vulkan::VulkanCommandList::VulkanCommandList(vk::CommandBuffer commandBuffer,
	vk::CommandBufferLevel level, QueueType ownedQueueType): CommandList(ownedQueueType), cmd_(commandBuffer), level_(level)
{}
